include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libgcrypt
VERSION=1.4.0
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST)
# ac_cv_sys_symbol_underscore=no \

ifeq ($(PLATFORM),x86)
CONFIGURE+=--disable-asm
endif

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p1 < ../gcrypt-disable-tests.patch
	cd $(PLATFORM); $(CONFIGURE)
	cd $(PLATFORM); patch -p1 < ../01-gcrypt-android-select.patch

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libgcrypt.la $(PREFIX)/lib/libgcrypt.so $(PREFIX)/lib/libgcrypt.so.11
	mv -f $(PREFIX)/lib/libgcrypt.so.11.4.3 $(PREFIX)/lib/libgcrypt.so
	rpl -e "libgcrypt.so.11" "libgcrypt.so\x00\x00\x00" $(PREFIX)/lib/libgcrypt.so
	readelf --dynamic $(PREFIX)/lib/libgcrypt.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
