
if HAVE_DOCBOOK2MAN
all_man = all_man
install_man = install_man
uninstall_man = uninstall_man
else
all_man = 
install_man = 
uninstall_man = 
endif

all-local: $(all_man)

all_man: $(srcdir)/../comedilib.sgml $(SGML)
	$(DOCBOOK2MAN) $(srcdir)/../comedilib.sgml

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

