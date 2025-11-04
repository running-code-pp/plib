/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-02 21:33:28
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-02 21:37:35
 * @FilePath: \plib\src\framework\include\cache\cache_types.hpp
 * @Description:
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_FRAMEWORK_CACHE_CACHE_TYPES_HPP_
#define PLIB_FRAMEWORK_CACHE_CACHE_TYPES_HPP_

#include "../../core/include/typedef.hpp"
#include "../../core/include/plib_macros.hpp"
#include "../../core/include/type/flatmap.hpp"
#include <tuple>
#include <string>
#include <array>

namespace plib::framework::cache
{
    /**
     * @brief: 缓存键,128位
     */
    struct Key
    {
        uint64 high;
        uint64 low;

        [[nodiscard]] bool isValid() const
        {
            return high != 0 || low != 0;
        }

        explicit operator bool() const
        {
            return isValid();
        }
    };

    P_FORCE_INLINE inline bool operator==(const Key &lhs, const Key &rhs)
    {
        return lhs.high == rhs.high && lhs.low == rhs.low;
    }

    P_FORCE_INLINE inline bool operator!=(const Key &lhs, const Key &rhs)
    {
        return !(lhs == rhs);
    }

    P_FORCE_INLINE inline bool operator<(const Key &lhs, const Key &rhs)
    {
        return std::tie(lhs.high, lhs.low) < std::tie(rhs.high, rhs.low);
    }

    /**
     * @brief : 错误码
     */

    struct Error
    {
        enum class Type : uint8
        {
            None = 0,  // 无错误
            IO,        // IO错误(读写磁盘错误)
            WrongKey,  // 加密密钥错误
            LockFailed // 文件锁定失败，可能其他进程正在使用
        };
        Type type{Type::None};
        std::string path; // 发生错误的缓存数据文件
        static Error NoError()
        {
            return Error{Type::None, ""};
        }
    };

    namespace details
    {
        using RecordType = uint8;
        using PlaceId = std::array<uint8, 7>;      // 数据存储位置id
        using EntrySize = std::array<uint8, 3>;    // 数据条目大小
        using RecordsCount = std::array<uint8, 3>; // 批量记录数据

        // 位置大小
        constexpr auto kRecordSizeUnknown = int64(-1);
        constexpr auto kRecordSizeInvalid = int64(-2);
        constexpr auto kBundledRecordsLimit = int64(1) << (std::tuple_size_v<RecordsCount> * sizeof(uint8)); // 批量记录数量上限 2^24
        constexpr auto kDataSizeLimit = int64(1) << (std::tuple_size_v<EntrySize> * sizeof(uint8));          // 数据条目大小上限 16MB

        // 配置参数
        struct Settings
        {
            // 单词批量操作的最大记录数
            int64 maxBundledRecords = 16 * 1024;
            // 读取binglog时块的大小
            int64 readBlockSize = 8 * 1024 * 1024;
            // 单个数据条目的最大大小
            int64 maxDataSize = kDataSizeLimit - 1;
            // 写入批量操作的延迟时间
            int64 writeBundleDelayMs = 15 * 60 * 1000;
            // 每次清理过期数据时处理的条目数量
            int64 staleRemoveChunk = 256;
            // binlog超过正常大小多少字节后触发压缩
            int64 compactAfterExcess = 8 * 1024 * 1024;
            // 是否跟踪数据的访问时间
            bool trackEstimatedTime = true;
            // 缓存总大小限制
            int64 totalSizeLimit = 1024 * 1024 * 1024;
            // 缓存数据的时间限制
            int64 totalTimeLimit = 31 * 24 * 60 * 60;
            // 清理过期数据的检查间隔
            int64 pruneTimeout = 5 * 1000;
            // 清理检查的最大间隔
            int64 maxPruneCheckTimeout = 3600 * 1000;
            // 加密密钥错误时是否清空缓存
            bool clearOnWrongKey = false;
        };

        // 运行时更新的配置（不需要重启）
        struct SettingsUpdate
        {
            int64 totalSizeLimit = Settings().totalSizeLimit; // 更新缓存大小限制
            int64 totalTimeLimit = Settings().totalTimeLimit; // 更新时间限制
        };

        struct TaggedValue
        {
            TaggedValue() = default;
            TaggedValue(bytearray &&bytes, uint8 tag);

            bytearray bytes; // 实际的数据内容
            uint8 tag = 0;   // 标签（0-255），用于分类
        };

        // 按标签统计的摘要信息
        struct TaggedSummary
        {
            int64 count = 0;     // 该标签下的条目数量
            int64 totalSize = 0; // 该标签下的总数据大小（字节）
        };

        // 缓存统计信息
        struct Stats
        {
            TaggedSummary full;                                      // 所有数据的统计
            plib::core::type::flat_map<uint8, TaggedSummary> tagged; // 按标签分类的统计
            bool clearing = false;                                   // 是否正在清理中
        };

        using Version = int32; // 缓存版本号，用于数据格式升级

        // 计算规范化的基础路径（确保以 '/' 结尾）
        QString ComputeBasePath(const QString &original);

        // 获取版本文件的路径
        QString VersionFilePath(const QString &base);

        // 读取版本号
        std::optional<Version> ReadVersionValue(const QString &base);

        // 写入版本号
        bool WriteVersionValue(const QString &base, Version value);

