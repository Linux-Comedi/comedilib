
if HAVE_DOCBOOK2MAN
all_man = $(srcdir)/dummy
install_man = install_man
uninstall_man = uninstall_man
else
all_man =
install_man =
uninstall_man =
endif

all-local: $(all_man)

dummy: $(srcdir)/../comedilib.sgml
	$(DOCBOOK2MAN) $(srcdir)/../comedilib.sgml
	touch $(srcdir)/dummy

install-data-local: $(install_man)

install_man:
	$(INSTALL) -d $(mandir)/man3
	for each in *.3 ; do $(INSTALL_DATA) $$each $(mandir)/man3 ; done

uninstall-local: $(uninstall_man)

uninstall_man:
	for each in *.3 ; do rm $(mandir)/man3/$$each ; done

clean-local:
	rm -f *.3
	rm -f manpage.links manpage.refs
	rm -f dummy

