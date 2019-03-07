#include "libwinpath.h"
#include <stdio.h>
#include <errno.h>

char libwinpath_disable_wrapper = 0;

FILE* libwinpath_fopen(const char* filename, const char* mode) {
  if (libwinpath_disable_wrapper)
    return fopen(filename, mode);

  char* dst;

  int disposition = LIBWINPATH_FILE_OPEN;
  if (mode) {
    if (mode[0] == 'w')
      disposition = LIBWINPATH_FILE_CREATE;
    else if (mode[0] == 'a')
      disposition = LIBWINPATH_FILE_OVERWRITE;
  }

  if ((errno = -libwinpath_getpath(&dst, filename, disposition))) {
    return NULL;
  }

  return fopen(dst, mode);
}
