
SGML = drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml comedilib.sgml

EXTRA_DIST = $(SGML) funcref mkref drivers.txt mkdr FAQ $(all_html) $(all_man) \
	comedilib.pdf acq-seq.gif html_targets man_targets

BUILT_SOURCES = funcref.sgml drivers.sgml

if HAVE_DOCBOOK2PDF
pkgdata_DATA = comedilib.pdf
else
pkgdata_DATA =
endif

if HAVE_DOCBOOK2HTML
all_html = $(shell cat html_targets)
install_html = install_html
uninstall_html = uninstall_html
else
all_html =
install_html =
uninstall_html =
endif

if HAVE_DOCBOOK2MAN
all_man = $(shell cat man_targets)
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

html/%.html: $(SGML)
	{ $(DOCBOOK2HTML) -o html $(srcdir)/comedilib.sgml && touch html; } || { $(RM) -r html; exit 1; }
	$(MAKE) html_targets

install_html:
	$(INSTALL) -d $(pkgdatadir)/html
	for each in $(srcdir)/html/*.html $(srcdir)/*.gif ; do \
		$(INSTALL_DATA) $$each $(pkgdatadir)/html ; done

uninstall_html:
	for each in $(srcdir)/html/*.html $(srcdir)/*.gif ; do \
		$(RM) $(pkgdatadir)/html/`basename $$each` ; done

man/%: $(SGML)
	{ $(DOCBOOK2MAN) -o man $(srcdir)/comedilib.sgml && touch man; } || { $(RM) -r man; exit 1; }
	$(MAKE) man_targets

install_man:
	$(INSTALL) -d $(mandir)/man3
	for each in $(srcdir)/man/*.3 ; do $(INSTALL_DATA) $$each $(mandir)/man3 ; done

uninstall_man:
	for each in $(srcdir)/man/*.3 ; do $(RM) $(mandir)/man3/`basename $$each` ; done

comedilib.pdf: $(SGML)
	$(DOCBOOK2PDF) $(srcdir)/comedilib.sgml

funcref.sgml: funcref mkref
	$(srcdir)/mkref funcref >funcref.sgml

drivers.sgml: drivers.txt mkdr
	$(srcdir)/mkdr drivers.txt >drivers.sgml

maintainer-clean-local:
	$(RM) -r html man
	$(RM) comedilib.pdf

.PHONY: html_targets man_targets

html_targets:
	ls html/*.html > html_targets

man_targets:
	ls man/*.3 > man_targets

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done


