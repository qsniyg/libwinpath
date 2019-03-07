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

#define INJECT_ORIG(x, ret, ...)\
  typedef ret (*orig_##x##_f_type)(__VA_ARGS__);\
  ret original_##x(__VA_ARGS__) {\
  orig_##x##_f_type orig_##x = (orig_##x##_f_type) dlsym(RTLD_NEXT, #x);
#define INJECT_ORIG1(x, ...) return orig_##x(__VA_ARGS__);}

#define INJECT(x, ret, ...)\
  ret x(__VA_ARGS__) {
#define INJECT1(x, ...) return libwinpath_##x(__VA_ARGS__);}
#else
#define INJECT_ORIG(x, ret, ...)\
  ret original_##x(__VA_ARGS__) {
#define INJECT_ORIG1(x, ...) return x(__VA_ARGS__);}

#define INJECT(...)
#define INJECT1(...)
#endif

INJECT_ORIG(open, int, const char *pathname, int flags);INJECT_ORIG1(open, pathname, flags);
INJECT(open, int, const char *pathname, int flags, ...);INJECT1(open, pathname, flags);

INJECT_ORIG(__xstat, int, int ver, const char *path, struct stat *buf);INJECT_ORIG1(__xstat, ver, path, buf);
INJECT(__xstat, int, int ver, const char *path, struct stat *buf);INJECT1(__xstat, ver, path, buf);

INJECT_ORIG(__lxstat, int, int ver, const char *path, struct stat *buf);INJECT_ORIG1(__lxstat, ver, path, buf);
INJECT(__lxstat, int, int ver, const char *path, struct stat *buf);INJECT1(__lxstat, ver, path, buf);


#define WRAP(x, ret, bad, patharg, ...)                \
  ret libwinpath_##x(__VA_ARGS__) {\
    char* dst;\
    if (!(dst = libwinpath_getpath_errno(patharg, LIBWINPATH_FILE_OPEN))){ \
      return bad;}
#define WRAP1(x, ...) return original_##x(__VA_ARGS__);}

WRAP(__xstat, int, -1, path, int ver, const char *path, struct stat *buf);WRAP1(__xstat, ver, dst, buf);
WRAP(__lxstat, int, -1, path, int ver, const char *path, struct stat *buf);WRAP1(__lxstat, ver,dst, buf);

int libwinpath_open(const char *pathname, int flags, ...) {
  if (libwinpath_disable_wrapper)
    return original_open(pathname, flags);

  char* dst;

  int disposition = LIBWINPATH_FILE_OPEN;
  if (flags & O_CREAT)
    //disposition = LIBWINPATH_FILE_CREATE;
    disposition = LIBWINPATH_FILE_OVERWRITE;
  else if (flags & O_WRONLY) {
    disposition = LIBWINPATH_FILE_OVERWRITE;
  }

  if (!(dst = libwinpath_getpath_errno(pathname, disposition)))
    return -1;

  return original_open(dst, flags);
}

FILE* libwinpath_fopen(const char* filename, const char* mode) {
  if (libwinpath_disable_wrapper)
    return fopen(filename, mode);

  char* dst;

  int disposition = LIBWINPATH_FILE_OPEN;
  if (mode) {
    if (mode[0] == 'w')
      //disposition = LIBWINPATH_FILE_CREATE;
      disposition = LIBWINPATH_FILE_OVERWRITE;
    else if (mode[0] == 'a')
      disposition = LIBWINPATH_FILE_OVERWRITE;
  }

  if (!(dst = libwinpath_getpath_errno(filename, disposition)))
    return NULL;

  return fopen(dst, mode);
}
