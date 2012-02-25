include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=mysql
VERSION=5.1.55
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --build=$(BUILD) \
	--enable-thread-safe-client --with-extra-charsets=complex \
	--with-named-thread-libs=-libc --with-named-curses-libs=-lncurses \
	--without-readline --without-libedit \
	--without-server --without-bench --without-docs --without-man

LIBDYLIB=$(SOURCE)/$(LIBNAME)/.libs/$(LIBNAME).so

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	patch -Np0 -i 01-mysqlclient-cross-compile.patch
	patch -Np0 -i 02-mysqlclient-ios.patch
	patch -Np0 -i 03-mysqlclient-android.patch
	cd $(SOURCE); autoconf
	cd $(SOURCE); $(CONFIGURE) 

$(LIBDYLIB): $(SOURCE)
	make -j $(JOBS) -C $(SOURCE)

.installed:
	make -C $(SOURCE) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

