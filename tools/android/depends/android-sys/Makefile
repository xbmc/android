include ../Makefile.include
DEPS = ../Makefile.include Makefile iomx.cpp

LIBNAME=android-sys
VERSION=0.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

PRIVATE_LIBDIR=$(PLATFORM)/android-libs
PRIVATE_LIBS =$(PRIVATE_LIBDIR)/libutils.so
PRIVATE_LIBS+=$(PRIVATE_LIBDIR)/libmedia.so
PRIVATE_LIBS+=$(PRIVATE_LIBDIR)/libbinder.so
PRIVATE_LIBS+=$(PRIVATE_LIBDIR)/libstagefright.so

CXXFLAGS =$(PLATFORM_FLAGS) -I$(PREFIX)/include -fexceptions
LDFLAGS += -L$(PRIVATE_LIBDIR) -lgcc -lstagefright -lmedia -lutils -lbinder

SDK_PLATFORM_10=android-10
SDK_PLATFORM_13=android-13
SDK_PLATFORM_14=android-14

INCLUDES_10=$(INCLUDES)
INCLUDES_10+=-I$(PLATFORM)/$(SDK_PLATFORM_10)/system/core/include
INCLUDES_10+=-I$(PLATFORM)/$(SDK_PLATFORM_10)/frameworks/base/include
INCLUDES_10+=-I$(PLATFORM)/$(SDK_PLATFORM_10)/frameworks/base/include/media/stagefright/openmax
INCLUDES_10+=-I$(PLATFORM)/$(SDK_PLATFORM_10)/hardware/libhardware/include

INCLUDES_13=$(INCLUDES)
INCLUDES_13+=-I$(PLATFORM)/$(SDK_PLATFORM_13)/system/core/include
INCLUDES_13+=-I$(PLATFORM)/$(SDK_PLATFORM_13)/frameworks/base/include
INCLUDES_13+=-I$(PLATFORM)/$(SDK_PLATFORM_13)/frameworks/base/native/include
INCLUDES_13+=-I$(PLATFORM)/$(SDK_PLATFORM_13)/hardware/libhardware/include
INCLUDES_13+=-I$(PLATFORM)/$(SDK_PLATFORM_13)/frameworks/base/include/media/stagefright/openmax
INCLUDES_13+=-I$(PLATFORM)/$(SDK_PLATFORM_13)/hardware/libhardware/include

INCLUDES_14=$(INCLUDES)
INCLUDES_14+=-I$(PLATFORM)/$(SDK_PLATFORM_14)/system/core/include
INCLUDES_14+=-I$(PLATFORM)/$(SDK_PLATFORM_14)/frameworks/base/include
INCLUDES_14+=-I$(PLATFORM)/$(SDK_PLATFORM_14)/frameworks/base/native/include
INCLUDES_14+=-I$(PLATFORM)/$(SDK_PLATFORM_14)/hardware/libhardware/include
INCLUDES_14+=-I$(PLATFORM)/$(SDK_PLATFORM_14)/frameworks/base/include/media/stagefright/openmax
INCLUDES_14+=-I$(PLATFORM)/$(SDK_PLATFORM_14)/hardware/libhardware/include

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)

$(PRIVATE_LIBS:.so=.symbols): $(PLATFORM)

$(PRIVATE_LIBS:.so=.c): $(PRIVATE_LIBS:.so=.symbols)
	rm -f $@
	for s in `cat $(@:.c=.symbols)`; do echo "void $$s() {}" >> $@; done

$(PRIVATE_LIBS): $(PRIVATE_LIBS:.so=.c)
	$(CC) $(@:.so=.c) -shared -o $@

$(PLATFORM)/$(SDK_PLATFORM_10)/libiomx-10.so: iomx.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES_10) -shared -o $@ iomx.cpp $(LDFLAGS)

$(PLATFORM)/$(SDK_PLATFORM_13)/libiomx-13.so: iomx.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES_13) -shared -o $@ iomx.cpp $(LDFLAGS)

$(PLATFORM)/$(SDK_PLATFORM_14)/libiomx-14.so: iomx.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES_14) -shared -o $@ iomx.cpp $(LDFLAGS)

.installed-$(PLATFORM): $(PRIVATE_LIBS) $(PLATFORM)/$(SDK_PLATFORM_10)/libiomx-10.so $(PLATFORM)/$(SDK_PLATFORM_13)/libiomx-13.so $(PLATFORM)/$(SDK_PLATFORM_14)/libiomx-14.so
	mkdir -p $(PREFIX)/include/$(SDK_PLATFORM)
	mkdir -p $(PREFIX)/lib/$(SDK_PLATFORM)
	cp -f $(PLATFORM)/$(SDK_PLATFORM_10)/libiomx-10.so $(PREFIX)/lib/$(SDK_PLATFORM)/
	cp -f $(PLATFORM)/$(SDK_PLATFORM_13)/libiomx-13.so $(PREFIX)/lib/$(SDK_PLATFORM)/
	cp -f $(PLATFORM)/$(SDK_PLATFORM_14)/libiomx-14.so $(PREFIX)/lib/$(SDK_PLATFORM)/
	cp -f $(PLATFORM)/$(SDK_PLATFORM)/frameworks/base/include/media/stagefright/openmax/* $(PREFIX)/include/$(SDK_PLATFORM)/
	touch $@

clean:
	rm -f .installed-$(PLATFORM) $(PLATFORM)/libiomx.so $(PRIVATE_LIBS) $(PRIVATE_LIBS:.so=.c)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
