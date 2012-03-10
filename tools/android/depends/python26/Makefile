include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=Python
VERSION=2.6.5
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2
CWD=$(shell pwd)
HOSTPYTHONDIR=$(CWD)/$(SOURCE)/hostpython

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) --build=x86_64-linux-gnu \
	  --enable-shared --disable-toolbox-glue --disable-framework

CONFIGURE_NATIVE= CFLAGS="" CXXFLAGS="" LDFLAGS="" CPPFLAGS="" \
  CPP="cpp" \
  CC="gcc" \
  CXX="g++" \
  LD="ld" \
  ./configure --prefix=$(HOSTPYTHONDIR) --disable-shared --disable-toolbox-glue --disable-framework

LIBDYLIB=$(SOURCE)/libpython2.6.so

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(LIBDYLIB): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); patch -Np1 -i ../Python-2.6-ctypes.patch
	cd $(SOURCE); mkdir -p $(HOSTPYTHONDIR)
	cd $(SOURCE); $(CONFIGURE_NATIVE)
	cd $(SOURCE); make -j4 libpython2.6.a
	cd $(SOURCE); ranlib libpython2.6.a
	cd $(SOURCE); make -j4 python Parser/pgen
	cd $(SOURCE); make install
	cd $(SOURCE); cp Parser/pgen $(HOSTPYTHONDIR)/bin
	cd $(SOURCE); make distclean
	cd $(SOURCE); patch -Np1 -i ../Python-2.6.5-xcompile.patch
	cd $(SOURCE); autoreconf
	cd $(SOURCE); $(CONFIGURE)
	cd $(SOURCE); rm pyconfig.h
	cd $(SOURCE); rm Modules/config.c
	cd $(SOURCE); patch -Np1 -i ../Python-2.6.2-android.patch
	make -j 4 -C $(SOURCE) HOSTPYTHON=$(HOSTPYTHONDIR)/bin/python HOSTPGEN=$(HOSTPYTHONDIR)/bin/pgen CROSS_COMPILE_TARGET=yes

.installed:
	make -C $(SOURCE) HOSTPYTHON=$(HOSTPYTHONDIR)/bin/python HOSTPGEN=$(HOSTPYTHONDIR)/bin/pgen CROSS_COMPILE_TARGET=yes install
	find $(PREFIX)/lib/python2.6 -type f -name *.pyc -exec rm -f {} \;
	find $(PREFIX)/lib/python2.6 -type f -name *.pyo -exec rm -f {} \;
	#
	mv -f $(PREFIX)/lib/libpython2.6.so.1.0 $(PREFIX)/lib/libpython2.6.so
	rpl -e "libpython2.6.so.1.0" "libpython2.6.so\x00\x00\x00\x00" $(PREFIX)/lib/libpython2.6.so
	readelf --dynamic $(PREFIX)/lib/libpython2.6.so | grep ibrary
	#
	touch $@

clean:
	rm -rf $(SOURCE) .installed

distclean::
	rm -rf $(SOURCE) .installed
