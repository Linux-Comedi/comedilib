
DOC=comedilib
SGML= drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml \
	driverwriting.sgml advanced.sgml

EXTRA_DIST=$(SGML) comedilib.sgml funcref mkref drivers.txt mkdr acq-seq.gif

if HAVE_DOCBOOK
all-local: html $(DOC).pdf man
else
all-local:
endif

clean-local:
	rm -rf comedilib.pdf drivers.sgml funcref.sgml
	rm -rf html man

install-data-local:
	$(INSTALL) -d $(mandir)/man3
	$(INSTALL) man/*.3 $(mandir)/man3
	$(INSTALL) -d $(pkgdatadir)/html
	$(INSTALL) html/* $(pkgdatadir)/html

locales = de

$(DOC).pdf: $(DOC).sgml $(SGML)
	-$(JW_PATH) -f docbook -b pdf $(DOC).sgml

html: $(DOC).sgml $(SGML)
	-mkdir -p html
	-$(JW_PATH) -f docbook -b html -o html $(DOC).sgml && touch html

man: $(DOC).sgml $(SGML)
	-mkdir -p man
	-$(JW_PATH) -f docbook -b man -o man $(DOC).sgml && touch man

funcref.sgml: funcref mkref
	./mkref funcref >funcref.sgml

drivers.sgml: drivers.txt mkdr
	./mkdr drivers.txt >drivers.sgml

messages: .phony
	xgettext -k_ -k_s $(shell find .. -name '*.c')
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done

