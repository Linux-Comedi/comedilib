
if HAVE_PYTHON
pyexec_LTLIBRARIES = _comedi.la
else
pyexec_LTLIBRARIES =
endif

BUILT_SOURCES = comedi_python_wrap.c

_comedi_la_SOURCES = comedi_python_wrap.c
_comedi_la_CFLAGS = $(COMEDILIB_CFLAGS) $(PYTHON_INCLUDES) $(PYTHON_QUIET)
_comedi_la_LDFLAGS = -module -avoid-version $(COMEDILIB_LIBS)

pyexec_SCRIPTS = comedi.py

EXTRA_DIST = README.txt comedi.py setup.py test_comedi.py

if HAVE_SWIG
comedi_python_wrap.c: ../comedi.i
	$(SWIG) -python -o ./comedi_python_wrap.c ../comedi.i
else
comedi_python_wrap.c:
	echo WARNING: creating bogus comedi_python_wrap.c because you don\'t have swig.
	touch comedi_python_wrap.c
endif
