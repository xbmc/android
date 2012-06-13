include ../Makefile.include

# lib name, version
LIBNAME=tiff
VERSION=3.8.2
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess config/; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-shared

LIBDYLIB=$(PLATFORM)/libtiff/.libs/lib$(LIBNAME).a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

