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
	rm -f $(PREFIX)/lib/libssl.so
	mv -f $(PREFIX)/lib/libssl.so.0.9.8 $(PREFIX)/lib/libssl.so
	rpl -e "libcrypto.so.0.9.8" "libcrypto.so\x00\x00\x00\x00\x00\x00" $(PREFIX)/lib/libssl.so
	rpl -e "libssl.so.0.9.8" "libssl.so\x00\x00\x00\x00\x00\x00" $(PREFIX)/lib/libssl.so
	readelf --dynamic $(PREFIX)/lib/libssl.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libcrypto.so
	mv -f $(PREFIX)/lib/libcrypto.so.0.9.8 $(PREFIX)/lib/libcrypto.so
	rpl -e "libcrypto.so.0.9.8" "libcrypto.so\x00\x00\x00\x00\x00\x00" $(PREFIX)/lib/libcrypto.so
	readelf --dynamic $(PREFIX)/lib/libcrypto.so | grep ibrary
	#
	rpl -R -e "libcrypto.so.0.9.8" "libcrypto.so\x00\x00\x00\x00\x00\x00" $(PREFIX)/lib/engines
	#
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

