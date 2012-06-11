include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=flac
VERSION=1.2.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
export NASM=
# configuration settings
CONFIGURE=./autogen.sh --prefix=$(PREFIX) --host=$(HOST) --disable-xmms-plugin --disable-cpplibs

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p0 < ../nasm.patch
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed-$(PLATFORM):
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libFLAC.la $(PREFIX)/lib/libFLAC.so $(PREFIX)/lib/libFLAC.so.8
	mv -f $(PREFIX)/lib/libFLAC.so.8.2.0 $(PREFIX)/lib/libFLAC.so
	rpl -e "libFLAC.so.8" "libFLAC.so\x00\x00" $(PREFIX)/lib/libFLAC.so
	readelf --dynamic $(PREFIX)/lib/libFLAC.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

