
SUBDIRS = lib comedi_calibrate comedi_config man testing demo doc

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = comedilib.pc

EXTRA_DIST = include debian m4

dist-hook:
	rm -rf `find $(distdir) -name CVS`

