#! /bin/sh

libtoolize
aclocal --install -I m4 \
&& automake --add-missing \
&& autoreconf -i
