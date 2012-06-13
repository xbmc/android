include ../Makefile.include

# lib name, version
LIBNAME=openssl
VERSION=0.9.8r
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=MACHINE=$(HOST) ./config shared zlib shared --openssldir=$(PREFIX)

LIBDYLIB=$(PLATFORM)/libssl.a

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
	$(MAKE) -j 1 -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install_sw
	rm -f $(PREFIX)/lib/libssl.so $(PREFIX)/lib/libxbssl.so
	mv -f $(PREFIX)/lib/libssl.so.0.9.8 $(PREFIX)/lib/libxbssl.so
	ln -s $(PREFIX)/lib/libxbssl.so $(PREFIX)/lib/libssl.so
	rpl -e "libcrypto.so.0.9.8" "libxbcrypto.so\x00\x00\x00\x00" $(PREFIX)/lib/libxbssl.so
	rpl -e "libssl.so.0.9.8" "libxbssl.so\x00\x00\x00\x00" $(PREFIX)/lib/libxbssl.so
	readelf --dynamic $(PREFIX)/lib/libxbssl.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libcrypto.so rm -f $(PREFIX)/lib/libxbcrypto.so
	mv -f $(PREFIX)/lib/libcrypto.so.0.9.8 $(PREFIX)/lib/libxbcrypto.so
	ln -s $(PREFIX)/lib/libxbcrypto.so $(PREFIX)/lib/libcrypto.so
	rpl -e "libcrypto.so.0.9.8" "libxbcrypto.so\x00\x00\x00\x00" $(PREFIX)/lib/libxbcrypto.so
	readelf --dynamic $(PREFIX)/lib/libxbcrypto.so | grep ibrary
	#
	rpl -R -e "libcrypto.so.0.9.8" "libxbcrypto.so\x00\x00\x00\x00" $(PREFIX)/lib/engines
	#
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

