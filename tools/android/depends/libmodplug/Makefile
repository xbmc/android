include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libmodplug
VERSION=0.8.7
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
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libmodplug.la $(PREFIX)/lib/libmodplug.so $(PREFIX)/lib/libmodplug.so.0
	mv -f $(PREFIX)/lib/libmodplug.so.0.0.0 $(PREFIX)/lib/libmodplug.so
	rpl -e "libmodplug.so.0" "libmodplug.so\x00\x00" $(PREFIX)/lib/libmodplug.so
	readelf --dynamic $(PREFIX)/lib/libmodplug.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

