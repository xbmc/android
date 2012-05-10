include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=Python
VERSION=2.6.5
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2
CWD=$(shell pwd)
HOSTPYTHONDIR=$(CWD)/$(PLATFORM)/hostpython

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) --build=x86_64-linux-gnu \
	  --disable-toolbox-glue --disable-framework

CONFIGURE_NATIVE= CFLAGS="" CXXFLAGS="" LDFLAGS="" CPPFLAGS="" \
  CPP="cpp" \
  CC="gcc" \
  CXX="g++" \
  LD="ld" \
  ./configure --prefix=$(HOSTPYTHONDIR) --disable-shared --disable-toolbox-glue --disable-framework

LIBDYLIB=$(PLATFORM)/libpython2.6.a

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(LIBDYLIB): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); patch -Np1 -i ../Python-2.6-ctypes.patch
	cd $(PLATFORM); mkdir -p $(HOSTPYTHONDIR)
	cd $(PLATFORM); $(CONFIGURE_NATIVE)
	cd $(PLATFORM); make -j4 libpython2.6.a
	cd $(PLATFORM); ranlib libpython2.6.a
	cd $(PLATFORM); make -j4 python Parser/pgen
	cd $(PLATFORM); make install
	cd $(PLATFORM); cp Parser/pgen $(HOSTPYTHONDIR)/bin
	cd $(PLATFORM); make distclean
	cd $(PLATFORM); patch -Np1 -i ../Python-2.6.5-xcompile.patch
	cd $(PLATFORM); autoreconf
	cd $(PLATFORM); $(CONFIGURE)
	cd $(PLATFORM); rm pyconfig.h
	cd $(PLATFORM); rm Modules/config.c
	cd $(PLATFORM); patch -Np1 -i ../Python-2.6.2-android.patch
	cp modules.setup $(PLATFORM)/Modules/Setup
	make -j 4 -C $(PLATFORM) HOSTPYTHON=$(HOSTPYTHONDIR)/bin/python HOSTPGEN=$(HOSTPYTHONDIR)/bin/pgen CROSS_COMPILE_TARGET=yes libpython2.6.a

.installed:
	make -C $(PLATFORM) HOSTPYTHON=$(HOSTPYTHONDIR)/bin/python HOSTPGEN=$(HOSTPYTHONDIR)/bin/pgen CROSS_COMPILE_TARGET=yes install
	find $(PREFIX)/lib/python2.6 -type f -name "*.py" -delete
	find $(PREFIX)/lib/python2.6 -type f -name "*.pyc" -delete
	#
	touch $@

clean:
	rm -rf $(PLATFORM) .installed

distclean::
	rm -rf $(PLATFORM) .installed
