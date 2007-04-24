include ../../mk/rules.mk
include ../../mk/init.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/bone

include ../../mk/objective.mk

install-posthook:
	chown root:${SETEGID} ${DATA_PATH}/bone
	chmod 070 ${DATA_PATH}/bone
