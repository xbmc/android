include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=expat
VERSION=2.0.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess ./conftools; ./configure --prefix=$(PREFIX) --host=$(HOST)

LIBDYLIB=$(SOURCE)/.libs/(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -j $(JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	rm -f $(PREFIX)/lib/libexpat.la $(PREFIX)/lib/libexpat.so $(PREFIX)/lib/libexpat.so.1
	mv -f $(PREFIX)/lib/libexpat.so.1.5.2 $(PREFIX)/lib/libexpat.so
	rpl -e "libexpat.so.1" "libexpat.so\x00\x00" $(PREFIX)/lib/libexpat.so
	readelf --dynamic $(PREFIX)/lib/libexpat.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

