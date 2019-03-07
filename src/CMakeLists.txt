set(SOURCES
  wine/casemap.c
  wine/string.c
  wine/path.c
  path.c
  wrapper.c)

add_library(winpath ${SOURCES})
