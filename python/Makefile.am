
if HAVE_PYTHON
pyexec_LTLIBRARIES = _comedi.la
else
pyexec_LTLIBRARIES = 
endif

_comedi_la_SOURCES = comedi_wrap.c
_comedi_la_CFLAGS = $(COMEDILIB_CFLAGS) $(PYTHON_INCLUDES) $(PYTHON_QUIET)
_comedi_la_LDFLAGS = -module -avoid-version $(COMEDILIB_LIBS)

pyexec_SCRIPTS = comedi.py

EXTRA_DIST = comedi.i README.txt comedi.py setup.py test_comedi.py

