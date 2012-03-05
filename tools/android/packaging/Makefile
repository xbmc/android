include ../depends/Makefile.include
include ../depends/android-sdk.mk

OBJS = libgcrypt.so.11 libgpg-error.so.0 libfontconfig.so.1 libfreetype.so.6 \
  libiconv.so.2 libfribidi.so.0 libsqlite3.so.0 libpng12.so.0 libpcre.so.0 \
  libpcrecpp.so.0 libsamplerate.so.0 libpython2.6.so.1.0 libcrypto.so.0.9.8 \
  libpython2.6.so.1.0 libpng12.so.0 libyajl.so.2.0.1 \
  libtiff.so.3 libjpeg.so.8.0.2 libzip.so

all: package

package:
	mkdir -p xbmc/libs/armeabi images xbmc/assets
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(XBMCPREFIX)/lib/$(var) -o xbmc/libs/armeabi/`echo $(var) | sed -e 's/\.so.*/\.so/g'`;)
	$(STRIP) --strip-unneeded ../../../libxbmc.so -o xbmc/libs/armeabi/libxbmc.so
	$(STRIP) --strip-unneeded ../../../xbmc/android/libxbmcapp.so -o xbmc/libs/armeabi/libxbmcapp.so
	cp -rf ../../../system ../../../addons ../../../language xbmc/assets/
	./fixsolib.sh
	echo $(sdk.dir); cd xbmc; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug clean; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug
