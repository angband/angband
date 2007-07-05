.SILENT:

include ../../mk/rules.mk
include ../../mk/init.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/data

include ../../mk/objective.mk

clean-posthook:
	-$(RM) *.raw

install-posthook:
	if [ "x$(SETEGID)" != "x" ]; then \
		printf "%10s     %s*.raw\n" TOUCH ${DATA_PATH}/data/; \
		if [ "x$(DRY)" = "x" ]; then \
			touch ${DATA_PATH}/data/artifact.raw; \
			touch ${DATA_PATH}/data/cost_adj.raw; \
			touch ${DATA_PATH}/data/ego_item.raw; \
			touch ${DATA_PATH}/data/flavor.raw; \
			touch ${DATA_PATH}/data/limits.raw; \
			touch ${DATA_PATH}/data/monster.raw; \
			touch ${DATA_PATH}/data/object.raw; \
			touch ${DATA_PATH}/data/p_class.raw; \
			touch ${DATA_PATH}/data/p_hist.raw; \
			touch ${DATA_PATH}/data/p_race.raw; \
			touch ${DATA_PATH}/data/prices.raw; \
			touch ${DATA_PATH}/data/shop_own.raw; \
			touch ${DATA_PATH}/data/terrain.raw; \
			touch ${DATA_PATH}/data/vault.raw; \
			chown -R root:${SETEGID} ${DATA_PATH}/data; \
			chmod -R g+w ${DATA_PATH}/data; \
		fi; \
	fi
