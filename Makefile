
# Makefile for comedi

TOPDIR=`pwd`

include Config
include version

.EXPORT_ALL_VARIABLES:

MAJOR=0

TARGETS = comedilib
ifeq ($(with_python),yes)
TARGETS += python
endif
ifeq ($(with_perl),yes)
TARGETS += perl
endif

all:	$(TARGETS)

SUBDIRS= lib demo comedi_calibrate testing comedi_config

DOCFILES= README `find doc -type f`

INSTALLDIR=$(DESTDIR)$(PREFIX)
INSTALLDIR_LIB=$(DESTDIR)$(PREFIX)/lib
ifneq ($(DEB_BUILD_ARCH),)
INSTALLDIR_DOC=$(DESTDIR)$(PREFIX)/share/doc/libcomedi
INSTALLDIR_MAN=$(DESTDIR)$(PREFIX)/share/man
INSTALLDIR_PERL=$(DESTDIR)$(PREFIX)/lib/perl5/
else
INSTALLDIR_DOC=$(DESTDIR)$(PREFIX)/doc/libcomedi
INSTALLDIR_MAN=$(DESTDIR)$(PREFIX)/man
INSTALLDIR_PERL=$(DESTDIR)$(PREFIX)/lib/perl/
endif
INSTALLDIR_BIN=$(DESTDIR)$(PREFIX)/bin
INSTALLDIR_SBIN=$(DESTDIR)$(PREFIX)/sbin

comedilib:	subdirs

config:	dummy

install:	dummy
	install -d ${INSTALLDIR}/include
	install -m 644 include/comedilib.h ${INSTALLDIR}/include
	install -m 644 include/comedi.h ${INSTALLDIR}/include
	install lib/libcomedi.so.${version} ${INSTALLDIR_LIB}
	(cd $(INSTALLDIR_LIB);ln -sf libcomedi.so.${version} libcomedi.so.${MAJOR})
	(cd $(INSTALLDIR_LIB);ln -sf libcomedi.so.${version} libcomedi.so)
	install -m 644 lib/libcomedi.a ${INSTALLDIR_LIB}
ifneq ($(INSTALLDIR),)
	install -d ${INSTALLDIR_DOC}
	install ${DOCFILES} ${INSTALLDIR_DOC}
endif
	install man/*.7 ${INSTALLDIR_MAN}/man7
	install man/*.8 ${INSTALLDIR_MAN}/man8
	install -s -m 755 comedi_config/comedi_config ${INSTALLDIR_SBIN}
	install -s -m 755 comedi_calibrate/comedi_calibrate ${INSTALLDIR_BIN}

install_debian: install
	install -d ${INSTALLDIR_DOC}
	install -m 644 ${DOCFILES} ${INSTALLDIR_DOC}
	install -d $(DESTDIR)/etc/pcmcia/
	install -m 755 etc/pcmcia/comedi $(DESTDIR)/etc/pcmcia/
	install -m 644 etc/pcmcia/comedi.conf $(DESTDIR)/etc/pcmcia/
	install -m 644 etc/pcmcia/comedi.opts $(DESTDIR)/etc/pcmcia/
	install -m 755 etc/das1600.conf $(INSTALLDIR_DOC)/examples
	install -m 755 etc/dt282x.conf $(INSTALLDIR_DOC)/examples
ifeq ($(with_perl),yes)
	install -d $(INSTALLDIR_PERL)
	install -m 644 perl/blib/lib/Comedi.pm $(INSTALLDIR_PERL)/
	install -d $(INSTALLDIR_PERL)/Comedi
	install -m 644 perl/blib/lib/Comedi/Lib.pm $(INSTALLDIR_PERL)/Comedi
	install -m 644 perl/blib/lib/Comedi/Trigger.pm $(INSTALLDIR_PERL)/Comedi
	install -m 644 perl/blib/arch/auto/Comedi/Lib/Lib.so $(INSTALLDIR_PERL)/Comedi
	install -m 644 perl/blib/arch/auto/Comedi/Lib/Lib.bs $(INSTALLDIR_PERL)/Comedi
	#install -m 644 perl/blib/arch/auto/Comedi/Trigger.so $(INSTALLDIR_PERL)/Comedi
	#install -m 644 perl/blib/arch/auto/Comedi/Trigger.bs $(INSTALLDIR_PERL)/Comedi
	install -m 644 perl/blib/arch/auto/Comedi/Comedi.so $(INSTALLDIR_PERL)/Comedi
	install -m 644 perl/blib/arch/auto/Comedi/Comedi.bs $(INSTALLDIR_PERL)/Comedi
endif
ifeq ($(with_python),yes)
endif

lpr:	dummy
	find . -name '*.[chs]'|xargs enscript -2r -pit.ps

subdirs:	dummy
	set -e;for i in ${SUBDIRS};do ${MAKE} -C $$i ; done

clean:	dummy
	set -e;for i in $(SUBDIRS);do ${MAKE} clean -C $$i ; done
	# These will fail if nothing was built, but that's not a problem
	-$(MAKE) -C python distclean
	-$(MAKE) -C perl distclean

distclean:	clean

python: dummy
	$(MAKE) -C python -f Makefile.pre.in boot
	$(MAKE) -C python all

perl:	dummy
	(cd perl;perl Makefile.PL)
	$(MAKE) -C perl all
	
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

