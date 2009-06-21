MKPATH=../../mk/
include $(MKPATH)buildsys.mk

DATA = delete.me
PACKAGE = ${PACKAGE_NAME}/apex

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}$game_datadir/${PACKAGE}; \
			touch ${DESTDIR}$game_datadir/${PACKAGE}/scores.raw; \
			chown -R root:${SETEGID} ${DESTDIR}$game_datadir/${PACKAGE}; \
			chmod -R g+w ${DESTDIR}$game_datadir/${PACKAGE}; \
		fi; \
	fi
