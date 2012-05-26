include ../Makefile.include
include ../android-sdk.mk

LIBDYLIB=libxbmc.so

all: $(PLATFORM)/$(LIBDYLIB) .installed

$(PLATFORM):
	mkdir -p $(PLATFORM)

$(PLATFORM)/$(LIBDYLIB): $(PLATFORM) dummy-libxbmc.c
	$(CC) -shared -o $(PLATFORM)/libxbmc.so dummy-libxbmc.c
	echo $(PLATFORM) > .gitignore

.installed: $(PLATFORM)/$(LIBDYLIB)
	mkdir -p  $(PREFIX)/lib/dummy-libxbmc
	cp $(PLATFORM)/libxbmc.so $(PREFIX)/lib/dummy-libxbmc
	touch $@

clean:
	rm -rf $(PLATFORM)
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed
