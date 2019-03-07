#include "libwinpath.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char libwinpath_disable_wrapper = 0;


#ifdef LIBWINPATH_INJECT
#define __USE_GNU
#include <dlfcn.h>

typedef int (*orig_open_f_type)(const char *pathname, int flags);
static int original_open(const char *pathname, int flags) {
  orig_open_f_type orig_open;
  orig_open = (orig_open_f_type)dlsym(RTLD_NEXT, "open");
  return orig_open(pathname,flags);
}

int open(const char* pathname, int flags, ...) {
  return libwinpath_open(pathname, flags);
}
#else
#define original_open(...) open(__VA_ARGS__)
#endif

int libwinpath_open(const char *pathname, int flags, ...) {
  if (libwinpath_disable_wrapper)
    return original_open(pathname, flags);

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

  return original_open(dst, flags);
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
