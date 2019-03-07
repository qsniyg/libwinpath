#include "libwinpath.h"
#include "wine/common.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static int lookup_unix_name_utf8(char** dst, int pos, char* path, UINT disposition, BOOL check_case) {
  int path_len = strlen(path);
  int dst_len = path_len + 258;
  *dst = (char*)malloc(dst_len);
  WCHAR wchar_path[255] = {0};

  int new_path_len = ntdll_umbstowcs(0, path, path_len, wchar_path, 0);
  ntdll_umbstowcs(0, path, path_len, wchar_path, new_path_len);
  return lookup_unix_name(wchar_path, new_path_len, dst, dst_len, pos, disposition, check_case);
}

int libwinpath_getpath(char** dst, const char* path, int disposition) {
  int win_disposition = 0;
  switch (disposition) {
  case LIBWINPATH_FILE_OPEN:
    win_disposition = FILE_OPEN;
    break;
  case LIBWINPATH_FILE_CREATE:
    win_disposition = FILE_CREATE;
    break;
  case LIBWINPATH_FILE_OVERWRITE:
    win_disposition = FILE_OVERWRITE;
    break;
  }

  int ret = lookup_unix_name_utf8(dst, 0, (char*)path, win_disposition, 0);
  switch (ret) {
  case STATUS_SUCCESS:
    return 0;
  case STATUS_NO_SUCH_FILE:
  case STATUS_OBJECT_NAME_NOT_FOUND:
  case STATUS_OBJECT_PATH_NOT_FOUND:
    return -ENOENT;
  case STATUS_NO_MEMORY:
    return -ENOMEM;
  case STATUS_ACCESS_DENIED:
    return -EACCES;
  case STATUS_OBJECT_NAME_INVALID:
  case STATUS_OBJECT_PATH_SYNTAX_BAD:
  case STATUS_BAD_DEVICE_TYPE:
    return -EINVAL;
  default:
    return LIBWINPATH_UNKNOWN_ERROR;
  }
}
