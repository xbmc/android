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
	rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	echo 'using gcc : : gcc : ' > $(PLATFORM)/tools/build/v2/user-config.jam
	echo '	<cflags>"-fvisibility-inlines-hidden -DBOOST_AC_USE_PTHREADS -DBOOST_SP_USE_PTHREADS"' >> $(PLATFORM)/tools/build/v2/user-config.jam
	echo '  <cxxflags>"-fvisibility-inlines-hidden -DBOOST_AC_USE_PTHREADS -DBOOST_SP_USE_PTHREADS"' >> $(PLATFORM)/tools/build/v2/user-config.jam
	echo '	;' >> $(PLATFORM)/tools/build/v2/user-config.jam
	cd $(PLATFORM); patch -p1 < ../add-arm-mem-barrier.patch
	cd $(PLATFORM); patch -p1 < ../fix-deprecated-swp.patch
	cd $(PLATFORM); ./bootstrap.sh --prefix=$(PREFIX)
	cd $(PLATFORM); ./bjam --prefix=$(PREFIX) toolset=gcc --toolset-root=$(TOOLCHAIN)/$(HOST)/bin --disable-icu \
	--without-date_time --without-filesystem --without-graph --without-graph_parallel --without-iostreams --without-math --without-mpi --without-program_options \
--without-python --without-random --without-regex --without-serialization --without-signals --without-system --without-test --without-thread --without-wave install 
	touch .installed

clean:
	rm -rf $(PLATFORM) .installed

distclean::
	rm -rf $(PLATFORM) .installed
