include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=libxml2
VERSION=2.7.6
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --without-python

LIBDYLIB=$(PLATFORM)/$(LIBNAME)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	sed -ie "s|runtest$$(EXEEXT)||" "$(PLATFORM)/Makefile.in"
	sed -ie "s|testrecurse$$(EXEEXT)||" "$(PLATFORM)/Makefile.in"
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libxml2.la $(PREFIX)/lib/libxml2.so $(PREFIX)/lib/libxml2.so.2
	mv -f $(PREFIX)/lib/libxml2.so.2.7.6 $(PREFIX)/lib/libxml2.so
	rpl -e "libxml2.so.2" "libxml2.so\x00\x00" $(PREFIX)/lib/libxml2.so
	readelf --dynamic $(PREFIX)/lib/libxml2.so | grep ibrary
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

