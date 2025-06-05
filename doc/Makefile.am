out_html = html
out_man = man
out_pdf = pdf

BUILT_XML = calibration_funcref.xml command_funcref.xml deprecated_funcref.xml \
	dio_funcref.xml drivers.xml drivers-linux.xml error_funcref.xml \
	extensions_funcref.xml funcref.xml

STATIC_XML_IN = bindings.xml comedilib.ent comedilib.xml driverwriting.xml \
	glossary.xml install.xml intro.xml other.xml reference.xml tutorial.xml

XML_IN = $(STATIC_XML_IN) $(BUILT_XML)

TUTORIAL_IN = ../demo/tut1.c ../demo/tut2.c ../demo/tut3.c

MAN_OUT = $(out_man)/comedi_apply_calibration.3 \
	$(out_man)/comedi_apply_parsed_calibration.3 \
	$(out_man)/comedi_arm.3 $(out_man)/comedi_arm_channel.3 \
	$(out_man)/comedi_cancel.3 $(out_man)/comedi_cleanup_calibration.3 \
	$(out_man)/comedi_close.3 $(out_man)/comedi_command.3 \
	$(out_man)/comedi_command_test.3 $(out_man)/comedi_connect_route.3 \
	$(out_man)/comedi_data_read.3 $(out_man)/comedi_data_read_delayed.3 \
	$(out_man)/comedi_data_read_hint.3 $(out_man)/comedi_data_read_n.3 \
	$(out_man)/comedi_data_write.3 \
	$(out_man)/comedi_digital_trigger_disable.3 \
	$(out_man)/comedi_digital_trigger_enable_edges.3 \
	$(out_man)/comedi_digital_trigger_enable_levels.3 \
	$(out_man)/comedi_dio_bitfield.3 $(out_man)/comedi_dio_bitfield2.3 \
	$(out_man)/comedi_dio_config.3 $(out_man)/comedi_dio_get_config.3 \
	$(out_man)/comedi_dio_read.3 $(out_man)/comedi_dio_write.3 \
	$(out_man)/comedi_disarm.3 $(out_man)/comedi_disarm_channel.3 \
	$(out_man)/comedi_disconnect_route.3 $(out_man)/comedi_do_insn.3 \
	$(out_man)/comedi_do_insnlist.3 $(out_man)/comedi_errno.3 \
	$(out_man)/comedi_fileno.3 $(out_man)/comedi_find_range.3 \
	$(out_man)/comedi_find_subdevice_by_type.3 \
	$(out_man)/comedi_from_phys.3 $(out_man)/comedi_from_physical.3 \
	$(out_man)/comedi_get_board_name.3 \
	$(out_man)/comedi_get_buffer_contents.3 \
	$(out_man)/comedi_get_buffer_offset.3 \
	$(out_man)/comedi_get_buffer_read_count.3 \
	$(out_man)/comedi_get_buffer_read_offset.3 \
	$(out_man)/comedi_get_buffer_size.3 \
	$(out_man)/comedi_get_buffer_write_count.3 \
	$(out_man)/comedi_get_buffer_write_offset.3 \
	$(out_man)/comedi_get_clock_source.3 \
	$(out_man)/comedi_get_cmd_generic_timed.3 \
	$(out_man)/comedi_get_cmd_src_mask.3 \
	$(out_man)/comedi_get_cmd_timing_constraints.3 \
	$(out_man)/comedi_get_default_calibration_path.3 \
	$(out_man)/comedi_get_driver_name.3 \
	$(out_man)/comedi_get_gate_source.3 \
	$(out_man)/comedi_get_hardcal_converter.3 \
	$(out_man)/comedi_get_hardware_buffer_size.3 \
	$(out_man)/comedi_get_max_buffer_size.3 \
	$(out_man)/comedi_get_maxdata.3 $(out_man)/comedi_get_n_channels.3 \
	$(out_man)/comedi_get_n_ranges.3 $(out_man)/comedi_get_n_subdevices.3 \
	$(out_man)/comedi_get_range.3 $(out_man)/comedi_get_rangetype.3 \
	$(out_man)/comedi_get_read_subdevice.3 $(out_man)/comedi_get_routes.3 \
	$(out_man)/comedi_get_routing.3 \
	$(out_man)/comedi_get_softcal_converter.3 \
	$(out_man)/comedi_get_subdevice_flags.3 \
	$(out_man)/comedi_get_subdevice_type.3 $(out_man)/comedi_get_timer.3 \
	$(out_man)/comedi_get_version_code.3 \
	$(out_man)/comedi_get_write_subdevice.3 \
	$(out_man)/comedi_internal_trigger.3 $(out_man)/comedi_lock.3 \
	$(out_man)/comedi_loglevel.3 $(out_man)/comedi_lsampl_from_phys.3 \
	$(out_man)/comedi_lsampl_to_phys.3 \
	$(out_man)/comedi_mark_buffer_read.3 \
	$(out_man)/comedi_mark_buffer_written.3 \
	$(out_man)/comedi_maxdata_is_chan_specific.3 $(out_man)/comedi_open.3 \
	$(out_man)/comedi_parse_calibration_file.3 $(out_man)/comedi_perror.3 \
	$(out_man)/comedi_poll.3 $(out_man)/comedi_range_is_chan_specific.3 \
	$(out_man)/comedi_reset.3 $(out_man)/comedi_reset_channel.3 \
	$(out_man)/comedi_sampl_from_phys.3 $(out_man)/comedi_sampl_to_phys.3 \
	$(out_man)/comedi_set_buffer_size.3 \
	$(out_man)/comedi_set_clock_source.3 \
	$(out_man)/comedi_set_counter_mode.3 $(out_man)/comedi_set_filter.3 \
	$(out_man)/comedi_set_gate_source.3 \
	$(out_man)/comedi_set_global_oor_behavior.3 \
	$(out_man)/comedi_set_max_buffer_size.3 \
	$(out_man)/comedi_set_other_source.3 \
	$(out_man)/comedi_set_read_subdevice.3 $(out_man)/comedi_set_routing.3 \
	$(out_man)/comedi_set_write_subdevice.3 $(out_man)/comedi_strerror.3 \
	$(out_man)/comedi_sv_init.3 $(out_man)/comedi_sv_measure.3 \
	$(out_man)/comedi_sv_update.3 $(out_man)/comedi_test_route.3 \
	$(out_man)/comedi_timed_1chan.3 $(out_man)/comedi_to_phys.3 \
	$(out_man)/comedi_to_physical.3 $(out_man)/comedi_trigger.3 \
	$(out_man)/comedi_unlock.3

