
EXTRA_DIST = examples

BUILT_SOURCES = comedi_ruby_wrap.c

if HAVE_SWIG
comedi_ruby_wrap.c: ../comedi.i
	$(SWIG) -ruby -o ./comedi_ruby_wrap.c ../comedi.i
else
comedi_ruby_wrap.c:
	echo WARNING: creating bogus comedi_ruby_wrap.c because you don\'t have swig.
	touch comedi_ruby_wrap.c
endif
