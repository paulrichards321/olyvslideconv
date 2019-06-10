#! /bin/sh

aclocal --install -i m4 \
&& automake --add-missing \
&& autoreconf -i
