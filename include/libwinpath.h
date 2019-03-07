#ifndef __LIBWINPATH_H__
#define __LIBWINPATH_H__


#ifdef __cplusplus
extern "C" {
#endif

#define LIBWINPATH_FILE_OPEN 1
#define LIBWINPATH_FILE_CREATE 2
#define LIBWINPATH_FILE_OVERWRITE 4
#define LIBWINPATH_UNKNOWN_ERROR -999

#include <stdio.h>

/* Set this to 1 to disable the wrapper during runtime */
extern char libwinpath_disable_wrapper;

/* Returns a case-sensitive path to `dst` from the case-insensitive path passed to `path` */
int libwinpath_getpath(char** dst, const char* path, int disposition);

/* fopen wrapper */
FILE* libwinpath_fopen(const char* filename, const char* mode);

#ifdef LIBWINPATH_ENABLE_WRAPPER
#define fopen(...) libwinpath_fopen(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif


#endif
