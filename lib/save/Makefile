include ../../mk/rules.mk
include ../../mk/objective.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/save

install-posthook:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			chown root:${SETEGID} ${DATA_PATH}/save; \
			chmod 070 ${DATA_PATH}/save; \
		fi; \
	fi
