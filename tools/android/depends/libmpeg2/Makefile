include ../Makefile.include

# lib name, version
LIBNAME=libmpeg2
VERSION=0.5.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .auto/ ; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-sdl

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libmpeg2.la $(PREFIX)/lib/libmpeg2.so $(PREFIX)/lib/libmpeg2.so.0
	mv -f $(PREFIX)/lib/libmpeg2.so.0.1.0 $(PREFIX)/lib/libmpeg2.so
	rpl -e "libmpeg2.so.0" "libmpeg2.so\x00\x00" $(PREFIX)/lib/libmpeg2.so
	readelf --dynamic $(PREFIX)/lib/libmpeg2.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libmpeg2convert.la $(PREFIX)/lib/libmpeg2convert.so $(PREFIX)/lib/libmpeg2convert.so.0
	mv -f $(PREFIX)/lib/libmpeg2convert.so.0.0.0 $(PREFIX)/lib/libmpeg2convert.so
	rpl -e "libmpeg2convert.so.0" "libmpeg2convert.so\x00\x00" $(PREFIX)/lib/libmpeg2convert.so
	readelf --dynamic $(PREFIX)/lib/libmpeg2convert.so | grep ibrary
	#
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

