
# Makefile for comedi

.EXPORT_ALL_VARIABLES:

include version
MAJOR=0

CFLAGS = -Wall -O2

all:	comedilib

SUBDIRS= lib demo comedi_calibrate testing

DOCFILES= README INSTALL `find doc -type f`

INSTALLDIR=$(DESTDIR)/usr
INSTALLDIR_LIB=$(DESTDIR)/usr/lib
INSTALLDIR_DOC=$(DESTDIR)/usr/doc/libcomedi

comedilib:	subdirs

config:	dummy

install:	dummy
	install -d ${INSTALLDIR}/include
	(cd include;install -m 644 comedilib.h ${INSTALLDIR}/include)
	(cd include;install -m 644 comedi.h ${INSTALLDIR}/include)
	install lib/libcomedi.so.${MAJOR} ${INSTALLDIR_LIB}
	(cd $(INSTALLDIR_LIB);ln -s libcomedi.so.${version} libcomedi.so.${MAJOR})
	(cd $(INSTALLDIR_LIB);ln -s libcomedi.so.${version} libcomedi.so)
	install -m 644 lib/libcomedi.a ${INSTALLDIR_LIB}
	#/sbin/ldconfig -n ${INSTALLDIR}/lib
	install -d ${INSTALLDIR_DOC}
	install ${DOCFILES} ${INSTALLDIR_DOC}

lpr:	dummy
	find . -name '*.[chs]'|xargs enscript -2r -pit.ps

subdirs:	dummy
	set -e;for i in ${SUBDIRS};do ${MAKE} -C $$i ; done

clean:	dummy
	set -e;for i in $(SUBDIRS);do ${MAKE} clean -C $$i ; done

distclean:	clean

debian: dummy
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

