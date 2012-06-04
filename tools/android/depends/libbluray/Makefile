include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libbluray
VERSION=0.2.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --exec-prefix=$(PREFIX) --host=$(HOST) \
  --disable-examples --disable-doxygen-doc

LIBDYLIB=$(PLATFORM)/src/.libs/libbluray.so.1.0.0

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	# libbluray has borked Makefile.am with respect to CFLAGS, remove the offending line
	sed -i -e "s|CFLAGS=|#CFLAGS=|" $(PLATFORM)/src/Makefile.am
	sed -i -e "s|CFLAGS=|#CFLAGS=|" $(PLATFORM)/src/examples/Makefile.am
	cd $(PLATFORM); ./bootstrap
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(MAKE_JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libbluray.la $(PREFIX)/lib/libbluray.so $(PREFIX)/lib/libbluray.so.1
	mv -f $(PREFIX)/lib/libbluray.so.1.0.0 $(PREFIX)/lib/libbluray.so
	rpl -e "libbluray.so.1" "libbluray.so\x00\x00" $(PREFIX)/lib/libbluray.so
	readelf --dynamic $(PREFIX)/lib/libbluray.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
