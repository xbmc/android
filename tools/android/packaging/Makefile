include ../depends/Makefile.include
include ../depends/android-sdk.mk

include xbmc/jni/Application.mk

OBJS = libgcrypt.so libgpg-error.so libfontconfig.so libfreetype.so \
  libiconv.so libfribidi.so libsqlite3.so libxbpng.so libpcre.so \
  libpcrecpp.so libsamplerate.so libpython2.6.so libxbcrypto.so \
  libpython2.6.so libyajl.so libxbssl.so libcurl.so \
  libtiff.so libxbjpeg.so librtmp.so

XBMCROOT = $(CURDIR)/../../..
COPYDIRS = system addons language media
all: debug

debug: prepare
	mkdir -p tmp/res; aapt c -S xbmc/res -C tmp/res; cp -r -n xbmc/res tmp/
	aapt p -f -I $(SDKROOT)/platforms/android-10/android.jar -S tmp/res/ -M xbmc/AndroidManifest.xml -F images/xbmcapp-debug-skeleton.apk
	@rm -rf tmp/
	@cp images/xbmcapp-debug-skeleton.apk images/xbmcapp-debug-unaligned.apk
	@cd xbmc; zip -r -q ../images/xbmcapp-debug-unaligned.apk lib assets
	@jarsigner -keystore ~/.android/debug.keystore -storepass android images/xbmcapp-debug-unaligned.apk androiddebugkey
	@$(SDKROOT)/tools/zipalign -f 4 images/xbmcapp-debug-unaligned.apk images/xbmcapp-debug.apk
	@rm images/xbmcapp-debug-unaligned.apk
	@echo "images/xbmcapp-debug.apk created"

prepare:
	rm -rf xbmc/assets xbmc/lib tmp
	mkdir -p xbmc/assets xbmc/res xbmc/lib/$(APP_ABI) images
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(PREFIX)/lib/$(var) -o xbmc/lib/$(APP_ABI)/`echo $(var) | sed -e 's/\.so.*/\.so/g'`;)
	$(STRIP) --strip-unneeded $(XBMCROOT)/libxbmc.so -o xbmc/lib/$(APP_ABI)/libxbmc.so
	$(STRIP) --strip-unneeded $(XBMCROOT)/xbmc/android/activity/libxbmcapp.so -o xbmc/lib/$(APP_ABI)/libxbmcapp.so
	$(foreach var,$(COPYDIRS), cp -rfp ../../../$(var) ./xbmc/assets/;)
	$(STRIP) --strip-unneeded xbmc/assets/system/*.so
	$(STRIP) --strip-unneeded xbmc/assets/system/players/*/*.so
	find xbmc/assets/addons/skin.*/media/* -not -iname "Textures.xbt" -delete
	rm -rf xbmc/assets/addons/skin.confluence
	install -p $(NDKROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver ./xbmc/lib/$(APP_ABI)/gdbserver
	echo "set solib-search-path ./obj/local/$(APP_ABI):$(PREFIX)/lib:$(XBMCROOT):$(XBMCROOT)/xbmc/android/activity" > ./xbmc/lib/$(APP_ABI)/gdb.setup
	echo "directory $(NDKROOT)/platforms/$(APP_PLATFORM)/arch-arm/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++ $(XBMCROOT) $(XBMCROOT)/xbmc/android/activity $(PREFIX)/include jni" >> ./xbmc/lib/$(APP_ABI)/gdb.setup
