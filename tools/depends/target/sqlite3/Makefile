include ../../Makefile.include
DEPS= ../../Makefile.include Makefile fix-32bits-on-64bits.patch sqlite3.c.patch switch.patch

# lib name, version
LIBNAME=sqlite
VERSION=3140200
SOURCE=$(LIBNAME)-autoconf-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
export CXXFLAGS+=-DSQLITE_ENABLE_COLUMN_METADATA=1
export CFLAGS+=-DSQLITE_TEMP_STORE=3 -DSQLITE_DEFAULT_MMAP_SIZE=0x10000000
export TCLLIBDIR=/dev/null
CONFIGURE=cp -f $(CONFIG_SUB) $(CONFIG_GUESS) .; \
          ./configure --prefix=$(PREFIX) --disable-shared \
  --disable-threadsafe --disable-tcl --disable-readline --disable-dynamic-extensions \

LIBDYLIB=$(PLATFORM)/.libs/lib$(LIBNAME)3.a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
ifeq ($(OS),android)
	cd $(PLATFORM); patch -p0 < ../fix-32bits-on-64bits.patch
endif
# seems MAP_POPULATE is broken on aarch64
ifneq ($(OS),$(filter $(OS),android switch))
	cd $(PLATFORM); patch -p1 < ../sqlite3.c.patch
endif
ifeq ($(OS),switch)
	cd $(PLATFORM); patch -p0 < ../switch.patch
endif
	# do not build the program sqlite3
	sed -ie "s|bin_PROGRAMS = sqlite3||" "$(PLATFORM)/Makefile.am";
	cd $(PLATFORM); $(AUTORECONF) -vif
	cd $(PLATFORM); $(CONFIGURE)

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

