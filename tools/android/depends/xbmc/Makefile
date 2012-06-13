-include ../Makefile.include

SOURCE=../../../../

export PYTHON_VERSION=2.6
export PYTHON_LDFLAGS=-L$(PREFIX)/lib -lpython$(PYTHON_VERSION) -lc -ldl -lm
export PYTHON_CPPFLAGS=-I$(PREFIX)/include/python$(PYTHON_VERSION)
export PYTHON_SITE_PKG=$(PREFIX)/lib/python$(PYTHON_VERSION)/site-packages
export PYTHON_NOVERSIONCHECK=no-check
export USE_TEXTUREPACKER_NATIVE_ROOT=/usr
export MYSQL_CONFIG=$(PREFIX)/bin/mysql_config

# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess build-aux/ ;\
  ./configure --prefix=$(PREFIX) --host=$(HOST) \
  --enable-neon --enable-gles --enable-debug \
  --disable-sdl --disable-x11 --disable-xrandr \
  --disable-optical-drive --disable-joystick --disable-optimizations \
  --enable-shared-lib


all: $(SOURCE)/libxbmc.so $(SOURCE)/libxbmcapp.so

$(SOURCE)/libxbmc.so: $(SOURCE)/Makefile
	cd $(SOURCE); $(MAKE)

$(SOURCE)/Makefile: $(SOURCE)/configure
	cd $(SOURCE); $(CONFIGURE)

$(SOURCE)/configure:
	cd $(SOURCE); ./bootstrap

../Makefile.include:
	$(error Please run edit and run setup-sdk.sh)

clean:
	cd $(SOURCE); $(MAKE) clean

distclean:
	cd $(SOURCE); $(MAKE) clean
