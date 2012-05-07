include Makefile.include

BUILDTOOLS = help2man autoconf automake libtool pkg-config yasm cmake

SUBDIRS = \
	pcre expat libiconv gettext readline sqlite3 libgpg-error \
	libgcrypt bzip2 liblzo2 libzip freetype2 fontconfig \
	openssl curl libssh2 \
	libjpeg-turbo tiff jasper libpng \
	libogg libvorbis libflac libid3tag libmad fribidi libmpeg2 \
	libass libsamplerate \
	libmodplug librtmp libxml2 yajl libmicrohttpd \
	python26 \
	boost xbmc \

.PHONY: $(BUILDTOOLS) $(SUBDIRS)

all: $(BUILDTOOLS) $(SUBDIRS)

$(BUILDTOOLS):
	$(MAKE) -C $@

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for d in $(BUILDTOOLS); do $(MAKE) -C $$d clean; done
	for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done

distclean::
	for d in $(BUILDTOOLS); do $(MAKE) -C $$d distclean; done
	for d in $(SUBDIRS); do $(MAKE) -C $$d distclean; done
	rm -rf $(PREFIX)/*
