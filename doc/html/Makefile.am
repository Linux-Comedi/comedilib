
EXTRA_DIST = acq-seq.gif

if HAVE_DOCBOOK2HTML
all_html = all_html
install_html = install_html
uninstall_html = uninstall_html
else
all_html = 
install_html = 
uninstall_html = 
endif

all-local: $(all_html)

all_html: $(srcdir)/../comedilib.sgml $(SGML)
	$(DOCBOOK2HTML) $(srcdir)/../comedilib.sgml

install-data-local: $(install_html)

install_html:
	$(INSTALL) -d $(pkgdatadir)/html
	for each in *.html $(srcdir)/*.gif ; do $(INSTALL_DATA) $$each $(pkgdatadir)/html ; done

uninstall-local: $(uninstall_html)

uninstall_html:
	for each in *.html $(srcdir)/*.gif ; do rm $(pkgdatadir)/html/`basename $$each` ; done

clean-local:
	rm -rf *.html

