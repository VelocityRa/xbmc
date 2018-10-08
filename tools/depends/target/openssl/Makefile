include ../../Makefile.include
DEPS= ../../Makefile.include Makefile switch.patch u128-rename.patch

# lib name, version
LIBNAME=openssl
VERSION=1.1.0h
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=MACHINE=$(PLATFORM) ./config no-shared zlib no-asm --prefix=$(PREFIX) --with-zlib-include=$(PREFIX)/include --with-zlib-lib=$(PREFIX)/lib
ifeq ($(OS), android)
  CONFIGURE=./Configure no-shared zlib --prefix=$(PREFIX) --with-zlib-include=$(PREFIX)/include --with-zlib-lib=$(PREFIX)/lib linux-generic32 -D__ANDROID_API__=$(NDK_LEVEL)
endif
ifeq ($(OS), ios)
  CONFIGURE=./Configure iphoneos-cross no-shared zlib no-asm --prefix=$(PREFIX)
endif
ifeq ($(OS), osx)
  ifeq ($(CPU),x86_64)
    CONFIGURE=./Configure darwin64-$(CPU)-cc zlib no-asm no-shared --prefix=$(PREFIX)
  else
    CONFIGURE=./Configure darwin-$(CPU)-cc zlib no-asm no-shared --prefix=$(PREFIX)
  endif
endif
ifeq ($(OS),switch)
    CONFIGURE=./Configure nintendo-switch zlib no-asm no-shared no-threads no-ocsp no-sock no-tls no-ui no-unit-test no-whirlpool no-autoalginit no-stdio --prefix=$(PREFIX)
endif

LIBDYLIB=$(PLATFORM)/libssl.a

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
ifeq ($(OS),switch)
		cd $(PLATFORM); patch -p0 < ../switch.patch
		cd $(PLATFORM); patch -p0 < ../u128-rename.patch
endif
	cd $(PLATFORM); AR="$(AR)" CFLAGS="$(CFLAGS)" CC="$(CC)" RANLIB=$(RANLIB) $(CONFIGURE)
# ifeq ($(OS),osx)
# 		sed -ie "s|CC= /usr/bin/gcc-4.2|CC= $(CC)|" "$(PLATFORM)/Makefile";
# 		sed -E -ie "s|^CFLAGS=-|CFLAGS=$(CFLAGS) -|" "$(PLATFORM)/Makefile";
# endif
	# for iphoneos-cross config a sysroot argument is added
	# however sysroot already set in Makefile.include, so remove this
# ifeq ($(OS),ios)
# 		sed -E -ie "s|^CFLAGS=-|CFLAGS=$(CFLAGS) -|" "$(PLATFORM)/Makefile";
# 		sed -ie "s|-isysroot \$$(CROSS_TOP)/SDKs/\$$(CROSS_SDK) ||" "$(PLATFORM)/Makefile";
# 		sed -ie "s|static volatile sig_atomic_t intr_signal;|static volatile intr_signal;|" "$(PLATFORM)/crypto/ui/ui_openssl.c";
# endif
	sed -ie "s|apps test||" "$(PLATFORM)/Makefile";

# $(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
# 	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
# 	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
# ifeq ($(OS),"switch")
# 	  cd $(PLATFORM); patch -p0 < ../switch.patch
# endif
# 	cd $(PLATFORM); AR="$(AR)" CFLAGS="$(CFLAGS)" CC="$(CC)" RANLIB=$(RANLIB) $(CONFIGURE)
# ifeq ($(OS),osx)
# 		sed -ie "s|CC= /usr/bin/gcc-4.2|CC= $(CC)|" "$(PLATFORM)/Makefile";
# 		sed -E -ie "s|^CFLAGS=-|CFLAGS=$(CFLAGS) -|" "$(PLATFORM)/Makefile";
# endif
# 	# for iphoneos-cross config a sysroot argument is added
# 	# however sysroot already set in Makefile.include, so remove this
# ifeq ($(OS),ios)
# 		sed -E -ie "s|^CFLAGS=-|CFLAGS=$(CFLAGS) -|" "$(PLATFORM)/Makefile";
# 		sed -ie "s|-isysroot \$$(CROSS_TOP)/SDKs/\$$(CROSS_SDK) ||" "$(PLATFORM)/Makefile";
# 		sed -ie "s|static volatile sig_atomic_t intr_signal;|static volatile intr_signal;|" "$(PLATFORM)/crypto/ui/ui_openssl.c";
# endif
# 	sed -ie "s|apps test||" "$(PLATFORM)/Makefile";


$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)
	touch $@

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM) install_sw
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

