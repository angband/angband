/**
 * \file apinterface.c
 * \brief Glue between Angband's C code and the vendored APCc Archipelago client.
 *
 * Keeps all of the APCc / glib / libwebsockets / winsock surface confined to
 * this one translation unit.  The rest of the game talks to Archipelago only
 * through the small API declared in apinterface.h, and passes connection
 * details in as plain strings so that this file never has to include player.h
 * (whose PF_* player flags collide with winsock's PF_* protocol families).
 *
 * Threading: libwebsockets' lws_service() blocks waiting for I/O (seconds when
 * idle), so it cannot be called from the game's main loop without freezing the
 * game on every turn.  Instead a dedicated service thread owns all APCc/lws
 * interaction, and exchanges events with the game thread through two
 * thread-safe queues:
 *   - ap_in_q  (service -> game): received items, confirmed checks, "connected",
 *     and incoming DeathLinks, with names already resolved on the service thread.
 *   - ap_out_q (game -> service): location checks and DeathLinks to send.
 * The game thread only ever touches the queues (cheap); AP_WakeService() pokes
 * the blocked lws_service() so queued sends go out promptly.
 */

/*
 * Pull in the Archipelago client first.  APCc.h includes <glib.h>, which
 * defines MAX/MIN/ABS as function-like macros that collide with Angband's
 * definitions in h-basic.h.  We don't use glib's versions here, so undefine
 * them before the Angband headers so Angband's win cleanly.
 */
#include "apcc/APCc.h"

#undef MAX
#undef MIN
#undef ABS

#include "message.h"
#include "z-util.h"

#define AP_GAME_NAME "Angband"

/*
 * APCc's AP_Init() stores the ip/player-name/password *pointers* (not copies)
 * and dereferences them later, so keep our own static storage for them.
 */
static char ap_host[128];
static char ap_slot[64];
static char ap_password[1];           /* empty for now: no password support */

static bool ap_started = false;       /* connection set up and thread running */
static bool ap_failed = false;        /* setup failed; don't keep retrying */

/* Game-side handlers (set once before the thread starts; read on game thread). */
static void (*ap_check_handler)(const char *name) = NULL;
static void (*ap_item_handler)(const char *item_name, uint64_t index) = NULL;
static void (*ap_connect_handler)(void) = NULL;
static void (*ap_deathlink_handler)(void) = NULL;

/*
 * slot_data: artifacts-as-checks mode (written on service thread, read on game).
 * 0 = Off, 1 = One to One (per-artifact checks), 2 = Accumulated ("Find #X
 * Artifacts" milestone checks).  See angband/options.py.
 */
static volatile int ap_opt_artifacts_mode = 0;

/*
 * slot_data: Black Market artifact-location price multiplier (1-5, default 3).
 * Written on the service thread, read on the game thread.  See angband/options.py.
 */
static volatile int ap_opt_bm_multiplier = 3;

/*
 * slot_data: Resistances mode.  0 = Standard, 1 = Equipment Traits (unremovable
 * Archipelago Weapon improved by trait items; equipment restricted), 2 = Full
 * Traits (also the Archipelago race).  See angband/options.py.
 */
static volatile int ap_opt_resistances = 0;

/* --- Threading ------------------------------------------------------------- */

static GThread *ap_thread = NULL;
static volatile gint ap_thread_run = 0;
static GAsyncQueue *ap_in_q = NULL;   /* service -> game */
static GAsyncQueue *ap_out_q = NULL;  /* game -> service */

enum ap_in_type { AP_IN_ITEM, AP_IN_CHECK, AP_IN_DEATHLINK, AP_IN_CONNECTED };
struct ap_in_event {
	enum ap_in_type type;
	char *name;       /* heap copy, freed by the game thread */
	guint64 index;
};

enum ap_out_type { AP_OUT_CHECK, AP_OUT_DEATHLINK, AP_OUT_VICTORY };
struct ap_out_event {
	enum ap_out_type type;
	char *name;       /* heap copy, freed by the service thread */
};

