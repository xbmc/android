include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=sqlite
VERSION=3.6.11
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
export CXXFLAGS+=-DSQLITE_ENABLE_COLUMN_METADATA=1
export TCLLIBDIR=/dev/null
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --enable-shared \
  --enable-threadsafe --disable-tcl --disable-readline \

LIBDYLIB=$(SOURCE)/.libs/lib$(LIBNAME).so

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
	rm -f $(PREFIX)/lib/libsqlite3.la $(PREFIX)/lib/libsqlite3.so $(PREFIX)/lib/libsqlite3.so.0
	mv -f $(PREFIX)/lib/libsqlite3.so.0.8.6 $(PREFIX)/lib/libsqlite3.so
	rpl -e "libsqlite3.so.0" "libsqlite3.so\x00\x00" $(PREFIX)/lib/libsqlite3.so
	readelf --dynamic $(PREFIX)/lib/libsqlite3.so | grep ibrary
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

