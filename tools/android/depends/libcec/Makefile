include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libcec
VERSION=1.5.2
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) \
          ac_cv_search_dlopen=yes

LIBDYLIB=$(PLATFORM)/src/lib/.libs/libcec.so.1.0.5

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p1 < ../android.patch
	cd $(PLATFORM); autoreconf -vif
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)/src/lib

.installed:
	make -C $(PLATFORM)/src/lib install
	rm -f $(PREFIX)/lib/libcec.la $(PREFIX)/lib/libcec.so $(PREFIX)/lib/libcec.so.1
	mv -f $(PREFIX)/lib/libcec.so.1.0.5 $(PREFIX)/lib/libcec.so
	rpl -e "libcec.so.1" "libcec.so\x00\x00" $(PREFIX)/lib/libcec.so
	readelf --dynamic $(PREFIX)/lib/libcec.so | grep ibrary
	touch $@

clean:
	rm -rf $(PLATFORM) .installed

distclean::
	rm -rf $(PLATFORM) .installed

