
pyexec_LTLIBRARIES = _comedi.la

_comedi_la_SOURCES = comedi_wrap.c
_comedi_la_CFLAGS = $(COMEDILIB_CFLAGS) $(PYTHON_INCLUDES) \
	-fno-strict-aliasing -Wno-unused
_comedi_la_LDFLAGS = -module -avoid-version $(COMEDILIB_LIBS)

pyexec_SCRIPTS = comedi.py

EXTRA_DIST = comedi.i README.txt comedi.py setup.py test_comedi.py

install-exec-local:


#$(INSTALL) build/lib.linux-i686-2.2/_comedi.so $(pythondir)/
#$(INSTALL) comedi.py $(pythondir)/

