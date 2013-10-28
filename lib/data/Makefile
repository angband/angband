MKPATH=../../mk/
include $(MKPATH)buildsys.mk

PACKAGE = ${PACKAGE_NAME}/data
DATA_ = artifact ego_item flavor limits monster object p_class p_hist p_race prices shop_own spells terrain vault

install-extra:
	if [ "x$(SETEGID)" != "x" ]; then \
		${MKDIR_P} ${DESTDIR}${datadir}/$(PACKAGE); \
		for i in ${DATA_}; do \
			touch ${DESTDIR}${datadir}/$(PACKAGE)/$${i}.raw; \
		done; \
		chown -R root:${SETEGID} ${DESTDIR}${datadir}/$(PACKAGE); \
		chmod -R g+w ${DESTDIR}${datadir}/$(PACKAGE); \
	fi
