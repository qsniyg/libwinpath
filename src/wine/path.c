#include "common.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/vfs.h>
#include <string.h>
#include <stdlib.h>

#ifdef LIBWINPATH_INJECT
int original___xstat(int ver, const char *path, struct stat *buf);
int original_stat(const char* path, struct stat* buf) {
  return original___xstat(_STAT_VER_LINUX, path, buf);
}
#else
#define original_stat(...) stat(__VA_ARGS__)
#endif

// TODO (This is not WINE's implementation)
BOOLEAN RtlIsNameLegalDOS8Dot3( const UNICODE_STRING *unicode,
                                OEM_STRING *oem, BOOLEAN *spaces ) {
  return 0;
}


//
// --- dlls/ntdll/directory.c ---
//

#define IS_SEPARATOR(ch)   ((ch) == '\\' || (ch) == '/')
#define MAX_DIR_ENTRY_LEN 255

static BOOLEAN get_dir_case_sensitivity_stat( const char *dir )
{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    struct statfs stfs;

    if (statfs( dir, &stfs ) == -1) return FALSE;
    /* Assume these file systems are always case insensitive on Mac OS.
     * For FreeBSD, only assume CIOPFS is case insensitive (AFAIK, Mac OS
     * is the only UNIX that supports case-insensitive lookup).
     */
    if (!strcmp( stfs.f_fstypename, "fusefs" ) &&
        !strncmp( stfs.f_mntfromname, "ciopfs", 5 ))
        return FALSE;
#ifdef __APPLE__
    if (!strcmp( stfs.f_fstypename, "msdos" ) ||
        !strcmp( stfs.f_fstypename, "cd9660" ) ||
        !strcmp( stfs.f_fstypename, "udf" ) ||
        !strcmp( stfs.f_fstypename, "ntfs" ) ||
        !strcmp( stfs.f_fstypename, "smbfs" ))
        return FALSE;
#ifdef _DARWIN_FEATURE_64_BIT_INODE
     if (!strcmp( stfs.f_fstypename, "hfs" ) && (stfs.f_fssubtype == 0 ||
                                                 stfs.f_fssubtype == 1 ||
                                                 stfs.f_fssubtype == 128))
        return FALSE;
#else
     /* The field says "reserved", but a quick look at the kernel source
      * tells us that this "reserved" field is really the same as the
      * "fssubtype" field from the inode64 structure (see munge_statfs()
      * in <xnu-source>/bsd/vfs/vfs_syscalls.c).
      */
     if (!strcmp( stfs.f_fstypename, "hfs" ) && (stfs.f_reserved1 == 0 ||
                                                 stfs.f_reserved1 == 1 ||
                                                 stfs.f_reserved1 == 128))
        return FALSE;
#endif
#endif
    return TRUE;

#elif defined(__NetBSD__)
    struct statvfs stfs;

    if (statvfs( dir, &stfs ) == -1) return FALSE;
    /* Only assume CIOPFS is case insensitive. */
    if (strcmp( stfs.f_fstypename, "fusefs" ) ||
        strncmp( stfs.f_mntfromname, "ciopfs", 5 ))
        return TRUE;
    return FALSE;

#elif defined(__linux__)
    struct statfs stfs;
    struct stat st;
    char *cifile;

    /* Only assume CIOPFS is case insensitive. */
    if (statfs( dir, &stfs ) == -1) return FALSE;
    if (stfs.f_type != 0x65735546 /* FUSE_SUPER_MAGIC */)
        return TRUE;
    /* Normally, we'd have to parse the mtab to find out exactly what
     * kind of FUSE FS this is. But, someone on wine-devel suggested
     * a shortcut. We'll stat a special file in the directory. If it's
     * there, we'll assume it's a CIOPFS, else not.
     * This will break if somebody puts a file named ".ciopfs" in a non-
     * CIOPFS directory.
     */
    cifile = malloc(strlen( dir )+sizeof("/.ciopfs"));
    if (!cifile) return TRUE;
    strcpy( cifile, dir );
    strcat( cifile, "/.ciopfs" );
    if (original_stat( cifile, &st ) == 0)
    {
        free(cifile);
        return FALSE;
    }
    free(cifile);
    return TRUE;
#else
    return TRUE;
#endif
}

static BOOLEAN get_dir_case_sensitivity( const char *dir )
{
  /*#if defined(HAVE_GETATTRLIST) && defined(ATTR_VOL_CAPABILITIES) &&  \
    defined(VOL_CAPABILITIES_FORMAT) && defined(VOL_CAP_FMT_CASE_SENSITIVE)
    int case_sensitive = get_dir_case_sensitivity_attr( dir );
    if (case_sensitive != -1) return case_sensitive;
    #endif*/
    return get_dir_case_sensitivity_stat( dir );
}

struct file_identity
{
    dev_t dev;
    ino_t ino;
};

