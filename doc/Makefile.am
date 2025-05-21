
XML = calibration_funcref.xml command_funcref.xml dio_funcref.xml \
	deprecated_funcref.xml error_funcref.xml extensions_funcref.xml \
	drivers.xml drivers-linux.xml funcref.xml $(srcdir)/glossary.xml \
	$(srcdir)/install.xml $(srcdir)/intro.xml $(srcdir)/other.xml \
	$(srcdir)/reference.xml $(srcdir)/tutorial.xml \
	$(srcdir)/driverwriting.xml $(srcdir)/comedilib.xml \
	$(srcdir)/bindings.xml $(srcdir)/comedilib.ent

TUTORIALS = $(srcdir)/../demo/tut1.c $(srcdir)/../demo/tut2.c \
	    $(srcdir)/../demo/tut3.c

EXTRA_DIST = $(XML) calibration_funcref.txt command_funcref.txt dio_funcref.txt \
	deprecated_funcref.txt error_funcref.txt extensions_funcref.txt \
	funcref.txt mkref drivers.txt drivers-linux.txt mkdr FAQ \
	acq-seq.gif $(dist_html) $(dist_pdf) $(dist_man) comedilib_html_config.xsl \
	comedilib_dblatex_config.xsl \
	comedilib.css

BUILT_SOURCES = calibration_funcref.xml command_funcref.xml dio_funcref.xml \
	deprecated_funcref.xml error_funcref.xml extensions_funcref.xml \
	funcref.xml drivers.xml drivers-linux.xml bindings.xml

out_html = doc_html
out_man = man
out_pdf = pdf

if HAVE_XMLTO
all_html = doc_html
dist_html = $(out_html)
all_man = man
dist_man = $(out_man)
if BUILD_PDF
all_pdf = doc_pdf
dist_pdf = $(out_pdf)
else
all_pdf =
dist_pdf =
endif
else
all_html =
dist_html =
all_man =
dist_man =
all_pdf =
dist_pdf =
endif

if BUILD_PDF_USING_DBLATEX
XMLTO_PDF_OPTS = --with-dblatex -p "-p $(abs_srcdir)/comedilib_dblatex_config.xsl -x --path -x $(abs_srcdir):$(abs_builddir)"
else
if BUILD_PDF_USING_FOP
XMLTO_PDF_OPTS = --with-fop
else
XMLTO_PDF_OPTS =
endif
endif

all-local: $(all_html) $(all_pdf) $(all_man)

install-data-local: install_html install_pdf install_man

uninstall-local: uninstall_html uninstall_pdf uninstall_man

#named this doc_html to avoid phony html target that is automatically generated
#(at least by automake1.8)
doc_html: $(XML) $(TUTORIALS) $(srcdir)/comedilib_html_config.xsl
	{ $(XMLTO) -o $(out_html) --searchpath "$(abs_srcdir):$(abs_builddir)" -m comedilib_html_config.xsl --skip-validation html "$(srcdir)/comedilib.xml" && \
		$(RM) $(out_html)/comedilib.proc && \
		$(LN_S) -f ../comedilib.css $(out_html)/ && \
		$(LN_S) -f ../acq-seq.gif $(out_html)/ && \
		touch doc_html;} || \
		{ $(RM) -r $(out_html); exit 1; }

