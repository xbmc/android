include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libid3tag
VERSION=0.15.1b
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings

CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) \

LIBDYLIB=$(PLATFORM)/.libs/libid3tag.so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j 1 -C $(PLATFORM)

.installed-$(PLATFORM):
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libid3tag.la $(PREFIX)/lib/libid3tag.so $(PREFIX)/lib/libid3tag.so.0
	mv -f $(PREFIX)/lib/libid3tag.so.0.3.0 $(PREFIX)/lib/libid3tag.so
	rpl -e "libid3tag.so.0" "libid3tag.so\x00\x00" $(PREFIX)/lib/libid3tag.so
	readelf --dynamic $(PREFIX)/lib/libid3tag.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
