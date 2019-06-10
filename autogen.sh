#! /bin/sh

libtoolize -i -c --force
aclocal --install -I m4 \
&& automake --add-missing \
&& autoreconf -i
