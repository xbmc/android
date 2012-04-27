include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=openssl
VERSION=0.9.8r
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=CC=arm-linux-androideabi-gcc MACHINE=arm ./config shared zlib shared --openssldir=$(PREFIX)

LIBDYLIB=$(SOURCE)/src/.libs/lib$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -j1 -C $(SOURCE)

.installed:
	make -C $(SOURCE) install_sw
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
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