static NTSTATUS find_file_in_dir( char *unix_name, int pos, const WCHAR *name, int length,
                                  BOOLEAN check_case, BOOLEAN *is_win_dir )
{
    WCHAR buffer[MAX_DIR_ENTRY_LEN];
    UNICODE_STRING str;
    BOOLEAN spaces, is_name_8_dot_3;
    DIR *dir;
    struct dirent *de;
    struct stat st;
    int ret, used_default;

    /* try a shortcut for this directory */

    // added this to support relative paths
    /*if (pos == 0 && unix_name[pos] != '/') {
      unix_name[pos++] = '.';
      }*/
    unix_name[pos++] = '/';
    ret = ntdll_wcstoumbs( 0, name, length, unix_name + pos, MAX_DIR_ENTRY_LEN,
                           NULL, &used_default );
    /* if we used the default char, the Unix name won't round trip properly back to Unicode */
    /* so it cannot match the file we are looking for */
    if (ret >= 0 && !used_default)
    {
        unix_name[pos + ret] = 0;
        if (!original_stat( unix_name, &st ))
        {
            //if (is_win_dir) *is_win_dir = is_same_file( &windir, &st );
            return STATUS_SUCCESS;
        }
    }
    if (check_case) goto not_found;  /* we want an exact match */

    if (pos > 1) unix_name[pos - 1] = 0;
    else unix_name[1] = 0;  /* keep the initial slash */

    /* check if it fits in 8.3 so that we don't look for short names if we won't need them */

    str.Buffer = (WCHAR *)name;
    str.Length = length * sizeof(WCHAR);
    str.MaximumLength = str.Length;
    is_name_8_dot_3 = RtlIsNameLegalDOS8Dot3( &str, NULL, &spaces ) && !spaces;
#if 1//!defined(VFAT_IOCTL_READDIR_BOTH)
    is_name_8_dot_3 = is_name_8_dot_3 && length >= 8 && name[4] == '~';
#endif

    if (!is_name_8_dot_3 && !get_dir_case_sensitivity( unix_name )) goto not_found;

    /* now look for it through the directory */

#if 0//defined(VFAT_IOCTL_READDIR_BOTH)
    if (is_name_8_dot_3)
    {
        int fd = open( unix_name, O_RDONLY | O_DIRECTORY );
        if (fd != -1)
        {
            KERNEL_DIRENT *kde;

            RtlEnterCriticalSection( &dir_section );
            if ((kde = start_vfat_ioctl( fd )))
            {
                unix_name[pos - 1] = '/';
                while (kde[0].d_reclen)
                {
                    /* make sure names are null-terminated to work around an x86-64 kernel bug */
                    size_t len = min(kde[0].d_reclen, sizeof(kde[0].d_name) - 1 );
                    kde[0].d_name[len] = 0;
                    len = min(kde[1].d_reclen, sizeof(kde[1].d_name) - 1 );
                    kde[1].d_name[len] = 0;

                    if (kde[1].d_name[0])
                    {
                        ret = ntdll_umbstowcs( 0, kde[1].d_name, strlen(kde[1].d_name),
                                               buffer, MAX_DIR_ENTRY_LEN );
                        if (ret == length && !memicmpW( buffer, name, length))
                        {
                            strcpy( unix_name + pos, kde[1].d_name );
                            RtlLeaveCriticalSection( &dir_section );
                            close( fd );
                            goto success;
                        }
                    }
                    ret = ntdll_umbstowcs( 0, kde[0].d_name, strlen(kde[0].d_name),
                                           buffer, MAX_DIR_ENTRY_LEN );
                    if (ret == length && !memicmpW( buffer, name, length))
                    {
                        strcpy( unix_name + pos,
                                kde[1].d_name[0] ? kde[1].d_name : kde[0].d_name );
                        RtlLeaveCriticalSection( &dir_section );
                        close( fd );
                        goto success;
                    }
                    if (ioctl( fd, VFAT_IOCTL_READDIR_BOTH, (long)kde ) == -1)
                    {
                        RtlLeaveCriticalSection( &dir_section );
                        close( fd );
                        goto not_found;
                    }
                }
            }
            RtlLeaveCriticalSection( &dir_section );
            close( fd );
        }
        /* fall through to normal handling */
    }
#endif /* VFAT_IOCTL_READDIR_BOTH */

    if (!(dir = opendir( unix_name )))
    {
        if (errno == ENOENT || 1) return STATUS_OBJECT_PATH_NOT_FOUND;
        //else return FILE_GetNtStatus();
    }
    unix_name[pos - 1] = '/';
    str.Buffer = buffer;
    str.MaximumLength = sizeof(buffer);
    while ((de = readdir( dir )))
    {
        ret = ntdll_umbstowcs( 0, de->d_name, strlen(de->d_name), buffer, MAX_DIR_ENTRY_LEN );
        if (ret == length && !memicmpW( buffer, name, length ))
        {
            strcpy( unix_name + pos, de->d_name );
            closedir( dir );
            goto success;
        }

        if (!is_name_8_dot_3) continue;

#if 0
        str.Length = ret * sizeof(WCHAR);
        if (!RtlIsNameLegalDOS8Dot3( &str, NULL, &spaces ) || spaces)
        {
            WCHAR short_nameW[12];
            ret = hash_short_file_name( &str, short_nameW );
            if (ret == length && !memicmpW( short_nameW, name, length ))
            {
                strcpy( unix_name + pos, de->d_name );
                closedir( dir );
                goto success;
            }
        }
#endif
    }
    closedir( dir );

not_found:
    unix_name[pos - 1] = 0;
    return STATUS_OBJECT_PATH_NOT_FOUND;

success:
    //if (is_win_dir && !stat( unix_name, &st )) *is_win_dir = is_same_file( &windir, &st );
    return STATUS_SUCCESS;
}

