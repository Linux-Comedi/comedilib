
bin_PROGRAMS = comedi_calibrate

noinst_HEADERS = calib.h

comedi_calibrate_SOURCES = \
	comedi_calibrate.c ni.c cb.c cb64.c other.c save_cal.c cal_common.c ni_labpc.c
comedi_calibrate_CFLAGS = $(COMEDILIB_CFLAGS)
comedi_calibrate_LDADD = $(COMEDILIB_LIBS)

install-data-local:
	$(INSTALL) -d /etc/comedi/calibrations

EXTRA_DIST = results

