
EXTRA_DIST = demo ext lib

BUILT_SOURCES = ext/comedi_ruby_wrap.c

if HAVE_SWIG
ext/comedi_ruby_wrap.c: ../comedi.i
	$(SWIG) -ruby -o ./ext/comedi_ruby_wrap.c ../comedi.i
else
ext/comedi_ruby_wrap.c:
	echo WARNING: creating bogus comedi_ruby_wrap.c because you don\'t have swig.
	touch ext/comedi_ruby_wrap.c
endif

if HAVE_RUBY
all-local: ext/comedi.so
install-data-local:
	$(RUBY) setup.rb install
else
all-local:
install-data-local:
endif

ext/Makefile:
	$(RUBY) setup.rb config $(RUBY_CONFIG_OPTIONS)

ext/comedi.so ext/comedi_ruby_wrap.o: ext/Makefile ext/comedi_ruby_wrap.c
	$(RUBY) setup.rb setup

clean-local:
	$(RM) ext/comedi.so ext/Makefile

