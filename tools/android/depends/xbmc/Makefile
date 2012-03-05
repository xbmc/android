-include ../Makefile.include
include ../android-sdk.mk

SOURCE=../../../../

CXXFLAGS+=-I$(NDKROOT)/sources/android/native_app_glue
LIBS+=-llog

export PYTHON_VERSION=2.6
export PYTHON_LDFLAGS=-L$(PREFIX)/lib -lpython$(PYTHON_VERSION) -lc -ldl -lm
export PYTHON_CPPFLAGS=-I$(PREFIX)/include/python$(PYTHON_VERSION)
export PYTHON_SITE_PKG=$(PREFIX)/lib/python$(PYTHON_VERSION)/site-packages
export PYTHON_NOVERSIONCHECK=no-check
export USE_TEXTUREPACKER_NATIVE_ROOT=/usr

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) \
  --disable-neon --enable-gles --enable-debug \
  --disable-sdl --disable-x11 --disable-xrandr --disable-samba --disable-dvdcss \
  --disable-mysql --disable-optical-drive --disable-webserver --disable-joystick \
  --with-platform=android


all: $(SOURCE)/libxbmc.so

$(SOURCE)/libxbmc.so: $(SOURCE)/Makefile
	cd $(SOURCE); make -j4 libxbmc.so

$(SOURCE)/Makefile: $(SOURCE)/configure
	cd $(SOURCE); $(CONFIGURE)

$(SOURCE)/configure: $(SDKROOT)/platforms $(NDKROOT)/ndk-build
	echo $(NDKROOT)
	cd $(SOURCE); ./bootstrap

../Makefile.include:
	$(error Please run edit and run setup-sdk.sh)

$(NDKROOT)/ndk-build:
	$(error NDKROOT is missing. Please run setup-sdk.sh or check Makefile.include)

$(SDKROOT)/platforms:
	$(error SDKROOT is missing. Please run setup-sdk.sh or check Makefile.include)

clean:
	cd $(SOURCE); make clean

distclean:
	cd $(SOURCE); make clean
