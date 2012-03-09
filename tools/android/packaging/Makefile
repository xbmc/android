include ../depends/Makefile.include
include ../depends/android-sdk.mk

OBJS = libgcrypt.so.11 libgpg-error.so.0 libfontconfig.so.1 libfreetype.so.6 \
  libiconv.so.2 libfribidi.so.0 libsqlite3.so.0 libpng12.so.0 libpcre.so.0 \
  libpcrecpp.so.0 libsamplerate.so.0 libpython2.6.so.1.0 libcrypto.so.0.9.8 \
  libpython2.6.so.1.0 libpng12.so.0 libyajl.so.2.0.1 \
  libtiff.so.3 libjpeg.so.8.0.2 libzip.so
XBMCROOT = $(CURDIR)/../../../
COPYDIRS = system addons language media
all: package

package:
	mkdir -p xbmc/libs/armeabi images xbmc/assets
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(XBMCPREFIX)/lib/$(var) -o xbmc/libs/armeabi/`echo $(var) | sed -e 's/\.so.*/\.so/g'`;)
	$(STRIP) --strip-unneeded ../../../libxbmc.so -o xbmc/libs/armeabi/libxbmc.so
	$(STRIP) --strip-unneeded ../../../xbmc/android/libxbmcapp.so -o xbmc/libs/armeabi/libxbmcapp.so
	$(foreach var,$(COPYDIRS), cp -rf ../../../$(var) ./xbmc/assets/;)
	./fixsolib.sh
	install -p $(NDKROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver ./xbmc/libs/armeabi/gdbserver
	echo "set solib-search-path ./obj/local/armeabi:$(PREFIX)/lib:$(XBMCROOT):$(XBMCROOT)/xbmc/android" > ./xbmc/libs/armeabi/gdb.setup
	echo "directory $(NDKROOT)/platforms/android-9/arch-arm/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++ $(XBMCROOT) $(XBMCROOT)/xbmc/android $(PREFIX)/include jni" >> ./xbmc/libs/armeabi/gdb.setup

	echo $(sdk.dir); cd xbmc; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug clean; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug
