
SUBDIRS = html man

SGML= drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml 

BUILT_SOURCES = funcref.sgml drivers.sgml

EXTRA_DIST=$(SGML) comedilib.sgml funcref mkref drivers.txt mkdr

if HAVE_DOCBOOK2PDF
pkgdata_DATA = comedilib.pdf
else
pkgdata_DATA = 
endif

CLEANFILES = comedilib.pdf

comedilib.pdf: $(srcdir)/comedilib.sgml $(SGML)
	$(DOCBOOK2PDF) $(srcdir)/comedilib.sgml

funcref.sgml: funcref mkref
	$(srcdir)/mkref funcref >funcref.sgml

drivers.sgml: drivers.txt mkdr
	$(srcdir)/mkdr drivers.txt >drivers.sgml

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done

