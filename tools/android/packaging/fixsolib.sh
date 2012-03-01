#!/bin/bash

#This is SOO nasty, but it's enough to get things rolling for now

rpl -R -e libyajl.so.2 "libyajl.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libssl.so.0.9.8 "libssl.so\x00\x00\x00\x00\x00\x00" xbmc/libs/armeabi/
rpl -R -e libcrypto.so.0.9.8 "libcrypto.so\x00\x00\x00\x00\x00\x00" xbmc/libs/armeabi/
rpl -R -e libtiff.so.3 "libtiff.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libjpeg.so.8 "libjpeg.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libgcrypt.so.11 "libgcrypt.so\x00\x00\x00" xbmc/libs/armeabi/
rpl -R -e libgpg-error.so.0 "libgpg-error.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libfontconfig.so.1 "libfontconfig.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libexpat.so.1 "libexpat.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libfreetype.so.6 "libfreetype.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libiconv.so.2 "libiconv.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libfribidi.so.0 "libfribidi.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libsqlite3.so.0 "libsqlite3.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libpng12.so.0 "libpng12.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libpcre.so.0 "libpcre.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libpcrecpp.so.0 "libpcrecpp.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libsamplerate.so.0 "libsamplerate.so\x00\x00" xbmc/libs/armeabi/
rpl -R -e libpython2.6.so.1.0 "libpython2.6.so\x00\x00\x00\x00" xbmc/libs/armeabi/

