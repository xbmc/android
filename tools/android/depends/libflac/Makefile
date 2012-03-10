include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=flac
VERSION=1.2.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-xmms-plugin --disable-cpplibs

LIBDYLIB=$(SOURCE)/$(LIBNAME)/.libs/$(LIBNAME).so

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
	make -j $(JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	rm -f $(PREFIX)/lib/libFLAC.la $(PREFIX)/lib/libFLAC.so $(PREFIX)/lib/libFLAC.so.8
	mv -f $(PREFIX)/lib/libFLAC.so.8.2.0 $(PREFIX)/lib/libFLAC.so
	rpl -e "libFLAC.so.8" "libFLAC.so\x00\x00" $(PREFIX)/lib/libFLAC.so
	readelf --dynamic $(PREFIX)/lib/libFLAC.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

