EXTRA_DIST = lib/comedi.rb ext/extconf.rb ext/MANIFEST setup.rb

ext/comedi_ruby_wrap.c: $(srcdir)/../comedi.i
	$(SWIG) -ruby -o ext/comedi_ruby_wrap.c -I"$(top_srcdir)/include" "$(srcdir)/../comedi.i"

if HAVE_RUBY
all-local: ext/comedi.so
install-data-local:
	if test -n "$(DESTDIR)"; then \
		$(RUBY) setup.rb install --prefix="$(DESTDIR)" ; \
	else \
		$(RUBY) setup.rb install ; \
	fi
else
all-local:
install-data-local:
endif

ext/Makefile: ext/comedi_ruby_wrap.c
	$(RUBY) setup.rb config $(RUBY_CONFIG_OPTIONS) -- --with-comedilib-include="../$(top_srcdir)/include" --with-comedilib-lib="../$(top_builddir)/lib/.libs"

ext/comedi.so: ext/Makefile
	+$(RUBY) setup.rb setup

if HAVE_RUBY
clean-local:
	+$(RUBY) setup.rb clean
	$(RM) ext/comedi_ruby_wrap.c ext/Makefile
else
clean-local:
endif
