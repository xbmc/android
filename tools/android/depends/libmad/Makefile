include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libmad
VERSION=0.15.1b
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=rm config.sub config.guess; automake -ac; ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(SOURCE)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); patch -p1 < ../01-libmad-pkgconfig.patch
	cd $(SOURCE); sed -i '/-fforce-mem/d' configure; $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -j $(JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	rm -f $(PREFIX)/lib/libmad.la $(PREFIX)/lib/libmad.so $(PREFIX)/lib/libmad.so.0
	mv -f $(PREFIX)/lib/libmad.so.0.2.1 $(PREFIX)/lib/libmad.so
	rpl -e "libmad.so.0" "libmad.so\x00\x00" $(PREFIX)/lib/libmad.so
	readelf --dynamic $(PREFIX)/lib/libmad.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

