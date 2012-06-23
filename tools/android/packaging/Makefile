include ../depends/Makefile.include

OBJS = libcurl.so \
  librtmp.so  libmad.so \
  libnfs.so libafpclient.so  \
  libshairport.so libplist.so \

XBMCROOT = $(CURDIR)/../../..
COPYDIRS = system addons language media
AAPT = $(SDKROOT)/platform-tools/aapt
all: debug

debug: prepare
	mkdir -p tmp/res; $(AAPT) c -S xbmc/res -C tmp/res; cp -r -n xbmc/res tmp/ || true
	$(AAPT) p -f -I $(SDKROOT)/platforms/android-10/android.jar -S tmp/res/ -M xbmc/AndroidManifest.xml -F images/xbmcapp-debug-skeleton.apk
	@rm -rf tmp/
	@cp images/xbmcapp-debug-skeleton.apk images/xbmcapp-debug-unaligned.apk
	@cd xbmc; zip -r -q ../images/xbmcapp-debug-unaligned.apk lib assets
	@jarsigner -keystore ~/.android/debug.keystore -storepass android images/xbmcapp-debug-unaligned.apk androiddebugkey
	@$(SDKROOT)/tools/zipalign -f 4 images/xbmcapp-debug-unaligned.apk images/xbmcapp-debug.apk
	@rm images/xbmcapp-debug-unaligned.apk
	@echo "images/xbmcapp-debug.apk created"

prepare:
	rm -rf xbmc/assets xbmc/lib tmp
	mkdir -p xbmc/assets xbmc/res xbmc/lib/$(PLATFORM) images xbmc/assets/python2.6/lib/
	$(foreach var,$(OBJS),$(STRIP) --strip-unneeded $(PREFIX)/lib/$(var) -o xbmc/lib/$(PLATFORM)/`echo $(var) | sed -e 's/\.so.*/\.so/g'`;)
	$(STRIP) --strip-unneeded $(XBMCROOT)/libxbmc.so -o xbmc/lib/$(PLATFORM)/libxbmc.so
	$(foreach var,$(COPYDIRS), cp -rfp ../../../$(var) ./xbmc/assets/;)
	cp -rfp $(PREFIX)/lib/python2.6 xbmc/assets/python2.6/lib/
	cd xbmc/assets/python2.6/lib/python2.6/; rm -rf test config lib-dynload
	find xbmc/assets/system -name "*.so" -exec mv {} xbmc/lib/$(PLATFORM) \;
	$(STRIP) --strip-unneeded xbmc/lib/$(PLATFORM)/*.so
	cd xbmc/lib/$(PLATFORM); find . -name "*.so" -not -name "lib*.so" | sed "s/\.\///" | xargs -I@ mv @ lib@
	find `pwd`/xbmc/assets/addons/skin.*/media/* -not -iname "Textures.xbt" -exec rm {} \;
	install -p $(NDKROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver ./xbmc/lib/$(PLATFORM)/gdbserver
	echo "set solib-search-path ./obj/local/$(PLATFORM):$(PREFIX)/lib:$(XBMCROOT):$(XBMCROOT)/xbmc/android/activity" > ./xbmc/lib/$(PLATFORM)/gdb.setup
	echo "directory $(NDKROOT)/platforms/$(APP_PLATFORM)/arch-arm/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++ $(XBMCROOT) $(XBMCROOT)/xbmc/android/activity $(PREFIX)/include jni" >> ./xbmc/lib/$(PLATFORM)/gdb.setup