PDF_OUT = $(out_pdf)/comedilib.pdf

HTML_RESOURCE_OUT = $(out_html)/comedilib.css $(out_html)/acq-seq.gif

HTML_HTML_OUT = $(out_html)/acquisitionfunctions.html \
	$(out_html)/asyncprogram.html $(out_html)/boardspecific.html \
	$(out_html)/comedikernelgeneric.html \
	$(out_html)/comedilib-glossary.html $(out_html)/comedireference.html \
	$(out_html)/commandsstreaming.html $(out_html)/constantsmacros.html \
	$(out_html)/convertingsamples.html \
	$(out_html)/datatypesstructures.html $(out_html)/drivercallbacks.html \
	$(out_html)/drivercaveats.html $(out_html)/driverwriting.html \
	$(out_html)/examples.html $(out_html)/experimentalfunctionality.html \
	$(out_html)/func-ref-comedi-apply-calibration.html \
	$(out_html)/func-ref-comedi-apply-parsed-calibration.html \
	$(out_html)/func-ref-comedi-arm-channel.html \
	$(out_html)/func-ref-comedi-arm.html \
	$(out_html)/func-ref-comedi-cancel.html \
	$(out_html)/func-ref-comedi-cleanup-calibration.html \
	$(out_html)/func-ref-comedi-close.html \
	$(out_html)/func-ref-comedi-command-test.html \
	$(out_html)/func-ref-comedi-command.html \
	$(out_html)/func-ref-comedi-connect-route.html \
	$(out_html)/func-ref-comedi-data-read-delayed.html \
	$(out_html)/func-ref-comedi-data-read-hint.html \
	$(out_html)/func-ref-comedi-data-read-n.html \
	$(out_html)/func-ref-comedi-data-read.html \
	$(out_html)/func-ref-comedi-data-write.html \
	$(out_html)/func-ref-comedi-digital-trigger-disable.html \
	$(out_html)/func-ref-comedi-digital-trigger-enable-edges.html \
	$(out_html)/func-ref-comedi-digital-trigger-enable-levels.html \
	$(out_html)/func-ref-comedi-dio-bitfield.html \
	$(out_html)/func-ref-comedi-dio-bitfield2.html \
	$(out_html)/func-ref-comedi-dio-config.html \
	$(out_html)/func-ref-comedi-dio-get-config.html \
	$(out_html)/func-ref-comedi-dio-read.html \
	$(out_html)/func-ref-comedi-dio-write.html \
	$(out_html)/func-ref-comedi-disarm-channel.html \
	$(out_html)/func-ref-comedi-disarm.html \
	$(out_html)/func-ref-comedi-disconnect-route.html \
	$(out_html)/func-ref-comedi-do-insn.html \
	$(out_html)/func-ref-comedi-do-insnlist.html \
	$(out_html)/func-ref-comedi-errno.html \
	$(out_html)/func-ref-comedi-fileno.html \
	$(out_html)/func-ref-comedi-find-range.html \
	$(out_html)/func-ref-comedi-find-subdevice-by-type.html \
	$(out_html)/func-ref-comedi-from-phys.html \
	$(out_html)/func-ref-comedi-from-physical.html \
	$(out_html)/func-ref-comedi-get-board-name.html \
	$(out_html)/func-ref-comedi-get-buffer-contents.html \
	$(out_html)/func-ref-comedi-get-buffer-offset.html \
	$(out_html)/func-ref-comedi-get-buffer-read-count.html \
	$(out_html)/func-ref-comedi-get-buffer-read-offset.html \
	$(out_html)/func-ref-comedi-get-buffer-size.html \
	$(out_html)/func-ref-comedi-get-buffer-write-count.html \
	$(out_html)/func-ref-comedi-get-buffer-write-offset.html \
	$(out_html)/func-ref-comedi-get-clock-source.html \
	$(out_html)/func-ref-comedi-get-cmd-generic-timed.html \
	$(out_html)/func-ref-comedi-get-cmd-src-mask.html \
	$(out_html)/func-ref-comedi-get-cmd-timing-constraints.html \
	$(out_html)/func-ref-comedi-get-default-calibration-path.html \
	$(out_html)/func-ref-comedi-get-driver-name.html \
	$(out_html)/func-ref-comedi-get-gate-source.html \
	$(out_html)/func-ref-comedi-get-hardcal-converter.html \
	$(out_html)/func-ref-comedi-get-hardware-buffer-size.html \
	$(out_html)/func-ref-comedi-get-max-buffer-size.html \
	$(out_html)/func-ref-comedi-get-maxdata.html \
	$(out_html)/func-ref-comedi-get-n-channels.html \
	$(out_html)/func-ref-comedi-get-n-ranges.html \
	$(out_html)/func-ref-comedi-get-n-subdevices.html \
	$(out_html)/func-ref-comedi-get-range.html \
	$(out_html)/func-ref-comedi-get-rangetype.html \
	$(out_html)/func-ref-comedi-get-read-subdevice.html \
	$(out_html)/func-ref-comedi-get-routes.html \
	$(out_html)/func-ref-comedi-get-routing.html \
	$(out_html)/func-ref-comedi-get-softcal-converter.html \
	$(out_html)/func-ref-comedi-get-subdevice-flags.html \
	$(out_html)/func-ref-comedi-get-subdevice-type.html \
	$(out_html)/func-ref-comedi-get-timer.html \
	$(out_html)/func-ref-comedi-get-version-code.html \
	$(out_html)/func-ref-comedi-get-write-subdevice.html \
	$(out_html)/func-ref-comedi-internal-trigger.html \
	$(out_html)/func-ref-comedi-lock.html \
	$(out_html)/func-ref-comedi-loglevel.html \
	$(out_html)/func-ref-comedi-lsampl-from-phys.html \
	$(out_html)/func-ref-comedi-lsampl-to-phys.html \
	$(out_html)/func-ref-comedi-mark-buffer-read.html \
	$(out_html)/func-ref-comedi-mark-buffer-written.html \
	$(out_html)/func-ref-comedi-maxdata-is-chan-specific.html \
	$(out_html)/func-ref-comedi-open.html \
	$(out_html)/func-ref-comedi-parse-calibration-file.html \
	$(out_html)/func-ref-comedi-perror.html \
	$(out_html)/func-ref-comedi-poll.html \
	$(out_html)/func-ref-comedi-range-is-chan-specific.html \
	$(out_html)/func-ref-comedi-reset-channel.html \
	$(out_html)/func-ref-comedi-reset.html \
	$(out_html)/func-ref-comedi-sampl-from-phys.html \
	$(out_html)/func-ref-comedi-sampl-to-phys.html \
	$(out_html)/func-ref-comedi-set-buffer-size.html \
	$(out_html)/func-ref-comedi-set-clock-source.html \
	$(out_html)/func-ref-comedi-set-counter-mode.html \
	$(out_html)/func-ref-comedi-set-filter.html \
	$(out_html)/func-ref-comedi-set-gate-source.html \
	$(out_html)/func-ref-comedi-set-global-oor-behavior.html \
	$(out_html)/func-ref-comedi-set-max-buffer-size.html \
	$(out_html)/func-ref-comedi-set-other-source.html \
	$(out_html)/func-ref-comedi-set-read-subdevice.html \
	$(out_html)/func-ref-comedi-set-routing.html \
	$(out_html)/func-ref-comedi-set-write-subdevice.html \
	$(out_html)/func-ref-comedi-strerror.html \
	$(out_html)/func-ref-comedi-sv-init.html \
	$(out_html)/func-ref-comedi-sv-measure.html \
	$(out_html)/func-ref-comedi-sv-update.html \
	$(out_html)/func-ref-comedi-test-route.html \
	$(out_html)/func-ref-comedi-timed-1chan.html \
	$(out_html)/func-ref-comedi-to-phys.html \
	$(out_html)/func-ref-comedi-to-physical.html \
	$(out_html)/func-ref-comedi-trigger.html \
	$(out_html)/func-ref-comedi-unlock.html \
	$(out_html)/functionreference.html $(out_html)/gettinginformation.html \
	$(out_html)/index.html $(out_html)/install.html \
	$(out_html)/instructions.html \
	$(out_html)/instructionsconfiguration.html \
	$(out_html)/integratingdriver.html \
	$(out_html)/inttrigconfiguration.html \
	$(out_html)/languagebindings.html $(out_html)/lowleveldrivers.html \
	$(out_html)/secondprogram.html $(out_html)/slowlyvarying.html \
	$(out_html)/writingprograms.html

