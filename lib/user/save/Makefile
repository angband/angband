MKPATH=../../../mk/
include $(MKPATH)buildsys.mk

PACKAGE = save

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chown root:${SETEGID} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chmod 171 ${DESTDIR}${varshareddatadir}${PACKAGE}; \
		fi; \
	fi
