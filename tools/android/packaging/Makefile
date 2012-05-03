include ../depends/Makefile.include
include ../depends/android-sdk.mk

include xbmc/jni/Application.mk

OBJS = libgcrypt.so libgpg-error.so libfontconfig.so libfreetype.so \
  libiconv.so libfribidi.so libsqlite3.so libxbpng.so libpcre.so \
  libpcrecpp.so libsamplerate.so libpython2.6.so libxbcrypto.so \
  libpython2.6.so libyajl.so libxbssl.so libcurl.so \
  libtiff.so libxbjpeg.so

XBMCROOT = $(CURDIR)/../../../
COPYDIRS = system addons language media
all: package

package:
	rm -rf xbmc/assets xbmc/libs images
	mkdir -p xbmc/assets xbmc/libs/$(APP_ABI) images
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(XBMCPREFIX)/lib/$(var) -o xbmc/libs/$(APP_ABI)/`echo $(var) | sed -e 's/\.so.*/\.so/g'`;)
	$(STRIP) --strip-unneeded $(XBMCROOT)/libxbmc.so -o xbmc/libs/$(APP_ABI)/libxbmc.so
	$(STRIP) --strip-unneeded $(XBMCROOT)/xbmc/android/libxbmcapp.so -o xbmc/libs/$(APP_ABI)/libxbmcapp.so
	$(foreach var,$(COPYDIRS), cp -rf ../../../$(var) ./xbmc/assets/;)
	install -p $(NDKROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver ./xbmc/libs/$(APP_ABI)/gdbserver
	echo "set solib-search-path ./obj/local/$(APP_ABI):$(PREFIX)/lib:$(XBMCROOT):$(XBMCROOT)/xbmc/android" > ./xbmc/libs/$(APP_ABI)/gdb.setup
	echo "directory $(NDKROOT)/platforms/android-9/arch-arm/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++ $(XBMCROOT) $(XBMCROOT)/xbmc/android $(PREFIX)/include jni" >> ./xbmc/libs/$(APP_ABI)/gdb.setup

	echo $(sdk.dir); cd xbmc; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug clean; ant -Dout.dir=../images -Dsdk.dir=$(SDKROOT) debug
