add_subdirectory(libuemf)
add_subdirectory(libcroco)
add_subdirectory(libdepixelize)
add_subdirectory(adaptagrams)
add_subdirectory(autotrace)

if(WITH_INTERNAL_2GEOM)
    set(2GEOM_BUILD_SHARED ${BUILD_SHARED_LIBS} CACHE BOOL "")
    add_subdirectory(2geom)
endif()

if(WITH_INTERNAL_CAIRO)
    include(ExternalProject)
    if(WIN32)
    execute_process(
      COMMAND cygpath "${CMAKE_CURRENT_SOURCE_DIR}/cairo/autogen.sh"
      OUTPUT_VARIABLE AUTOGEN_PATH
      )
  set(CAIRO_CONF mintty.exe -w hide /bin/env MSYSTEM=$ENV{MSYSTEM} LDFLAGS=-lssp /bin/bash -l ${AUTOGEN_PATH} --prefix=${CMAKE_INSTALL_PREFIX} --disable-trace --disable-script-interpreter --libdir=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} && make clean)
  else()
      set(CAIRO_CONF "${CMAKE_CURRENT_SOURCE_DIR}/cairo/autogen.sh" --prefix=${CMAKE_INSTALL_PREFIX} --disable-trace --disable-script-interpreter --libdir=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} && make clean)
  endif()
    ExternalProject_Add(cairo_ext
        PREFIX cairo
        GIT_REPOSITORY https://gitlab.freedesktop.org/cairo/cairo.git
        GIT_TAG 1.17.6
        GIT_SHALLOW TRUE
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cairo
        PATCH_COMMAND git checkout ${CMAKE_CURRENT_SOURCE_DIR}/cairo/src && git apply --unsafe-paths --directory ${CMAKE_CURRENT_SOURCE_DIR}/cairo/ ${CMAKE_SOURCE_DIR}/packaging/cairo.patch
        BUILD_COMMAND make -j16
        CONFIGURE_COMMAND ${CAIRO_CONF}
        )
endif()
