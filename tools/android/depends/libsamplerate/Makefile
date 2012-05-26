include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libsamplerate
VERSION=0.1.7
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess Cfg; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-sndfile --disable-fftw

LIBDYLIB=$(PLATFORM)/.libs/$(LIBNAME).so

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
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libsamplerate.la $(PREFIX)/lib/libsamplerate.so $(PREFIX)/lib/libsamplerate.so.0
	mv -f $(PREFIX)/lib/libsamplerate.so.0.1.7 $(PREFIX)/lib/libsamplerate.so
	rpl -e "libsamplerate.so.0" "libsamplerate.so\x00\x00" $(PREFIX)/lib/libsamplerate.so
	readelf --dynamic $(PREFIX)/lib/libsamplerate.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

