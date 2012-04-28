include ../Makefile.include
include ../android-sdk.mk

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

LIBDYLIB=$(SOURCE)/.libs/lib$(LIBNAME).so

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
	make -j 1 -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	rm -f $(PREFIX)/lib/libjpeg.la $(PREFIX)/lib/libjpeg.so $(PREFIX)/lib/libxbjpeg.so $(PREFIX)/lib/libjpeg.so.8
	mv -f $(PREFIX)/lib/libjpeg.so.8.0.2 $(PREFIX)/lib/libxbjpeg.so
	ln -s $(PREFIX)/lib/libxbjpeg.so $(PREFIX)/lib/libjpeg.so
	rpl -e "libjpeg.so.8" "libxbjpeg.so" $(PREFIX)/lib/libxbjpeg.so
	readelf --dynamic $(PREFIX)/lib/libxbjpeg.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
