MKPATH=../../mk/
include $(MKPATH)buildsys.mk

PACKAGE = save

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${vardatadir}${PACKAGE}; \
			chown root:${SETEGID} ${DESTDIR}${vardatadir}${PACKAGE}; \
			chmod 070 ${DESTDIR}${vardatadir}${PACKAGE}; \
		fi; \
	fi
