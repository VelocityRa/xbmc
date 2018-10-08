include ../../Makefile.include
DEPS= ../../Makefile.include Makefile ID3v2Lookup.patch remove-boost.patch no-fileno.patch strdup.patch

LIBNAME=taglib
VERSION=1.11.1
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(LIBNAME)-$(VERSION).tar.gz

LIBDYLIB=$(PLATFORM)/build/$(LIBNAME)/libtag.a
all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); rm -rf build; mkdir -p build
	cd $(PLATFORM); patch -p1 -i ../ID3v2Lookup.patch
	cd $(PLATFORM); patch -p1 -i ../remove-boost.patch
ifeq ($(OS),switch)
	cd $(PLATFORM); patch -p0 < ../no-fileno.patch
	cd $(PLATFORM); patch -p0 < ../strdup.patch
endif
# HACK(velocity): don't use CMAKE var, the Toolchain.cmake arg (see Makefile.include) messes it up somehow
ifeq ($(OS),switch)
	cd $(PLATFORM)/build; $(NATIVEPREFIX)/bin/cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_LIBRARY_PATH=$(TOOLCHAIN)/lib/$(HOST) -DBUILD_SHARED_LIBS=OFF ..
else
	cd $(PLATFORM)/build; $(CMAKE) -DCMAKE_LIBRARY_PATH=$(TOOLCHAIN)/lib/$(HOST) -DBUILD_SHARED_LIBS=OFF ..
endif

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)/build
	touch $@
.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/build install
	rm -f $(PREFIX)/lib/libtag_c.*
	touch $@

clean:
	$(MAKE) -C $(PLATFORM)/build clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
