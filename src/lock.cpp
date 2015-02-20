
#if defined (_WIN32)
#include <tchar.h>
#include <share.h>
#include <sys/locking.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/file.h>
#define USE_POSIX_LOCKING

#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <stdexcept>
#include <cerrno>
#include <limits>
#include <cstring>

#include "lock.h"

#if defined (_WIN32)
#  if _WIN32_WINNT < 0x0501
#    define USE_WIN32_LOCKING
#  else
#    define USE_WIN32_LOCKFILEEX
#  endif
#else
#  if defined (O_EXLOCK)
#    define USE_O_EXLOCK
#  elif defined (HAVE_FCNTL) && defined (F_SETLKW)
#    define USE_SETLKW
#  elif defined (HAVE_LOCKF)
#    define USE_LOCKF
#  elif defined (HAVE_FLOCK)
#    define USE_FLOCK
#  endif
#  if defined (USE_O_EXLOCK) || defined (USE_SETLKW) \
    || defined (USE_LOCKF) || defined (USE_FLOCK)
#    define USE_POSIX_LOCKING
#  endif
#endif

#if ! defined (USE_POSIX_LOCKING) && ! defined (_WIN32)
#error "no usable file locking"
#endif


#if defined (_WIN32)
int const OPEN_FLAGS = _O_RDWR | _O_CREAT /*| _O_TEMPORARY*/ | _O_NOINHERIT;
int const OPEN_SHFLAGS = _SH_DENYNO;
int const OPEN_MODE = _S_IREAD | _S_IWRITE;

#undef max
#undef min

namespace {

static
HANDLE
get_os_HANDLE (int fd) {
    HANDLE fh = reinterpret_cast<HANDLE>(_get_osfhandle (fd));
    if (fh == INVALID_HANDLE_VALUE)
        printf("_get_osfhandle() failed\n");

    return fh;
}

} // namespace

#elif defined (USE_POSIX_LOCKING)
int const OPEN_FLAGS = O_RDWR | O_CREAT
#if defined (O_CLOEXEC)
                       | O_CLOEXEC
#endif
                       ;

mode_t const OPEN_MODE = (S_IRWXU ^ S_IXUSR)
                         | (S_IRWXG ^ S_IXGRP)
                         | (S_IRWXO ^ S_IXOTH);

#endif

struct MyLockFile::Impl {
#if defined (USE_POSIX_LOCKING) \
    || defined (_WIN32)
    int fd;

#endif
};


//
//
//

MyLockFile::MyLockFile (std::string const & lf)
    : lock_file_name (lf)
    , data (new MyLockFile::Impl) {
#if defined (USE_O_EXLOCK)
    data->fd = -1;

#else
    this->open(OPEN_FLAGS);

#endif
}


MyLockFile::~MyLockFile () {
    close ();
    delete data;
}


void
MyLockFile::open (int open_flags) const {
#if defined (_WIN32)
    data->fd = _sopen (lock_file_name.c_str (), open_flags,
                       OPEN_SHFLAGS, OPEN_MODE);
    if (data->fd == -1)
        printf("could not open or create file %s", lock_file_name);

#elif defined (USE_POSIX_LOCKING)
    data->fd = ::open (lock_file_name.c_str(),
                       open_flags, OPEN_MODE);
    if (data->fd == -1)
        printf("could not open or create file %s", lock_file_name.c_str());

#if ! defined (O_CLOEXEC) && defined (FD_CLOEXEC)
    int ret = fcntl (data->fd, F_SETFD, FD_CLOEXEC);
    if (ret == -1)
        printf("could not set FD_CLOEXEC on file %s", lock_file_name);

#endif
#endif
}


void
MyLockFile::close () const {
#if defined (_WIN32)
    if (data->fd >= 0)
        _close (data->fd);

    data->fd = -1;

#elif defined (USE_POSIX_LOCKING)
    if (data->fd >= 0)
        ::close (data->fd);

    data->fd = -1;

#endif
}

