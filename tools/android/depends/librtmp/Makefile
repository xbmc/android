include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=rtmpdump
VERSION=2.3
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tgz

# configuration settings

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM)/librtmp; patch -p0 < ../../prefix.patch

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)/librtmp CROSS_COMPILE=$(CROSSTOOLS) PREFIX=$(PREFIX)

.installed-$(PLATFORM):
	make -C $(PLATFORM)/librtmp install PREFIX=$(PREFIX)
	rm -f $(PREFIX)/lib/librtmp.la $(PREFIX)/lib/librtmp.so
	mv -f $(PREFIX)/lib/librtmp.so.0 $(PREFIX)/lib/librtmp.so
	rpl -e "librtmp.so.0" "librtmp.so\x00\x00" $(PREFIX)/lib/librtmp.so
	readelf --dynamic $(PREFIX)/lib/librtmp.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

