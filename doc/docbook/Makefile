
all: drivers.sgml funcref.sgml
	-mkdir html
	-mkdir man
	-docbook2html -o html comedilib.sgml
	-docbook2man -o man comedilib.sgml

funcref.sgml: funcref mkref
	./mkref funcref >funcref.sgml

drivers.sgml: drivers.txt mkdr
	./mkdr drivers.txt >drivers.sgml

clean:
	-rm -rf html
	-rm -rf man
	-rm -f drivers.sgml funcref.sgml

