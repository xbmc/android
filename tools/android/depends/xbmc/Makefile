include ../Makefile.include
include ../android-sdk.mk

SOURCE=../../../../

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
