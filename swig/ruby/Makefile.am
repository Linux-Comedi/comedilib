## explicitly list ext/comedi_ruby_wrap.c in EXTRA_DIST so that it
## gets built and put into tarball during 'make dist'
EXTRA_DIST = demo lib ext ext/comedi_ruby_wrap.c setup.rb

BUILT_SOURCES = ext/comedi_ruby_wrap.c

if HAVE_SWIG
$(srcdir)/ext/comedi_ruby_wrap.c: $(srcdir)/../comedi.i
	$(SWIG) -ruby -o $(srcdir)/ext/comedi_ruby_wrap.c $(srcdir)/../comedi.i
else
$(srcdir)/ext/comedi_ruby_wrap.c:
	echo WARNING: creating bogus comedi_ruby_wrap.c because you don\'t have swig.
	touch $(srcdir)/ext/comedi_ruby_wrap.c
endif

if HAVE_RUBY
all-local: ext/comedi.so
install-data-local:
	$(RUBY) $(srcdir)/setup.rb install
else
all-local:
install-data-local:
endif

ext/Makefile:
	$(RUBY) $(srcdir)/setup.rb config $(RUBY_CONFIG_OPTIONS)

ext/comedi.so ext/comedi_ruby_wrap.o: ext/Makefile $(srcdir)/ext/comedi_ruby_wrap.c
	$(RUBY) $(srcdir)/setup.rb setup

clean-local:
	$(RM) ext/comedi.so ext/Makefile ext/*.o