HTML_OUT = $(HTML_HTML_OUT) $(HTML_RESOURCE_OUT)

man_MANS = $(MAN_OUT)

BUILT_DIST = $(BUILT_XML) $(HTML_OUT) $(MAN_OUT) $(PDF_OUT) stamp-html stamp-man

EXTRA_DIST = $(BUILT_DIST) $(STATIC_XML_IN) FAQ acq-seq.gif \
	calibration_funcref.txt command_funcref.txt comedilib.css \
	comedilib_dblatex_config.xsl comedilib_fop.xconf.in \
	comedilib_html_config.xsl deprecated_funcref.txt dio_funcref.txt \
	drivers-linux.txt drivers.txt error_funcref.txt extensions_funcref.txt \
	funcref.txt mkdr mkref

MAINTAINERCLEANFILES = $(BUILT_DIST)

all_html = html-local
all_man = man
all_pdf = pdf-local

install_html = install-html-local
install_pdf = install-pdf-local

if BUILD_PDF_USING_DBLATEX
XMLTO_PDF_OPTS = --with-dblatex -p "-p $(abs_srcdir)/comedilib_dblatex_config.xsl -x --path -x $(abs_srcdir):$(abs_builddir)"
else
if BUILD_PDF_USING_FOP
XMLTO_PDF_OPTS = --with-fop -p "-c $(abs_builddir)/comedilib_fop.xconf" --stringparam keep.relative.image.uris="1"
else
XMLTO_PDF_OPTS =
endif
endif

