include ../Makefile.include
DEPS= ../Makefile.include Makefile iomx.cpp

LIBNAME=android-sys
VERSION=0.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

PRIVATE_LIBDIR=$(PLATFORM)/android-libs
PRIVATE_LIBS =$(PRIVATE_LIBDIR)/libutils.so
PRIVATE_LIBS+=$(PRIVATE_LIBDIR)/libmedia.so
PRIVATE_LIBS+=$(PRIVATE_LIBDIR)/libbinder.so
PRIVATE_LIBS+=$(PRIVATE_LIBDIR)/libstagefright.so

LDFLAGS  =-nostdlib -Wl,-soname,$(notdir $@) -Wl,--gc-sections -Wl,-shared,-Bsymbolic -lc -lm -lstdc++
LDFLAGS += -L$(PRIVATE_LIBDIR) -lstagefright -lmedia -lutils -lbinder -llog -landroid
CXXFLAGS =$(PLATFORM_FLAGS) -I$(PREFIX)/include
INCLUDES+=-I$(PLATFORM)/android-10/system/core/include
INCLUDES+=-I$(PLATFORM)/android-10/frameworks/base/include
INCLUDES+=-I$(PLATFORM)/android-10/frameworks/base/include/media/stagefright/openmax


all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)

$(PRIVATE_LIBDIR)/%.c: $(PRIVATE_LIBDIR)/%.symbols
	rm -f $@
	for s in `cat $<`; do echo "void $$s() {}" >> $@; done

$(PRIVATE_LIBDIR)/%.so: $(PRIVATE_LIBDIR)/%.c
	$(CC) $< -shared -o $@ --sysroot=$(SYSROOT)

$(PLATFORM)/libiomx.so: $(PRIVATE_LIBS) iomx.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ iomx.cpp $(LDFLAGS)
	cp -f $@ $(PREFIX)/lib/

.installed-$(PLATFORM): $(PLATFORM) $(PLATFORM)/libiomx.so
	touch $@

clean:
	rm -f .installed-$(PLATFORM) $(PLATFORM)/libiomx.so

distclean:: clean
	rm -rf $(PLATFORM) $(PRIVATE_LIBDIR) .installed-$(PLATFORM)
