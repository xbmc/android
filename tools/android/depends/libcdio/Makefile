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
          --without-cd-read --without-iso-info --without-iso-read

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

	mv $(PREFIX)/lib/libcdio.so.12.0.0 $(PREFIX)/lib/libcdio.so
	rpl -e "libcdio.so.12" "libcdio.so\x00\x00\x00" $(PREFIX)/lib/libcdio.so

	mv $(PREFIX)/lib/libudf.so.0.0.0 $(PREFIX)/lib/libudf.so
	rpl -e "libudf.so.0" "libudf.so\x00\x00" $(PREFIX)/lib/libudf.so
	rpl -e "libcdio.so.12" "libcdio.so\x00\x00\x00" $(PREFIX)/lib/libudf.so

	mv $(PREFIX)/lib/libcdio++.so.0.0.2 $(PREFIX)/lib/libcdio++.so
	rpl -e "libcdio++.so.0" "libcdio++.so\x00\x00" $(PREFIX)/lib/libcdio++.so
	rpl -e "libcdio.so.12" "libcdio.so\x00\x00\x00" $(PREFIX)/lib/libcdio++.so

	mv $(PREFIX)/lib/libiso9660.so.7.0.0 $(PREFIX)/lib/libiso9660.so
	rpl -e "libiso9660.so.7" "libiso9660.so\x00\x00" $(PREFIX)/lib/libiso9660.so
	rpl -e "libcdio.so.12" "libcdio.so\x00\x00\x00" $(PREFIX)/lib/libiso9660.so

	mv $(PREFIX)/lib/libiso9660++.so.0.0.0 $(PREFIX)/lib/libiso9660++.so
	rpl -e "libiso9660++.so.0" "libiso9660++.so\x00\x00" $(PREFIX)/lib/libiso9660++.so
	rpl -e "libcdio.so.12" "libcdio.so\x00\x00\x00" $(PREFIX)/lib/libiso9660++.so
	rpl -e "libiso9660.so.7" "libiso9660.so\x00\x00" $(PREFIX)/lib/libiso9660++.so

	readelf --dynamic $(PREFIX)/lib/libcdio.so | grep ibrary
	readelf --dynamic $(PREFIX)/lib/libudf.so | grep ibrary
	readelf --dynamic $(PREFIX)/lib/libcdio++.so | grep ibrary
	readelf --dynamic $(PREFIX)/lib/libiso9660.so | grep ibrary
	readelf --dynamic $(PREFIX)/lib/libiso9660++.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

