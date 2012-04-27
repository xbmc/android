include ../Makefile.include
include ../android-sdk.mk

#Headers Only!
LIBNAME=boost
VERSION=1_44_0
SOURCE=$(LIBNAME)_$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

.installed: $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cat user-config.jam-android.in > $(SOURCE)/tools/build/v2/user-config.jam
	cd $(SOURCE); patch -p1 < ../add-arm-mem-barrier.patch
	cd $(SOURCE); patch -p1 < ../fix-deprecated-swp.patch
	cd $(SOURCE); ./bootstrap.sh --prefix=$(PREFIX)
	cd $(SOURCE); ./bjam --prefix=$(PREFIX) toolset=gcc --toolset-root=$(TOOLCHAIN) --architecture=arm --disable-icu \
	--without-date_time --without-filesystem --without-graph --without-graph_parallel --without-iostreams --without-math --without-mpi --without-program_options \
--without-python --without-random --without-regex --without-serialization --without-signals --without-system --without-test --without-thread --without-wave install 
	touch .installed

clean:
	rm -rf $(SOURCE) .installed

distclean::
	rm -rf $(SOURCE) .installed
