#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/devkitA64/base_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
SOURCES		:=	source
DATA		:=	data
INCLUDES	:=  include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

CFLAGS	:=	-g -Wall -Werror \
			-ffunction-sections \
			-fdata-sections \
			$(ARCH) \
			$(BUILD_CFLAGS)

DEFINES_THREAD := -Dpthread_t=Thread* -Dpthread_mutex_t=void* -Dpthread_rmutex_t=RMutex -Dpthread_mutexattr_t='void*' -Dpthread_attr_t=int -Dpthread_cond_t=CondVar -Dpthread_condattr_t='int'
DEFINES := -DHAVE_THREADS -U__linux__ -U__linux -D_SYS__PTHREADTYPES_H_ $(DEFINES_THREAD)
CFLAGS	+=	$(INCLUDE) $(DEFINES)

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11

ASFLAGS	:=	-g $(ARCH)

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I. \
			-I/opt/devkitpro/libnx/include

.PHONY: clean all

#---------------------------------------------------------------------------------
all: lib/lib$(TARGET).a lib/lib$(TARGET)d.a

lib:
	@[ -d $@ ] || mkdir -p $@

release:
	@[ -d $@ ] || mkdir -p $@

debug:
	@[ -d $@ ] || mkdir -p $@

lib/lib$(TARGET).a : lib release $(SOURCES) $(INCLUDES)
	@$(MAKE) BUILD=release OUTPUT=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=1 -O2" \
	DEPSDIR=$(CURDIR)/release \
	--no-print-directory -C release \
	-f $(CURDIR)/Makefile

lib/lib$(TARGET)d.a : lib debug $(SOURCES) $(INCLUDES)
	@$(MAKE) BUILD=debug OUTPUT=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DDEBUG=1 -Og" \
	DEPSDIR=$(CURDIR)/debug \
	--no-print-directory -C debug \
	-f $(CURDIR)/Makefile

dist-bin: all
	@tar --exclude=*~ -cjf lib$(TARGET).tar.bz2 include lib

dist-src:
	@tar --exclude=*~ -cjf lib$(TARGET)-src.tar.bz2 include source Makefile

dist: dist-src dist-bin

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr release debug lib *.bz2

install:
	cp -f lib/lib$(TARGET).a /home/velo/switch-tools/kodi-deps/aarch64-none-elf-release/lib/libpthread.a
	cp -f lib/lib$(TARGET).a /home/velo/switch-tools/kodi-deps/aarch64-none-elf-release/lib/libpthreads.a
	cp -f lib/lib$(TARGET)d.a /home/velo/switch-tools/kodi-deps/aarch64-none-elf-release/lib/libpthreadd.a
	cp -f include/switch_pthread.h /home/velo/switch-tools/kodi-deps/aarch64-none-elf-release/include/pthread.h
#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT)	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES)

#---------------------------------------------------------------------------------
%_bin.h %.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

