include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libasound
VERSION=1.0.23
SOURCE=alsa-lib-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2
BASE_URL=ftp://ftp.alsa-project.org/pub/lib

# configuration settings
CFLAGS += \
	-fPIC -DPIC \
	-finline-limit=300 -finline-functions -fno-inline-functions-called-once \

CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          autoreconf -vif; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) \
          --with-alsa-devdir=/dev/snd/ --with-plugindir=/system/usr/lib/alsa-lib --with-configdir=/data/data/org.xbmc/cache/apk/assets/system/alsa \
          --with-ctl-plugins=ext \
          --with-pcm-plugins="copy,linear,route,mulaw,alaw,adpcm,rate,plug,multi,file,null,empty,share,meter,hooks,lfloat,ladspa,asym,iec958,softvol,extplug,ioplug,mmap_emul" \
          --disable-resmgr --enable-aload --enable-mixer  --enable-pcm  --disable-rawmidi  --enable-hwdep  --disable-seq  --disable-alisp  --disable-old-symbols --disable-python \
          --with-softfloat=yes --with-libdl=yes --with-pthread=yes --with-librt=no \

LIBDYLIB=$(PLATFORM)/src/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p1 < ../removeshm-upstream.patch
	cd $(PLATFORM); patch -p0 < ../removeshm-2.patch
	cd $(PLATFORM); patch -p0 < ../removeshm-3.patch
	cd $(PLATFORM); patch -p0 < ../timeval.patch
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)/src

.installed:
	make -C $(PLATFORM)/src install-libLTLIBRARIES
	make -C $(PLATFORM)/include install
	rm -f $(PREFIX)/lib/libasound.la $(PREFIX)/lib/libasound.so $(PREFIX)/lib/libxbasound.so $(PREFIX)/lib/libasound.so.2
	mv -f $(PREFIX)/lib/libasound.so.2.0.0 $(PREFIX)/lib/libxbasound.so
	ln -s $(PREFIX)/lib/libxbasound.so $(PREFIX)/lib/libasound.so
	rpl -e "libasound.so.2" "libxbasound.so" $(PREFIX)/lib/libxbasound.so
	readelf --dynamic $(PREFIX)/lib/libxbasound.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
