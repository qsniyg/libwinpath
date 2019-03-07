#include "libwinpath.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char libwinpath_disable_wrapper = 0;

int libwinpath_open(const char *pathname, int flags, mode_t mode) {
  if (libwinpath_disable_wrapper)
    return open(pathname, flags, mode);

  char* dst;

  int disposition = LIBWINPATH_FILE_OPEN;
  if (flags & O_CREAT)
    disposition = LIBWINPATH_FILE_CREATE;
  else if (flags & O_WRONLY) {
    disposition = LIBWINPATH_FILE_OVERWRITE;
  }

  if ((errno = -libwinpath_getpath(&dst, pathname, disposition))) {
    return -1;
  }

  return open(pathname, flags, mode);
}

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
