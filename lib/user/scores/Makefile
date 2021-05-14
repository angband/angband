MKPATH=../../../mk/
include $(MKPATH)buildsys.mk

PACKAGE = scores

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			touch ${DESTDIR}${varshareddatadir}${PACKAGE}/scores.raw; \
			chown -R root:${SETEGID} ${DESTDIR}${varshareddatadir}${PACKAGE}; \
			chmod -R g+w ${DESTDIR}${varshareddatadir}${PACKAGE}; \
		fi; \
	fi
