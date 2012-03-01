include ../depends/Makefile.include
include ../depends/android-sdk.mk

OBJS = libgcrypt.so.11 libgpg-error.so.0 libfontconfig.so.1 libfreetype.so.6 \
  libiconv.so.2 libfribidi.so.0 libsqlite3.so.0 libpng12.so.0 libpcre.so.0 \
  libpcrecpp.so.0 libsamplerate.so.0 libpython2.6.so.1.0 libxbmcapp.so \
  libxbmc.so

all: package

package:
	mkdir -p xbmc/lib/armeabi images
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(XBMCPREFIX)/lib/$(var) -o xbmc/lib/armeabi/$(var);)
	echo $(sdk.dir); cd xbmc; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug
