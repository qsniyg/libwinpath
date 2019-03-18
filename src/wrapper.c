#include "libwinpath.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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

INJECT_ORIG(__fxstatat, int, int ver, int dirfd, const char *pathname, struct stat *statbuf, int flags);
INJECT_ORIG1(__fxstatat, ver, dirfd, pathname, statbuf, flags);
INJECT(__fxstatat, int, int ver, int dirfd, const char *pathname, struct stat *statbuf, int flags);
INJECT1(__fxstatat, ver, dirfd, pathname, statbuf, flags);

INJECT_ORIG(faccessat, int, int fd, const char *file, int mode, int flag);
INJECT_ORIG1(faccessat, fd, file, mode, flag);
INJECT(faccessat, int, int fd, const char *file, int mode, int flag);
INJECT1(faccessat, fd, file, mode, flag);

INJECT_ORIG(unlink, int, const char *path);INJECT_ORIG1(unlink, path);
INJECT(unlink, int, const char *path);INJECT1(unlink, path);

INJECT_ORIG(rename, int, const char *old, const char *new);INJECT_ORIG1(rename, old, new);
INJECT(rename, int, const char *old, const char *new);INJECT1(rename, old, new);

INJECT_ORIG(renameat, int, int oldfd, const char *old, int newfd, const char *new);
INJECT_ORIG1(renameat, oldfd, old, newfd, new);
INJECT(renameat, int, int oldfd, const char *old, int newfd, const char *new);
INJECT1(renameat, oldfd, old, newfd, new);

INJECT_ORIG(linkat, int, int fromfd, const char *from, int tofd, const char *to, int flags);
INJECT_ORIG1(linkat, fromfd, from, tofd, to, flags);
INJECT(linkat, int, int fromfd, const char *from, int tofd, const char *to, int flags);
INJECT1(linkat, fromfd, from, tofd, to, flags);

INJECT_ORIG(symlinkat, int, const char *from, int fd, const char *to);INJECT_ORIG1(symlinkat, from, fd, to);
INJECT(symlinkat, int, const char *from, int fd, const char *to);INJECT1(symlinkat, from, fd, to);

INJECT_ORIG(mkdir, int, const char *path, mode_t mode);INJECT_ORIG1(mkdir, path, mode);
INJECT(mkdir, int, const char *path, mode_t mode);INJECT1(mkdir, path, mode);

#ifndef LIBWINPATH_NO_GNULIB
int
renameatu (int fd1, char const *src, int fd2, char const *dst,
           unsigned int flags);

INJECT_ORIG(renameatu, int, int fd1, char const *src, int fd2, char const* dst, unsigned int flags);
INJECT_ORIG1(renameatu, fd1, src, fd2, dst, flags);
INJECT(renameatu, int, int fd1, char const *src, int fd2, char const* dst, unsigned int flags);
INJECT1(renameatu, fd1, src, fd2, dst, flags);
#endif

#define WRAP(x, ret, bad, patharg, ...)                \
  ret libwinpath_##x(__VA_ARGS__) {\
    char* NEWPATH;\
    if (!(NEWPATH = libwinpath_getpath_errno(patharg, LIBWINPATH_FILE_ANY))){ \
      return bad;}
#define WRAP1(x, ...) return original_##x(__VA_ARGS__);}

#define WRAPSD(x, ret, bad, srcarg, dstarg, ...)       \
  ret libwinpath_##x(__VA_ARGS__) {\
    char *NEWSRC, *NEWDST;                 \
    if (!(NEWSRC = libwinpath_getpath_errno(srcarg, LIBWINPATH_FILE_ANY))){ \
      return bad;}                                                 \
    if (!(NEWDST = libwinpath_getpath_errno(dstarg, LIBWINPATH_FILE_CREATE))){ \
      return bad;}
#define WRAPSD1(x, ...) return original_##x(__VA_ARGS__);}

WRAP(__xstat, int, -1, path, int ver, const char *path, struct stat *buf);WRAP1(__xstat, ver, NEWPATH, buf);
WRAP(__lxstat, int, -1, path, int ver, const char *path, struct stat *buf);WRAP1(__lxstat, ver, NEWPATH, buf);
WRAP(__fxstatat, int, -1, pathname, int ver, int dirfd, const char *pathname, struct stat *statbuf, int flags);
WRAP1(__fxstatat, ver, dirfd, NEWPATH, statbuf, flags);
WRAP(faccessat, int, -1, file, int fd, const char* file, int mode, int flag);
WRAP1(faccessat, fd, NEWPATH, mode, flag);
WRAP(mkdir, int, -1, path, const char *path, mode_t mode);WRAP1(mkdir, NEWPATH, mode);

WRAP(unlink, int, -1, path, const char *path);WRAP1(unlink, NEWPATH);
WRAPSD(rename, int, -1, old, new, const char *old, const char* new);WRAPSD1(rename, NEWSRC, NEWDST);

WRAPSD(renameat, int, -1, old, new, int oldfd, const char *old, int newfd, const char *new);
WRAPSD1(renameat, oldfd, NEWSRC, newfd, NEWDST);

WRAPSD(linkat, int, -1, from, to, int fromfd, const char *from, int tofd, const char *to, int flags);
WRAPSD1(linkat, fromfd, NEWSRC, tofd, NEWDST, flags);

WRAPSD(symlinkat, int, -1, from, to, const char *from, int fd, const char *to);
WRAPSD1(symlinkat, NEWSRC, fd, NEWDST);

#ifndef LIBWINPATH_NO_GNULIB
WRAPSD(renameatu, int, -1, src, dst, int fd1, char const *src, int fd2, char const* dst, unsigned int flags);
WRAPSD1(renameatu, fd1, NEWSRC, fd2, NEWDST, flags);
#endif

int libwinpath_open(const char *pathname, int flags, ...) {
  if (libwinpath_disable_wrapper)
    return original_open(pathname, flags);

  char* dst;

  int disposition = LIBWINPATH_FILE_OPEN;
  if (flags & O_CREAT)
    //disposition = LIBWINPATH_FILE_CREATE;
    disposition = LIBWINPATH_FILE_ANY;
  else if (flags & O_WRONLY) {
    disposition = LIBWINPATH_FILE_ANY;
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
