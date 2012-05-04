include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=pcre
VERSION=7.9
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-stack-for-recursion

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
	rm -f $(PREFIX)/lib/libpcre.la $(PREFIX)/lib/libpcre.so $(PREFIX)/lib/libpcre.so.0
	mv -f $(PREFIX)/lib/libpcre.so.0.0.1 $(PREFIX)/lib/libpcre.so
	rpl -e "libpcre.so.0" "libpcre.so\x00\x00" $(PREFIX)/lib/libpcre.so
	readelf --dynamic $(PREFIX)/lib/libpcre.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libpcrecpp.la $(PREFIX)/lib/libpcrecpp.so $(PREFIX)/lib/libpcrecpp.so.0
	mv -f $(PREFIX)/lib/libpcrecpp.so.0.0.0 $(PREFIX)/lib/libpcrecpp.so
	rpl -e "libpcrecpp.so.0" "libpcrecpp.so\x00\x00" $(PREFIX)/lib/libpcrecpp.so
	rpl -e "libpcre.so.0" "libpcre.so\x00\x00" $(PREFIX)/lib/libpcrecpp.so
	readelf --dynamic $(PREFIX)/lib/libpcrecpp.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libpcreposix.la $(PREFIX)/lib/libpcreposix.so $(PREFIX)/lib/libpcreposix.so.0
	mv -f $(PREFIX)/lib/libpcreposix.so.0.0.0 $(PREFIX)/lib/libpcreposix.so
	rpl -e "libpcreposix.so.0" "libpcreposix.so\x00\x00" $(PREFIX)/lib/libpcreposix.so
	rpl -e "libpcre.so.0" "libpcre.so\x00\x00" $(PREFIX)/lib/libpcreposix.so
	readelf --dynamic $(PREFIX)/lib/libpcreposix.so | grep ibrary
	#
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
