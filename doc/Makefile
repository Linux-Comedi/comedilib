
locales = de

all:
	sgml2html comedilib.sgml
	sgml2txt comedilib.sgml

messages: .phony
	xgettext -k_ -k_s $(shell find .. -name '*.c')
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done

distclean: clean
	rm -f *.html *.txt
	rm -rf locale

clean:
	for i in messages $(locales);do \
	  rm -f $$i.po ; \
	done

.phony:

