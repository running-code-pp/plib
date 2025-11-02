/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-01 00:34:20
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 20:02:50
 * @FilePath: \plib\src\core\include\utils\zlib_helper.hpp
 * @Description: 在内存中操作zip文件流,依赖minizip
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_ZLIB_HELPER_HPP_
#define PLIB_CORE_UTILS_ZLIB_HELPER_HPP_

#include <ioapi.h>
#include <zip.h>
#include <unzip.h>
#include <vector>
#include <algorithm>
#include <cstring>

namespace plib::core::utils
{

    /**
     * @brief: 内存文件抽象
     */
    class InMemoryFile
    {

    public:
        InMemoryFile(const std::vector<uint8_t> &data) : _error(0), _position(0), _data(data) {}

        /**
         * @brief: 获取zlib_filefunc_def结构体指针
         */
        zlib_filefunc_def funcs()
        {
            zlib_filefunc_def result;
            result.zopen_file = &InMemoryFile::Open;
            result.zerror_file = &InMemoryFile::Error;
            result.zread_file = &InMemoryFile::Read;
            result.zwrite_file = &InMemoryFile::Write;
            result.zclose_file = &InMemoryFile::Close;
            result.zseek_file = &InMemoryFile::Seek;
            result.ztell_file = &InMemoryFile::Tell;
            result.opaque = this;

            return result;
        }

        int error() const
        {
            return _error;
        }

        std::vector<uint8_t> result() const
        {
            return _data;
        }

    private:
        // 封装zlib_filefunc_def中的各种api

        /**
         * @brief: 打开内存文件
         * @param filename 文件名
         * @param mode 打开模式
         */
        voidpf open(const char *filename, int mode)
        {
            if (mode & ZLIB_FILEFUNC_MODE_READ)
            {
                _position = 0;
            }
            else if (mode & ZLIB_FILEFUNC_MODE_WRITE)
            {
                _data.clear();
            }
            _error = 0;
            return this;
        }

        /**
         * @brief: 从内存文件中读取数据到缓冲区buf
         * @param stream 内存文件流
         * @return buf: 接受数据的缓冲区
         * @return size: 读取的字节数
         */
        unsigned long read(voidpf stream, void *buf, unsigned long size)
        {
            if (_error)
            {
                return 0;
            }

            if (buf == nullptr || size == 0)
            {
                return 0;
            }

            if (_position >= _data.size())
            {
                _error = -1;
                return 0;
            }

            const unsigned long available = static_cast<unsigned long>(_data.size() - _position);
            const unsigned long bytes_to_read = (std::min)(size, available);

            if (bytes_to_read > 0)
            {
                std::memcpy(buf, _data.data() + _position, bytes_to_read);
                _position += bytes_to_read;
            }

            if (bytes_to_read < size)
            {
                _error = -1;
            }

            return bytes_to_read;
        }

        /**
         * @brief: 向内存文件中写入数据
         * @param stream 内存文件流
         * @return buf: 要写入的数据缓冲区
         * @return size: 写入的字节数
         */
        unsigned long write(voidpf stream, const void *buf, unsigned long size)
        {
            if (_data.size() < _position + size)
            {
                _data.resize(_position + size);
            }
            memcpy(_data.data() + _position, buf, size);
            _position += size;
            return size;
        }

        /**
         * @brief: 关闭内存文件流
         */
        int close(voidpf stream)
        {
            int result = _error;
            _position = 0;
            _error = 0;
            return result;
        }

        /**
         * @brief: 获取错误码
         */
        int error(voidpf stream) const
        {
            return _error;
        }

        /**
         * @brief: 返回当前内存文件中文件指针位置
         */
        long tell(voidpf stream) const
        {
            return static_cast<long>(_position);
        }

        /**
         * @brief: 设置内存文件中文件指针位置
         */
        long seek(voidpf stream, long offset, int origin)
        {
            if (_error)
            {
                switch (origin)
                {
                case ZLIB_FILEFUNC_SEEK_CUR:
                    _position += offset;
                    break;
                case ZLIB_FILEFUNC_SEEK_END:
                    _position = _data.size() + offset;
                    break;
                case ZLIB_FILEFUNC_SEEK_SET:
                    _position = offset;
                    break;
                }
                if (_position > _data.size())
                {
                    _error = -1;
                }
            }
            return _error;
        }

        static voidpf Open(voidpf opaque, const char *filename, int mode)
        {
            InMemoryFile *memFile = static_cast<InMemoryFile *>(opaque);
            return memFile->open(filename, mode);
        }

        static unsigned long Read(voidpf opaque, voidpf stream, void *buf, unsigned long size)
        {
            InMemoryFile *memFile = static_cast<InMemoryFile *>(stream);
            return memFile->read(stream, buf, size);
        }

        static uLong Write(voidpf opaque, voidpf stream, const void *buf, uLong size)
        {
            return static_cast<InMemoryFile *>(opaque)->write(stream, buf, size);
        }

        static int Close(voidpf opaque, voidpf stream)
        {
            return static_cast<InMemoryFile *>(opaque)->close(stream);
        }

        static int Error(voidpf opaque, voidpf stream)
        {
            return static_cast<InMemoryFile *>(opaque)->error(stream);
        }

        static long Tell(voidpf opaque, voidpf stream)
        {
            return static_cast<InMemoryFile *>(opaque)->tell(stream);
        }

        static long Seek(voidpf opaque, voidpf stream, uLong offset, int origin)
        {
            return static_cast<InMemoryFile *>(opaque)->seek(stream, offset, origin);
        }

    private:
        int _error;
        unsigned long _position;
        std::vector<uint8_t> _data;
    };

} // namespace plib::core::utils

#endif // PLIB_CORE_UTILS_ZLIB_HELPER_HPP_