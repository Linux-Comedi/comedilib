Summary: Data Acquisition library for the Comedi DAQ driver.
Name: comedilib
Version: 0.7.18
Release: 1
Copyright: GPL
Group: System Environment/Kernel
Source: http://stm.lbl.gov/comedi/download/comedilib-0.7.18.tgz
Patch: comedilib.patch
BuildRoot: /var/tmp/%{name}-buildroot
requires: comedi >= 0.7.63, kernel = 2.4.7, kernel-source = 2.4.7
provides: comedilib

%description
Comedilib is the library for the Comedi data acquisition driver
for Linux.  It allows Linux processes to acquire data from
supported DAQ cards, such as those from National Instruments.

%prep
%setup -q
%patch -p1 -b .buildroot

%build
#called when the rpm is built
make


%install
#install also gets called while compiling the RPM
#apparently rpm requires all the files it archives to be located
#in $RPM_BUILD_ROOT, so we copy over the files we need to save
rm -rf $RPM_BUILD_ROOT
#mkdir -p $RPM_BUILD_ROOT/usr/include

#lets try copying the install sequence...
install -d $RPM_BUILD_ROOT/usr/include
install -m 644 include/comedilib.h $RPM_BUILD_ROOT/usr/include
install -m 644 include/comedi.h $RPM_BUILD_ROOT/usr/include

install -d $RPM_BUILD_ROOT/usr/lib
install -m 644 lib/libcomedi.a $RPM_BUILD_ROOT/usr/lib
install lib/libcomedi.so.0.7.18 $RPM_BUILD_ROOT/usr/lib

install -d $RPM_BUILD_ROOT/usr/sbin
install -d $RPM_BUILD_ROOT/usr/bin
install -s -m 755 comedi_config/comedi_config $RPM_BUILD_ROOT/usr/sbin
install -s -m 755 comedi_calibrate/comedi_calibrate $RPM_BUILD_ROOT/usr/bin

install -d $RPM_BUILD_ROOT/usr/share/doc/libcomedi
install README `find doc -type f` $RPM_BUILD_ROOT/usr/share/doc/libcomedi
install -d $RPM_BUILD_ROOT/usr/share/man/man7
install -d $RPM_BUILD_ROOT/usr/share/man/man8
install man/*.7 $RPM_BUILD_ROOT/usr/share/man/man7
install man/*.8 $RPM_BUILD_ROOT/usr/share/man/man8

#I'm not sure if I should include the demos or not, but
install -d $RPM_BUILD_ROOT/usr/local/comedilib/demo
cp demo/*.c $RPM_BUILD_ROOT/usr/local/comedilib/demo
cp demo/*.h $RPM_BUILD_ROOT/usr/local/comedilib/demo
cp demo/Makefile $RPM_BUILD_ROOT/usr/local/comedilib/demo

%post
#post gets called on the user's system after the files have been copied over
#"make install" calls install_dev, install_runtime, and install_doc:

(cd /usr/lib;ln -sf libcomedi.so.0.7.18 libcomedi.so)
(cd /usr/lib;ln -sf libcomedi.so.0.7.18 libcomedi.so.0)

#ldconfig?

%postun
#postun is called after the files have been uninstalled

%clean
#clean can be called after building the package

%files
%defattr(-,root,root)
#add whatever files here
/usr/include/comedilib.h
/usr/include/comedi.h
/usr/lib/libcomedi.so.0.7.18
/usr/lib/libcomedi.a
/usr/sbin/comedi_config
/usr/bin/comedi_calibrate
/usr/share/doc/libcomedi/
/usr/share/man/man7/comedi.7.gz
/usr/share/man/man8/comedi_calibrate.8.gz
/usr/share/man/man8/comedi_config.8.gz
/usr/local/comedilib/


%changelog
* Wed Feb 21 2002 Tim Ousley <tim.ousley@ni.com>
- initial build of comedilib RPM

