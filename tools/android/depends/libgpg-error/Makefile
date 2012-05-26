include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libgpg-error
VERSION=1.10
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) \
  --disable-languages \
  --disable-nls

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).so

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
	rm -f $(PREFIX)/lib/libgpg-error.la $(PREFIX)/lib/libgpg-error.so $(PREFIX)/lib/libgpg-error.so.0
	mv -f $(PREFIX)/lib/libgpg-error.so.0.8.0 $(PREFIX)/lib/libgpg-error.so
	rpl -e "libgpg-error.so.0" "libgpg-error.so\x00\x00" $(PREFIX)/lib/libgpg-error.so
	readelf --dynamic $(PREFIX)/lib/libgpg-error.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
