#!/bin/sh


aclocal -I m4 && \
libtoolize --copy --force && \
autoheader && \
autoconf && \
automake -a -c && \
./configure --enable-maintainer-mode

