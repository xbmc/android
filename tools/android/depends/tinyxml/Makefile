include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=tinyxml
VERSION=2.6.2_2
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) --disable-shared


LIBDYLIB=$(PLATFORM)/src/.libs/libtinyxml.so.0.0.0

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); autoreconf -vif
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j 1 -C $(PLATFORM)/src

.installed:
	make -C $(PLATFORM)/src install
	touch $@

clean:
	rm -rf $(PLATFORM) .installed

distclean::
	rm -rf $(PLATFORM) .installed

