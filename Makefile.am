
SUBDIRS = lib comedi_calibrate comedi_config man testing demo doc python \
	include

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = comedilib.pc

EXTRA_DIST = include debian m4 comedilib.spec

dist-hook:
	rm -rf `find $(distdir) -name CVS`

