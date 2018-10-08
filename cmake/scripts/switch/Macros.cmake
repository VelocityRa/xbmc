function(core_link_library lib wraplib)
  message("WARNING: core_link_library called (lib: ${lib}, wraplib: ${wraplib}")
  #set(export -Wl,--unresolved-symbols=ignore-all
  #           `cat ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/cores/dll-loader/exports/wrapper.def`
  #           ${CMAKE_BINARY_DIR}/${CORE_BUILD_DIR}/cores/dll-loader/exports/CMakeFiles/wrapper.dir/wrapper.c.o)
  set(check_arg "")
  if(TARGET ${lib})
    set(target ${lib})
    set(link_lib $<TARGET_FILE:${lib}>)
    set(check_arg ${ARGV2})
    set(data_arg  ${ARGV3})
  else()
    set(target ${ARGV2})
    set(link_lib ${lib})
    set(check_arg ${ARGV3})
    set(data_arg ${ARGV4})
  endif()

  string(REGEX REPLACE "[ ]+" ";" _flags "${CMAKE_SHARED_LINKER_FLAGS}")

endfunction()

function(find_soname lib)
  message("WARNING: find_soname called (lib: ${lib}")
  cmake_parse_arguments(arg "REQUIRED" "" "" ${ARGN})

  string(TOLOWER ${lib} liblow)
  if(${lib}_LDFLAGS)
    set(link_lib "${${lib}_LDFLAGS}")
  else()
    if(IS_ABSOLUTE "${${lib}_LIBRARIES}")
      set(link_lib "${${lib}_LIBRARIES}")
    else()
      set(link_lib -l${${lib}_LIBRARIES})
    endif()
  endif()
  execute_process(COMMAND ${CMAKE_C_COMPILER} -nostdlib -o /dev/null -Wl,-M ${link_lib}
                  COMMAND grep LOAD.*${liblow}
                  ERROR_QUIET
                  OUTPUT_VARIABLE ${lib}_FILENAME)
  string(REPLACE "LOAD " "" ${lib}_FILENAME "${${lib}_FILENAME}")
  string(STRIP "${${lib}_FILENAME}" ${lib}_FILENAME)
  if(NOT ${lib}_FILENAME)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -nostdlib -o /dev/null -Wl,-t ${link_lib}
                    ERROR_QUIET
                    OUTPUT_VARIABLE _TMP_FILENAME)
    string(REGEX MATCH ".*lib${liblow}.so" ${lib}_FILENAME ${_TMP_FILENAME})
  endif()
  if(${lib}_FILENAME)
    execute_process(COMMAND ${CMAKE_OBJDUMP} -p ${${lib}_FILENAME}
                    COMMAND grep SONAME.*${liblow}
                    ERROR_QUIET
                    OUTPUT_VARIABLE ${lib}_SONAME)
    string(REPLACE "SONAME " "" ${lib}_SONAME ${${lib}_SONAME})
    string(STRIP ${${lib}_SONAME} ${lib}_SONAME)
    if(VERBOSE)
      message(STATUS "${lib} soname: ${${lib}_SONAME}")
    endif()
    set(${lib}_SONAME ${${lib}_SONAME} PARENT_SCOPE)
  endif()
  if(arg_REQUIRED AND NOT ${lib}_SONAME)
    message(FATAL_ERROR "Could not find dynamically loadable library ${lib}")
  endif()
endfunction()
