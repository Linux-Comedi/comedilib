
SGML = drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml comedilib.sgml

EXTRA_DIST = $(SGML) funcref mkref drivers.txt mkdr FAQ \
	comedilib.pdf acq-seq.gif html man

BUILT_SOURCES = funcref.sgml drivers.sgml

if HAVE_DOCBOOK2PDF
pkgdata_DATA = comedilib.pdf
else
pkgdata_DATA =
endif

if HAVE_DOCBOOK2HTML
all_html = html
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

html: $(SGML)
	{ $(DOCBOOK2HTML) -o html $(srcdir)/comedilib.sgml && touch html; } || { $(RM) -r html; exit 1; }

install_html:
	$(INSTALL) -d $(pkgdatadir)/html
	for each in $(srcdir)/html/*.html $(srcdir)/*.gif ; do \
		$(INSTALL_DATA) $$each $(pkgdatadir)/html ; done

uninstall_html:
	for each in $(srcdir)/html/*.html $(srcdir)/*.gif ; do \
		$(RM) $(pkgdatadir)/html/`basename $$each` ; done

man: $(SGML)
	{ $(DOCBOOK2MAN) -o man $(srcdir)/comedilib.sgml && touch man; } || { $(RM) -r man; exit 1; }

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

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done


