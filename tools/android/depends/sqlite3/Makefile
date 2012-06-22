include ../Makefile.include
DEPS= ../Makefile.include Makefile

# lib name, version
LIBNAME=sqlite
VERSION=3071000
SOURCE=$(LIBNAME)-autoconf-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
BASE_URL=http://www.sqlite.org

# configuration settings
export CXXFLAGS+=-DSQLITE_ENABLE_COLUMN_METADATA=1
export CFLAGS+=-DSQLITE_TEMP_STORE=3
export TCLLIBDIR=/dev/null
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-shared \
  --enable-threadsafe --disable-tcl --disable-readline \

LIBDYLIB=$(PLATFORM)/.libs/lib$(LIBNAME)3.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

