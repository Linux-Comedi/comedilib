
EXTRA_DIST = acq-seq.gif

if HAVE_DOCBOOK2HTML
all_html = $(srcdir)/dummy
install_html = install_html
uninstall_html = uninstall_html
else
all_html =
install_html =
uninstall_html =
endif

all-local: $(all_html)

dummy: $(srcdir)/../comedilib.sgml
	$(DOCBOOK2HTML) $(srcdir)/../comedilib.sgml
	touch $(srcdir)/dummy

install-data-local: $(install_html)

install_html:
	$(INSTALL) -d $(pkgdatadir)/html
	for each in *.html $(srcdir)/*.gif ; do $(INSTALL_DATA) $$each $(pkgdatadir)/html ; done

uninstall-local: $(uninstall_html)

uninstall_html:
	for each in *.html $(srcdir)/*.gif ; do rm $(pkgdatadir)/html/`basename $$each` ; done

clean-local:
	rm -rf *.html
	rm -f dummy
