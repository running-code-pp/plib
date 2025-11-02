/**
 * @Author: running-code-pp
 * @Date: 2025-10-28 22:21:33
 * @LastEditors: running-code-pp
 * @LastEditTime: 2025-10-29 13:14:09
 * @FilePath: \plib\test\other\simd_json_test.cpp
 * @Description: 配置第三方库测试
 * @Copyright: Copyright (c) 2025 by running-code-pp 3320996652@qq.com, All Rights Reserved.
 */
#include <simdjson.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stack>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
#include "utils/yaml_json.hpp"

#ifdef TEST_SELF_TRANS
nlohmann::json yaml_node_to_json(const YAML::Node &node)
{
    switch (node.Type())
    {
    case YAML::NodeType::Scalar:
    {
        const std::string tag = node.Tag();
        std::cout << "tag: " << tag << std::endl;
        if (tag == "tag:yaml.org,2002:null")
            return nullptr;
        if (tag == "tag:yaml.org,2002:bool")
            return node.as<bool>();
        if (tag == "tag:yaml.org,2002:int")
            return node.as<int64_t>();
        if (tag == "tag:yaml.org,2002:float")
            return node.as<double>();
        if (tag == "tag:yaml.org,2002:str")
            return node.as<std::string>();

        const std::string scalar = node.Scalar();
        if (scalar == "null")
            return nullptr;
        if (scalar == "true" || scalar == "false")
            return node.as<bool>();

        try
        {
            std::size_t pos = 0;
            long long value = std::stoll(scalar, &pos, 10);
            if (pos == scalar.size())
                return value;
        }
        catch (...)
        {
        }

        try
        {
            std::size_t pos = 0;
            double value = std::stod(scalar, &pos);
            if (pos == scalar.size())
                return value;
        }
        catch (...)
        {
        }

        return scalar;
    }

    case YAML::NodeType::Sequence:
    {
        nlohmann::json j_array = nlohmann::json::array();
        for (size_t i = 0; i < node.size(); ++i)
        {
            j_array.push_back(yaml_node_to_json(node[i]));
        }
        return j_array;
    }

    case YAML::NodeType::Map:
    {
        nlohmann::json j_object = nlohmann::json::object();
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            std::string key = it->first.as<std::string>();
            j_object[key] = yaml_node_to_json(it->second);
        }
        return j_object;
    }

    case YAML::NodeType::Null:
        return nullptr;

    case YAML::NodeType::Undefined:
        throw std::runtime_error("Undefined YAML node encountered");
    }
    return nullptr;
}

// 高层接口：从文件加载并转为 JSON
nlohmann::json yaml_to_json(const std::string &yaml_file)
{
    YAML::Node root = YAML::LoadFile(yaml_file);
    return yaml_node_to_json(root);
}

YAML::Node json_to_yaml_node(const nlohmann::json &j)
{
    switch (j.type())
    {
    case nlohmann::json ::value_t::null:
        return YAML::Node(YAML::NodeType::Null);

    case nlohmann::json ::value_t::boolean:
        return YAML::Node(j.get<bool>());

    case nlohmann::json ::value_t::number_integer:
    case nlohmann::json ::value_t::number_unsigned:
        return YAML::Node(j.get<int64_t>());

    case nlohmann::json ::value_t::number_float:
        return YAML::Node(j.get<double>());

    case nlohmann::json ::value_t::string:
    {
        YAML::Node scalar(j.get<std::string>());
        return scalar;
    }

    case nlohmann::json ::value_t::array:
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (const auto &item : j)
        {
            node.push_back(json_to_yaml_node(item));
        }
        return node;
    }

    case nlohmann::json ::value_t::object:
    {
        YAML::Node node(YAML::NodeType::Map);
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            node[it.key()] = json_to_yaml_node(it.value());
        }
        return node;
    }

    default:
        throw std::runtime_error("Unsupported JSON type for YAML conversion");
    }
}

// 高层接口：将 JSON 转为 YAML 并保存
std::string json_to_yaml_string(const nlohmann::json &j)
{
    YAML::Node root = json_to_yaml_node(j);
    return YAML::Dump(root);
}

void json_to_yaml_file(const nlohmann::json &j, const std::string &yaml_file)
{
    std::ofstream fout(yaml_file);
    fout << json_to_yaml_string(j);
}

#endif

/*
    // YAML -> JSON 核心转换函数
    nlohmann::json yaml_to_json(const std::string &yaml_str);
    nlohmann::json yaml_node_to_json(const YAML::Node &node);

    // JSON -> YAML 核心转换函数
    std::string json_to_yaml(const nlohmann::json &j);
    YAML::Node json_to_yaml_node(const nlohmann::json &j);

*/
// 测试json到yaml的相互转换
void test_json_yaml()
{
    nlohmann::json raw_json;
    std::ifstream ifs("test.json");
    if (!ifs.is_open())
    {
        std::cerr << "Failed to open test.json" << std::endl;
        return;
    }
    ifs >> raw_json;
    auto raw_yaml = YAML::LoadFile("test.yaml");
    auto converted_yaml = plib::core::utils::json_to_yaml(raw_json);
    auto converted_json = plib::core::utils::yaml_to_json(YAML::Dump(raw_yaml)).dump(4);
    if (converted_json == raw_json.dump(4))
    {
        std::cout << "JSON to YAML and back conversion successful!" << std::endl;
    }
    else
    {
        std::cout << "Conversion failed!" << std::endl;
        // 输出转换后的json和之前的json
        std::cout << "Converted JSON:\n"
                  << converted_json << std::endl;
        std::cout << "Raw JSON:\n"
                  << raw_json.dump(4) << std::endl;
    }

    if (converted_yaml == YAML::Dump(raw_yaml))
    {
        std::cout << "YAML to JSON and back conversion successful!" << std::endl;
    }
    else
    {
        std::cout << "Conversion failed!" << std::endl;
        // 输出转换后的yaml 和 原始yaml
        std::cout << "Converted YAML:\n"
                  << converted_yaml << std::endl;
        std::cout << "Raw YAML:\n"
                  << YAML::Dump(raw_yaml) << std::endl;
    }
}
int main()
{
    std::ifstream ifs("test.json");
    nlohmann::json morden_json;

    if (!ifs.is_open())
    {
        std::cerr << "Failed to open test.json" << std::endl;
        return 1;
    }

    std::string json;
    json.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    morden_json = nlohmann::json::parse(json);
    std::cout << morden_json.dump(4) << std::endl;
    std::cout << "\n\n\n\n\n\n";

    simdjson::dom::parser parser;
    simdjson::dom::object obj;
    auto error = parser.parse(json).get(obj);
    if (error)
    {
        std::cerr << "Error: " << error << std::endl;
        return 1;
    }
    for (const auto &[key, value] : obj)
    {
        std::cout << key << ": " << value << std::endl;
    }

    test_json_yaml();

    return 0;
}