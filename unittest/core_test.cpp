#include <gtest/gtest.h>
#include"../core/config/conf/conf.hpp"
#include <sstream>
#include "../core/config/yaml/yaml.hpp"
#include "../core/type/flags.hpp"

using namespace plib::core::config;
//using namespace plib::core::type; dont use this

namespace MethodNamespace {

template <typename Enum>
void TestFlags(Enum a, Enum b, Enum c) {
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

namespace FlagsNamespace {

enum class Flag : int {
    one = (1 << 0),
    two = (1 << 1),
    three = (1 << 2),
};
inline constexpr auto is_flag_type(Flag) { return true; }

class Class {
public:
    enum class Public : long {
        one = (1 << 2),
        two = (1 << 1),
        three = (1 << 0),
    };
    friend inline constexpr auto is_flag_type(Public) { return true; }

    static void TestPrivate();

private:
    enum class Private : long {
        one = (1 << 0),
        two = (1 << 1),
        three = (1 << 2),
    };
    friend inline constexpr auto is_flag_type(Private) { return true; }

};

void Class::TestPrivate() {
    MethodNamespace::TestFlags(Private::one, Private::two, Private::three);
}

} // namespace FlagsNamespace

namespace ExtendedNamespace {

enum class Flag : int {
    one = (1 << 3),
    two = (1 << 4),
    three = (1 << 5),
};

} // namespace ExtendedNamespace

namespace plib::core::type {

template<>
struct extended_flags<ExtendedNamespace::Flag> {
    using type = FlagsNamespace::Flag;
};

} // namespace plib::core::type



TEST(ConfParserTest, BasicParseAndGet) {
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

TEST(ConfParserTest, TryParseAndDefault) {
    std::string config_text = "flag=notabool";
    ConfParser parser;
    Conf conf = parser.from_str(config_text);
    // try_parse返回默认值
    EXPECT_EQ(conf["flag"].try_parse<bool>(false), false);
    EXPECT_EQ(conf["flag"].try_parse<int>(42), 42);
}

TEST(ConfParserTest, CommentAndWhitespace) {
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

TEST(YamlConfigTest, BasicYamlRead) {
    using namespace plib::core::config;
    std::string yaml_text = R"(
key1: 123
key2: hello
key3: true
key4: 3.14
)";
    YamlReader node= read_yaml_str(yaml_text);
    EXPECT_EQ(node["key1"].as<int>(), 123);
    EXPECT_EQ(node["key2"].as<std::string>(), "hello");
    EXPECT_EQ(node["key3"].as<bool>(), true);
    EXPECT_DOUBLE_EQ(node["key4"].as<double>(), 3.14);
}

TEST(YamlConfigTest, BasicYamlWrite) {
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

TEST(FlagsTest, NonMemberFlags) {
    MethodNamespace::TestFlags(
        FlagsNamespace::Flag::one,
        FlagsNamespace::Flag::two,
        FlagsNamespace::Flag::three);
}

TEST(FlagsTest, PublicMemberFlags) {
    MethodNamespace::TestFlags(
        FlagsNamespace::Class::Public::one,
        FlagsNamespace::Class::Public::two,
        FlagsNamespace::Class::Public::three);
}

TEST(FlagsTest, PrivateMemberFlags) {
    FlagsNamespace::Class::TestPrivate();
}

//TEST(FlagsTest, ExtendedFlags) {
//    MethodNamespace::TestFlags(
//        ExtendedNamespace::Flag::one,
//        ExtendedNamespace::Flag::two,
//        ExtendedNamespace::Flag::three);
//
//    auto onetwo = FlagsNamespace::Flag::one | ExtendedNamespace::Flag::two;
//    auto twoone = ExtendedNamespace::Flag::two | FlagsNamespace::Flag::one;
//    EXPECT_EQ(onetwo, twoone);
//}

