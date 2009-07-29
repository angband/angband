MKPATH=../../mk/
include $(MKPATH)buildsys.mk

DATA = delete.me
PACKAGE = ${PACKAGE_NAME}/apex

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		if [ "x$(DRY)" = "x" ]; then \
			${MKDIR_P} ${DESTDIR}${datadir}/${PACKAGE}; \
			touch ${DESTDIR}${datadir}/${PACKAGE}/scores.raw; \
			chown -R root:${SETEGID} ${DESTDIR}${datadir}/${PACKAGE}; \
			chmod -R g+w ${DESTDIR}${datadir}/${PACKAGE}; \
		fi; \
	fi
