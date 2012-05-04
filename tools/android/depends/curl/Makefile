include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=curl
VERSION=7.21.3
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2
BASE_URL=http://curl.haxx.se/download/
CFLAGS+=-DANDROID

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --without-libssh2 \
ac_cv_file__dev_urandom=yes \

LIBDYLIB=$(PLATFORM)/.libs/(LIBNAME).so

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
	rm -f $(PREFIX)/lib/libcurl.la $(PREFIX)/lib/libcurl.so $(PREFIX)/lib/libcurl.so.5
	mv -f $(PREFIX)/lib/libcurl.so.5.2.0 $(PREFIX)/lib/libcurl.so
	rpl -e "libcurl.so.5" "libcurl.so\x00\x00" $(PREFIX)/lib/libcurl.so
	readelf --dynamic $(PREFIX)/lib/libcurl.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

