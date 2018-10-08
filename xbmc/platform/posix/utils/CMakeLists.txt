set(SOURCES PosixInterfaceForCLog.cpp)

set(HEADERS FileHandle.h
            PosixInterfaceForCLog.h)

if(NOT SWITCH)
  list(APPEND SOURCES Mmap.cpp
                      SharedMemory.cpp)
  list(APPEND HEADERS Mmap.h
                      SharedMemory.h)
endif()

core_add_library(platform_posix_utils)