/* Service-thread-only state. */
static uint64_t ap_item_seq = 0;      /* stable 0-based index of received items */
static bool ap_announced = false;     /* "connected" event already emitted */
#define AP_PENDING_MAX 1024
static uint64_t ap_pending_checks[AP_PENDING_MAX];  /* checks awaiting data sync */
static int ap_pending_count = 0;

/* --- Service thread: produce incoming events ------------------------------- */

static void ap_push_in(enum ap_in_type type, const char *name, guint64 index)
{
	struct ap_in_event *e = g_new0(struct ap_in_event, 1);
	e->type = type;
	e->name = name ? g_strdup(name) : NULL;
	e->index = index;
	g_async_queue_push(ap_in_q, e);
}

/* --- Required APCc callbacks (all run on the service thread) ---------------- */

static void ap_cb_item_clear(void)
{
	/* (Re)sync is starting: items are about to be replayed from the top. */
	ap_item_seq = 0;
}

static void ap_cb_item_recv(uint64_t item_id, int sending_player, bool notify)
{
	char *name;

	(void)sending_player;
	(void)notify;       /* deliver replays too: a fresh character's home is empty */

	name = AP_GetItemName(item_id);
	ap_push_in(AP_IN_ITEM, name, ap_item_seq);

	/* Advance even with no name so indices stay aligned with AP. */
	ap_item_seq++;
}

static void ap_deliver_check_now(uint64_t loc_id)
{
	char *name = AP_GetLocationName(loc_id);
	if (name) ap_push_in(AP_IN_CHECK, name, 0);
}

static void ap_cb_location_checked(uint64_t loc_id)
{
	/*
	 * If the data package is loaded we can resolve the name now; otherwise
	 * defer (the checked-location replay arrives with Connected, before the
	 * data package on a first connect).
	 */
	if (AP_GetDataPackageStatus() == Synced)
		ap_deliver_check_now(loc_id);
	else if (ap_pending_count < AP_PENDING_MAX)
		ap_pending_checks[ap_pending_count++] = loc_id;
	else
		ap_deliver_check_now(loc_id);   /* overflow: best effort */
}

/* slot_data int callback.  APCc passes a pointer to the value (see APCc.c). */
static void ap_cb_slotdata_artifacts(uint64_t *val)
{
	ap_opt_artifacts_mode = val ? (int)*val : 0;
}

static void ap_cb_slotdata_bm_multiplier(uint64_t *val)
{
	int m = val ? (int)*val : 3;
	if (m < 1) m = 1;
	if (m > 5) m = 5;
	ap_opt_bm_multiplier = m;
}

static void ap_cb_slotdata_resistances(uint64_t *val)
{
	ap_opt_resistances = val ? (int)*val : 0;
}

/* --- Service thread: consume outgoing events + run the event loop ---------- */

static void ap_drain_out(void)
{
	struct ap_out_event *o;

	while ((o = g_async_queue_try_pop(ap_out_q)) != NULL) {
		if (o->type == AP_OUT_CHECK && o->name) {
			int64_t id = AP_GetLocationIdByName(o->name);
			if (id >= 0) AP_SendItem((uint64_t)id);
		} else if (o->type == AP_OUT_DEATHLINK) {
			AP_DeathLinkSend();
		} else if (o->type == AP_OUT_VICTORY) {
			AP_StoryComplete();
		}
		g_free(o->name);
		g_free(o);
	}
}

static gpointer ap_thread_fn(gpointer data)
{
	(void)data;

	while (g_atomic_int_get(&ap_thread_run)) {
		ap_drain_out();

		/* Blocks on socket I/O; that is fine on this dedicated thread. */
		AP_WebService();

		/* Flush checks deferred until the data package arrived. */
		if (ap_pending_count > 0 && AP_GetDataPackageStatus() == Synced) {
			int i;
			for (i = 0; i < ap_pending_count; i++)
				ap_deliver_check_now(ap_pending_checks[i]);
			ap_pending_count = 0;
		}

		if (!ap_announced && AP_GetConnectionStatus() == Authenticated) {
			ap_announced = true;
			ap_push_in(AP_IN_CONNECTED, NULL, 0);
		}

		if (AP_DeathLinkPending()) {
			AP_DeathLinkClear();
			ap_push_in(AP_IN_DEATHLINK, NULL, 0);
		}
	}

	return NULL;
}

