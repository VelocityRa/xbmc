set(SOURCES DVDFactoryInputStream.cpp
            DVDInputStream.cpp
            DVDInputStreamFFmpeg.cpp
            DVDInputStreamFile.cpp
            DVDInputStreamMemory.cpp
            DVDInputStreamStack.cpp
            DVDStateSerializer.cpp
            InputStreamAddon.cpp
            InputStreamMultiSource.cpp
            InputStreamPVRBase.cpp
            InputStreamPVRChannel.cpp
            InputStreamPVRRecording.cpp)

set(HEADERS DVDFactoryInputStream.h
            DVDInputStream.h
            DVDInputStreamFFmpeg.h
            DVDInputStreamFile.h
            DVDInputStreamMemory.h
            DVDInputStreamNavigator.h
            DVDInputStreamStack.h
            DVDStateSerializer.h
            InputStreamAddon.h
            InputStreamMultiStreams.h
            InputStreamMultiSource.h
            InputStreamPVRBase.h
            InputStreamPVRChannel.h
            InputStreamPVRRecording.h)

if(NOT SWITCH)
  list(APPEND SOURCES DVDInputStreamNavigator.cpp)
  list(APPEND HEADERS DllDvdNav.h)
endif()
          
if(BLURAY_FOUND)
  list(APPEND SOURCES DVDInputStreamBluray.cpp)
  list(APPEND HEADERS DVDInputStreamBluray.h)
endif()

core_add_library(dvdinputstreams)
