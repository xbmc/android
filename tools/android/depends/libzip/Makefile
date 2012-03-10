include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libzip
VERSION=0.10
SOURCE=$(LIBNAME)-$(VERSION)
BASE_URL=http://www.nih.at/libzip
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
        ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(SOURCE)/.libs/$(LIBNAME).dylib

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -j $(MAKE_JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	rm -f $(PREFIX)/lib/libzip.la $(PREFIX)/lib/libzip.so $(PREFIX)/lib/libzip.so.2
	mv -f $(PREFIX)/lib/libzip.so.2.1.0 $(PREFIX)/lib/libzip.so
	rpl -e "libzip.so.2" "libzip.so\x00\x00" $(PREFIX)/lib/libzip.so
	readelf --dynamic $(PREFIX)/lib/libzip.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