all-local: $(all_html) $(all_pdf)

install-data-local: $(install_html) $(install_pdf)

uninstall-local: uninstall-html uninstall-pdf

html-local: $(HTML_OUT)

stamp-html: $(XML_IN) $(TUTORIAL_IN) comedilib_html_config.xsl
if HAVE_XMLTO
	$(XMLTO) -o $(out_html) --searchpath "$(abs_srcdir):$(abs_builddir)" \
		-m comedilib_html_config.xsl --skip-validation html \
		"$(srcdir)/comedilib.xml"
endif
	touch $@

$(HTML_HTML_OUT): stamp-html

$(out_html)/comedilib.css: comedilib.css
	$(MKDIR_P) $(out_html)
	$(LN_S) -f ../comedilib.css $(out_html)/

$(out_html)/acq-seq.gif: acq-seq.gif
	$(MKDIR_P) $(out_html)
	$(LN_S) -f ../acq-seq.gif $(out_html)/

# based on install-html-am from Automake's texinfos.am
# but install into $(htmldir)/html instead of $(htmldir)
install-html-local: html-local
	@$(NORMAL_INSTALL)
	@list='$(HTML_OUT)'; list2=; test -n "$(htmldir)" || list=; \
	if test -n "$$list"; then \
	  echo " $(MKDIR_P) '$(DESTDIR)$(htmldir)/html'"; \
	  $(MKDIR_P) "$(DESTDIR)$(htmldir)/html" || exit 1; \
	fi; \
	for p in $$list; do \
	  if test -f "$$p" || test -d "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  $(am__strip_dir) \
## This indirection is required to work around a bug of the Solaris 10
## shell /usr/xpg4/bin/sh.  The description of the bug can be found at
## <https://lists.gnu.org/archive/html/bug-autoconf/2011-11/msg00005.html>
## and the report of the original failure can be found at automake
## bug#10026 <https://debbugs.gnu.org/cgi/bugreport.cgi?bug=10026#23>
	  d2=$$d$$p; \
	  if test -d "$$d2"; then \
	    echo " $(MKDIR_P) '$(DESTDIR)$(htmldir)/html/$$f'"; \
	    $(MKDIR_P) "$(DESTDIR)$(htmldir)/html/$$f" || exit 1; \
	    echo " $(INSTALL_DATA) '$$d2'/* '$(DESTDIR)$(htmldir)/html/$$f'"; \
	    $(INSTALL_DATA) "$$d2"/* "$(DESTDIR)$(htmldir)/html/$$f" || exit $$?; \
	  else \
	    list2="$$list2 $$d2"; \
	  fi; \
	done; \
	test -z "$$list2" || { echo "$$list2" | $(am__base_list) | \
	while read files; do \
	  echo " $(INSTALL_DATA) $$files '$(DESTDIR)$(htmldir)/html'"; \
	  $(INSTALL_DATA) $$files "$(DESTDIR)$(htmldir)/html" || exit $$?; \
	done; }

# based on uninstall-html-am from Automake's texinfos.am
# but uninstall from $(htmldir)/html instead of $(htmldir)
uninstall-html:
	@$(NORMAL_UNINSTALL)
	@list='$(HTML_OUT)'; test -n "$(htmldir)" || list=; \
	for p in $$list; do \
	  $(am__strip_dir) \
## $f can be a directory, hence the -r.
	  echo " rm -rf '$(DESTDIR)$(htmldir)/html/$$f'"; \
	  rm -rf "$(DESTDIR)$(htmldir)/html/$$f"; \
	done

stamp-man: $(XML_IN)
if HAVE_XMLTO
	$(XMLTO) -o $(out_man) --searchpath "$(abs_srcdir):$(abs_builddir)" \
		--skip-validation man "$(srcdir)/comedilib.xml"
endif
	touch $@

$(MAN_OUT): stamp-man

man: $(MAN_OUT)

pdf-local: $(PDF_OUT)

# based on install-pdf-am from Automake's texinfos.am
install-pdf-local: $(PDF_OUT)
	@$(NORMAL_INSTALL)
	@list='$(PDF_OUT)'; test -n "$(pdfdir)" || list=; \
	if test -n "$$list"; then \
	  echo " $(MKDIR_P) '$(DESTDIR)$(pdfdir)'"; \
	  $(MKDIR_P) "$(DESTDIR)$(pdfdir)" || exit 1; \
	fi; \
	for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  echo "$$d$$p"; \
	done | $(am__base_list) | \
	while read files; do \
	  echo " $(INSTALL_DATA) $$files '$(DESTDIR)$(pdfdir)'"; \
	  $(INSTALL_DATA) $$files "$(DESTDIR)$(pdfdir)" || exit $$?; done

# based on uninstall-pdf-am from Automake's texinfos.am
uninstall-pdf:
	@$(NORMAL_UNINSTALL)
	@list='$(PDF_OUT)'; test -n "$(pdfdir)" || list=; \
	for p in $$list; do \
	  $(am__strip_dir) \
	  echo " rm -f '$(DESTDIR)$(pdfdir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pdfdir)/$$f"; \
	done

$(out_pdf)/comedilib.pdf: $(XML_IN) $(TUTORIAL_IN) comedilib_dblatex_config.xsl
if HAVE_XMLTO
if BUILD_PDF
	$(XMLTO) -o $(out_pdf) --searchpath "$(abs_srcdir):$(abs_builddir)" --skip-validation $(XMLTO_PDF_OPTS) pdf "$(srcdir)/comedilib.xml"
endif
endif

funcref.xml: funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/funcref.txt > $@~ && mv -f $@~ $@

calibration_funcref.xml: calibration_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/calibration_funcref.txt > $@~ && mv -f $@~ $@

command_funcref.xml: command_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/command_funcref.txt > $@~ && mv -f $@~ $@

dio_funcref.xml: dio_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/dio_funcref.txt > $@~ && mv -f $@~ $@

deprecated_funcref.xml: deprecated_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/deprecated_funcref.txt > $@~ && mv -f $@~ $@

error_funcref.xml: error_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/error_funcref.txt > $@~ && mv -f $@~ $@

extensions_funcref.xml: extensions_funcref.txt mkref
	$(srcdir)/mkref $(srcdir)/extensions_funcref.txt > $@~ && mv -f $@~ $@

# Note: if drivers.txt is updated from the out-of-tree comedi.org driver
# sources, please update the 'comedi_date' entity in 'comedilib.ent'.
drivers.xml: drivers.txt mkdr
	$(srcdir)/mkdr "ll-drivers-comedi" "Drivers from comedi.org" $(srcdir)/drivers.txt > $@~ && mv -f $@~ $@

# Note: if drivers-linux.txt is updated from Linux kernel sources, please
# update the 'linux_version' entity in 'comedilib.ent'.
drivers-linux.xml: drivers-linux.txt mkdr
	$(srcdir)/mkdr "ll-drivers-linux" "Drivers from Linux kernel sources" $(srcdir)/drivers-linux.txt > $@~ && mv -f $@~ $@

locales = de

messages:
	xgettext -k_ -k_s `find .. -name '*.c'`
	for i in $(locales);do \
	  msgmerge po/$$i.po messages.po -o $$i.po; \
	  mkdir -p locale/$$i/LC_MESSAGES; \
	  msgfmt $$i.po -o locale/$$i/LC_MESSAGES/comedilib.mo; \
	done

.PHONY: uninstall-pdf uninstall-html messages
