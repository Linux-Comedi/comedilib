
SGML = drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml comedilib.sgml

EXTRA_DIST = $(SGML) funcref mkref drivers.txt mkdr FAQ \
	comedilib.pdf acq-seq.gif doc_html man

BUILT_SOURCES = funcref.sgml drivers.sgml

if HAVE_DOCBOOK2PDF
pdf_DATA = comedilib.pdf
else
pdf_DATA =
endif

if HAVE_DOCBOOK2HTML
all_html = doc_html
install_html = install_html
uninstall_html = uninstall_html
else
all_html =
install_html =
uninstall_html =
endif

if HAVE_DOCBOOK2MAN
all_man = man
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
doc_html: $(SGML)
	{ $(DOCBOOK2HTML) -o doc_html $(srcdir)/comedilib.sgml && touch doc_html; } || { $(RM) -r doc_html; exit 1; }

install_html:
	$(mkdir_p) $(DESTDIR)$(htmldir)/html
	chmod u+w $(DESTDIR)$(htmldir)/html
	for each in $(srcdir)/doc_html/*.html $(srcdir)/*.gif ; do \
		$(INSTALL_DATA) $$each $(DESTDIR)$(htmldir)/html ; done

uninstall_html:
	for each in $(srcdir)/doc_html/*.html $(srcdir)/*.gif ; do \
		$(RM) $(DESTDIR)$(htmldir)/html/`basename $$each` ; done

man: $(SGML)
	{ $(DOCBOOK2MAN) -o man $(srcdir)/comedilib.sgml && touch man; } || { $(RM) -r man; exit 1; }

install_man:
	$(mkdir_p) -m 755 $(DESTDIR)$(mandir)/man3
	chmod u+w $(DESTDIR)$(mandir)/man3
	for each in $(srcdir)/man/*.3 ; do $(INSTALL_DATA) $$each $(DESTDIR)$(mandir)/man3 ; done

uninstall_man:
	for each in $(srcdir)/man/*.3 ; do $(RM) $(DESTDIR)$(mandir)/man3/`basename $$each` ; done

comedilib.pdf: $(SGML)
	$(DOCBOOK2PDF) $(srcdir)/comedilib.sgml

funcref.sgml: funcref mkref
	$(srcdir)/mkref $(srcdir)/funcref >$(srcdir)/funcref.sgml

drivers.sgml: drivers.txt mkdr
	$(srcdir)/mkdr $(srcdir)/drivers.txt >$(srcdir)/drivers.sgml

maintainer-clean-local:
	$(RM) -r doc_html man
	$(RM) comedilib.pdf

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done


