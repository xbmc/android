include ../Makefile.include

APPNAME=yasm
VERSION=1.1.0
SOURCE=$(APPNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX)

APP=$(PLATFORM)/$(APPNAME)

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: $(APP) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE)
	-rm -rf $(PLATFORM)/*; mkdir $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(PLATFORM) > .gitignore
	cd $(PLATFORM); $(CONFIGURE)

$(APP): $(PLATFORM)
	make -j $(JOBS) -C $(PLATFORM)

.installed:
	make -C $(PLATFORM) install
	rm -rf $(PREFIX)/lib/libyasm.a
	touch $@

clean:
	make -C $(PLATFORM) clean
	rm  .installed

distclean::
	rm -rf $(PLATFORM) .installed

