MKPATH=../../../mk/
include $(MKPATH)buildsys.mk

PACKAGE = archive

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chown root:${SETEGID} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chmod g+w ${DESTDIR}${varshareddatadir}${PACKAGE}; \
		fi; \
	fi
