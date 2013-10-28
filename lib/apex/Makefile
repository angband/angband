include ../../mk/rules.mk
include ../../mk/objective.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/apex

install-posthook:
	if [ "x$(SETEGID)" != "x" ]; then \
		printf "%10s %-20s\n" TOUCH ${DATA_PATH}/apex/scores.raw; \
		if [ "x$(DRY)" = "x" ]; then \
			touch ${DATA_PATH}/apex/scores.raw; \
			chown -R root:${SETEGID} ${DATA_PATH}/apex; \
			chmod -R g+w ${DATA_PATH}/apex; \
		fi; \
	fi;
