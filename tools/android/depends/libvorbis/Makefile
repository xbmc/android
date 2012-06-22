include ../Makefile.include
DEPS= ../Makefile.include Makefile

# lib name, version
LIBNAME=libvorbis
VERSION=1.2.3
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

export LIBS=-lm
# configuration settings
CONFIGURE=cp -f $(PREFIX)/share/automake-1.11/config.sub $(PREFIX)/share/automake-1.11/config.guess .; \
          ./configure --prefix=$(PREFIX) --host=$(HOST) --disable-oggtest --disable-docs --disable-examples

LIBDYLIB=$(PLATFORM)/lib/.libs/$(LIBNAME).a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	sed -ie "s|SUBDIRS = m4 include vq lib examples test doc|SUBDIRS = m4 include lib|" "$(PLATFORM)/Makefile.in"
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	rm -f $(PREFIX)/lib/libvorbis.la $(PREFIX)/lib/libvorbis.so $(PREFIX)/lib/libvorbis.so.0
	mv -f $(PREFIX)/lib/libvorbis.so.0.4.3 $(PREFIX)/lib/libvorbis.so
	rpl -e "libvorbis.so.0" "libvorbis.so\x00\x00" $(PREFIX)/lib/libvorbis.so
	-$(READELF) --dynamic $(PREFIX)/lib/libvorbis.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libvorbisfile.la $(PREFIX)/lib/libvorbisfile.so $(PREFIX)/lib/libvorbisfile.so.3
	mv -f $(PREFIX)/lib/libvorbisfile.so.3.3.2 $(PREFIX)/lib/libvorbisfile.so
	rpl -e "libvorbisfile.so.3" "libvorbisfile.so\x00\x00" $(PREFIX)/lib/libvorbisfile.so
	rpl -e "libvorbis.so.0" "libvorbis.so\x00\x00" $(PREFIX)/lib/libvorbisfile.so
	-$(READELF) --dynamic $(PREFIX)/lib/libvorbisfile.so | grep ibrary
	#
	rm -f $(PREFIX)/lib/libvorbisenc.la $(PREFIX)/lib/libvorbisenc.so $(PREFIX)/lib/libvorbisenc.so.2
	mv -f $(PREFIX)/lib/libvorbisenc.so.2.0.6 $(PREFIX)/lib/libvorbisenc.so
	rpl -e "libvorbisenc.so.2" "libvorbisenc.so\x00\x00" $(PREFIX)/lib/libvorbisenc.so
	rpl -e "libvorbis.so.0" "libvorbis.so\x00\x00" $(PREFIX)/lib/libvorbisenc.so
	-$(READELF) --dynamic $(PREFIX)/lib/libvorbisenc.so | grep ibrary
	#
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

