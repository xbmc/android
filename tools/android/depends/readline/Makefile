include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=readline
VERSION=6.0
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess support/; ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(PLATFORM)/shlib/(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	-rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -Np1 -i ../01-readline-setpwent.patch
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libreadline.la $(PREFIX)/lib/libreadline.so $(PREFIX)/lib/libreadline.so.6
	mv -f $(PREFIX)/lib/libreadline.so.6.0 $(PREFIX)/lib/libreadline.so
	rpl -e "libreadline.so.6" "libreadline.so\x00\x00" $(PREFIX)/lib/libreadline.so
	readelf --dynamic $(PREFIX)/lib/libreadline.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libhistory.la $(PREFIX)/lib/libhistory.so $(PREFIX)/lib/libhistory.so.6
	mv -f $(PREFIX)/lib/libhistory.so.6.0 $(PREFIX)/lib/libhistory.so
	rpl -e "libhistory.so.6" "libhistory.so\x00\x00" $(PREFIX)/lib/libhistory.so
	readelf --dynamic $(PREFIX)/lib/libhistory.so | grep ibrary
	#
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

