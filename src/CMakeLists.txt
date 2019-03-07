set(SOURCES
  wine/casemap.c
  wine/string.c
  wine/path.c
  path.c
  wrapper.c)

add_library(winpath ${SOURCES})

add_library(winpath_inject SHARED ${SOURCES})
target_compile_definitions(winpath_inject PUBLIC -DLIBWINPATH_INJECT)
target_link_libraries(winpath_inject dl)