int MyLockFile::read(void *buffer,  int nbyte) {
    int ret = 0;
    if (data->fd >= 0) {
#if defined (_WIN32)
        ret = _read(data->fd,  buffer,  nbyte);
#elif defined (USE_POSIX_LOCKING)
        ret = ::read(data->fd,  buffer,  nbyte);
#endif
    }
    return ret;
}

int MyLockFile::write(void *buffer,  int nbyte) {
    int ret = 0;
    if (data->fd >= 0) {
#if defined (_WIN32)
        ret = _write(data->fd,  buffer,  nbyte);
#elif defined (USE_POSIX_LOCKING)
        ret = ::write(data->fd,  buffer,  nbyte);
#endif
    }
    return ret;
}

void
MyLockFile::lock () const {
    int ret = 0;

#if defined (USE_WIN32_LOCKFILEEX)
    HANDLE fh = get_os_HANDLE (data->fd);

    OVERLAPPED overlapped;
    memset (&overlapped, 0, sizeof (overlapped));
    overlapped.hEvent = 0;

    ret = LockFileEx(fh, LOCKFILE_EXCLUSIVE_LOCK, 0,
                     (STD_NAMESPACE numeric_limits<DWORD>::max) (),
                     (STD_NAMESPACE numeric_limits<DWORD>::max) (), &overlapped);
    if (! ret)
        loglog.error (tstring (TEXT ("LockFileEx() failed: "))
                      + convertIntegerToString (GetLastError ()), true);

#elif defined (USE_WIN32_LOCKING)
    ret = _locking (data->fd, _LK_LOCK, std::numeric_limits<long>::max());
    //if (ret != 0)

#elif defined (USE_O_EXLOCK)
    open (OPEN_FLAGS | O_EXLOCK);

#elif defined (USE_SETLKW)
    do {
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        ret = fcntl (data->fd, F_SETLKW, &fl);
        if (ret == -1 && errno != EINTR)
            printf("fcntl(F_SETLKW) failed: "))
            + convertIntegerToString (errno), true);
        } while (ret == -1);

#elif defined (USE_LOCKF)
    do {
        ret = lockf (data->fd, F_LOCK, 0);
        if (ret == -1 && errno != EINTR)
            printf("lockf() failed: "))
            + convertIntegerToString (errno), true);
        } while (ret == -1);

#elif defined (USE_FLOCK)
    do {
        ret = flock (data->fd, LOCK_EX);
        if (ret == -1 && errno != EINTR)
            printf("flock() failed: "))
            + convertIntegerToString (errno), true);
        } while (ret == -1);

#endif
}


void MyLockFile::unlock () const {
    int ret = 0;

#if defined (USE_WIN32_LOCKFILEEX)
    HANDLE fh = get_os_HANDLE (data->fd, loglog);

    ret = UnlockFile(fh, 0, 0, (STD_NAMESPACE numeric_limits<DWORD>::max) (),
                     (STD_NAMESPACE numeric_limits<DWORD>::max) ());
    if (! ret)
        loglog.error (tstring (TEXT ("UnlockFile() failed: "))
                      + convertIntegerToString (GetLastError ()), true);

#elif defined (USE_WIN32_LOCKING)
    ret = _locking (data->fd, _LK_UNLCK, std::numeric_limits<long>::max());
    if (ret != 0)
        printf("_locking() failed: %d", errno);

#elif defined (USE_O_EXLOCK)
    close ();

#elif defined (USE_SETLKW)
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    ret = fcntl (data->fd, F_SETLKW, &fl);
    if (ret != 0)
        printf("fcntl(F_SETLKW) failed: "))
        + convertIntegerToString (errno), true);

#elif defined (USE_LOCKF)
    ret = lockf (data->fd, F_ULOCK, 0);
    if (ret != 0)
        printf("lockf() failed: "))
        + convertIntegerToString (errno), true);

#elif defined (USE_FLOCK)
    ret = flock (data->fd, LOCK_UN);
    if (ret != 0)
        printf("flock() failed: "))
        + convertIntegerToString (errno), true);

#endif

}
