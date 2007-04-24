include ../../mk/rules.mk
include ../../mk/init.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/save

include ../../mk/objective.mk

install-posthook:
	chown root:${SETEGID} ${DATA_PATH}/save
	chmod 070 ${DATA_PATH}/save
