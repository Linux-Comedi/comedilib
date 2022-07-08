EXTRA_DIST = demo/cmd demo/common.rb demo/inp demo/outp lib/comedi.rb \
	     ext/extconf.rb ext/MANIFEST setup.rb

$(srcdir)/ext/comedi_ruby_wrap.c: $(srcdir)/../comedi.i
	$(SWIG) -ruby -o $(srcdir)/ext/comedi_ruby_wrap.c -I$(top_srcdir)/include $(srcdir)/../comedi.i

if HAVE_RUBY
all-local: ext/comedi.so
install-data-local:
	$(RUBY) $(srcdir)/setup.rb install
else
all-local:
install-data-local:
endif

ext/Makefile .config: $(srcdir)/ext/comedi_ruby_wrap.c
	$(RUBY) $(srcdir)/setup.rb config $(RUBY_CONFIG_OPTIONS) -- --with-comedilib-dir=$(abs_top_srcdir)

ext/comedi.so ext/comedi_ruby_wrap.o: ext/Makefile .config
	$(RUBY) $(srcdir)/setup.rb setup

if HAVE_RUBY
clean-local:
	$(RUBY) $(srcdir)/setup.rb clean
	$(RM) $(srcdir)/ext/comedi_ruby_wrap.c
	$(RM) ext/Makefile .config
distclean-local:
	$(RUBY) $(srcdir)/setup.rb distclean
else
clean-local:
distclean-local:
endif