install_html:
	@d="$(builddir)/$(out_html)" ; \
	if test ! -d "$$d" ; \
	then \
	    d="$(srcdir)/$(out_html)" ; \
	fi ; \
	if test -d "$$d" ; \
	then \
		echo "Installing html from \"$$d\"..." ; \
		echo $(mkdir_p) -m 755 "$(DESTDIR)$(htmldir)/html" ; \
		$(mkdir_p) -m 755 "$(DESTDIR)$(htmldir)/html" ; \
		echo chmod u+w "$(DESTDIR)$(htmldir)/html" ; \
		chmod u+w "$(DESTDIR)$(htmldir)/html" ; \
		for each in "$$d"/*.html "$$d"/*.gif "$$d"/*.css ; \
		do \
			echo $(INSTALL_DATA) "$$each" "$(DESTDIR)$(htmldir)/html/" ; \
			$(INSTALL_DATA) "$$each" "$(DESTDIR)$(htmldir)/html/" ; \
		done ; \
	fi


uninstall_html:
	@d="$(builddir)/$(out_html)" ; \
	if test ! -d "$$d" ; \
	then \
	    d="$(srcdir)/$(out_html)" ; \
	fi ; \
	if test -d "$$d/$(out_html)" ; \
	then \
		echo "Uninstalling html from \"$$d\"..." ; \
		for each in "$$d"/*.html "$$d"/*.gif "$$d/"*.css ; \
		do \
			echo $(RM) "$(DESTDIR)$(htmldir)/html/`basename "$$each"`" ; \
			$(RM) "$(DESTDIR)$(htmldir)/html/`basename "$$each"`" ; \
		done ; \
	fi

man: $(XML)
	{ $(XMLTO) -o $(out_man) --searchpath "$(abs_srcdir):$(abs_builddir)" --skip-validation man "$(srcdir)/comedilib.xml" && touch man; } || { $(RM) -r $(out_man); exit 1; }

install_man:
	@d="$(builddir)/$(out_man)" ; \
	if test ! -d "$$d" ; \
	then \
	    d="$(srcdir)/$(out_man)" ; \
	fi ; \
	if test -d "$$d" ; \
	then \
		echo "Installing man pages from \"$$d\"..." ; \
		echo $(mkdir_p) -m 755 "$(DESTDIR)$(mandir)/man3" ; \
		$(mkdir_p) -m 755 "$(DESTDIR)$(mandir)/man3" ; \
		echo chmod u+w "$(DESTDIR)$(mandir)/man3" ; \
		chmod u+w "$(DESTDIR)$(mandir)/man3" ; \
		for each in `find "$$d/" -name '*.3'` ; \
		do \
			echo $(INSTALL_DATA) "$$each" "$(DESTDIR)$(mandir)/man3/" ; \
			$(INSTALL_DATA) "$$each" "$(DESTDIR)$(mandir)/man3/" ; \
		done ; \
	fi

uninstall_man:
	@d="$(builddir)/$(out_man)" ; \
	if test ! -d "$$d" ; \
	then \
	    d="$(srcdir)/$(out_man)" ; \
	fi ; \
	if test -d "$$d" ; \
	then \
		echo "Uninstalling man pages from \"$$d\"..." ; \
		for each in `find "$$d/" -name '*.3'` ; \
		do \
			echo $(RM) "$(DESTDIR)$(mandir)/man3/`basename "$$each"`" ; \
			$(RM) "$(DESTDIR)$(mandir)/man3/`basename "$$each"`" ; \
		done ; \
	fi


doc_pdf: $(out_pdf)/comedilib.pdf

install_pdf:
	@d="$(builddir)/$(out_pdf)" ; \
	if test ! -d "$$d" ; \
	then \
	    d="$(srcdir)/$(out_pdf)" ; \
	fi ; \
	if test -d "$$d" ; \
	then \
		echo "Installing PDFs from \"$$d\"..." ; \
		echo $(mkdir_p) -m 755 "$(DESTDIR)$(pdfdir)/pdf" ; \
		$(mkdir_p) -m 755 "$(DESTDIR)$(pdfdir)/pdf" ; \
		echo chmod u+w "$(DESTDIR)$(pdfdir)/pdf" ; \
		chmod u+w "$(DESTDIR)$(pdfdir)/pdf" ; \
		for each in "$$d"/*.pdf; \
		do \
			echo $(INSTALL_DATA) "$$each" "$(DESTDIR)$(pdfdir)/pdf" ; \
			$(INSTALL_DATA) "$$each" "$(DESTDIR)$(pdfdir)/pdf" ; \
		done ; \
	fi

uninstall_pdf:
	d="$(builddir)/$(out_pdf)" ; \
	if test ! -d "$$d" ; \
	then \
	    d="$(srcdir)/$(out_pdf)" ; \
	fi ; \
	if test -d "$$d" ; \
	then \
		echo "Uninstalling PDFs from \"$$d\"..." ; \
		for each in "$$d"/*.pdf ; \
		do \
			echo $(RM) "$(DESTDIR)$(pdfdir)/pdf/`basename "$$each"`" ; \
			$(RM) "$(DESTDIR)$(pdfdir)/pdf/`basename "$$each"`" ; \
		done ; \
	fi

$(out_pdf)/comedilib.pdf: $(XML) $(TUTORIALS) $(srcdir)/comedilib_dblatex_config.xsl
	$(XMLTO) -o $(out_pdf) --searchpath "$(abs_srcdir):$(abs_builddir)" --skip-validation $(XMLTO_PDF_OPTS) pdf $(srcdir)/comedilib.xml

funcref.xml: $(srcdir)/funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/funcref.txt > funcref.xml

calibration_funcref.xml: $(srcdir)/calibration_funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/calibration_funcref.txt > calibration_funcref.xml

command_funcref.xml: $(srcdir)/command_funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/command_funcref.txt > command_funcref.xml

dio_funcref.xml: $(srcdir)/dio_funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/dio_funcref.txt > dio_funcref.xml

deprecated_funcref.xml: $(srcdir)/deprecated_funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/deprecated_funcref.txt > deprecated_funcref.xml

error_funcref.xml: $(srcdir)/error_funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/error_funcref.txt > error_funcref.xml

extensions_funcref.xml: $(srcdir)/extensions_funcref.txt $(srcdir)/mkref
	$(srcdir)/mkref $(srcdir)/extensions_funcref.txt > extensions_funcref.xml

# Note: if drivers.txt is updated from the out-of-tree comedi.org driver
# sources, please update the 'comedi_date' entity in 'comedilib.ent'.
drivers.xml: $(srcdir)/drivers.txt $(srcdir)/mkdr
	$(srcdir)/mkdr "ll-drivers-comedi" "Drivers from comedi.org" $(srcdir)/drivers.txt > drivers.xml

# Note: if drivers-linux.txt is updated from Linux kernel sources, please
# update the 'linux_version' entity in 'comedilib.ent'.
drivers-linux.xml: $(srcdir)/drivers-linux.txt $(srcdir)/mkdr
	$(srcdir)/mkdr "ll-drivers-linux" "Drivers from Linux kernel sources" $(srcdir)/drivers-linux.txt > drivers-linux.xml

maintainer-clean-local:
	$(RM) -r $(out_html) $(out_man) $(out_pdf)

locales = de

messages: .phony
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done
