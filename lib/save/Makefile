MKPATH=../../mk/
include $(MKPATH)buildsys.mk

DATA = delete.me
PACKAGE = save

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${datadir}/${PACKAGE}; \
			chown root:${SETEGID} ${DESTDIR}${datadir}/${PACKAGE}; \
			chmod 070 ${DESTDIR}${datadir}/${PACKAGE}; \
		fi; \
	fi
