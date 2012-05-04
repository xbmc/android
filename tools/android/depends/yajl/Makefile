include ../Makefile.include
include ../android-sdk.mk

# lib name, version
LIBNAME=yajl
VERSION=2.0.1
SOURCE=lloyd-$(LIBNAME)-f4b2b1a
ARCHIVE=$(LIBNAME)-$(VERSION).tar.gz
BASE_URL=http://github.com/lloyd/yajl/tarball/2.0.1
CMAKE=$(PREFIX)/bin/cmake
# configuration settings
LIBDYLIB=$(PLATFORM)/build/$(LIBNAME)-$(VERSION)/lib/libyajl.so

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	sed -ie "s|ADD_CUSTOM_TARGET|#ADD_CUSTOM_TARGET|g" "$(PLATFORM)/CMakeLists.txt"
	sed -ie "s|./run_tests.sh|#./run_tests.sh|g" "$(PLATFORM)/CMakeLists.txt"
	sed -ie "s|WORKING_DIRECTORY|#WORKING_DIRECTORY|g" "$(PLATFORM)/CMakeLists.txt"
	sed -ie "s|ADD_SUBDIRECTORY(reformatter)|#ADD_SUBDIRECTORY(reformatter)|g" "$(PLATFORM)/CMakeLists.txt"
	cd $(PLATFORM); patch -p0 < ../libmath.patch
	cd $(PLATFORM); mkdir build; cd build; $(CMAKE) -DCMAKE_INSTALL_PREFIX='$(PREFIX)' ../

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)/build

.installed:
	make -C $(PLATFORM)/build install
	rm -f $(PREFIX)/lib/libyajl.la $(PREFIX)/lib/libyajl.so $(PREFIX)/lib/libyajl.so.2
	mv -f $(PREFIX)/lib/libyajl.so.2.0.1 $(PREFIX)/lib/libyajl.so
	rpl -e "libyajl.so.2" "libyajl.so\x00\x00" $(PREFIX)/lib/libyajl.so
	readelf --dynamic $(PREFIX)/lib/libyajl.so | grep SONAME
	touch $@

clean:
	make -C $(PLATFORM)/build clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed

