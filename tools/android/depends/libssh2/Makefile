include ../Makefile.include

# lib name, version
LIBNAME=libssh2
VERSION=1.2.7
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --with-openssl

SHELL := /bin/bash
RM    := rm -rf

LIBDYLIB=$(PLATFORM)/src/.libs/$(LIBNAME).a

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
	rm -f $(PREFIX)/lib/libssh2.la $(PREFIX)/lib/libssh2.so $(PREFIX)/lib/libssh2.so.1
	mv -f $(PREFIX)/lib/libssh2.so.1.0.1 $(PREFIX)/lib/libssh2.so
	rpl -e "libssh2.so.1" "libssh2.so\x00\x00" $(PREFIX)/lib/libssh2.so
	readelf --dynamic $(PREFIX)/lib/libssh2.so | grep ibrary
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

