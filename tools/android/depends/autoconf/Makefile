NATIVE_BUILD=1
include ../Makefile.include

# lib name, version
LIBNAME=autoconf
VERSION=2.68
BASE_URL=http://ftp.gnu.org/gnu/autoconf/
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX)

LIBDYLIB=$(PLATFORM)/bin/autoconf

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	-rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore

$(LIBDYLIB): $(PLATFORM)
	cd $(PLATFORM); $(CONFIGURE)
	make -j $(JOBS) -C $(PLATFORM)

.installed-$(PLATFORM):
	make -C $(PLATFORM) install
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
