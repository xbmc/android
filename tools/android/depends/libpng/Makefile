include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libpng
#VERSION=1.4.4
VERSION=1.2.38
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
	rm -f $(PREFIX)/lib/libpng.la $(PREFIX)/lib/libpng.so $(PREFIX)/lib/libxbpng.so $(PREFIX)/lib/libpng.so.3
	rm -f $(PREFIX)/lib/libpng.so.3.38.0
	rm -f $(PREFIX)/lib/libpng12.la $(PREFIX)/lib/libpng12.so $(PREFIX)/lib/libpng12.so.0
	mv -f $(PREFIX)/lib/libpng12.so.0.38.0 $(PREFIX)/lib/libxbpng.so
	ln -s $(PREFIX)/lib/libxbpng.so $(PREFIX)/lib/libpng.so
	rpl -e "libpng12.so.0" "libxbpng.so\x00\x00" $(PREFIX)/lib/libxbpng.so
	readelf --dynamic $(PREFIX)/lib/libxbpng.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
