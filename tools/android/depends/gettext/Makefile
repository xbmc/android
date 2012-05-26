include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=gettext
VERSION=0.17
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess build-aux/; \
    ./configure --prefix=$(PREFIX) --host=$(HOST) \
    --disable-csharp --disable-native-java --disable-java --without-emacs \
    --disable-libasprintf --disable-openmp \
    --with-included-gettext \
    --with-included-glib \
    --with-included-libcroco \
    --with-included-libxml \
    --without-git --without-cvs \
    gl_cv_header_working_stdint_h=yes 

LIBDYLIB=$(PLATFORM)/.libs/lib$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -p1 < ../01-gettext_pthread_cancel.patch
	cd $(PLATFORM); patch -p1 < ../02-gettext_msginit.patch
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libgettextpo.la $(PREFIX)/lib/libgettextpo.so $(PREFIX)/lib/libgettextpo.so.0
	mv -f $(PREFIX)/lib/libgettextpo.so.0.4.0 $(PREFIX)/lib/libgettextpo.so
	rpl -e "libgettextpo.so.0" "libgettextpo.so\x00\x00" $(PREFIX)/lib/libgettextpo.so
	rpl -e "libintl.so.8" "libintl.so\x00\x00" $(PREFIX)/lib/libgettextpo.so
	readelf --dynamic $(PREFIX)/lib/libgettextpo.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libgettextsrc.la $(PREFIX)/lib/libgettextsrc.so
	mv -f $(PREFIX)/lib/libgettextsrc-0.17.so $(PREFIX)/lib/libgettextsrc.so
	rpl -e "libgettextsrc-0.17.so" "libgettextsrc.so\x00\x00\x00\x00\x00" $(PREFIX)/lib/libgettextsrc.so
	rpl -e "libgettextlib-0.17.so" "libgettextlib.so\x00\x00\x00\x00\x00" $(PREFIX)/lib/libgettextsrc.so
	rpl -e "libintl.so.8" "libintl.so\x00\x00" $(PREFIX)/lib/libgettextsrc.so
	readelf --dynamic $(PREFIX)/lib/libgettextsrc.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libgettextlib.la $(PREFIX)/lib/libgettextlib.so
	mv -f $(PREFIX)/lib/libgettextlib-0.17.so $(PREFIX)/lib/libgettextlib.so
	rpl -e "libgettextlib-0.17.so" "libgettextlib.so\x00\x00\x00\x00\x00" $(PREFIX)/lib/libgettextlib.so
	rpl -e "libintl.so.8" "libintl.so\x00\x00" $(PREFIX)/lib/libgettextlib.so
	readelf --dynamic $(PREFIX)/lib/libgettextlib.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libintl.la $(PREFIX)/lib/libintl.so $(PREFIX)/lib/libintl.so.8
	mv -f $(PREFIX)/lib/libintl.so.8.0.2 $(PREFIX)/lib/libintl.so
	rpl -e "libintl.so.8" "libintl.so\x00\x00" $(PREFIX)/lib/libintl.so
	readelf --dynamic $(PREFIX)/lib/libintl.so | grep ibrary
	#
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

