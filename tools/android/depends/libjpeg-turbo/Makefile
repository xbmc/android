include ../Makefile.include

# lib name, version
LIBNAME=libjpeg-turbo
VERSION=1.2.0
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(LIBNAME)-$(VERSION).tar.gz
BASE_URL=http://sourceforge.net/projects/libjpeg-turbo/files/$(VERSION)
# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --with-jpeg8

CFLAGS+=-O3

LIBDYLIB=$(PLATFORM)/.libs/libjpeg.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -j 1 -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libjpeg.la $(PREFIX)/lib/libjpeg.so $(PREFIX)/lib/libxbjpeg.so $(PREFIX)/lib/libjpeg.so.8
	mv -f $(PREFIX)/lib/libjpeg.so.8.0.2 $(PREFIX)/lib/libxbjpeg.so
	ln -s $(PREFIX)/lib/libxbjpeg.so $(PREFIX)/lib/libjpeg.so
	rpl -e "libjpeg.so.8" "libxbjpeg.so" $(PREFIX)/lib/libxbjpeg.so
	readelf --dynamic $(PREFIX)/lib/libxbjpeg.so | grep ibrary
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
