include ../depends/Makefile.include
include ../depends/android-sdk.mk

include xbmc/jni/Application.mk

OBJS = libxbpng.so libcurl.so \
  libtiff.so libxbjpeg.so librtmp.so libmad.so \
  libnfs.so libafpclient.so \
  libshairport.so libfontconfig.so

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
	mkdir -p xbmc/assets xbmc/res xbmc/lib/$(APP_ABI) images xbmc/assets/python2.6/lib/
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(PREFIX)/lib/$(var) -o xbmc/lib/$(APP_ABI)/`echo $(var) | sed -e 's/\.so.*/\.so/g'`;)
	$(STRIP) --strip-unneeded $(XBMCROOT)/libxbmc.so -o xbmc/lib/$(APP_ABI)/libxbmc.so
	$(STRIP) --strip-unneeded $(XBMCROOT)/libxbmcapp.so -o xbmc/lib/$(APP_ABI)/libxbmcapp.so
	$(foreach var,$(COPYDIRS), cp -rfp ../../../$(var) ./xbmc/assets/;)
	cp -rfp $(PREFIX)/lib/python2.6 xbmc/assets/python2.6/lib/
	cd xbmc/assets/python2.6/lib/python2.6/; rm -rf test config lib-dynload
	cd xbmc/assets/python2.6/; zip -q -r -m python26.zip *
	find xbmc/assets/system -name "*.so" -exec mv {} xbmc/lib/$(APP_ABI) \;
	$(STRIP) --strip-unneeded xbmc/lib/$(APP_ABI)/*.so
	cd xbmc/lib/$(APP_ABI); find  -name "*.so" -not -name "lib*.so" -printf "%f\n" | xargs -I@ mv @ lib@
	find xbmc/assets/addons/skin.*/media/* -not -iname "Textures.xbt" -delete
	install -p $(NDKROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver ./xbmc/lib/$(APP_ABI)/gdbserver
	echo "set solib-search-path ./obj/local/$(APP_ABI):$(PREFIX)/lib:$(XBMCROOT):$(XBMCROOT)/xbmc/android/activity" > ./xbmc/lib/$(APP_ABI)/gdb.setup
	echo "directory $(NDKROOT)/platforms/$(APP_PLATFORM)/arch-arm/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++ $(XBMCROOT) $(XBMCROOT)/xbmc/android/activity $(PREFIX)/include jni" >> ./xbmc/lib/$(APP_ABI)/gdb.setup
