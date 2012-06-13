include ../Makefile.include

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
    --disable-shared gl_cv_header_working_stdint_h=yes 

LIBDYLIB=$(PLATFORM)/gettext-tools/src/.libs/libgettextsrc.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

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

.installed-$(PLATFORM): $(LIBDYLIB)
	make -C $(PLATFORM) install
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

