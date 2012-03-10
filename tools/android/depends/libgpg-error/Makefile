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

LIBDYLIB=$(SOURCE)/$(LIBNAME)/.libs/$(LIBNAME).so

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
	make -j $(JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	rm -f $(PREFIX)/lib/libgpg-error.la $(PREFIX)/lib/libgpg-error.so $(PREFIX)/lib/libgpg-error.so.0
	mv -f $(PREFIX)/lib/libgpg-error.so.0.8.0 $(PREFIX)/lib/libgpg-error.so
	rpl -e "libgpg-error.so.0" "libgpg-error.so\x00\x00" $(PREFIX)/lib/libgpg-error.so
	readelf --dynamic $(PREFIX)/lib/libgpg-error.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
