
SUBDIRS = lib comedi_calibrate comedi_config man testing demo doc python \
	include etc

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = comedilib.pc

EXTRA_DIST = debian m4 comedilib.spec 

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	rm -rf `find $(distdir) -name CVS`

