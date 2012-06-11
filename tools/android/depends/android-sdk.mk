export SYSROOT=$(TOOLCHAIN)/sysroot
export CROSSTOOLS=$(TOOLCHAIN)/bin/$(HOST)-
export LD=$(CROSSTOOLS)ld
export CC=$(CROSSTOOLS)gcc
export CXX=$(CROSSTOOLS)g++
export AR=$(CROSSTOOLS)ar
export AS=$(CROSSTOOLS)as
export NM=$(CROSSTOOLS)nm
export STRIP=$(CROSSTOOLS)strip
export RANLIB=$(CROSSTOOLS)ranlib
export OBJDUMP=$(CROSSTOOLS)objdump
export CFLAGS=$(PLATFORM_FLAGS) -I$(PREFIX)/include -fexceptions
export LDFLAGS=$(PLATFORM_FLAGS) -L$(PREFIX)/lib
export CPPFLAGS=$(PLATFORM_FLAGS) -I$(PREFIX)/include -fexceptions -frtti
export CXXFLAGS=$(PLATFORM_FLAGS) -I$(PREFIX)/include -fexceptions -frtti
export LIBS=-lstdc++

export PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig
export PKG_CONFIG=$(PREFIX)/bin/pkg-config

export AUTOMAKE=$(PREFIX)/bin/automake
export ACLOCAL=$(PREFIX)/bin/aclocal
export LIBTOOLIZE=$(PREFIX)/bin/libtoolize
export AUTORECONF=$(PREFIX)/bin/autoreconf
