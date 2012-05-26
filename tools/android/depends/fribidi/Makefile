include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=fribidi
VERSION=0.10.9
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-docs --enable-malloc gl_cv_header_working_stdint_h=yes

LIBDYLIB=$(PLATFORM)/.libs/lib$(LIBNAME).so

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
	rm -f $(PREFIX)/lib/libfribidi.la $(PREFIX)/lib/libfribidi.so $(PREFIX)/lib/libfribidi.so.0
	mv -f $(PREFIX)/lib/libfribidi.so.0.0.0 $(PREFIX)/lib/libfribidi.so
	rpl -e "libfribidi.so.0" "libfribidi.so\x00\x00" $(PREFIX)/lib/libfribidi.so
	readelf --dynamic $(PREFIX)/lib/libfribidi.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

