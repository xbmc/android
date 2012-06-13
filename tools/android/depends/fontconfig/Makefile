include ../Makefile.include

# lib name, version
LIBNAME=fontconfig
VERSION=2.8.0
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) \
  --with-freetype-config=$(PREFIX)/bin/freetype-config \
  --disable-libxml2 --disable-docs --with-arch=arm

LIBDYLIB=$(PLATFORM)/src/.libs/lib$(LIBNAME).a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); patch -p1 < ../01-fontconfig-cross-compile-fix.patch
	cd $(PLATFORM); patch -p1 < ../lconv.patch 
	cd $(PLATFORM); $(AUTORECONF) -vif
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libfontconfig.la $(PREFIX)/lib/libfontconfig.so $(PREFIX)/lib/libfontconfig.so.1
	mv -f $(PREFIX)/lib/libfontconfig.so.1.4.4 $(PREFIX)/lib/libfontconfig.so
	rpl -e "libfontconfig.so.1" "libfontconfig.so\x00\x00" $(PREFIX)/lib/libfontconfig.so
	readelf --dynamic $(PREFIX)/lib/libfontconfig.so | grep ibrary
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

