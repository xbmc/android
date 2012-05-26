include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libiconv
VERSION=1.13.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
BASE_URL=http://ftp.gnu.org/pub/gnu/libiconv

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess build-aux/; \
          cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess libcharset/build-aux/; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) gl_cv_header_working_stdint_h=yes

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).so

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
	rm $(PREFIX)/lib/preloadable_libiconv.so
	rm -f $(PREFIX)/lib/libiconv.la $(PREFIX)/lib/libiconv.so $(PREFIX)/lib/libiconv.so.2
	mv -f $(PREFIX)/lib/libiconv.so.2.5.0 $(PREFIX)/lib/libiconv.so
	rpl -e "libiconv.so.2" "libiconv.so\x00\x00" $(PREFIX)/lib/libiconv.so
	readelf --dynamic $(PREFIX)/lib/libiconv.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libcharset.la $(PREFIX)/lib/libcharset.so $(PREFIX)/lib/libcharset.so.1
	mv -f $(PREFIX)/lib/libcharset.so.1.0.0 $(PREFIX)/lib/libcharset.so
	rpl -e "libcharset.so.1" "libcharset.so\x00\x00" $(PREFIX)/lib/libcharset.so
	readelf --dynamic $(PREFIX)/lib/libcharset.so | grep ibrary
	#
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

