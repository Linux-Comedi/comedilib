
all:
	sgml2html comedilib.sgml
	sgml2txt comedilib.sgml

messages: .phony
	xgettext -k_ -k_s $(shell find .. -name '*.c')
	for i in de;do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  msgfmt $$i.po -o $$i/LC_MESSAGES/comedilib.mo; \
	done

.phony:

