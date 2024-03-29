
#   COMEDILIB
### The Linux Control and Measurement Device Interface Library
##  David Schleef <ds@schleef.org>
---

## About Comedi

Comedi is a collection of drivers for data acquisition hardware.
These drivers work with Linux, and also with Linux combined with
the real-time extensions RTAI and RTLinux.  The Comedi core, which
ties all the driver together, allows applications to be written
that are completely hardware independent.

Comedi supports a variety of data acquisition hardware; an
incomplete list can be found in the Comedi source.

This distribution contains only the user-space library.  You will
almost certainly also want to download the Comedi kernel modules
found in the *comedi* tarball.  Also, the calibration programs
comedi_calibrate and comedi_soft_calibrate can be found in the
*comedi_calibrate* tarball.  A few boards also need firmware
found in the *comedi-nonfree-firmware* tarball.

## Installation

Installation instructions are found in [INSTALL.md](INSTALL.md).

## Mailing List

Questions about Comedi and Comedilib should be sent to the Comedi
mailing list, <comedi_list@googlegroups.com>.  It is necessary to
join the group before posting (see below).  It is also possible to
post to the list using the web interface (see below).  Mailing the
maintainer directly is always acceptable, but since the mailing list
is archived and questions are often answered more quickly by others,
the mailing list is preferred.

To subscribe to and unsubscribe from the mailing list, or to read or
post messages via the web interface, go to
<https://groups.google.com/group/comedi_list>.  Alternatively, you
can send a blank email to <comedi_list+subscribe@googlegroups.com>
to subscribe, or to <comedi_list+unsubscribe@googlegroups.com> to
unsubscribe (making sure the "From:" email address matches the
address you originally subscribed with!).

Traffic on the list is light, and mainly questions/answers about
comedi installation, bugs, and programming.  General questions
about data acquisition are also welcome.

## More Information

Comedi also has a web page at <https://www.comedi.org/> from where
updated versions may be downloaded.

Often bugfixes and new features that are not in the current release
can be found in the Git repository.  Instructions for access to the
Comedi and Comedilib repositories can be found at
<https://www.comedi.org/download.html>.  Git snapshots for Comedilib
can be created automatically at
<https://github.com/Linux-Comedi/comedilib/tarball/master>.  The Git
repository can be cloned locally using:

    git clone https://github.com/Linux-Comedi/comedilib.git

The Git repository was previously hosted at comedi.org.  A previously
cloned repository may need its URL updating to the current repository
on github.com as follows:

    cd /path/to/comedilib
    git remote set-url origin https://github.com/Linux-Comedi/comedilib.git

Comedilib may be freely distibuted and modified in accordance with
the GNU Lesser General Public License.  Portions of the Comedilib
distribution fall under different licenses; see the individual files
for details.

The person behind all this misspelled humor is David Schleef
<ds@schleef.org>.