        // 检查记录类型是否适合加密（必须是 16 字节对齐）
        // AES 等加密算法通常要求数据块是 16 字节的倍数
        template <typename Record>
        constexpr auto GoodForEncryption = ((sizeof(Record) & 0x0F) == 0);

        // 数据格式版本
        enum class Format : uint32
        {
            Format_0, // 初始格式版本
        };

        // binlog 文件的基础头部
        struct BasicHeader
        {
            BasicHeader();

            // 标志位：是否跟踪访问时间
            static constexpr auto kTrackEstimatedTime = 0x01U;

            Format getFormat() const
            {
                return static_cast<Format>(format);
            }
            void setFormat(Format format)
            {
                this->format = static_cast<uint32>(format);
            }

            uint32 format : 8;     // 格式版本（8位）
            uint32 flags : 24;     // 标志位（24位）
            uint32 systemTime = 0; // 系统时间戳（用于时间同步）
            uint32 reserved1 = 0;  // 保留字段（未来扩展）
            uint32 reserved2 = 0;  // 保留字段
        };

        // 估算的时间点，用于 LRU（最近最少使用）策略
        // 分成相对时间（程序运行时间）和系统时间
        struct EstimatedTimePoint
        {
            uint32 relative1 = 0; // 相对时间低 32 位
            uint32 relative2 = 0; // 相对时间高 32 位
            uint32 system = 0;    // 系统时间戳

            // 设置 64 位相对时间（拆分成两个 32 位）
            void setRelative(uint64 value)
            {
                relative1 = uint32(value & 0xFFFFFFFFU);
                relative2 = uint32((value >> 32) & 0xFFFFFFFFU);
            }
            // 获取 64 位相对时间（合并两个 32 位）
            uint64 getRelative() const
            {
                return uint64(relative1) | (uint64(relative2) << 32);
            }
        };

        // 存储记录：记录一个键值对的元数据
        struct Store
        {
            static constexpr auto kType = RecordType(0x01); // 记录类型标识符

            void setSize(int64 size); // 设置数据大小
            int64 getSize() const;    // 获取数据大小

            RecordType type = kType; // 记录类型（必须是 0x01）
            uint8 tag = 0;           // 数据标签（用于分类）
            EntrySize size = {{0}};  // 数据大小（3字节，最多 16MB）
            PlaceId place = {{0}};   // 数据在文件中的位置（7字节）
            uint32 checksum = 0;     // 校验和（用于验证数据完整性）
            Key key;                 // 键（128位）
        };

        // 带时间戳的存储记录（继承自 Store）
        // 当 trackEstimatedTime 启用时使用此记录类型
        struct StoreWithTime : Store
        {
            EstimatedTimePoint time; // 访问时间
            uint32 reserved = 0;     // 保留字段
        };

        // 批量存储记录的头部
        // 用于一次性写入多个 Store 记录，提高效率
        struct MultiStore
        {
            static constexpr auto kType = RecordType(0x02); // 记录类型标识符

            explicit MultiStore(int64 count = 0);

            RecordType type = kType;    // 记录类型（必须是 0x02）
            RecordsCount count = {{0}}; // 后续的 Store 记录数量
            uint32 reserved1 = 0;       // 保留字段
            uint32 reserved2 = 0;
            uint32 reserved3 = 0;

            using Part = Store;              // 每个部分的类型是 Store
            int64 validateCount() const; // 验证记录数量是否有效
        };

        // 带时间戳的批量存储记录
        struct MultiStoreWithTime : MultiStore
        {
            using MultiStore::MultiStore; // 继承构造函数

            using Part = StoreWithTime; // 每个部分的类型是 StoreWithTime
        };

        // 批量删除记录的头部
        struct MultiRemove
        {
            static constexpr auto kType = RecordType(0x03); // 记录类型标识符

            explicit MultiRemove(int64 count = 0);

            RecordType type = kType;    // 记录类型（必须是 0x03）
            RecordsCount count = {{0}}; // 后续要删除的 Key 数量
            uint32 reserved1 = 0;       // 保留字段
            uint32 reserved2 = 0;
            uint32 reserved3 = 0;

            using Part = Key;                // 每个部分是一个 Key
            int64 validateCount() const; // 验证记录数量是否有效
        };

        // 批量访问记录（更新访问时间）
        struct MultiAccess
        {
            static constexpr auto kType = RecordType(0x04); // 记录类型标识符

            explicit MultiAccess(
                EstimatedTimePoint time,
                int64 count = 0);

            RecordType type = kType;    // 记录类型（必须是 0x04）
            RecordsCount count = {{0}}; // 后续被访问的 Key 数量
            EstimatedTimePoint time;    // 访问时间

            using Part = Key;                // 每个部分是一个 Key
            int64 validateCount() const; // 验证记录数量是否有效
        };

    } // namespace details

} // namespace plib::framework::cache

// 为 std::unordered_map 和 std::unordered_set 提供 Key 的哈希函数
namespace std
{

    template <>
    struct hash<plib::framework::cache::Key>
    {
        size_t operator()(const plib::framework::cache::Key &key) const
        {
            // 使用异或组合两个 64 位哈希值
            return (hash<uint64>()(key.high) ^ hash<uint64>()(key.low));
        }
    };

} // namespace std

#endif // PLIB_FRAMEWORK_CACHE_CACHE_TYPES_HPP_