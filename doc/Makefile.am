
SUBDIRS = html man

SGML= drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml 

BUILT_SOURCES = funcref.sgml drivers.sgml

EXTRA_DIST=$(SGML) comedilib.sgml funcref mkref drivers.txt mkdr

if HAVE_DOCBOOK2PDF
all_pdf = comedilib.pdf
install_pdf = install_pdf
else
all_pdf = 
install_pdf = 
endif

all-local: $(all_pdf)

install-data-local: $(install_pdf)

install_pdf:
	$(INSTALL) -d $(pkgdatadir)
	$(INSTALL_DATA) comedilib.pdf $(pkgdatadir)

uninstall-local:
	rm -f $(pkgdatadir)/comedilib.pdf

clean-local:
	rm -f comedilib.pdf drivers.sgml funcref.sgml

comedilib.pdf: $(srcdir)/comedilib.sgml $(SGML)
	$(DOCBOOK2PDF) -f docbook -b pdf $(srcdir)/comedilib.sgml

funcref.sgml: funcref mkref
	./mkref funcref >funcref.sgml

drivers.sgml: drivers.txt mkdr
	./mkdr drivers.txt >drivers.sgml

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done

