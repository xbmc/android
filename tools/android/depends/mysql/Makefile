include ../Makefile.include

# lib name, version
LIBNAME=mysql
VERSION=5.1.55
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --build=$(BUILD) \
	--enable-thread-safe-client --with-extra-charsets=complex \
	--with-named-thread-libs=-lc --with-named-curses-libs=-lncurses \
	--without-readline --without-libedit \
	--without-server --without-bench --without-docs --without-man

LIBDYLIB=$(PLATFORM)/lib$(LIBNAME)/.libs/lib$(LIBNAME)client.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -Np1 -i ../01-mysqlclient-cross-compile.patch
	cd $(PLATFORM); patch -Np1 -i ../02-mysqlclient-ios.patch
	cd $(PLATFORM); patch -Np1 -i ../03-mysqlclient-android.patch
	cd $(PLATFORM); autoconf
	cd $(PLATFORM); $(CONFIGURE) 

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)/include
	$(MAKE) -C $(PLATFORM)/libmysql

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/libmysql install
	$(MAKE) -C $(PLATFORM)/scripts/ install-binSCRIPTS
	$(MAKE) -C $(PLATFORM)/include/ install
	rm -f $(PREFIX)/lib/mysql/libmysqlclient.la $(PREFIX)/lib/mysql/libmysqlclient.so $(PREFIX)/lib/mysql/libmysqlclient.so.16
	mv -f $(PREFIX)/lib/mysql/libmysqlclient.so.16.0.0 $(PREFIX)/lib/mysql/libmysqlclient.so
	rpl -e "libmysqlclient.so.16" "libmysqlclient.so\x00\x00\x00" $(PREFIX)/lib/mysql/libmysqlclient.so
	readelf --dynamic $(PREFIX)/lib/mysql/libmysqlclient.so | grep ibrary
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

