
# Makefile for comedi

.EXPORT_ALL_VARIABLES:

include version
MAJOR=0

CFLAGS = -Wall -O2

all:	comedilib

SUBDIRS= lib demo comedi_calibrate testing comedi_config

DOCFILES= README INSTALL `find doc -type f`

INSTALLDIR=$(DESTDIR)/usr
INSTALLDIR_LIB=$(DESTDIR)/usr/lib
INSTALLDIR_DOC=$(DESTDIR)/usr/share/doc/libcomedi
INSTALLDIR_MAN=$(DESTDIR)/usr/share/man
INSTALLDIR_BIN=$(DESTDIR)/usr/bin
INSTALLDIR_SBIN=$(DESTDIR)/usr/sbin

comedilib:	subdirs

config:	dummy

install:	dummy
	install -d ${INSTALLDIR}/include
	(cd include;install -m 644 comedilib.h ${INSTALLDIR}/include)
	(cd include;install -m 644 comedi.h ${INSTALLDIR}/include)
	install lib/libcomedi.so.${version} ${INSTALLDIR_LIB}
	(cd $(INSTALLDIR_LIB);ln -sf libcomedi.so.${version} libcomedi.so.${MAJOR})
	(cd $(INSTALLDIR_LIB);ln -sf libcomedi.so.${version} libcomedi.so)
	install -m 644 lib/libcomedi.a ${INSTALLDIR_LIB}
	#/sbin/ldconfig -n ${INSTALLDIR}/lib
	install -d ${INSTALLDIR_DOC}
	install ${DOCFILES} ${INSTALLDIR_DOC}
	install man/*.7 ${INSTALLDIR_MAN}/man7
	install man/*.8 ${INSTALLDIR_MAN}/man8
	install -m 755 comedi_config/comedi_config ${INSTALLDIR_SBIN}
	install -m 755 comedi_calibrate/comedi_calibrate ${INSTALLDIR_BIN}

lpr:	dummy
	find . -name '*.[chs]'|xargs enscript -2r -pit.ps

subdirs:	dummy
	set -e;for i in ${SUBDIRS};do ${MAKE} -C $$i ; done

clean:	dummy
	set -e;for i in $(SUBDIRS);do ${MAKE} clean -C $$i ; done

distclean:	clean

debian: dummy
	chmod 755 debian/rules
	dpkg-buildpackage -rfakeroot

dev:	dummy
	-rm /dev/comedi*
	/bin/mknod /dev/comedi0 c 98 0
	/bin/mknod /dev/comedi1 c 98 1
	/bin/mknod /dev/comedi2 c 98 2
	/bin/mknod /dev/comedi3 c 98 3
	chown root.root /dev/comedi*
	chmod 666 /dev/comedi*

dummy:

