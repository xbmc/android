#export ARCH_FLAGS=-march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__ -DANDROID
export ARCH_FLAGS=-march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__ -DANDROID
#export ARCH_FLAGS=-march=armv5te -mtune=xscale -msoft-float -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -DANDROID
export HOST=arm-linux-androideabi
export SYSROOT=$(TOOLCHAIN)/sysroot
export CROSSTOOLS=$(TOOLCHAIN)/bin/arm-linux-androideabi-
export LD=$(CROSSTOOLS)ld
export CC=$(CROSSTOOLS)gcc
export CXX=$(CROSSTOOLS)g++
export AR=$(CROSSTOOLS)ar
export AS=$(CROSSTOOLS)as
export NM=$(CROSSTOOLS)nm
export STRIP=$(CROSSTOOLS)strip
export RANLIB=$(CROSSTOOLS)ranlib
export OBJDUMP=$(CROSSTOOLS)objdump
export CFLAGS=$(ARCH_FLAGS) -I$(XBMCPREFIX)/include -fexceptions
export LDFLAGS=$(ARCH_FLAGS) -L$(XBMCPREFIX)/lib
export CPPFLAGS=$(ARCH_FLAGS) -I$(XBMCPREFIX)/include -fexceptions -frtti
export CXXFLAGS=$(ARCH_FLAGS) -I$(XBMCPREFIX)/include -fexceptions -frtti
export LIBS=-lstdc++
export PKG_CONFIG_PATH=$(XBMCPREFIX)/lib/pkgconfig
export PATH:=${XBMCPREFIX}/bin:${TOOLCHAIN}/bin:$(PATH)
export PREFIX=$(XBMCPREFIX)
