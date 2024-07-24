/**
 * \file borg-messages.c
 * \brief Code to read and parse messages that come from the game
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "borg-messages.h"

#ifdef ALLOW_BORG

#include "../ui-term.h"

#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-fight-attack.h"
#include "borg-fight-defend.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-stairs.h"
#include "borg-io.h"
#include "borg-messages-react.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/*
 * Hack -- message memory
 */
int16_t  borg_msg_len;
int16_t  borg_msg_siz;
char    *borg_msg_buf;
int16_t  borg_msg_num;
int16_t  borg_msg_max;
int16_t *borg_msg_pos;
int16_t *borg_msg_use;

static char **suffix_pain;

/*
 * Status message search string
 */
char borg_match[128] = "plain gold ring";

/*
 * Hack -- methods of killing a monster (order not important).
 *
 * See "mon_take_hit()" for details.
 */
static const char *prefix_kill[]
    = { "You have killed ", 
        "You have slain ", 
        "You have destroyed ", 
        NULL };

/*
 * Hack -- methods of monster death (order not important).
 *
 * See "project_m()", "do_cmd_fire()", "mon_take_hit()" for details.
 */
static const char *suffix_died[] = { 
    " dies.", 
    " is destroyed.", 
    " is drained dry!", 
    NULL };

static const char *suffix_blink[] = { 
    " disappears!", /* from teleport other */
    " intones strange words.", /* from polymorph spell */
    " teleports away.", /* RF6_TPORT */
    " blinks.", /* RF6_BLINK */
    " makes a soft 'pop'.", 
    NULL };

/* a message can have up to three parts broken up by variables */
/* ex: "{name} hits {pronoun} followers with {type} ax." */
/* " hits ", " followers with ", " ax." */
/* if the message has more parts than that, they are ignored so  */
/* ex: "{name} hits {pronoun} followers with {type} ax and {type} breath." */
/* would end up as */
/* " hits ", " followers with ", " ax and " */
/* hopefully this is enough to keep the messages as unique as possible */
struct borg_read_message {
    char *message_p1;
    char *message_p2;
    char *message_p3;
};

struct borg_read_messages {
    int                       count;
    int                       allocated;
    struct borg_read_message *messages;
    int                      *index;
};

/*  methods of hitting the player */
static struct borg_read_messages suffix_hit_by;

/*  methods of casting spells at the player */
static struct borg_read_messages spell_msgs;
static struct borg_read_messages spell_invis_msgs;

/* check a message for a string */
static bool borg_message_contains(
    const char *value, struct borg_read_message *message)
{
    if (strstr(value, message->message_p1)
        && (!message->message_p2 || strstr(value, message->message_p2))
        && (!message->message_p3 || strstr(value, message->message_p3)))
        return true;
    return false;
}

/*
 * Hack -- Spontaneous level feelings (order important).
 *
 * See "do_cmd_feeling()" for details.
 */
static const char *prefix_feeling_danger[] = {
    "You are still uncertain about this place",
    "Omens of death haunt this place", 
    "This place seems murderous",
    "This place seems terribly dangerous", 
    "You feel anxious about this place",
    "You feel nervous about this place", 
    "This place does not seem too risky",
    "This place seems reasonably safe", 
    "This seems a tame, sheltered place",
    "This seems a quiet, peaceful place", 
    NULL
};

static const char *suffix_feeling_stuff[] = { 
    "Looks like any other level.",
    "you sense an item of wondrous power!", 
    "there are superb treasures here.",
    "there are excellent treasures here.",
    "there are very good treasures here.", 
    "there are good treasures here.",
    "there may be something worthwhile here.",
    "there may not be much interesting here.",
    "there aren't many treasures here.", 
    "there are only scraps of junk here.",
    "there is naught but cobwebs here.", 
    NULL };

/*
 * Hack -- Parse a message from the world
 *
 * Note that detecting "death" is EXTREMELY important, to prevent
 * all sorts of errors arising from attempting to parse the "tomb"
 * screen, and to allow the user to "observe" the "cause" of death.
 *
 * Note that detecting "failure" is EXTREMELY important, to prevent
 * bizarre situations after failing to use a staff of perceptions,
 * which would otherwise go ahead and send the "item index" which
 * might be a legal command (such as "a" for "aim").  This method
 * is necessary because the Borg cannot parse "prompts", and must
 * assume the success of the prompt-inducing command, unless told
 * otherwise by a failure message.  Also, we need to detect failure
 * because some commands, such as detection spells, need to induce
 * further processing if they succeed, but messages are only given
 * if the command fails.
 *
 * Note that certain other messages may contain useful information,
 * and so they are "analyzed" and sent to "borg_react()", which just
 * queues the messages for later analysis in the proper context.
 *
 * Along with the actual message, we send a special formatted buffer,
 * containing a leading "opcode", which may contain extra information,
 * such as the index of a spell, and an "argument" (for example, the
 * capitalized name of a monster), with a "colon" to separate them.
 *
 * XXX XXX XXX Several message strings take a "possessive" of the form
 * "his" or "her" or "its".  These strings are all represented by the
 * encoded form "XXX" in the various match strings.  Unfortunately,
 * the encode form is never decoded, so the Borg currently ignores
 * messages about several spells (heal self and haste self).
 *
 * XXX XXX XXX We notice a few "terrain feature" messages here so
 * we can acquire knowledge about wall types and door types.
 */
