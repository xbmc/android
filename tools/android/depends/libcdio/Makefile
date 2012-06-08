include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libcdio
VERSION=0.82
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
BASE_URL=http://ftp.gnu.org/gnu/libcdio

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST)  --without-cd-drive --without-cd-info --without-cd-paranoia \
          --without-cdda-player --without-cd-read --without-iso-info --without-iso-read --disable-example-progs \
          --disable-cpp-progs --without-cd-drive --without-cd-info --without-cd-paranoia --without-cdda-player \
          --without-cd-read --without-iso-info --without-iso-read --disable-shared

LIBDYLIB=$(PLATFORM)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p0 < ../configure.patch
	cd $(PLATFORM); autoreconf -vif
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	cp $(PLATFORM)/include/cdio/cdtext.h $(PREFIX)/include/cdio/
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