/* --- Helpers --------------------------------------------------------------- */

/**
 * Split "host:port" (e.g. "archipelago.gg:32000") into host and port.
 * \return true on success.
 */
static bool ap_parse_server(const char *src, char *host, size_t hostlen,
		int *port)
{
	const char *colon = strrchr(src, ':');
	size_t hlen;

	if (!colon || colon == src) return false;

	hlen = (size_t)(colon - src);
	if (hlen >= hostlen) hlen = hostlen - 1;
	memcpy(host, src, hlen);
	host[hlen] = '\0';

	*port = atoi(colon + 1);
	return (*port > 0);
}

/**
 * Initialise the APCc client, then start the service thread.
 */
static void ap_begin(const char *server, const char *slotname)
{
	int port = 0;

	if (!server || server[0] == '\0') {
		ap_failed = true;       /* Archipelago disabled: no server configured */
		return;
	}

	if (!ap_parse_server(server, ap_host, sizeof(ap_host), &port)) {
		msg("AP: could not parse server '%s' (expected host:port).", server);
		ap_failed = true;
		return;
	}

	my_strcpy(ap_slot, slotname ? slotname : "", sizeof(ap_slot));
	ap_password[0] = '\0';

	/*
	 * AP_Init() allocates APCc's internal callback registries, so it must run
	 * before any registration (AP_RegisterSlotDataIntCallback in particular
	 * dereferences them).  It also leaves client_version NULL and dereferences
	 * it when building the Connect packet, so set that here too.
	 */
	AP_Init(ap_host, port, AP_GAME_NAME, ap_slot, ap_password);

	{
		struct AP_NetworkVersion ver = { 0, 6, 4 };
		AP_SetClientVersion(&ver);
	}

	AP_SetItemClearCallback(ap_cb_item_clear);
	AP_SetItemRecvCallback(ap_cb_item_recv);
	AP_SetLocationCheckedCallback(ap_cb_location_checked);
	AP_SetDeathLinkSupported(true);

	/* APCc invokes int slot-data callbacks with a uint64_t* (its header's
	 * by-value declaration is inaccurate), so register with a matching cast. */
	AP_RegisterSlotDataIntCallback("artifacts_as_checks",
		(void (*)(uint64_t))ap_cb_slotdata_artifacts);
	AP_RegisterSlotDataIntCallback("black_market_price_multiplier",
		(void (*)(uint64_t))ap_cb_slotdata_bm_multiplier);
	AP_RegisterSlotDataIntCallback("resistances",
		(void (*)(uint64_t))ap_cb_slotdata_resistances);

	AP_Start();

	ap_in_q = g_async_queue_new();
	ap_out_q = g_async_queue_new();
	g_atomic_int_set(&ap_thread_run, 1);
	ap_thread = g_thread_new("ap-service", ap_thread_fn, NULL);

	ap_started = true;
	msg("AP: connecting to %s:%d as '%s'...", ap_host, port, ap_slot);
}

/* --- Public API (all called on the game thread) ---------------------------- */