static void borg_parse_aux(char *msg, int len)
{
    int i, tmp;

    int y9;
    int x9;
    int ax, ay;
    int d;

    char who[256];
    char buf[256];

    borg_grid *ag = &borg_grids[borg.goal.g.y][borg.goal.g.x];

    /* Log (if needed) */
    if (borg_cfg[BORG_VERBOSE])
        borg_note(format("# Parse Msg bite <%s>", msg));

    /* Hack -- Notice death */
    if (prefix(msg, "You die.")) {
        /* Abort (unless cheating) */
        if (!(player->wizard || OPT(player, cheat_live) || borg_cheat_death)) {
            /* Abort */
            borg_oops("death");

            /* Abort right now! */
            borg_active = false;
            /* Noise XXX XXX XXX */
            Term_xtra(TERM_XTRA_NOISE, 1);
        }

        /* Done */
        return;
    }

    /* Hack -- Notice "failure" */
    if (prefix(msg, "You failed ")) {
        /* Hack -- store the keypress */
        borg_note("# Normal failure.");

        /* Set the failure flag */
        borg_failure = true;

        /* Flush our key-buffer */
        borg_flush();

        /* If we were casting a targetted spell and failed */
        /* it does not mean we can't target that location */
        successful_target = 0;

        /* Incase we failed our emergency use of MM */
        borg_confirm_target = false;

        /* Incase it was a Resistance refresh */
        if (borg_attempting_refresh_resist) {
            if (borg.resistance > 1)
                borg.resistance -= 25000;
            borg_attempting_refresh_resist = false;
        }

        return;
    }

    /* Mega-Hack -- Check against the search string */
    if (borg_match[0] && strstr(msg, borg_match)) {
        /* Clean cancel */
        borg_cancel = true;
    }

    /* Ignore teleport trap */
    if (prefix(msg, "You hit a teleport"))
        return;

    /* Ignore arrow traps */
    if (prefix(msg, "An arrow "))
        return;

    /* Ignore dart traps */
    if (prefix(msg, "A small dart "))
        return;

    if (prefix(msg, "The cave ")) {
        borg_react(msg, "QUAKE:Somebody");
        borg_needs_new_sea = true;
        return;
    }

    if (prefix(msg, "You are too afraid to attack ")) {
        tmp = strlen("You are too afraid to attack ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "AFRAID:%s", who);
        borg_react(msg, buf);
        return;
    }


    /* Nexus attacks, need to check everything! */
    if (prefix(msg, "Your body starts to scramble...")) {
        for (i = 0; i < STAT_MAX; i++) {
            //            my_need_stat_check[i] = true;
            /* max stats may have lowered */
            borg.stat_max[i] = 0;
        }
    }

    /* amnesia attacks, re-id wands, staves, equipment. */
    if (prefix(msg, "You feel your memories fade.")) {
        /* Set the borg flag */
        borg.trait[BI_ISFORGET] = true;
    }
    if (streq(msg, "Your memories come flooding back.")) {
        borg.trait[BI_ISFORGET] = false;
    }

    if (streq(msg, "You have been knocked out.")) {
        borg_note("Ignoring Messages While KO'd");
        borg_dont_react = true;
    }
    if (streq(msg, "You are paralyzed")) {
        borg_note("Ignoring Messages While Paralyzed");
        borg_dont_react = true;
    }

    /* Hallucination -- Open */
    if (streq(msg, "You feel drugged!")) {
        borg_note("# Hallucinating.  Special control of wanks.");
        borg.trait[BI_ISIMAGE] = true;
    }

    /* Hallucination -- Close */
    if (streq(msg, "You can see clearly again.")) {
        borg_note("# Hallucination ended.  Normal control of wanks.");
        borg.trait[BI_ISIMAGE] = false;
    }

    /* Hit somebody */
    if (prefix(msg, "You hit ")) {
        tmp = strlen("You hit ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%s", who);
        borg_react(msg, buf);
        return;
    }
    if (prefix(msg, "You bite ")) {
        tmp = strlen("You bite ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%s", who);
        borg_react(msg, buf);
        target_closest = 1;
        return;
    }
    if (prefix(msg, "You draw power from")) {
        target_closest = 1;
        return;
    }
    if (prefix(msg, "No Available Target.")) {
        target_closest = -12;
        return;
    }
    if (prefix(msg, "Not enough room next to ")) {
        target_closest = -12;
        return;
    }

    /* Miss somebody */
    if (prefix(msg, "You miss ")) {
        tmp = strlen("You miss ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody (because of fear) */
    if (prefix(msg, "You are too afraid to attack ")) {
        tmp = strlen("You are too afraid to attack ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%s", who);
        borg_react(msg, buf);
        return;
    }

    /* "Your <equipment> is unaffected!"
     * Note that this check must be before the suffix_pain
     * because suffix_pain will look for 'is unaffected!' and
     * assume it is talking about a monster which in turn will
     * yield to the Player Ghost being created.
     */
    if (prefix(msg, "Your ")) {
        if (suffix(msg, " is unaffected!")) {
            /* Your equipment ignored the attack.
             * Ignore the message
             */
            return;
        }
    } else {
        /* "It screams in pain." (etc) */
        for (i = 0; suffix_pain[i]; i++) {
            /* "It screams in pain." (etc) */
            if (suffix(msg, suffix_pain[i])) {
                tmp = strlen(suffix_pain[i]);
                strnfmt(who, 1 + len - tmp, "%s", msg);
                strnfmt(buf, 256, "PAIN:%s", who);
                borg_react(msg, buf);
                return;
            }
        }

        /* "You have killed it." (etc) */
        for (i = 0; prefix_kill[i]; i++) {
            /* "You have killed it." (etc) */
            if (prefix(msg, prefix_kill[i])) {
                tmp = strlen(prefix_kill[i]);
                strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
                strnfmt(buf, 256, "KILL:%s", who);
                borg_react(msg, buf);
                return;
            }
        }

        /* "It dies." (etc) */
        for (i = 0; suffix_died[i]; i++) {
            /* "It dies." (etc) */
            if (suffix(msg, suffix_died[i])) {
                tmp = strlen(suffix_died[i]);
                strnfmt(who, 1 + len - tmp, "%s", msg);
                strnfmt(buf, 256, "DIED:%s", who);
                borg_react(msg, buf);
                return;
            }
        }

        /* "It blinks or telports." (etc) */
        for (i = 0; suffix_blink[i]; i++) {
            /* "It teleports." (etc) */
            if (suffix(msg, suffix_blink[i])) {
                tmp = strlen(suffix_blink[i]);
                strnfmt(who, 1 + len - tmp, "%s", msg);
                strnfmt(buf, 256, "BLINK:%s", who);
                borg_react(msg, buf);
                return;
            }
        }

        /* "It misses you." */
        if (suffix(msg, " misses you.")) {
            tmp = strlen(" misses you.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "MISS_BY:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* "It is repelled.." */
        /* treat as a miss */
        if (suffix(msg, " is repelled.")) {
            tmp = strlen(" is repelled.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "MISS_BY:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* "It hits you." (etc) */
        for (i = 0; suffix_hit_by.messages[i].message_p1; i++) {
            /* "It hits you." (etc) */
            if (borg_message_contains(msg, &suffix_hit_by.messages[i])) {
                char *start = strstr(msg, suffix_hit_by.messages[i].message_p1);
                if (start) {
                    strnfmt(who, (start - msg), "%s", msg);
                    strnfmt(buf, 256, "HIT_BY:%s", who);
                    borg_react(msg, buf);

                    /* If I was hit, then I am not on a glyph */
                    if (track_glyph.num) {
                        /* erase them all and
                         * allow the borg to scan the screen and rebuild the
                         * array. He won't see the one under him though.  So a
                         * special check must be made.
                         */
                        /* Remove the entire array */
                        for (i = 0; i < track_glyph.num; i++) {
                            /* Stop if we already new about this glyph */
                            track_glyph.x[i] = 0;
                            track_glyph.y[i] = 0;
                        }
                        track_glyph.num = 0;

                        /* Check for glyphs under player -- Cheat*/
                        if (square_iswarded(cave, borg.c)) {
                            track_glyph.x[track_glyph.num] = borg.c.x;
                            track_glyph.y[track_glyph.num] = borg.c.y;
                            track_glyph.num++;
                        }
                    }
                    return;
                }
            }
        }

        for (i = 0; spell_invis_msgs.messages[i].message_p1; i++) {
            /* get rid of the messages that aren't for invisible spells */
            if (!prefix(msg, "Something ") && !prefix(msg, "You "))
                break;
            if (borg_message_contains(msg, &spell_invis_msgs.messages[i])) {
                strnfmt(buf, 256, "SPELL_%03d:%s", spell_invis_msgs.index[i],
                    "Something");
                borg_react(msg, buf);
                return;
            }
        }
        for (i = 0; spell_msgs.messages[i].message_p1; i++) {
            if (borg_message_contains(msg, &spell_msgs.messages[i])) {
                char *start = strstr(msg, spell_msgs.messages[i].message_p1);
                if (start) {
                    strnfmt(who, (start - msg), "%s", msg);
                    strnfmt(
                        buf, 256, "SPELL_%03d:%s", spell_msgs.index[i], who);
                    borg_react(msg, buf);
                    return;
                }
            }
        }

        /* State -- Asleep */
        if (suffix(msg, " falls asleep!")) {
            tmp = strlen(" falls asleep!");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_SLEEP:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- confused */
        if (suffix(msg, " looks confused.")) {
            tmp = strlen(" looks confused.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_CONFUSED:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- confused */
        if (suffix(msg, " looks more confused.")) {
            tmp = strlen(" looks more confused.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_CONFUSED:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Asleep */
        if (suffix(msg, " wakes up.")) {
            tmp = strlen(" wakes up.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_AWAKE:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Afraid */
        if (suffix(msg, " flees in terror!")) {
            tmp = strlen(" flees in terror!");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__FEAR:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Afraid */
        if (suffix(msg, " recovers his courage.")) {
            tmp = strlen(" recovers his courage.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__BOLD:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Afraid */
        if (suffix(msg, " recovers her courage.")) {
            tmp = strlen(" recovers her courage.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__BOLD:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Afraid */
        if (suffix(msg, " recovers its courage.")) {
            tmp = strlen(" recovers its courage.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__BOLD:%s", who);
            borg_react(msg, buf);
            return;
        }
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "The door appears to be broken.")) {
        /* Only process open doors */
        if (ag->feat == FEAT_OPEN) {
            /* Mark as broken */
            ag->feat = FEAT_BROKEN;

            /* Clear goals */
            borg.goal.type = 0;
        }
        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "This seems to be permanent rock.")) {
        /* Only process walls */
        if ((ag->feat >= FEAT_GRANITE) && (ag->feat <= FEAT_PERM)) {
            /* Mark the wall as permanent */
            ag->feat = FEAT_PERM;

            /* Clear goals */
            borg.goal.type = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the granite wall.")) {
        /* reseting my panel clock */
        borg.time_this_panel = 1;

        /* Only process walls */
        if ((ag->feat >= FEAT_GRANITE) && (ag->feat <= FEAT_PERM)) {
            /* Mark the wall as granite */
            ag->feat = FEAT_GRANITE;

            /* Clear goals */
            borg.goal.type = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the quartz vein.")) {
        /* Process magma veins with treasure */
        if (ag->feat == FEAT_MAGMA_K) {
            /* Mark the vein */
            ag->feat = FEAT_QUARTZ_K;

            /* Clear goals */
            borg.goal.type = 0;
        }

        /* Process magma veins */
        else if (ag->feat == FEAT_MAGMA) {
            /* Mark the vein */
            ag->feat = FEAT_QUARTZ;

            /* Clear goals */
            borg.goal.type = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the magma vein.")) {
        /* Process quartz veins with treasure */
        if (ag->feat == FEAT_QUARTZ_K) {
            /* Mark the vein */
            ag->feat = FEAT_MAGMA_K;

            /* Clear goals */
            borg.goal.type = 0;
        }

        /* Process quartz veins */
        else if (ag->feat == FEAT_QUARTZ) {
            /* Mark the vein */
            ag->feat = FEAT_MAGMA;

            /* Clear goals */
            borg.goal.type = 0;
        }

        return;
    }

    /* Word of Recall -- Ignition */
    if (prefix(msg, "The air about you becomes ")) {
        /* Initiate recall */
        /* Guess how long it will take to lift off */
        /* Guess. game turns x 1000 ( 15+rand(20))*/
        borg.goal.recalling = 15000 + 5000;
        return;
    }

    /* Word of Recall -- Lift off */
    if (prefix(msg, "You feel yourself yanked ")) {
        /* Recall complete */
        borg.goal.recalling = 0;
        return;
    }

    /* Word of Recall -- Cancelled */
    if (prefix(msg, "A tension leaves ")) {
        /* Hack -- Oops */
        borg.goal.recalling = 0;
        return;
    }

    /* Wearing Cursed Item */
    if (prefix(msg, "Oops! It feels deathly cold!")) {
        /* this should only happen with STICKY items, The Crown of Morgoth or
         * The One Ring */
        /* !FIX !TODO !AJG handle crown eventually */
        return;
    }

    /* protect from evil */
    if (prefix(msg, "You feel safe from evil!")) {
        borg.temp.prot_from_evil = true;
        return;
    }
    if (prefix(msg, "You no longer feel safe from evil.")) {
        borg.temp.prot_from_evil = false;
        return;
    }
    /* haste self */
    if (prefix(msg, "You feel yourself moving faster!")) {
        borg.temp.fast = true;
        return;
    }
    if (prefix(msg, "You feel yourself slow down.")) {
        borg.temp.fast = false;
        return;
    }
    /* Bless */
    if (prefix(msg, "You feel righteous")) {
        borg.temp.bless = true;
        return;
    }
    if (prefix(msg, "The prayer has expired.")) {
        borg.temp.bless = false;
        return;
    }

    /* fastcast */
    if (prefix(msg, "You feel your mind accelerate.")) {
        borg.temp.fastcast = true;
        return;
    }
    if (prefix(msg, "You feel your mind slow again.")) {
        borg.temp.fastcast = false;
        return;
    }

    /* hero */
    if (prefix(msg, "You feel like a hero!")) {
        borg.temp.hero = true;
        return;
    }
    if (prefix(msg, "You no longer feel heroic.")) {
        borg.temp.hero = false;
        return;
    }

    /* berserk */
    if (prefix(msg, "You feel like a killing machine!")) {
        borg.temp.berserk = true;
        return;
    }
    if (prefix(msg, "You no longer feel berserk.")) {
        borg.temp.berserk = false;
        return;
    }

    /* Sense Invisible */
    if (prefix(msg, "Your eyes feel very sensitive!")) {
        borg.see_inv = 30000;
        return;
    }
    if (prefix(msg, "Your eyes no longer feel so sensitive.")) {
        borg.see_inv = 0;
        return;
    }

    /* check for wall blocking but not when confused*/
    if ((prefix(msg, "There is a wall ") && (!borg.trait[BI_ISCONFUSED]))) {
        my_need_redraw = true;
        my_need_alter  = true;
        borg.goal.type = 0;
        return;
    }

    /* check for closed door but not when confused*/
    if ((prefix(msg, "There is a closed door blocking your way.")
            && (!borg.trait[BI_ISCONFUSED] && !borg.trait[BI_ISIMAGE]))) {
        my_need_redraw = true;
        my_need_alter  = true;
        borg.goal.type = 0;
        return;
    }

    /* check for mis-alter command.  Sometime induced by never_move guys*/
    if (prefix(msg, "You spin around.") && !borg.trait[BI_ISCONFUSED]) {
        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++) {

            borg_kill *kill = &borg_kills[i];

            /* Skip dead monsters */
            if (!kill->r_idx)
                continue;

            /* Now do distance considerations */
            x9 = kill->pos.x;
            y9 = kill->pos.y;

            /* Distance components */
            ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
            ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

            /* Distance */
            d = MAX(ax, ay);

            /* if the guy is too close then delete him. */
            if (d < 4) {
                /* Hack -- kill em */
                borg_delete_kill(i);
            }
        }

        my_no_alter    = true;
        borg.goal.type = 0;
        return;
    }

    /* Check for the missing staircase */
    if (prefix(msg, "No known path to ") || 
        prefix(msg, "Something is here.")) {
        /* make sure the aligned dungeon is on */

        /* make sure the borg does not think he's on one */
        /* Remove all stairs from the array. */
        track_less.num                      = 0;
        track_more.num                      = 0;
        borg_grids[borg.c.y][borg.c.x].feat = FEAT_BROKEN;

        return;
    }

    /* Feature XXX XXX XXX */
    if (prefix(msg, "You see nothing there ")) {
        ag->feat    = FEAT_BROKEN;

        my_no_alter = true;
        /* Clear goals */
        borg.goal.type = 0;
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Illegal ")) {
        /* Hack -- Oops */
        borg_respawning = 7;
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg.time_this_panel += 100;
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "You have nothing to identify")) {
        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg.time_this_panel += 100;

        /* ID all items (equipment) */
        for (i = INVEN_WIELD; i <= INVEN_FEET; i++) {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty)
                continue;

            item->ident = true;
        }

        /* ID all items  (inventory) */
        for (i = 0; i <= z_info->pack_size; i++) {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty)
                continue;

            item->ident = true;
        }
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Identifying The Phial")) {

        /* ID item (equipment) */
        borg_item *item = &borg_items[INVEN_LIGHT];
        item->ident     = true;

        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg.time_this_panel += 100;
    }

    /* resist acid */
    if (prefix(msg, "You feel resistant to acid!")) {
        borg.temp.res_acid = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to acid.")) {
        borg.temp.res_acid = false;
        return;
    }
    /* resist electricity */
    if (prefix(msg, "You feel resistant to electricity!")) {
        borg.temp.res_elec = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to electricity.")) {
        borg.temp.res_elec = false;
        return;
    }
    /* resist fire */
    if (prefix(msg, "You feel resistant to fire!")) {
        borg.temp.res_fire = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to fire.")) {
        borg.temp.res_fire = false;
        return;
    }
    /* resist cold */
    if (prefix(msg, "You feel resistant to cold!")) {
        borg.temp.res_cold = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to cold.")) {
        borg.temp.res_cold = false;
        return;
    }
    /* resist poison */
    if (prefix(msg, "You feel resistant to poison!")) {
        borg.temp.res_pois = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to poison.")) {
        borg.temp.res_pois = false;
        return;
    }

    /* Shield */
    if (prefix(msg, "A mystic shield forms around your body!")
        || prefix(msg, "Your skin turns to stone.")) {
        borg.temp.shield = true;
        return;
    }
    if (prefix(msg, "Your mystic shield crumbles away.")
        || prefix(msg, "A fleshy shade returns to your skin.")) {
        borg.temp.shield = false;
        return;
    }

    /* Glyph of Warding (the spell no longer gives a report)*/
    /* Sadly  Rune of Protection has no message */
    if (prefix(msg, "You inscribe a mystic symbol on the ground!")) {
        /* Check for an existing glyph */
        for (i = 0; i < track_glyph.num; i++) {
            /* Stop if we already new about this glyph */
            if ((track_glyph.x[i] == borg.c.x)
                && (track_glyph.y[i] == borg.c.y))
                break;
        }

        /* Track the newly discovered glyph */
        if ((i == track_glyph.num) && (i < track_glyph.size)) {
            borg_note("# Noting the creation of a glyph.");
            track_glyph.x[i] = borg.c.x;
            track_glyph.y[i] = borg.c.y;
            track_glyph.num++;
        }

        return;
    }
    if (prefix(msg, "The rune of protection is broken!")) {
        /* we won't know which is broken so erase them all and
         * allow the borg to scan the screen and rebuild the array.
         * He won't see the one under him though.  So a special check
         * must be made.
         */

        /* Remove the entire array */
        for (i = 0; i < track_glyph.num; i++) {
            /* Stop if we already new about this glyph */
            track_glyph.x[i] = 0;
            track_glyph.y[i] = 0;
        }
        /* no known glyphs */
        track_glyph.num = 0;

        /* Check for glyphs under player -- Cheat*/
        if (square_iswarded(cave, borg.c)) {
            track_glyph.x[track_glyph.num] = borg.c.x;
            track_glyph.y[track_glyph.num] = borg.c.y;
            track_glyph.num++;
        }
        return;
    }
    /* failed glyph spell message */
    if (prefix(msg, "The object resists the spell")
        || prefix(msg, "There is no clear floor")) {

        /* Forget the newly created-though-failed  glyph */
        track_glyph.x[track_glyph.num] = 0;
        track_glyph.y[track_glyph.num] = 0;
        track_glyph.num--;

        /* note it */
        borg_note("# Removing the Glyph under me, placing with broken door.");

        /* mark that we are not on a clear spot.  The borg ignores
         * broken doors and this will keep him from casting it again.
         */
        ag->feat = FEAT_BROKEN;
        return;
    }

    /* Removed rubble.  Important when out of lite */
    if (prefix(msg, "You have removed the ")) {
        int x, y;
        /* remove rubbles from array */
        for (y = borg.c.y - 1; y < borg.c.y + 1; y++) {
            for (x = borg.c.x - 1; x < borg.c.x + 1; x++) {
                /* replace all rubble with broken doors, the borg ignores
                 * broken doors.  This routine is only needed if the borg
                 * is out of lite and searching in the dark.
                 */
                if (borg.trait[BI_CURLITE])
                    continue;

                if (ag->feat == FEAT_RUBBLE)
                    ag->feat = FEAT_BROKEN;
            }
        }
        return;
    }

    if (prefix(msg, "The enchantment failed")) {
        /* reset our panel clock for this */
        borg.time_this_panel = 1;
        return;
    }

    /* need to kill monsters when WoD is used */
    if (prefix(msg, "There is a searing blast of light!")) {
        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++) {
            borg_kill *kill = &borg_kills[i];

            x9              = kill->pos.x;
            y9              = kill->pos.y;

            /* Skip dead monsters */
            if (!kill->r_idx)
                continue;

            /* Distance components */
            ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
            ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

            /* Distance */
            d = MAX(ax, ay);

            /* Minimal distance */
            if (d > 12)
                continue;

            /* Hack -- kill em */
            borg_delete_kill(i);
        }

        /* Remove the region fear as well */
        borg_fear_region[borg.c.y / 11][borg.c.x / 11] = 0;

        return;
    }

    /* Be aware and concerned of busted doors */
    if (prefix(msg, "You hear a door burst open!")) {
        /* on level 1 and 2 be concerned.  Could be Grip or Fang */
        if (borg.trait[BI_CDEPTH] <= 3 && borg.trait[BI_CLEVEL] <= 5)
            scaryguy_on_level = true;
    }

    /* Some spells move the borg from his grid */
    if (prefix(msg, "commands you to return.")
        || prefix(msg, "teleports you away.")
        || prefix(msg, "gestures at your feet.")) {
        /* If in Lunal mode better shut that off, he is not on the stairs
         * anymore */
        borg.lunal_mode = false;
        borg_note("# Disconnecting Lunal Mode due to monster spell.");
    }

    /* Sometimes the borg will overshoot the range limit of his shooter */
    if (prefix(msg, "Target out of range.")) {
        /* Fire Anyway? [Y/N] */
        borg_keypress('y');
    }

    /* Feelings about the level */
    for (i = 0; prefix_feeling_danger[i]; i++) {
        /* "You feel..." (etc) */
        if (prefix(msg, prefix_feeling_danger[i])) {
            strnfmt(buf, 256, "FEELING_DANGER:%d", i);
            borg_react(msg, buf);
            return;
        }
    }

    for (i = 0; suffix_feeling_stuff[i]; i++) {
        /* "You feel..." (etc) */
        if (suffix(msg, suffix_feeling_stuff[i])) {
            strnfmt(buf, 256, "FEELING_STUFF:%d", i);
            borg_react(msg, buf);
            return;
        }
    }
}

/*
 * Parse a message, piece of a message, or set of messages.
 *
 * We must handle long messages which are "split" into multiple
 * pieces, and also multiple messages which may be "combined"
 * into a single set of messages.
 */
void borg_parse(char *msg)
{
    static int  len = 0;
    static char buf[1024];

    /* Note the long message */
    if (borg_cfg[BORG_VERBOSE] && msg)
        borg_note(format("# Parsing msg <%s>", msg));

    /* Flush messages */
    if (len && (!msg || (msg[0] != ' '))) {
        int i, j;

        /* Split out punctuation */
        for (j = i = 0; i < len - 1; i++) {
            /* Check for punctuation */
            if ((buf[i] == '.') || (buf[i] == '!') || (buf[i] == '?')
                || (buf[i] == '"')) {
                /* Require space */
                if (buf[i + 1] == ' ') {
                    /* Terminate */
                    buf[i + 1] = '\0';

                    /* Parse fragment */
                    borg_parse_aux(buf + j, (i + 1) - j);

                    /* Restore */
                    buf[i + 1] = ' ';

                    /* Advance past spaces */
                    for (j = i + 2; buf[j] == ' '; j++) /* loop */
                        ;
                }
            }
        }

        /* Parse tail */
        borg_parse_aux(buf + j, len - j);

        /* Forget */
        len = 0;
    }

    /* No message */
    if (!msg) {
        /* Start over */
        len = 0;
    }

    /* Continued message */
    else if (msg[0] == ' ') {
        /* Collect, verify, and grow */
        len += strnfmt(buf + len, 1024 - len, "%s", msg + 1);
    }

    /* New message */
    else {
        /* Collect, verify, and grow */
        len = strnfmt(buf, 1024, "%s", msg);
    }
}

/* all parts equal or same nullness */
static bool borg_read_message_equal(
    struct borg_read_message *msg1, struct borg_read_message *msg2)
{
    if (((msg1->message_p1 && msg2->message_p1
             && streq(msg1->message_p1, msg2->message_p1))
            || (!msg1->message_p1 && !msg2->message_p1))
        && ((msg1->message_p2 && msg2->message_p2
                && streq(msg1->message_p2, msg2->message_p2))
            || (!msg1->message_p2 && !msg2->message_p2))
        && ((msg1->message_p3 && msg2->message_p3
                && streq(msg1->message_p3, msg2->message_p3))
            || (!msg1->message_p3 && !msg2->message_p3)))
        return true;
    return false;
}

static void insert_msg(struct borg_read_messages *msgs,
    struct borg_read_message *msg, int spell_number)
{
    int  i;
    bool found_dup = false;

    /* this way we don't have to pre-create the array */
    if (msgs->messages == NULL) {
        msgs->allocated = 10;
        msgs->messages
            = mem_alloc(sizeof(struct borg_read_message) * msgs->allocated);
        msgs->index = mem_alloc(sizeof(int) * msgs->allocated);
    }

    if (msg == NULL) {
        msgs->messages[msgs->count].message_p1 = NULL;
        msgs->messages[msgs->count].message_p2 = NULL;
        msgs->messages[msgs->count].message_p3 = NULL;
        msgs->count++;
        /* shrink array down, we are done*/
        msgs->messages = mem_realloc(
            msgs->messages, sizeof(struct borg_read_message) * msgs->count);
        msgs->index     = mem_realloc(msgs->index, sizeof(int) * msgs->count);
        msgs->allocated = msgs->count;
        return;
    }

    for (i = 0; i < msgs->count; i++) {
        if (borg_read_message_equal(&msgs->messages[i], msg)) {
            found_dup = true;
            break;
        }
    }
    if (!found_dup) {
        memcpy(&(msgs->messages[msgs->count]), msg,
            sizeof(struct borg_read_message));
        msgs->index[msgs->count] = spell_number;
        msgs->count++;
        if (msgs->count == msgs->allocated) {
            msgs->allocated += 10;
            msgs->messages = mem_realloc(msgs->messages,
                sizeof(struct borg_read_message) * msgs->allocated);
            msgs->index
                = mem_realloc(msgs->index, sizeof(int) * msgs->allocated);
        }
    } else {
        string_free(msg->message_p1);
        string_free(msg->message_p2);
        string_free(msg->message_p3);
    }
}

static void clean_msgs(struct borg_read_messages *msgs)
{
    int i;

    for (i = 0; i < msgs->count; ++i) {
        string_free(msgs->messages[i].message_p1);
        string_free(msgs->messages[i].message_p2);
        string_free(msgs->messages[i].message_p3);
    }
    mem_free(msgs->messages);
    msgs->messages = NULL;
    mem_free(msgs->index);
    msgs->index     = NULL;
    msgs->count     = 0;
    msgs->allocated = 0;
}

/* get rid of leading spaces */
static char *borg_trim_lead_space(char *orig)
{
    if (!orig || orig[0] != ' ')
        return orig;

    return &orig[1];
}

/*
 * break a string into a borg_read_message
 *
 * a message can have up to three parts broken up by variables
 * ex: "{name} hits {pronoun} followers with {type} ax."
 * " hits ", " followers with ", " ax."
 * if the message has more parts than that, they are ignored so
 * ex: "{name} hits {pronoun} followers with {type} ax and {type} breath."
 * would end up as
 * " hits ", " followers with ", " ax and "
 * hopefully this is enough to keep the messages as unique as possible
 */
static void borg_load_read_message(
    char *message, struct borg_read_message *read_message)
{
    read_message->message_p1 = NULL;
    read_message->message_p2 = NULL;
    read_message->message_p3 = NULL;

    char *suffix             = strchr(message, '}');
    if (!suffix) {
        /* no variables, use message as is */
        read_message->message_p1 = string_make(borg_trim_lead_space(message));
        return;
    }
    /* skip leading variable, if there is one */
    if (message[0] == '{')
        suffix++;
    else
        suffix = message;
    char *var = strchr(suffix, '{');
    if (!var) {
        /* one variable, use message post variable */
        read_message->message_p1 = string_make(borg_trim_lead_space(suffix));
        return;
    }
    while (suffix[0] == ' ')
        suffix++;
    int part_len             = strlen(suffix) - strlen(var);
    read_message->message_p1 = string_make(format("%.*s", part_len, suffix));
    suffix += part_len;
    suffix = strchr(var, '}');
    if (!suffix)
        return; /* this should never happen but ... if a string is { with no }*/
    suffix++;
    var = strchr(suffix, '{');
    if (!var) {
        while (suffix[0] == ' ')
            suffix++;

        /* two variables, ignore if last part is just . */
        if (strlen(suffix) && !streq(suffix, "."))
            read_message->message_p2 = string_make(suffix);
        return;
    }
    while (suffix[0] == ' ')
        suffix++;
    part_len = strlen(suffix) - strlen(var);
    if (part_len) {
        read_message->message_p2 = string_make(
            borg_trim_lead_space(format("%.*s", part_len, suffix)));
    }
    suffix += part_len;
    suffix = strchr(var, '}');
    if (!suffix)
        return; /* this should never happen but ... if a string is { with no }*/
    suffix++;
    var = strchr(suffix, '{');
    if (!var) {
        while (suffix[0] == ' ')
            suffix++;
        /* three variables, ignore if last part is just . */
        if (strlen(suffix) && !streq(suffix, ".")) {
            if (read_message->message_p2) {
                read_message->message_p3
                    = string_make(borg_trim_lead_space(suffix));
            } else {
                read_message->message_p2
                    = string_make(borg_trim_lead_space(suffix));
            }
        }
        return;
    }
    while (suffix[0] == ' ')
        suffix++;
    part_len = strlen(suffix) - strlen(var);
    if (read_message->message_p2)
        read_message->message_p3 = string_make(
            borg_trim_lead_space(format("%.*s", part_len, suffix)));
    else
        read_message->message_p2 = string_make(
            borg_trim_lead_space(format("%.*s", part_len, suffix)));

    return;
}

/* load monster spell messages */
static void borg_init_spell_messages(void)
{
    const struct monster_spell       *spell = monster_spells;
    const struct monster_spell_level *spell_level;
    struct borg_read_message          read_message;

    while (spell) {
        spell_level = spell->level;
        while (spell_level) {
            if (spell_level->blind_message) {
                borg_load_read_message(
                    spell_level->blind_message, &read_message);
                insert_msg(&spell_invis_msgs, &read_message, spell->index);
            }
            if (spell_level->message) {
                borg_load_read_message(spell_level->message, &read_message);
                insert_msg(&spell_msgs, &read_message, spell->index);
            }
            if (spell_level->miss_message) {
                borg_load_read_message(
                    spell_level->miss_message, &read_message);
                insert_msg(&spell_msgs, &read_message, spell->index);
            }
            spell_level = spell_level->next;
        }
        spell = spell->next;
    }
    /* null terminate */
    insert_msg(&spell_invis_msgs, NULL, 0);
    insert_msg(&spell_msgs, NULL, 0);
}

/* HACK pluralize ([|] parsing) code stolen from mon-msg.c */
/* State machine constants for get_message_text() */
#define MSG_PARSE_NORMAL 0
#define MSG_PARSE_SINGLE 1
#define MSG_PARSE_PLURAL 2

/* load monster in pain messages */
static char *borg_get_parsed_pain(const char *pain, bool do_plural)
{
    size_t buflen = strlen(pain) + 1;
    char  *buf    = mem_zalloc(buflen);

    int    state  = MSG_PARSE_NORMAL;
    size_t maxlen = strlen(pain);
    size_t pos    = 1;

    /* for the borg, always start with a space */
    buf[0] = ' ';

    /* Put the message characters in the buffer */
    /* XXX This logic should be used everywhere for pluralising strings */
    for (size_t i = 0; i < maxlen && pos < buflen - 1; i++) {
        char cur = pain[i];

        /*
         * The characters '[|]' switch parsing mode and are never output.
         * The syntax is [singular|plural]
         */
        if (state == MSG_PARSE_NORMAL && cur == '[') {
            state = MSG_PARSE_SINGLE;
        } else if (state == MSG_PARSE_SINGLE && cur == '|') {
            state = MSG_PARSE_PLURAL;
        } else if (state != MSG_PARSE_NORMAL && cur == ']') {
            state = MSG_PARSE_NORMAL;
        } else if (state == MSG_PARSE_NORMAL
                   || (state == MSG_PARSE_SINGLE && do_plural == false)
                   || (state == MSG_PARSE_PLURAL && do_plural == true)) {
            /* Copy the characters according to the mode */
            buf[pos++] = cur;
        }
    }
    return buf;
}

static void borg_insert_pain(const char *pain, int *capacity, int *count)
{
    char *new_message;
    if (*capacity <= (*count) + 2) {
        *capacity += 14;
        suffix_pain = mem_realloc(suffix_pain, sizeof(char *) * (*capacity));
    }

    new_message             = borg_get_parsed_pain(pain, false);
    suffix_pain[(*count)++] = new_message;
    new_message             = borg_get_parsed_pain(pain, true);
    suffix_pain[(*count)++] = new_message;
}

static void borg_init_pain_messages(void)
{
    int                  capacity = 1;
    int                  count    = 0;
    int                  idx, i;
    struct monster_pain *pain;

    suffix_pain = mem_alloc(sizeof(char *) * capacity);

    for (idx = 0; idx < z_info->mp_max; idx++) {
        pain = &pain_messages[idx];
        for (i = 0; i < 7; i++) {
            if (pain == NULL || pain->messages[i] == NULL)
                break;
            borg_insert_pain(pain->messages[i], &capacity, &count);
        }
    }

    if ((count + 1) != capacity)
        suffix_pain = mem_realloc(suffix_pain, sizeof(char *) * (count + 1));
    suffix_pain[count] = NULL;
}

/* load player hit by messages */
static void borg_init_hit_by_messages(void)
{
    struct borg_read_message read_message;

    for (int i = 0; i < z_info->blow_methods_max; i++) {
        struct blow_message *messages = blow_methods[i].messages;

        while (messages) {
            if (messages->act_msg) {
                borg_load_read_message(messages->act_msg, &read_message);
                insert_msg(&suffix_hit_by, &read_message, blow_methods[i].msgt);
            }
            messages = messages->next;
        }
    }
    /* null terminate */
    insert_msg(&suffix_hit_by, NULL, 0);
}

/* init all messages used by the borg */
void borg_init_messages(void)
{
    borg_init_spell_messages();
    borg_init_pain_messages();
    borg_init_hit_by_messages();

    /*** Message tracking ***/

    /* No chars saved yet */
    borg_msg_len = 0;

    /* Maximum buffer size */
    borg_msg_siz = 4096;

    /* Allocate a buffer */
    borg_msg_buf = mem_zalloc(borg_msg_siz * sizeof(char));

    /* No msg's saved yet */
    borg_msg_num = 0;

    /* Maximum number of messages */
    borg_msg_max = 256;

    /* Allocate array of positions */
    borg_msg_pos = mem_zalloc(borg_msg_max * sizeof(int16_t));

    /* Allocate array of use-types */
    borg_msg_use = mem_zalloc(borg_msg_max * sizeof(int16_t));
}

/* free all messages used by the borg */
void borg_free_messages(void)
{
    int i;

    mem_free(borg_msg_use);
    borg_msg_use = NULL;
    mem_free(borg_msg_pos);
    borg_msg_pos = NULL;
    borg_msg_num = 0;
    mem_free(borg_msg_buf);
    borg_msg_buf = NULL;
    borg_msg_siz = 0;

    if (suffix_pain) {
        for (i = 0; suffix_pain[i]; ++i) {
            mem_free(suffix_pain[i]);
            suffix_pain[i] = NULL;
        }
        mem_free(suffix_pain);
        suffix_pain = NULL;
    }
    clean_msgs(&suffix_hit_by);
    clean_msgs(&spell_invis_msgs);
    clean_msgs(&spell_msgs);
}

#endif
