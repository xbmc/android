include ../Makefile.include

# lib name, version
# lib name, version
LIBNAME=pkg-config
VERSION=0.23
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --enable-indirect-deps --with-pc-path=$(PREFIX)/lib/pkgconfig:$(PREFIX)/share/pkgconfig

LIBDYLIB=$(PLATFORM)/bin/pkg-config

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	-rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed-$(PLATFORM):
	make -C $(PLATFORM) install
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
