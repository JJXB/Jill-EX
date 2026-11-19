// Xbox file-open glue with path-separator normalisation.
//
// NXDK's pdclib passes file names straight to CreateFileA(), which forwards
// them unchanged to NtCreateFile().  The NT object manager only treats '\\' as
// a separator, so any '/' in a name makes the open fail with
// STATUS_OBJECT_PATH_NOT_FOUND.
//
// That matters here because the DOS-era configuration and the engine use '/':
//   * jill.ini       palette = profiles/onesimus.pal
//   * engine join()  D:\jillt + "jill.dma" -> D:\jillt/jill.dma
//   * pack.ini       art/shapes/foo.sha
//
// fopen(), std::ifstream and std::ofstream all funnel through pdclib's
// _PDCLIB_open(), so normalising the name here fixes every file open in the
// engine without touching the shared sources.  The implementation mirrors the
// glue in NXDK's lib/pdclib/platform/xbox/functions/_PDCLIB/_PDCLIB_open.c;
// our definition wins over the archive member because it is linked directly.

#include <windows.h>

#include <pdclib/_PDCLIB_int.h>
#include <stdio.h>

#include <string>

extern "C" int _PDCLIB_w32errno(DWORD werror);
extern "C" _PDCLIB_fd_t _PDCLIB_open(const char* const filename,
                                     unsigned int mode);

namespace {

std::string native_path(const char* filename) {
    if (filename == nullptr) {
        return std::string();
    }
    std::string path(filename);
    for (char& c : path) {
        if (c == '/') {
            c = '\\';
        }
    }
    return path;
}

}  // namespace

extern "C" _PDCLIB_fd_t _PDCLIB_open(const char* const filename,
                                     unsigned int mode) {
    int access_flags = 0;
    int create_flags = 0;

    switch (mode & (_PDCLIB_FREAD | _PDCLIB_FWRITE | _PDCLIB_FAPPEND |
                    _PDCLIB_FRW)) {
        case _PDCLIB_FREAD: /* "r" */
            access_flags = GENERIC_READ;
            create_flags = OPEN_EXISTING;
            break;
        case _PDCLIB_FWRITE: /* "w" */
            access_flags = GENERIC_WRITE;
            create_flags = CREATE_ALWAYS;
            break;
        case _PDCLIB_FWRITE | _PDCLIB_FAPPEND: /* "a" */
            access_flags = GENERIC_WRITE;
            create_flags = OPEN_ALWAYS;
            break;
        case _PDCLIB_FREAD | _PDCLIB_FRW: /* "r+" */
            access_flags = GENERIC_READ | GENERIC_WRITE;
            create_flags = OPEN_EXISTING;
            break;
        case _PDCLIB_FWRITE | _PDCLIB_FRW: /* "w+" */
            access_flags = GENERIC_READ | GENERIC_WRITE;
            create_flags = CREATE_ALWAYS | TRUNCATE_EXISTING;
            break;
        case _PDCLIB_FAPPEND | _PDCLIB_FRW: /* "a+" */
            access_flags = GENERIC_READ | GENERIC_WRITE;
            create_flags = OPEN_ALWAYS;
            break;
        default: /* Invalid mode */
            return _PDCLIB_NOHANDLE;
    }

    const std::string native = native_path(filename);
    HANDLE handle = CreateFileA(native.c_str(), access_flags,
                                FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                                create_flags, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        if (mode & _PDCLIB_FAPPEND) {
            LARGE_INTEGER new_pos;
            new_pos.QuadPart = 0;
            BOOL ok = SetFilePointerEx(handle, new_pos, &new_pos, FILE_END);
            if (!ok) {
                *_PDCLIB_errno_func() = _PDCLIB_w32errno(GetLastError());
                CloseHandle(handle);
                return _PDCLIB_NOHANDLE;
            }
        }
        return handle;
    }

    *_PDCLIB_errno_func() = _PDCLIB_w32errno(GetLastError());
    return _PDCLIB_NOHANDLE;
}