void ap_service(const char *server, const char *slotname)
{
	struct ap_in_event *e;

	if (ap_failed) return;
	if (!ap_started) {
		ap_begin(server, slotname);
		if (!ap_started) return;
	}

	/* Apply everything the service thread has queued for us. */
	while ((e = g_async_queue_try_pop(ap_in_q)) != NULL) {
		switch (e->type) {
		case AP_IN_ITEM:
			if (ap_item_handler && e->name)
				ap_item_handler(e->name, e->index);
			break;
		case AP_IN_CHECK:
			if (ap_check_handler && e->name)
				ap_check_handler(e->name);
			break;
		case AP_IN_CONNECTED:
			msg("AP: connected to Archipelago as '%s'.", ap_slot);
			if (ap_connect_handler) ap_connect_handler();
			break;
		case AP_IN_DEATHLINK:
			if (ap_deathlink_handler) ap_deathlink_handler();
			break;
		}
		g_free(e->name);
		g_free(e);
	}
}

bool ap_is_connected(void)
{
	return ap_started && AP_GetConnectionStatus() == Authenticated;
}

void ap_shutdown(void)
{
	if (!ap_started) return;

	/* Stop and join the service thread. */
	g_atomic_int_set(&ap_thread_run, 0);
	AP_WakeService();
	if (ap_thread) {
		g_thread_join(ap_thread);
		ap_thread = NULL;
	}

	AP_Shutdown();

	/*
	 * Free the cross-thread queues (the thread is joined, so nothing else
	 * touches them) along with any events still queued, then reset the
	 * service-thread state.  This keeps a later reconnect -- ap_begin() makes
	 * fresh queues -- from leaking the old ones, which matters for unattended
	 * Borg respawn loops that tear down and rebuild the connection repeatedly.
	 */
	if (ap_out_q) {
		struct ap_out_event *o;
		while ((o = g_async_queue_try_pop(ap_out_q)) != NULL) {
			g_free(o->name);
			g_free(o);
		}
		g_async_queue_unref(ap_out_q);
		ap_out_q = NULL;
	}
	if (ap_in_q) {
		struct ap_in_event *e;
		while ((e = g_async_queue_try_pop(ap_in_q)) != NULL) {
			g_free(e->name);
			g_free(e);
		}
		g_async_queue_unref(ap_in_q);
		ap_in_q = NULL;
	}

	ap_item_seq = 0;
	ap_pending_count = 0;
	ap_started = false;
	ap_announced = false;
}

void ap_send_check(const char *name)
{
	struct ap_out_event *o;

	if (!ap_started || !name) return;

	o = g_new0(struct ap_out_event, 1);
	o->type = AP_OUT_CHECK;
	o->name = g_strdup(name);
	g_async_queue_push(ap_out_q, o);
	AP_WakeService();
}

void ap_send_deathlink(void)
{
	struct ap_out_event *o;

	if (!ap_started) return;

	o = g_new0(struct ap_out_event, 1);
	o->type = AP_OUT_DEATHLINK;
	o->name = NULL;
	g_async_queue_push(ap_out_q, o);
	AP_WakeService();
}

void ap_send_victory(void)
{
	struct ap_out_event *o;

	if (!ap_started) return;

	o = g_new0(struct ap_out_event, 1);
	o->type = AP_OUT_VICTORY;
	o->name = NULL;
	g_async_queue_push(ap_out_q, o);
	AP_WakeService();
}

int ap_artifacts_mode(void)
{
	return ap_opt_artifacts_mode;
}

bool ap_artifacts_as_checks(void)
{
	return ap_opt_artifacts_mode != 0;
}

bool ap_artifacts_accumulated(void)
{
	return ap_opt_artifacts_mode == 2;
}

int ap_black_market_multiplier(void)
{
	return ap_opt_bm_multiplier;
}

int ap_resistances_mode(void)
{
	return ap_opt_resistances;
}

bool ap_trait_mode(void)
{
	return ap_opt_resistances != 0;
}

void ap_set_check_handler(void (*fn)(const char *name))
{
	ap_check_handler = fn;
}

void ap_set_item_handler(void (*fn)(const char *item_name, uint64_t index))
{
	ap_item_handler = fn;
}

void ap_set_connect_handler(void (*fn)(void))
{
	ap_connect_handler = fn;
}

void ap_set_deathlink_handler(void (*fn)(void))
{
	ap_deathlink_handler = fn;
}
