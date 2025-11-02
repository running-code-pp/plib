#include <gtest/gtest.h>
#include "utils/lru.hpp"

#include <string>
#include <vector>

namespace plib::core::utils
{
    // 测试基本的 up 操作
    TEST(LRUTest, BasicUp)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        
        // 最久未访问的应该是 1
        EXPECT_EQ(lru.take_lowest(), 1);
        EXPECT_EQ(lru.take_lowest(), 2);
        EXPECT_EQ(lru.take_lowest(), 3);
    }

    // 测试重复访问提升优先级
    TEST(LRUTest, UpRefreshPriority)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        lru.up(1); // 1 重新被访问，优先级提升到最高
        
        // 现在顺序应该是: 2, 3, 1
        EXPECT_EQ(lru.take_lowest(), 2);
        EXPECT_EQ(lru.take_lowest(), 3);
        EXPECT_EQ(lru.take_lowest(), 1);
    }

    // 测试多次提升同一元素
    TEST(LRUTest, MultipleUpSameElement)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        lru.up(2); // 2 提升
        lru.up(2); // 2 再次提升（已经在最高位置，不应改变）
        
        // 顺序应该是: 1, 3, 2
        EXPECT_EQ(lru.take_lowest(), 1);
        EXPECT_EQ(lru.take_lowest(), 3);
        EXPECT_EQ(lru.take_lowest(), 2);
    }

    // 测试 remove 操作
    TEST(LRUTest, Remove)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        lru.remove(2); // 删除中间元素
        
        EXPECT_EQ(lru.take_lowest(), 1);
        EXPECT_EQ(lru.take_lowest(), 3);
    }

    // 测试删除不存在的元素
    TEST(LRUTest, RemoveNonExistent)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.remove(99); // 删除不存在的元素，不应崩溃
        
        EXPECT_EQ(lru.take_lowest(), 1);
        EXPECT_EQ(lru.take_lowest(), 2);
    }

    // 测试 clear 操作
    TEST(LRUTest, Clear)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        lru.clear();
        
        // 清空后 take_lowest 应该返回默认值
        EXPECT_EQ(lru.take_lowest(), 0);
    }

    // 测试空 LRU 的 take_lowest
    TEST(LRUTest, TakeLowestFromEmpty)
    {
        LRU<int> lru;
        
        // 空 LRU 返回默认构造的值
        EXPECT_EQ(lru.take_lowest(), 0);
    }

    // 测试 LRU 缓存场景（模拟固定大小缓存）
    TEST(LRUTest, CacheSimulation)
    {
        LRU<int> lru;
        const int cache_size = 3;
        
        // 添加超过容量的元素
        for (int i = 1; i <= 5; ++i)
        {
            lru.up(i);
            // 如果超过容量，移除最低优先级的
            // 这里假设外部逻辑维护大小，我们只测试顺序
        }
        
        // 访问顺序: 1, 2, 3, 4, 5
        // 最久未访问的是 1
        EXPECT_EQ(lru.take_lowest(), 1);
        EXPECT_EQ(lru.take_lowest(), 2);
        EXPECT_EQ(lru.take_lowest(), 3);
    }

    // 测试复杂的访问模式
    TEST(LRUTest, ComplexAccessPattern)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        lru.up(4);
        lru.up(2); // 2 被访问，提升到最高
        lru.up(3); // 3 被访问，提升到最高
        lru.up(1); // 1 被访问，提升到最高
        
        // 顺序应该是: 4, 2, 3, 1
        EXPECT_EQ(lru.take_lowest(), 4);
        EXPECT_EQ(lru.take_lowest(), 2);
        EXPECT_EQ(lru.take_lowest(), 3);
        EXPECT_EQ(lru.take_lowest(), 1);
    }

    // 测试字符串类型
    TEST(LRUTest, StringType)
    {
        LRU<std::string> lru;
        
        lru.up("apple");
        lru.up("banana");
        lru.up("cherry");
        lru.up("apple"); // apple 重新访问
        
        EXPECT_EQ(lru.take_lowest(), "banana");
        EXPECT_EQ(lru.take_lowest(), "cherry");
        EXPECT_EQ(lru.take_lowest(), "apple");
    }

    // 测试删除最低优先级后继续操作
    TEST(LRUTest, TakeLowestAndContinue)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        
        int lowest = lru.take_lowest();
        EXPECT_EQ(lowest, 1);
        
        lru.up(4);
        lru.up(2); // 2 重新访问
        
        // 现在顺序是: 3, 4, 2
        EXPECT_EQ(lru.take_lowest(), 3);
        EXPECT_EQ(lru.take_lowest(), 4);
        EXPECT_EQ(lru.take_lowest(), 2);
    }

    // 测试单个元素
    TEST(LRUTest, SingleElement)
    {
        LRU<int> lru;
        
        lru.up(42);
        EXPECT_EQ(lru.take_lowest(), 42);
        
        // 再次 take 应该返回默认值
        EXPECT_EQ(lru.take_lowest(), 0);
    }

    // 测试多次 up 同一元素且它已经在最高位置
    TEST(LRUTest, UpAlreadyHighest)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        
        // 3 已经在最高位置
        lru.up(3);
        lru.up(3);
        lru.up(3);
        
        // 顺序不应改变: 1, 2, 3
        EXPECT_EQ(lru.take_lowest(), 1);
        EXPECT_EQ(lru.take_lowest(), 2);
        EXPECT_EQ(lru.take_lowest(), 3);
    }

    // 测试删除所有元素
    TEST(LRUTest, RemoveAll)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        lru.up(3);
        
        lru.remove(1);
        lru.remove(2);
        lru.remove(3);
        
        // 所有元素都被删除
        EXPECT_EQ(lru.take_lowest(), 0);
    }

    // 测试交替操作
    TEST(LRUTest, InterleavedOperations)
    {
        LRU<int> lru;
        
        lru.up(1);
        lru.up(2);
        EXPECT_EQ(lru.take_lowest(), 1);
        
        lru.up(3);
        lru.up(4);
        lru.remove(3);
        
        // 剩余: 2, 4
        EXPECT_EQ(lru.take_lowest(), 2);
        EXPECT_EQ(lru.take_lowest(), 4);
        EXPECT_EQ(lru.take_lowest(), 0);
    }

    // 测试大量元素
    TEST(LRUTest, LargeNumberOfElements)
    {
        LRU<int> lru;
        const int count = 1000;
        
        for (int i = 0; i < count; ++i)
        {
            lru.up(i);
        }
        
        // 验证顺序
        for (int i = 0; i < count; ++i)
        {
            EXPECT_EQ(lru.take_lowest(), i);
        }
    }

    // 测试指针类型（需要自定义 hash）
    struct Data
    {
        int value;
        
        Data() : value(0) {}
        Data(int v) : value(v) {}
        
        bool operator==(const Data& other) const
        {
            return value == other.value;
        }
    };
}

namespace std
{
    template<>
    struct hash<plib::core::utils::Data>
    {
        size_t operator()(const plib::core::utils::Data& d) const
        {
            return hash<int>()(d.value);
        }
    };
}

namespace plib::core::utils
{
    TEST(LRUTest, CustomType)
    {
        LRU<Data> lru;
        
        lru.up(Data(10));
        lru.up(Data(20));
        lru.up(Data(30));
        lru.up(Data(10)); // 重新访问
        
        EXPECT_EQ(lru.take_lowest().value, 20);
        EXPECT_EQ(lru.take_lowest().value, 30);
        EXPECT_EQ(lru.take_lowest().value, 10);
    }
}
