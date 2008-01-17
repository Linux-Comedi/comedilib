
SGML = calibration_funcref.sgml command_funcref.sgml dio_funcref.sgml \
	deprecated_funcref.sgml error_funcref.sgml extensions_funcref.sgml \
	drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml comedilib.sgml

EXTRA_DIST = $(SGML) calibration_funcref.txt command_funcref.txt dio_funcref.txt \
	deprecated_funcref.txt error_funcref.txt extensions_funcref.txt \
	funcref mkref drivers.txt mkdr FAQ \
	acq-seq.gif doc_html man

BUILT_SOURCES = calibration_funcref.sgml command_funcref.sgml dio_funcref.sgml \
	deprecated_funcref.sgml error_funcref.sgml extensions_funcref.sgml \
	funcref.sgml drivers.sgml

if HAVE_DOCBOOK2PDF
dist_pdf_DATA = comedilib.pdf
else
dist_pdf_DATA =
endif

if HAVE_DOCBOOK2HTML
all_html = $(srcdir)/doc_html
install_html = install_html
uninstall_html = uninstall_html
else
all_html =
install_html =
uninstall_html =
endif

if HAVE_DOCBOOK2MAN
all_man = $(srcdir)/man
install_man = install_man
uninstall_man = uninstall_man
else
all_man =
install_man =
uninstall_man =
endif

all-local: $(all_html) $(all_man)

install-data-local: $(install_html) $(install_man)

uninstall-local: $(uninstall_html) $(uninstall_man)

#named this doc_html to avoid phony html target that is automatically generated
#(at least by automake1.8)
$(srcdir)/doc_html: $(SGML)
	{ $(DOCBOOK2HTML) -o $(srcdir)/doc_html $(srcdir)/comedilib.sgml && touch $(srcdir)/doc_html; } || { $(RM) -r $(srcdir)/doc_html; exit 1; }

install_html:
	$(mkdir_p) $(DESTDIR)$(htmldir)/html
	chmod u+w $(DESTDIR)$(htmldir)/html
	for each in $(srcdir)/doc_html/*.html $(srcdir)/*.gif ; do \
		$(INSTALL_DATA) $$each $(DESTDIR)$(htmldir)/html ; done

uninstall_html:
	for each in $(srcdir)/doc_html/*.html $(srcdir)/*.gif ; do \
		$(RM) $(DESTDIR)$(htmldir)/html/`basename $$each` ; done

$(srcdir)/man: $(SGML)
	{ $(DOCBOOK2MAN) -o $(srcdir)/man $(srcdir)/comedilib.sgml && touch $(srcdir)/man; } || { $(RM) -r $(srcdir)/man; exit 1; }

install_man:
	$(mkdir_p) -m 755 $(DESTDIR)$(mandir)/man3
	chmod u+w $(DESTDIR)$(mandir)/man3
	for each in `find $(srcdir)/man/ -name '*.3'`; do $(INSTALL_DATA) $$each $(DESTDIR)$(mandir)/man3 ; done

uninstall_man:
	for each in `find $(srcdir)/man/ -name '*.3'`; do $(RM) $(DESTDIR)$(mandir)/man3/`basename $$each` ; done

comedilib.pdf: $(SGML)
	$(DOCBOOK2PDF) -o $(srcdir) $(srcdir)/comedilib.sgml

funcref.sgml: funcref mkref
	$(srcdir)/mkref $(srcdir)/funcref >$(srcdir)/funcref.sgml

calibration_funcref.sgml: calibration_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/calibration_funcref.txt >$(srcdir)/calibration_funcref.sgml

command_funcref.sgml: command_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/command_funcref.txt >$(srcdir)/command_funcref.sgml

dio_funcref.sgml: dio_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/dio_funcref.txt >$(srcdir)/dio_funcref.sgml

deprecated_funcref.sgml: deprecated_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/deprecated_funcref.txt >$(srcdir)/deprecated_funcref.sgml

error_funcref.sgml: error_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/error_funcref.txt >$(srcdir)/error_funcref.sgml

extensions_funcref.sgml: extensions_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/extensions_funcref.txt >$(srcdir)/extensions_funcref.sgml

drivers.sgml: drivers.txt mkdr
	$(srcdir)/mkdr $(srcdir)/drivers.txt >$(srcdir)/drivers.sgml

maintainer-clean-local:
	$(RM) -r $(srcdir)/doc_html $(srcdir)/man
	$(RM) $(srcdir)/comedilib.pdf

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done
