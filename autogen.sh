#!/bin/sh

if [ ! -f NEWS ] ; then
	touch NEWS
fi
if [ ! -f AUTHORS ] ; then
	touch AUTHORS
fi
if [ ! -f ChangeLog ] ; then
	touch ChangeLog
fi

aclocal -I m4 && \
libtoolize --copy --force && \
autoheader && \
autoconf && \
automake -a -c && \
./configure --enable-maintainer-mode

