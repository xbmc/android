include ../Makefile.include

# lib name, version
LIBNAME=libtool
VERSION=2.2.6
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE)a.tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) \
  --disable-shared

LIBDYLIB=$(PLATFORM)/bin/libtool

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	-rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -rf $(PREFIX)/lib/libltdl.a $(PREFIX)/lib/libltdl.la
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