NTSTATUS lookup_unix_name( const WCHAR *name, int name_len, char **buffer, int unix_len, int pos,
                           UINT disposition, BOOLEAN check_case )
{
    NTSTATUS status;
    int ret, used_default, len;
    struct stat st;
    char *unix_name = *buffer;
    //const BOOL redirect = nb_redirects && ntdll_get_thread_data()->wow64_redir;
    const BOOL redirect = 0;

    /* try a shortcut first */

    ret = ntdll_wcstoumbs( 0, name, name_len, unix_name + pos, unix_len - pos - 1,
                           NULL, &used_default );

    while (name_len && IS_SEPARATOR(*name))
    {
        name++;
        name_len--;
    }

    if (ret >= 0 && !used_default)  /* if we used the default char the name didn't convert properly */
    {
        char *p;
        unix_name[pos + ret] = 0;
        for (p = unix_name + pos ; *p; p++) if (*p == '\\') *p = '/';
        if (!redirect || (!strstr( unix_name, "/windows/") && strncmp( unix_name, "windows/", 8 )))
        {
            if (!original_stat( unix_name, &st ))
            {
                /* creation fails with STATUS_ACCESS_DENIED for the root of the drive */
                if (disposition == FILE_CREATE)
                    return name_len ? STATUS_OBJECT_NAME_COLLISION : STATUS_ACCESS_DENIED;
                return STATUS_SUCCESS;
            }
        }
    }

    if (!name_len)  /* empty name -> drive root doesn't exist */
        return STATUS_OBJECT_PATH_NOT_FOUND;
    if (check_case && !redirect && (disposition == FILE_OPEN || disposition == FILE_OVERWRITE))
        return STATUS_OBJECT_NAME_NOT_FOUND;

    /* now do it component by component */
    unix_name[pos++] = '.';
    //unix_name[pos++] = '/';

    while (name_len)
    {
        const WCHAR *end, *next;
        BOOLEAN is_win_dir = FALSE;

        end = name;
        while (end < name + name_len && !IS_SEPARATOR(*end)) end++;
        next = end;
        while (next < name + name_len && IS_SEPARATOR(*next)) next++;
        name_len -= next - name;

        /* grow the buffer if needed */

        if (unix_len - pos < MAX_DIR_ENTRY_LEN + 2)
        {
            char *new_name;
            unix_len += 2 * MAX_DIR_ENTRY_LEN;
            if (!(new_name = realloc(unix_name, unix_len)))
                return STATUS_NO_MEMORY;
            unix_name = *buffer = new_name;
        }

        status = find_file_in_dir( unix_name, pos, name, end - name,
                                   check_case, redirect ? &is_win_dir : NULL );
        /* if this is the last element, not finding it is not necessarily fatal */
        if (!name_len)
        {
            //printf("%i\n", name_len);
            if (status == STATUS_OBJECT_PATH_NOT_FOUND)
            {
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                if (disposition != FILE_OPEN && disposition != FILE_OVERWRITE)
                {
                    ret = ntdll_wcstoumbs( 0, name, end - name, unix_name + pos + 1,
                                           MAX_DIR_ENTRY_LEN, NULL, &used_default );
                    if (ret > 0 && !used_default)
                    {
                        /*unix_name[pos] = '.';
                          unix_name[pos + 1] = '/';*/
                        unix_name[pos] = '/';
                        unix_name[pos + 1 + ret] = 0;
                        status = STATUS_NO_SUCH_FILE;
                        break;
                    }
                }
            }
            else if (status == STATUS_SUCCESS)
            {
                if (disposition == FILE_CREATE)
                    status = STATUS_OBJECT_NAME_COLLISION;
            }
        }

        if (status != STATUS_SUCCESS) break;

        pos += strlen( unix_name + pos );
        name = next;

        /*if (is_win_dir && (len = get_redirect_path( unix_name, pos, name, name_len, check_case )))
        {
            name += len;
            name_len -= len;
            pos += strlen( unix_name + pos );
            //TRACE( "redirecting -> %s + %s\n", debugstr_a(unix_name), debugstr_w(name) );
            }*/
    }

    return status;
}
