MKPATH=../../mk/
include $(MKPATH)buildsys.mk

DATA = delete.me
PACKAGE = ${PACKAGE_NAME}/save

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}$game_datadir/${PACKAGE}; \
			chown root:${SETEGID} ${DESTDIR}$game_datadir/${PACKAGE}; \
			chmod 070 ${DESTDIR}$game_datadir/${PACKAGE}; \
		fi; \
	fi
