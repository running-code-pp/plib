#ifndef PLIB_CORE_UTILS_YAMLJSON_HPP_
#define PLIB_CORE_UTILS_YAMLJSON_HPP_
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace plib::core::utils
{
    // YAML -> JSON 核心转换函数
    nlohmann::json yaml_to_json(const std::string &yaml_str);
    nlohmann::json yaml_node_to_json(const YAML::Node &node);

    // JSON -> YAML 核心转换函数
    std::string json_to_yaml(const nlohmann::json &j);
    YAML::Node json_to_yaml_node(const nlohmann::json &j);

    // 核心类：YamJSON - 统一的YAML/JSON处理接口
    class YamJSON
    {
    private:
        std::string original_yaml_; // 保存原始YAML文本，包含注释
        nlohmann::json json_data_;  // 保存转换后的JSON数据
        std::string file_path_;     // 文件路径，用于读写操作

    public:
        // 构造函数
        YamJSON();
        explicit YamJSON(const std::string &yaml_content);
        explicit YamJSON(const nlohmann::json &json_data);

        // 工厂方法
        static YamJSON load(const std::string &file_path);     // 从文件加载
        static YamJSON parse(const std::string &yaml_content); // 从字符串解析

        // 转换方法
        nlohmann::json to_json() const { return json_data_; } // 转换为JSON
        std::string to_yaml() const;                          // 转换为YAML字符串（保留注释）
        YAML::Node to_yaml_node() const;                      // 转换为YAML::Node对象

        // 从JSON/YAML转换 - 使用重载函数替代多个命名函数
        static YamJSON from_json(const nlohmann::json &json_data);
        static YamJSON from_yaml(const std::string &yaml_content); // 从YAML字符串创建
        static YamJSON from_yaml(const YAML::Node &node);          // 从YAML::Node创建，使用重载

        // 美观输出，默认缩进为2
        std::string dump(int indent = 2, bool as_json = false) const;

        // 获取内部数据
        const nlohmann::json &get_json() const { return json_data_; }
        nlohmann::json &get_json() { return json_data_; }
        const std::string &original_yaml() const { return original_yaml_; }

        // 保存方法
        bool save() const;                                // 保存到当前文件
        bool save_to(const std::string &file_path) const; // 保存到指定文件

        // 文件路径操作
        void set_file_path(const std::string &file_path) { file_path_ = file_path; }
        const std::string &get_file_path() const { return file_path_; }

        // 重新加载
        bool reload();

        // 路径更新
        bool update_value(const std::vector<std::string> &path, const nlohmann::json &value);

        // 操作符重载
        template <typename T>
        auto operator[](T &&key) -> decltype(std::declval<nlohmann::json>()[std::forward<T>(key)])
        {
            return json_data_[std::forward<T>(key)];
        }

        template <typename T>
        auto operator[](T &&key) const -> decltype(std::declval<const nlohmann::json>()[std::forward<T>(key)])
        {
            return json_data_[std::forward<T>(key)];
        }

        // 赋值操作符
        YamJSON &operator=(const nlohmann::json &json_data);
        YamJSON &operator=(const std::string &yaml_content);
        YamJSON &operator=(const YAML::Node &node);
    };

    // 为 YamJSON 提供序列化支持
    void to_json(nlohmann::json &j, const YamJSON &yam);
    void from_json(const nlohmann::json &j, YamJSON &yam);
} // namespace plib::core::utils

// 简化的ADL集成接口
namespace nlohmann
{
    // 从 YAML 字符串解析
    inline void from_yaml(const std::string &yaml_str, json &j)
    {
        j = plib::core::utils::yaml_to_json(yaml_str);
    }

    // YAML 字符串 -> 任意类型 (遵循 from_json 的模式)
    template <typename ValueType>
    inline void from_yaml(const std::string &yaml_str, ValueType &val)
    {
        json j = plib::core::utils::yaml_to_json(yaml_str);
        j.get_to(val); // 使用 json 的 get_to 方法
    }

    // 将 JSON 转换为 YAML 字符串
    inline std::string to_yaml(const json &j)
    {
        return plib::core::utils::json_to_yaml(j);
    }
}

#endif // PLIB_CORE_UTILS_YAMLJSON_HPP_