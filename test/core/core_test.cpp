#include <gtest/gtest.h>
#include "config/conf/conf.hpp"
#include <sstream>
#include "config/yaml/yaml.hpp"
#include "type/flags.hpp"
#include "log/log.hpp"
#include "log/logger.hpp"
#include "log/manager.hpp"
#include "log/config.hpp"
#include "type/flatmap.hpp"
#include "type/flatset.hpp"
#include "utils/common_util.hpp"
#include "utils/path_util.hpp"
#include <string>
#include <memory>
#include <cmath>
#include "memory/memorypool.hpp"
using namespace plib::core::memory;
using namespace plib::core::config;
// using namespace plib::core::type; dont use this
namespace MethodNamespace
{
	template <typename Enum>
	void TestFlags(Enum a, Enum b, Enum c)
	{
		auto abc = a | b;
		abc |= c;
		auto test = abc != a;
		EXPECT_NE(abc, a);
		EXPECT_NE(abc, (a | b));
		EXPECT_EQ((abc & a), a);
		EXPECT_EQ((abc & b), b);
		EXPECT_EQ((abc & c), c);
		EXPECT_EQ((abc & ~a), (b | c));
		EXPECT_EQ((abc & ~(b | c)), a);
		EXPECT_EQ((abc ^ a), (abc & ~a));

		auto another = a | b;
		another |= c;
		EXPECT_EQ(abc, another);
		another &= ~b;
		EXPECT_EQ(another, (a | c));
		another ^= a;
		EXPECT_EQ(another, c);
		another = 0;
		another = nullptr;
		auto is_zero = ((another & abc) == 0);
		EXPECT_TRUE(is_zero);
		EXPECT_FALSE(another & abc);
		auto more = a | another;
		auto just = a | 0;
		EXPECT_EQ(more, just);
		EXPECT_TRUE(just);
	}
} // namespace MethodNamespace

namespace FlagsNamespace
{
	enum class Flag : int
	{
		one = (1 << 0),
		two = (1 << 1),
		three = (1 << 2),
	};
	inline constexpr auto is_flag_type(Flag) { return true; }

	class Class
	{
	public:
		enum class Public : long
		{
			one = (1 << 2),
			two = (1 << 1),
			three = (1 << 0),
		};
		friend inline constexpr auto is_flag_type(Public) { return true; }

		static void TestPrivate();

	private:
		enum class Private : long
		{
			one = (1 << 0),
			two = (1 << 1),
			three = (1 << 2),
		};
		friend inline constexpr auto is_flag_type(Private) { return true; }
	};

	void Class::TestPrivate()
	{
		MethodNamespace::TestFlags(Private::one, Private::two, Private::three);
	}
} // namespace FlagsNamespace

namespace ExtendedNamespace
{
	enum class Flag : int
	{
		one = (1 << 3),
		two = (1 << 4),
		three = (1 << 5),
	};
} // namespace ExtendedNamespace

namespace plib::core::type
{
	template <>
	struct extended_flags<ExtendedNamespace::Flag>
	{
		using type = FlagsNamespace::Flag;
	};
} // namespace plib::core::type::::

TEST(ConfParserTest, BasicParseAndGet)
{
	std::string config_text = R"(
        key1=123
        key2=hello
        key3=True
        key4=3.14
    )";
	ConfParser parser;
	Conf conf = parser.from_str(config_text);

	EXPECT_EQ(conf.size(), 4);
	EXPECT_EQ(conf["key1"].as_string(), "123");
	EXPECT_EQ(conf["key2"].as_string(), "hello");
	EXPECT_EQ(conf["key3"].as_string(), "True");
	EXPECT_EQ(conf["key4"].as_string(), "3.14");

	// 类型转换
	EXPECT_EQ(conf["key1"].parse<int>(), 123);
	EXPECT_EQ(conf["key3"].parse<bool>(), true);
	EXPECT_DOUBLE_EQ(conf["key4"].parse<double>(), 3.14);
}

