
SUBDIRS = lib comedi_calibrate comedi_config man testing demo doc python \
	include

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = comedilib.pc

EXTRA_DIST = debian m4 etc comedilib.spec 

dist-hook:
	rm -rf `find $(distdir) -name CVS`

