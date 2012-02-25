include ../Makefile.include

# lib name, version
LIBNAME=autoconf
VERSION=2.68
BASE_URL=http://ftp.gnu.org/gnu/autoconf/
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
export PREFIX=$(XBMCPREFIX)
export PATH:=$(PREFIX)/bin:$(PATH)
CONFIGURE=./configure --prefix=$(PREFIX)

LIBDYLIB=$(SOURCE)/bin/autoconf

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	-rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore

$(LIBDYLIB): $(SOURCE)
	cd $(SOURCE); $(CONFIGURE)
	make -j $(JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
