include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=rtmpdump
VERSION=2.3
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tgz

# configuration settings

LIBDYLIB=$(SOURCE)/$(LIBNAME)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE)/librtmp; patch -p0 < ../../prefix.patch

$(LIBDYLIB): $(SOURCE)
	make -j $(JOBS) -C $(SOURCE)/librtmp CROSS_COMPILE=$(CROSSTOOLS) PREFIX=$(PREFIX)

.installed:
	make -C $(SOURCE)/librtmp install
	rm -f $(PREFIX)/lib/librtmp.la $(PREFIX)/lib/librtmp.so
	mv -f $(PREFIX)/lib/librtmp.so.0 $(PREFIX)/lib/librtmp.so
	rpl -e "librtmp.so.0" "librtmp.so\x00\x00" $(PREFIX)/lib/librtmp.so
	readelf --dynamic $(PREFIX)/lib/librtmp.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

