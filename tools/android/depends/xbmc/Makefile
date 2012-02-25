include ../Makefile.include
include ../android-sdk.mk

SOURCE=../../../../

export PYTHON_VERSION=2.6
export PYTHON_LDFLAGS=-L$(PREFIX)/usr/lib -lpython$(PYTHON_VERSION) -llibc -ldl -lutil -lm
export PYTHON_CPPFLAGS=-I$(PREFIX)/usr/include/python$(PYTHON_VERSION)
export PYTHON_SITE_PKG=$(PREFIX)/usr/lib/python$(PYTHON_VERSION)/site-packages
export PYTHON_NOVERSIONCHECK=no-check
export USE_TEXTUREPACKER_NATIVE_ROOT=/usr

# configuration settings
export CXXFLAGS+=-isystem$(SDKSTAGE)/include/linux_user
CONFIGURE=./configure --prefix=$(PREFIX) --host=$(HOST) \
  --enable-neon --enable-gles --enable-debug \
  --disable-sdl --disable-x11 --disable-xrandr --disable-samba --disable-dvdcss \
  --disable-optical-drive --disable-webserver \
  --with-platform=android

all: configure

configure:
	cd $(SOURCE); ./bootstrap
	cd $(SOURCE); $(CONFIGURE)
	cd $(SOURCE); make -j $(JOBS)
	cd $(SOURCE); make install
clean:
	cd $(SOURCE); make clean

distclean:
	cd $(SOURCE); make clean
