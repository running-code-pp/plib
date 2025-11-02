/**
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-28 22:00:33
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-29 01:39:53
 * @FilePath: \plib\test\core\reflection_test.cpp
 * @Description:
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
// #include<yaml-cpp/yaml.h>
// #include<nlohmann/json.hpp>
// #include<reflection/reflection.hpp>

// struct Address {
//     std::string street;
//     std::string city;
//     std::string zipcode;
//     double latitude;
//     double longitude;
// };

// struct Job {
//     std::string title;
//     std::string company;
//     int start_year;
//     bool is_current;
//     std::vector<std::string> responsibilities;
// };

// struct Person {
//     int id;
//     std::string name;
//     double height;         // 身高（米）
//     bool is_active;
//     std::string email;

//     // 嵌套结构体
//     Address home_address;
//     Job employment;

//     // 向量（数组）
//     std::vector<std::string> phone_numbers;
//     std::vector<int> favorite_numbers;

//     // Map：技能名称 → 熟练度（1-10）
//     std::map<std::string, int> skills;

//     // Map 嵌套 vector
//     std::map<std::string, std::vector<std::string>> projects;

//     // Map 嵌套自定义结构体（比如历史住址）
//     std::map<std::string, Address> past_addresses;

//     // Vector 嵌套 map
//     std::vector<std::map<std::string, std::string>> metadata;
// };

#include "reflection/reflection.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <set>
#include "config/conf/conf.hpp"
#include <map>
#include <fstream>
struct Score
{
    ENABLE_REFLECT("Score", sizeof(Score))
    REFLECTABLE(Score, std::string, subject)
    REFLECTABLE(Score, float, score)
};

struct Person
{
    ENABLE_REFLECT("Person", sizeof(Person))
    REFLECTABLE(Person, std::string, name)
    REFLECTABLE(Person, int, age)
    REFLECTABLE(Person, std::vector<int>, _vec_int)
    REFLECTABLE(Person, std::vector<Score>, score)
};

// 反射序列化反序列化测试--conf配置序列化反序列化
void test_conf_serialization()
{
    using namespace plib::core::config;
    ConfParser parser;

    std::ifstream ifs("test.conf");
    if (!ifs.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return;
    }
    std::string conf_content = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    Conf conf = parser.from_str(conf_content);
    if (conf.get("name").parse<std::string>() == "example_config")
    {
        std::cout << "Config parsed successfully!" << std::endl;
    }
}

void test_json()
{
    Person p;
    p.name = "Alice";
    p.age = 30;
    p._vec_int = {1, 2, 3, 4, 5};
    p.score.emplace_back("Mathematics", 95.5f);
    p.score.emplace_back("Physics", 89.0f);
    p.score.emplace_back("Chemistry", 92.0f);
    auto str = reflection::getDescriptor<Person>()->dump(&p);
    auto p1 = reflection::parse_json<Person>(str);
    if (str == reflection::getDescriptor<Person>()->dump(&p1))
    {
        std::cout << "JSON serialization/deserialization successful!" << std::endl;
    }
    else
    {
        std::cerr << "JSON failed!" << std::endl;
    }
}

void test_yaml()
{
    Person p;
    p.name = "Alice";
    p.age = 30;
    p._vec_int = {1, 2, 3, 4, 5};
    p.score.emplace_back("Mathematics", 95.5f);
    p.score.emplace_back("Physics", 89.0f);
    p.score.emplace_back("Chemistry", 92.0f);
    auto str = reflection::getDescriptor<Person>()->dump(&p);
    auto p1 = reflection::parse_json<Person>(str);
    if (str == reflection::getDescriptor<Person>()->dump(&p1))
    {
        std::cout << "JSON serialization/deserialization successful!" << std::endl;
    }
    else
    {
        std::cerr << "JSON failed!" << std::endl;
    }
}

int main()
{
    // test_conf_serialization();

    return 0;
}