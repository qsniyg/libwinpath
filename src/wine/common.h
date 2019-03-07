// This file is independent from WINE
#include <wchar.h>

typedef unsigned char BOOLEAN;
typedef BOOLEAN BOOL;
typedef char CHAR;
typedef CHAR *PCHAR;
typedef unsigned short USHORT;
typedef USHORT WCHAR;
typedef WCHAR* PWSTR;
typedef unsigned int NTSTATUS;
typedef unsigned int DWORD;
typedef DWORD UINT;

#define STATUS_SUCCESS 0
#define TRUE 1
#define FALSE 0
#define FILE_OPEN 1
#define FILE_CREATE 2
#define FILE_OVERWRITE 4
#define STATUS_NO_SUCH_FILE 0xC000000F
#define STATUS_NO_MEMORY 0xC0000017
#define STATUS_ACCESS_DENIED 0xC0000022
#define STATUS_OBJECT_NAME_INVALID 0xC0000033
#define STATUS_OBJECT_NAME_NOT_FOUND 0xC0000034
#define STATUS_OBJECT_NAME_COLLISION 0xC0000035
#define STATUS_OBJECT_PATH_NOT_FOUND 0xC000003A
#define STATUS_OBJECT_PATH_SYNTAX_BAD 0xC000003B
#define STATUS_BAD_DEVICE_TYPE 0xC00000CB

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _STRING {
  USHORT Length;
  USHORT MaximumLength;
  PCHAR  Buffer;
} STRING, ANSI_STRING, OEM_STRING, *POEM_STRING;


int memicmpW( const WCHAR *str1, const WCHAR *str2, int n );
int ntdll_umbstowcs(DWORD flags, const char* src, int srclen, WCHAR* dst, int dstlen);
int ntdll_wcstoumbs(DWORD flags, const WCHAR* src, int srclen, char* dst, int dstlen,
                    const char* defchar, int *used );
NTSTATUS lookup_unix_name( const WCHAR *name, int name_len, char **buffer, int unix_len, int pos,
                           UINT disposition, BOOLEAN check_case );
