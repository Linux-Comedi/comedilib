
locales = de

all: drivers.sgml funcref.sgml
	-mkdir -p html
	-mkdir -p man
	-docbook2html -o html comedilib.sgml
	-docbook2man -o man comedilib.sgml

pdf: drivers.sgml funcref.sgml .phony
	-docbook2pdf -o pdf comedilib.sgml

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

distclean: clean
	rm -rf locale
	rm -rf html
	rm -rf man
	rm -f drivers.sgml funcref.sgml

clean:
	for i in messages $(locales);do \
	  rm -f $$i.po ; \
	done

.phony:

