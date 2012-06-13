include ../Makefile.include

# lib name, version
LIBNAME=flac
VERSION=1.2.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
export NASM=
# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./autogen.sh --prefix=$(PREFIX) --host=$(HOST) --disable-xmms-plugin --disable-cpplibs

LIBDYLIB=$(PLATFORM)/src/libFLAC/.libs/libFLAC.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p0 < ../nasm.patch
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libFLAC.la $(PREFIX)/lib/libFLAC.so $(PREFIX)/lib/libFLAC.so.8
	mv -f $(PREFIX)/lib/libFLAC.so.8.2.0 $(PREFIX)/lib/libFLAC.so
	rpl -e "libFLAC.so.8" "libFLAC.so\x00\x00" $(PREFIX)/lib/libFLAC.so
	readelf --dynamic $(PREFIX)/lib/libFLAC.so | grep ibrary
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

