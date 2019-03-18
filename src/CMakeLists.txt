set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(SOURCES
  wine/casemap.c
  wine/string.c
  wine/path.c
  path.c
  wrapper.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

add_library(winpath ${SOURCES})

add_library(winpath_inject SHARED ${SOURCES})
target_compile_definitions(winpath_inject PUBLIC -DLIBWINPATH_INJECT)
target_link_libraries(winpath_inject dl)

configure_file("${CMAKE_SOURCE_DIR}/src/winpath.sh" "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/bin/winpath" COPYONLY)
file(COPY "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/bin/winpath"
  DESTINATION "${CMAKE_BINARY_DIR}/bin"
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(PROGRAMS winpath.sh DESTINATION bin RENAME winpath)
install(TARGETS winpath DESTINATION lib)
install(TARGETS winpath_inject DESTINATION lib)
