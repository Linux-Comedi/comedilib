
if HAVE_PYTHON
pyexec_LTLIBRARIES = _comedi.la
else
pyexec_LTLIBRARIES = 
endif

_comedi_la_SOURCES = comedi_python_wrap.c
_comedi_la_CFLAGS = $(COMEDILIB_CFLAGS) $(PYTHON_INCLUDES) $(PYTHON_QUIET)
_comedi_la_LDFLAGS = -module -avoid-version $(COMEDILIB_LIBS)

pyexec_SCRIPTS = comedi.py

BUILT_SOURCES = comedi_python_wrap.c

EXTRA_DIST = README.txt comedi.py setup.py test_comedi.py

comedi_python_wrap.c: ../comedi.i
	$(SWIG) -python -o ./comedi_python_wrap.c ../comedi.i

