include ../../mk/rules.mk
include ../../mk/objective.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/bone

install-posthook:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			chown root:${SETEGID} ${DATA_PATH}/bone; \
			chmod 070 ${DATA_PATH}/bone; \
		fi; \
	fi
