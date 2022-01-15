MKPATH=../../../mk/
include $(MKPATH)buildsys.mk

PACKAGE = panic

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chown root:${SETEGID} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chmod 770 ${DESTDIR}${varshareddatadir}${PACKAGE}; \
		fi; \
	fi