TEST(ConfParserTest, TryParseAndDefault)
{
	std::string config_text = "flag=notabool";
	ConfParser parser;
	Conf conf = parser.from_str(config_text);
	// try_parse返回默认值
	EXPECT_EQ(conf["flag"].try_parse<bool>(false), false);
	EXPECT_EQ(conf["flag"].try_parse<int>(42), 42);
}

TEST(ConfParserTest, CommentAndWhitespace)
{
	std::string config_text = R"(
        # This is a comment
        key1 = value1
        ; Another comment
        key2= value2
    )";
	ConfParser parser;
	Conf conf = parser.from_str(config_text);
	EXPECT_EQ(conf.size(), 2);
	EXPECT_EQ(conf["key1"].as_string(), "value1");
	EXPECT_EQ(conf["key2"].as_string(), "value2");
}

TEST(YamlConfigTest, BasicYamlRead)
{
	using namespace plib::core::config;
	std::string yaml_text = R"(
key1: 123
key2: hello
key3: true
key4: 3.14
)";
	YamlReader node = read_yaml_str(yaml_text);
	EXPECT_EQ(node["key1"].as<int>(), 123);
	EXPECT_EQ(node["key2"].as<std::string>(), "hello");
	EXPECT_EQ(node["key3"].as<bool>(), true);
	EXPECT_DOUBLE_EQ(node["key4"].as<double>(), 3.14);
}

TEST(YamlConfigTest, BasicYamlWrite)
{
	YamlReader node;
	node["key1"] = 123;
	node["key2"] = "hello";
	node["key3"] = true;
	node["key4"] = 3.14;

	YamlWriter out;
	out << node;

	std::string yaml_str = out.c_str();
	// 解析回去，验证内容一致
	YamlReader parsed = read_yaml_str(yaml_str);
	EXPECT_EQ(parsed["key1"].as<int>(), 123);
	EXPECT_EQ(parsed["key2"].as<std::string>(), "hello");
	EXPECT_EQ(parsed["key3"].as<bool>(), true);
	EXPECT_DOUBLE_EQ(parsed["key4"].as<double>(), 3.14);
}

TEST(FlagsTest, NonMemberFlags)
{
	MethodNamespace::TestFlags(
		FlagsNamespace::Flag::one,
		FlagsNamespace::Flag::two,
		FlagsNamespace::Flag::three);
}

TEST(FlagsTest, PublicMemberFlags)
{
	MethodNamespace::TestFlags(
		FlagsNamespace::Class::Public::one,
		FlagsNamespace::Class::Public::two,
		FlagsNamespace::Class::Public::three);
}

TEST(FlagsTest, PrivateMemberFlags)
{
	FlagsNamespace::Class::TestPrivate();
}

struct int_wrap
{
	int value;
};
struct int_wrap_comparator
{
	inline bool operator()(const int_wrap &a, const int_wrap &b) const
	{
		return a.value < b.value;
	}
	using is_transparent = void;
	inline bool operator()(const int &a, const int_wrap &b) const
	{
		return a < b.value;
	}
	inline bool operator()(const int_wrap &a, const int &b) const
	{
		return a.value < b;
	}
	inline bool operator()(const int &a, const int &b) const
	{
		return a < b;
	}
};

// flat_map tests

TEST(FlatMapTest, ShouldKeepItemsSortedByKey)
{
	plib::core::type::flat_map<int, std::string> v;
	v.emplace(0, "a");
	v.emplace(5, "b");
	v.emplace(4, "d");
	v.emplace(2, "e");

	auto checkSorted = [&]
	{
		auto prev = v.begin();
		ASSERT_NE(prev, v.end());
		for (auto i = prev + 1; i != v.end(); prev = i, ++i)
		{
			ASSERT_LT(prev->first, i->first);
		}
	};
	ASSERT_EQ(v.size(), 4);
	checkSorted();

	// SECTION("adding item puts it in the right position")
	v.emplace(3, "c");
	ASSERT_EQ(v.size(), 5);
	ASSERT_NE(v.find(3), v.end());
	checkSorted();
}

