include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libmicrohttpd
VERSION=0.4.6
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(PLATFORM)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p0 < ../fix-android-include.patch
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)/src/daemon
	make -j $(JOBS) -C $(PLATFORM)/src/include install

.installed:
	rm -f $(PREFIX)/lib/libmicrohttpd.so $(PREFIX)/lib/libmicrohttpd.so.5 $(PREFIX)/lib/libmicrohttpd.so.5.2.1
	make -C $(PLATFORM)/src/daemon install
	mv $(PREFIX)/lib/libmicrohttpd.so.5.2.1 $(PREFIX)/lib/libmicrohttpd.so
	rpl -e "libmicrohttpd.so.5" "libmicrohttpd.so\x00\x00" $(PREFIX)/lib/libmicrohttpd.so
	readelf --dynamic $(PREFIX)/lib/libmicrohttpd.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

