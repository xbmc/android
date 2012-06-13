include ../Makefile.include

# lib name, version
LIBNAME=yajl
VERSION=2.0.1
SOURCE=lloyd-$(LIBNAME)-f4b2b1a
ARCHIVE=$(LIBNAME)-$(VERSION).tar.gz
BASE_URL=http://github.com/lloyd/yajl/tarball/2.0.1
CMAKE=$(PREFIX)/bin/cmake
# configuration settings
LIBDYLIB=$(PLATFORM)/build/$(LIBNAME)-$(VERSION)/lib/libyajl.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed-$(PLATFORM)

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
	cd $(PLATFORM); patch -p0 < ../static.patch
	cd $(PLATFORM); mkdir build; cd build; BUILD_SHARED_LIBRARIES=0 CMAKE_BUILD_TYPE=MinSizeRel $(CMAKE) --debug-output -DCMAKE_INSTALL_PREFIX='$(PREFIX)' ../

$(LIBDYLIB): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)/build

.installed-$(PLATFORM): $(LIBDYLIB)
	make -C $(PLATFORM)/build install
	touch $@

clean:
	make -C $(PLATFORM)/build clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

