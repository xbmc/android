include ../depends/Makefile.include

OBJS = libcurl.so \
  librtmp.so  libmad.so \
  libafpclient.so  \
  libshairport.so libplist.so \

XBMCROOT = $(CURDIR)/../../..
COPYDIRS = system addons language media
AAPT = $(SDKROOT)/platform-tools/aapt
GCC_VERSION=$(shell $(CC) -dumpversion)

#this fixes a android ndk fuckup where the pathes to 
#prebuilt stuff follow different name shemes for
#arm and x86
ifeq ($(findstring x86,$(PLATFORM)),x86)
GDBPATH=$(NDKROOT)/toolchains/$(PLATFORM)-$(GCC_VERSION)/prebuilt/gdbserver
else
GDBPATH=$(NDKROOT)/toolchains/$(HOST)-$(GCC_VERSION)/prebuilt/gdbserver
endif

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
	rm -rf xbmc/assets xbmc/lib xbmc/libs xbmc/obj tmp
	mkdir -p xbmc/assets xbmc/res xbmc/lib/$(PLATFORM) images xbmc/assets/python2.6/lib/ xbmc/libs/$(PLATFORM) xbmc/obj/local/$(PLATFORM)
	 $(foreach var,$(OBJS),cp -fp $(PREFIX)/lib/$(var) xbmc/obj/local/$(PLATFORM)/;)
	cp -fp $(XBMCROOT)/libxbmc.so xbmc/obj/local/$(PLATFORM)/
	$(foreach var,$(COPYDIRS), cp -rfp ../../../$(var) ./xbmc/assets/;)
	cp -rfp $(PREFIX)/lib/python2.6 xbmc/assets/python2.6/lib/
	cd xbmc/assets/python2.6/lib/python2.6/; rm -rf test config lib-dynload
	find xbmc/assets/system -name "*.so" -exec mv {} xbmc/obj/local/$(PLATFORM)/ \;
	cd xbmc/obj/local/$(PLATFORM)/; find . -name "*.so" -not -name "lib*.so" | sed "s/\.\///" | xargs -I@ mv @ lib@
	cp -fp xbmc/obj/local/$(PLATFORM)/*.so xbmc/lib/$(PLATFORM)/
	$(STRIP) --strip-unneeded xbmc/lib/$(PLATFORM)/*.so
	find `pwd`/xbmc/assets/addons/skin.*/media/* -not -iname "Textures.xbt" -exec rm {} \;
	install -p $(GDBPATH) ./xbmc/libs/$(PLATFORM)/gdbserver
	echo "set solib-search-path ./obj/local/$(PLATFORM)" > ./xbmc/libs/$(PLATFORM)/gdb.setup
	echo "directory $(TOOLCHAIN)/sysroot/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++/include/$(GCC_VERSION) $(XBMCROOT)  $(PREFIX)/include jni" >> ./xbmc/libs/$(PLATFORM)/gdb.setup
	cp -fp xbmc/libs/$(PLATFORM)/* xbmc/lib/$(PLATFORM)
