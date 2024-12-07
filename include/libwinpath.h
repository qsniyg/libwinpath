#ifndef __LIBWINPATH_H__
#define __LIBWINPATH_H__


#ifdef __cplusplus
extern "C" {
#endif

#if defined (LIBWINPATH_ENABLE_WRAPPER) && (!defined(_WIN32) || defined(LIBWINPATH_FORCE_WINDOWS))
#define _LIBWINPATH_ENABLE_WRAPPER
#endif

#define LIBWINPATH_FILE_OPEN 1
#define LIBWINPATH_FILE_CREATE 2
#define LIBWINPATH_FILE_OVERWRITE 4
#define LIBWINPATH_FILE_ANY 5
#define LIBWINPATH_UNKNOWN_ERROR -999

#include <stdio.h>
#include <sys/stat.h>

/* Set this to 1 to disable the wrapper during runtime */
extern char libwinpath_disable_wrapper;

/* Returns a case-sensitive path to `dst` from the case-insensitive path passed to `path` */
int libwinpath_getpath(char** dst, const char* path, int disposition);

/* Convenience function that sets errno, and will return NULL if there's an error */
char* libwinpath_getpath_errno(const char* path, int disposition);

/* Wrappers */
FILE* libwinpath_fopen(const char* filename, const char* mode);
int libwinpath_open(const char *pathname, int flags, ...);
int libwinpath_openat(int dirfd, const char *pathname, int flags, ...);
int libwinpath___xstat(int ver, const char *path, struct stat *buf);
int libwinpath___lxstat(int ver, const char *path, struct stat *buf);

#ifdef _LIBWINPATH_ENABLE_WRAPPER
#define fopen(...) libwinpath_fopen(__VA_ARGS__)
#define open(...) libwinpath_open(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif


#endif
