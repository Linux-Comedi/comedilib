

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

# We follow the filesystem standard.  If you don't like it, tough.
INSTALLDIR=$(DESTDIR)$(PREFIX)/
INSTALLDIR_LIB=$(DESTDIR)$(PREFIX)/lib/
INSTALLDIR_DOC=$(DESTDIR)$(PREFIX)/share/doc/libcomedi/
INSTALLDIR_MAN=$(DESTDIR)$(PREFIX)/share/man/
INSTALLDIR_PERL=$(DESTDIR)$(PREFIX)/lib/perl5/
INSTALLDIR_BIN=$(DESTDIR)$(PREFIX)/bin/
INSTALLDIR_SBIN=$(DESTDIR)$(PREFIX)/sbin/

comedilib:	subdirs

config:	dummy

install:	install_dev install_runtime install_doc install_man

install_dev:	dummy
	install -d $(INSTALLDIR)/include
	install -m 644 include/comedilib.h $(INSTALLDIR)/include
	install -m 644 include/comedi.h $(INSTALLDIR)/include
	ln -sf libcomedi.so.$(version) $(INSTALLDIR_LIB)/libcomedi.so
	install -m 644 lib/libcomedi.a $(INSTALLDIR_LIB)

install_runtime:
	install lib/libcomedi.so.$(version) $(INSTALLDIR_LIB)
	ln -sf libcomedi.so.$(version) $(INSTALLDIR_LIB)/libcomedi.so.$(MAJOR)
	install -s -m 755 comedi_config/comedi_config $(INSTALLDIR_SBIN)
	install -s -m 755 comedi_calibrate/comedi_calibrate $(INSTALLDIR_BIN)

install_man:
	install -d $(INSTALLDIR_MAN)/man3/
	install -d $(INSTALLDIR_MAN)/man7/
	install -d $(INSTALLDIR_MAN)/man8/
	-install doc/man/*.3 $(INSTALLDIR_MAN)/man3/
	install man/*.7 $(INSTALLDIR_MAN)/man7/
	install man/*.8 $(INSTALLDIR_MAN)/man8/

install_doc:
	install -d $(INSTALLDIR_DOC)
	install README doc/FAQ doc/drivers.txt $(INSTALLDIR_DOC)
	install -d $(INSTALLDIR_DOC)/html/
	-install doc/html/*.html $(INSTALLDIR_DOC)/html/
	install -d $(INSTALLDIR_DOC)/etc/
	install -m 755 etc/das1600.conf $(INSTALLDIR_DOC)/etc/
	install -m 755 etc/dt282x.conf $(INSTALLDIR_DOC)/etc/
	install -d $(INSTALLDIR_DOC)/examples/
	install -m 644 demo/README demo/*.c $(INSTALLDIR_DOC)/examples/
	install -d $(DESTDIR)/usr/share/locale/de/LC_MESSAGES/
	install -m 644 doc/locale/de/LC_MESSAGES/comedilib.mo $(DESTDIR)/usr/share/locale/de/LC_MESSAGES/

install_distro: install install_perl install_python
	install -d $(DESTDIR)/etc/pcmcia/
	install -m 755 etc/pcmcia/comedi $(DESTDIR)/etc/pcmcia/
	install -m 644 etc/pcmcia/comedi.conf $(DESTDIR)/etc/pcmcia/
	install -m 644 etc/pcmcia/comedi.opts $(DESTDIR)/etc/pcmcia/

install_perl:
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

install_python:
ifeq ($(with_python),yes)
endif

lpr:	dummy
	find . -name '*.[chs]'|xargs enscript -2r -pit.ps

subdirs:	dummy
	set -e;for i in $(SUBDIRS);do $(MAKE) -C $$i ; done

clean:	dummy
	set -e;for i in $(SUBDIRS);do $(MAKE) clean -C $$i ; done
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

