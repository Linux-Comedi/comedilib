
pyexec_LTLIBRARIES = pycomedi.la

pycomedi_la_SOURCES = comedi_wrap.c
pycomedi_la_CFLAGS = $(COMEDILIB_CFLAGS) -I/usr/include/python2.2
pycomedi_la_LDFLAGS = -module -avoid-version $(COMEDILIB_LIBS)

pyexec_SCRIPTS = comedi.py

EXTRA_DIST = comedi.i

install-exec-local:


#$(INSTALL) build/lib.linux-i686-2.2/_comedi.so $(pythondir)/
#$(INSTALL) comedi.py $(pythondir)/

