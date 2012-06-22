include ../Makefile.include
DEPS= ../Makefile.include Makefile Python-2.6.2-android.patch \
      Python-2.6.5-xcompile.patch Python-2.6-ctypes.patch \
      Python-no-modules.patch modules.setup

# lib name, version
LIBNAME=Python
VERSION=2.6.5
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2
CWD=$(shell pwd)
HOSTPYTHONDIR=$(CWD)/$(PLATFORM)/hostpython

# configuration settings
CONFIGURE=OPT=-Os \
  LIBS=-lm \
  ./configure --prefix=$(PREFIX) --host=$(HOST) \
  --disable-toolbox-glue --disable-framework --with-system-ffi \
  ac_cv_func_gethostbyname_r=no ac_cv_header_sys_un_h=yes

CONFIGURE_NATIVE= CFLAGS="" CXXFLAGS="" LDFLAGS="" CPPFLAGS="" \
  CPP="cpp" \
  CC="gcc" \
  CXX="g++" \
  LD="ld" \
  RANLIB="ranlib" \
  AR="ar" \
  ./configure --prefix=$(HOSTPYTHONDIR) --disable-shared --disable-toolbox-glue --disable-framework

LIBDYLIB=$(PLATFORM)/libpython2.6.a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(LIBDYLIB): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); patch -Np1 -i ../Python-2.6-ctypes.patch
	cd $(PLATFORM); mkdir -p $(HOSTPYTHONDIR)
	cd $(PLATFORM); $(CONFIGURE_NATIVE)
	cd $(PLATFORM); $(MAKE) libpython2.6.a
	cd $(PLATFORM); ranlib libpython2.6.a
	cd $(PLATFORM); make python Parser/pgen
	cd $(PLATFORM); make install
	cd $(PLATFORM); cp Parser/pgen $(HOSTPYTHONDIR)/bin
	cd $(PLATFORM); make distclean
	cd $(PLATFORM); patch -Np1 -i ../Python-2.6.5-xcompile.patch
	cd $(PLATFORM); patch -Np1 -i ../Python-2.6.2-android.patch
	cd $(PLATFORM); patch -p0 < ../Python-no-modules.patch
	cd $(PLATFORM); sed -ie 's|-framework CoreFoundation||' configure.in
	cd $(PLATFORM); $(AUTORECONF)
	cd $(PLATFORM); $(CONFIGURE)
	cp modules.setup $(PLATFORM)/Modules/Setup
	$(MAKE) -C $(PLATFORM) HOSTPYTHON=$(HOSTPYTHONDIR)/bin/python HOSTPGEN=$(HOSTPYTHONDIR)/bin/pgen CROSS_COMPILE_TARGET=yes libpython2.6.a

.installed-$(PLATFORM): $(LIBDYLIB)
	make -C $(PLATFORM) HOSTPYTHON=$(HOSTPYTHONDIR)/bin/python HOSTPGEN=$(HOSTPYTHONDIR)/bin/pgen CROSS_COMPILE_TARGET=yes install
	find $(PREFIX)/lib/python2.6 -type f -name "*.py" -delete
	find $(PREFIX)/lib/python2.6 -type f -name "*.pyc" -delete
	#
	touch $@

clean:
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
