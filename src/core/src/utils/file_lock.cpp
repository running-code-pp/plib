#include "utils/file_lock.hpp"
#include "utils/os_util.hpp"
#include "typedef.hpp"
#include <filesystem>

namespace plib::core::utils
{

    // windows标准实现
#ifdef _WIN32
    class FileLock::Lock
    {
    public:
        static int Acquire(const FILE *file);

        explicit Lock(int descriptor);
        explicit Lock() = default;
        ~Lock();

    private:
        static constexpr auto offsetLow = DWORD(kLockOffset);
        static constexpr auto offsetHigh = DWORD(0);
        static constexpr auto limitLow = DWORD(kLockLimit);
        static constexpr auto limitHigh = DWORD(0);
        int _descriptor = 0;
    };

    FileLock::Lock::Lock(int descriptor) : _descriptor(descriptor) {}

    FileLock::Lock::~Lock()
    {
        if (const auto handle = HANDLE(_get_osfhandle(_descriptor)); handle)
        {
            UnlockFile(handle, offsetLow, offsetHigh, limitLow, limitHigh);
        }
    }

    int FileLock::Lock::Acquire(const FILE *file)
    {
        const auto descriptor = _fileno(const_cast<FILE *>(file));
        if (!descriptor)
        {
            return false;
        }
        const auto file_handle = (HANDLE)_get_osfhandle(descriptor);
        if (!file_handle)
        {
            return false;
        }

        return LockFile(file_handle, offsetLow, offsetHigh, limitLow, limitHigh) ? descriptor : 0;
    }

    FileLock::FileLock():_file(nullptr)
    {
    }
    FileLock::~FileLock()
    {
    }

    bool FileLock::lock(const char* file_path,const char* mode)
    {
        
        if (_lock || !file_path || !std::filesystem::exists(file_path))
        {
            return false;
        }
        unlock();
        fclose(_file);
        do {
            if(!(_file = fopen(file_path, mode))){
                return false;
            }else if(const auto descriptor = Lock::Acquire(_file)){
                _lock = std::make_unique<Lock>(descriptor);
                return true;
            }
            fclose(_file);
        } while (plib::core::utils::CloseProcesses(file_path));
     
    }

    void FileLock::unlock()
    {

    }

    bool FileLock::is_locked() const
    {
        return false;
    }

    void FileLock::setFile(FILE* file)
    {
        _file = file;
    }
// posix标准实现
#else

    class FileLock::Lock
    {
    public:
        using Result = std::variant<p_descriptor_t, p_pid_t>;
        static Result Acquire(const FILE *file);
        explicit Lock(int descriptor);
        Lock() = default;

        ~Lock();

    private:
        int _descriptor = 0;
    };

    FileLock::Lock(int descriptor) : _descriptor(descriptor) {}
    FileLock::Lock::~Lock()
    {
        struct flock unlock;
        unlock.l_type = F_UNLCK;
        unlock.l_whence = SEEK_SET;
        unlock.l_start = kLockOffset;
        unlock.l_len = kLockLimit;
        fcntl(_descriptor, F_SETLK, &unlock);
    }

    FileLock::Lock::Result FileLock::Lock::Acquire(const FILE *file)
    {
        const auto descriptor = _fileno(const_cast<FILE *>(file));
        if (!descriptor)
        {
            return p_descriptor_t{0};
        }
        while (true)
        {
            struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = kLockOffset;
            lock.l_len = kLockLimit;
            if (fcntl(descriptor, F_SETLK, &lock) == 0)
            {
                return p_descriptor{descriptor};
            }
            else if (fcntl(descriptor, F_GETLK, &lock) == 0)
            {
                return p_pid_t{0};
            }
            else if (lock.l_type == F_UNLCK)
            {
                return p_pid_t{lock.l_pid};
            }
        }
    }
#endif

} // namespace plib::core::utils