TEST(FlatMapTest, CopyConstructor)
{
	plib::core::type::flat_map<int, std::string> v;
	v.emplace(0, "a");
	v.emplace(2, "b");
	auto u = v;
	ASSERT_EQ(u.size(), 2);
	ASSERT_EQ(u.find(0), u.begin());
	ASSERT_EQ(u.find(2), u.end() - 1);
}

TEST(FlatMapTest, Assignment)
{
	plib::core::type::flat_map<int, std::string> v, u;
	v.emplace(0, "a");
	v.emplace(2, "b");
	u = v;
	ASSERT_EQ(u.size(), 2);
	ASSERT_EQ(u.find(0), u.begin());
	ASSERT_EQ(u.find(2), u.end() - 1);
}

TEST(FlatMapTest, CustomComparator)
{
	plib::core::type::flat_map<int_wrap, std::string, int_wrap_comparator> v;
	v.emplace(int_wrap{0}, "a");
	v.emplace(int_wrap{5}, "b");
	v.emplace(int_wrap{4}, "d");
	v.emplace(int_wrap{2}, "e");

	auto checkSorted = [&]
	{
		auto prev = v.begin();
		ASSERT_NE(prev, v.end());
		for (auto i = prev + 1; i != v.end(); prev = i, ++i)
		{
			ASSERT_TRUE(int_wrap_comparator()(prev->first, i->first));
		}
	};
	ASSERT_EQ(v.size(), 4);
	checkSorted();

	// SECTION("adding item puts it in the right position")
	v.emplace(int_wrap{3}, "c");
	ASSERT_EQ(v.size(), 5);
	ASSERT_NE(v.find({3}), v.end());
	checkSorted();
}

TEST(FlatMapTest, StructuredBindings)
{
	plib::core::type::flat_map<int, std::unique_ptr<double>> v;
	v.emplace(0, std::make_unique<double>(0.));
	v.emplace(1, std::make_unique<double>(1.));

	// structred binded range-based for loop
	for (const auto &[key, value] : v)
	{
		ASSERT_EQ(key, int(std::round(*value)));
	}

	// non-const structured binded range-based for loop
	plib::core::type::flat_map<int, int> second = {
		{1, 1},
		{2, 2},
		{2, 3},
		{3, 3},
	};
	ASSERT_EQ(second.size(), 3);
	for (const auto [a, b] : second)
	{
		ASSERT_EQ(a, b);
	}
}

// flat_set tests
TEST(FlatSetTest, ShouldKeepItemsSorted)
{
	plib::core::type::flat_set<int> v;
	v.insert(0);
	v.insert(5);
	v.insert(4);
	v.insert(2);

	ASSERT_TRUE(v.contains(4));

	auto checkSorted = [&]
	{
		auto prev = v.begin();
		ASSERT_NE(prev, v.end());
		for (auto i = prev + 1; i != v.end(); prev = i, ++i)
		{
			ASSERT_LT(*prev, *i);
		}
	};
	ASSERT_EQ(v.size(), 4);
	checkSorted();

	// SECTION("adding item puts it in the right position")
	v.insert(3);
	ASSERT_EQ(v.size(), 5);
	ASSERT_NE(v.find(3), v.end());
	checkSorted();
}

TEST(FlatSetTest, CustomComparators)
{
	plib::core::type::flat_set<int_wrap, int_wrap_comparator> v;
	v.insert({0});
	v.insert({5});
	v.insert({4});
	v.insert({2});

	ASSERT_NE(v.find(4), v.end());

	auto checkSorted = [&]
	{
		auto prev = v.begin();
		ASSERT_NE(prev, v.end());
		for (auto i = prev + 1; i != v.end(); prev = i, ++i)
		{
			ASSERT_LT(prev->value, i->value);
		}
	};
	ASSERT_EQ(v.size(), 4);
	checkSorted();

	// SECTION("adding item puts it in the right position")
	v.insert({3});
	ASSERT_EQ(v.size(), 5);
	ASSERT_NE(v.find(3), v.end());
	checkSorted();
}

