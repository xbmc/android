include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=freetype
VERSION=2.3.9
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

# configuration settings
# force using internal libtool
export LIBTOOL=builds/unix/libtool
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess builds/unix; \
          ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(PLATFORM)/objs/.libs/lib$(LIBNAME).so

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
	make -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libfreetype.la $(PREFIX)/lib/libfreetype.so $(PREFIX)/lib/libfreetype.so.6
	mv -f $(PREFIX)/lib/libfreetype.so.6.3.20 $(PREFIX)/lib/libfreetype.so
	rpl -e "libfreetype.so.6" "libfreetype.so\x00\x00" $(PREFIX)/lib/libfreetype.so
	readelf --dynamic $(PREFIX)/lib/libfreetype.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

