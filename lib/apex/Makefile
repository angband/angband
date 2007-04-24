.SILENT:

include ../../mk/rules.mk
include ../../mk/init.mk

OBJECTIVE_DATA = \
	delete.me:${DATA_PATH}/apex

include ../../mk/objective.mk

install-posthook:
	touch ${DATA_PATH}/apex/scores.raw
	chown -R root:${SETEGID} ${DATA_PATH}/apex
	chmod -R g+w ${DATA_PATH}/apex
