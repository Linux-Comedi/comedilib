
EXTRA_DIST = examples

BUILT_SOURCES = comedi_ruby_wrap.c

comedi_ruby_wrap.c: ../comedi.i
	$(SWIG) -ruby -o ./comedi_ruby_wrap.c ../comedi.i

