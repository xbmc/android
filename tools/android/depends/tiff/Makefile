include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=tiff
VERSION=3.8.2
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess config/; \
          ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(PLATFORM)/libtiff/.libs/lib$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libtiffxx.la $(PREFIX)/lib/libtiffxx.so $(PREFIX)/lib/libtiffxx.so.3
	mv -f $(PREFIX)/lib/libtiffxx.so.3.8.2 $(PREFIX)/lib/libtiffxx.so
	rpl -e "libtiffxx.so.3" "libtiffxx.so\x00\x00" $(PREFIX)/lib/libtiffxx.so
	rpl -e "libtiff.so.3" "libtiff.so\x00\x00" $(PREFIX)/lib/libtiffxx.so
	readelf --dynamic $(PREFIX)/lib/libtiffxx.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libtiff.la $(PREFIX)/lib/libtiff.so $(PREFIX)/lib/libtiff.so.3
	mv -f $(PREFIX)/lib/libtiff.so.3.8.2 $(PREFIX)/lib/libtiff.so
	rpl -e "libtiff.so.3" "libtiff.so\x00\x00" $(PREFIX)/lib/libtiff.so
	readelf --dynamic $(PREFIX)/lib/libtiff.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

