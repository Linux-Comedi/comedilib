
DOC=comedilib
SGML= advanced.sgml drivers.sgml funcref.sgml glossary.sgml \
	install.sgml intro.sgml other.sgml reference.sgml tutorial.sgml

EXTRA_DIST=$(SGML) funcref mkref drivers.txt mkdr

all-local: html $(DOC).pdf man

locales = de

$(DOC).pdf: $(DOC).sgml
	-docbook2pdf $(DOC).sgml
	
html: $(DOC).sgml
	-mkdir -p html
	-docbook2html -o html $(DOC).sgml

man: $(DOC).sgml
	-mkdir -p man
	-docbook2html -o man $(DOC).sgml

funcref.sgml: funcref mkref
	./mkref funcref >funcref.sgml

drivers.sgml: drivers.txt mkdr
	./mkdr drivers.txt >drivers.sgml

$(DOC).sgml: $(SGML)

messages: .phony
	xgettext -k_ -k_s $(shell find .. -name '*.c')
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done