TEST(LoggerTest, BasicLogToFile)
{
	// 假设 Logger 支持设置日志文件
	using namespace plib::core::log;
	LogConfig config;
	config.appName = "plib-test";
	config.enableAsync = true;
	config.enableColor = true;
	config.enableConsole = true;
	config.enableFile = true;
	config.logLevel = LogLevel::DEBUG;
	config.maxFiles = 10;
	config.maxFileSize = 32 * 1024 * 1024;
	config.logPath = plib::core::utils::get_executable_dir();
	LogManager::init(config);
	LOG_INFO("float: {:.2f}", 3.14159);
	LOG_WARN("hex: {:#x}", 255);
	LOG_ERROR("oct: {:#o}, bin: {:#b}", 10, 10);
	LOG_INFO("|{:>6}| |{:<6}| |{:^6}| |{:0>6}|", 42, 42, 42, 42);
	LOG_DEBUG("str: '{}', escaped: '{{}}'", "hi");
	LOG_INFO("{:.3}", "abcdefg");
	LOG_INFO("{:8.3f}", 3.14159);
	LOG_INFO("{} + {} = {}", 1, 2, 1 + 2);
	int x = 42;
	// LOG_DEBUG("ptr: {:p}", &x);
	LOG_INFO("{:.2e}", 12345.6789);
	LOG_INFO("{:+d} {:+d}", 42, -42);
#if __cpp_lib_chrono >= 201907L
	using namespace std::chrono;
	auto tp = sys_days{year{2024} / 5 / 20};
	LOG_INFO("date: {:%Y-%m-%d}", tp);
#endif
}

// 测试 MemoryPool 对基本类型的分配、构造和释放
TEST(MemoryPoolTest, BasicAllocationAndReuse)
{
	MemoryPool<int> pool;

	// 使用 newElement 分配一个 int，并构造为 10
	int *a = pool.newElement(10);
	ASSERT_NE(a, nullptr);
	EXPECT_EQ(*a, 10);

	// 释放 a，使内存进入 freeSlots
	pool.deleteElement(a);

	// 再次分配一个 int，赋值为 20
	int *b = pool.newElement(20);
	ASSERT_NE(b, nullptr);
	EXPECT_EQ(*b, 20);

	pool.deleteElement(b);
}

// 测试 MemoryPool 对复杂类型的分配与释放
TEST(MemoryPoolTest, StringAllocation)
{
	MemoryPool<std::string> pool;

	// 使用 newElement 分配一个 std::string 对象
	std::string *s = pool.newElement("hello world");
	ASSERT_NE(s, nullptr);
	EXPECT_EQ(*s, "hello world");

	pool.deleteElement(s);
}

// 测试多次分配后内存的复用情况
TEST(MemoryPoolTest, MultipleAllocationAndDeletion)
{
	MemoryPool<int> pool;
	const int allocCount = 100;
	std::vector<int *> ptrs;

	// 连续分配 100 个 int 对象
	for (int i = 0; i < allocCount; ++i)
	{
		int *p = pool.newElement(i);
		ASSERT_NE(p, nullptr);
		ptrs.push_back(p);
	}

	// 验证所有分配的对象内容正确
	for (int i = 0; i < allocCount; ++i)
	{
		EXPECT_EQ(*ptrs[i], i);
	}

	// 删除所有对象
	for (auto p : ptrs)
	{
		pool.deleteElement(p);
	}

	// 再次分配测试是否能重用内存
	int *reuse = pool.newElement(999);
	ASSERT_NE(reuse, nullptr);
	EXPECT_EQ(*reuse, 999);
	pool.deleteElement(reuse);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}