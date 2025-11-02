#include "utils/yaml_json.hpp"
#include <stdexcept>
#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>
#include "utils/string_util.hpp"

namespace plib::core::utils
{

    // YAML 转 JSON 的核心递归转换函数
    nlohmann::json yaml_node_to_json(const YAML::Node &node)
    {
        using namespace nlohmann;

        switch (node.Type())
        {
        case YAML::NodeType::Scalar:
        {
            // 尝试解析标量类型
            try
            {
                return node.as<int>();
            }
            catch (...)
            {
                try
                {
                    return node.as<double>();
                }
                catch (...)
                {
                    std::string s = node.as<std::string>();

                    // 处理常见布尔值表示
                    if (s == "true" || s == "True" || s == "TRUE")
                        return true;
                    if (s == "false" || s == "False" || s == "FALSE")
                        return false;

                    // 处理 YAML 的 yes/no 布尔表示
                    if (s == "yes" || s == "Yes" || s == "YES")
                        return true;
                    if (s == "no" || s == "No" || s == "NO")
                        return false;

                    // 处理 null
                    if (s == "null" || s == "Null" || s == "NULL")
                        return nullptr;

                    // 判断是否是数字
                    if (plib::core::utils::isNumeric(s))
                        return std::stod(s);
                    return s;
                }
            }
        }
        case YAML::NodeType::Sequence:
        {
            json arr = json::array();
            for (const auto &child : node)
            {
                arr.push_back(yaml_node_to_json(child));
            }
            return arr;
        }
        case YAML::NodeType::Map:
        {
            json obj = json::object();
            for (const auto &pair : node)
            {
                std::string key = pair.first.as<std::string>();
                obj[key] = yaml_node_to_json(pair.second);
            }
            return obj;
        }
        case YAML::NodeType::Null:
            return nullptr;
        default:
            throw std::runtime_error("Unsupported YAML node type");
        }
    }

    // JSON 转 YAML 的核心递归转换函数
    YAML::Node json_to_yaml_node(const nlohmann::json &j)
    {
        using namespace nlohmann;

        if (j.is_object())
        {
            YAML::Node node(YAML::NodeType::Map);
            for (auto &[key, value] : j.items())
            {
                node[key] = json_to_yaml_node(value);
            }
            return node;
        }
        if (j.is_array())
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &element : j)
            {
                node.push_back(json_to_yaml_node(element));
            }
            return node;
        }
        if (j.is_null())
        {
            return YAML::Node(YAML::NodeType::Null);
        }
        if (j.is_boolean())
        {
            return YAML::Node(j.get<bool>());
        }
        if (j.is_number_integer())
        {
            return YAML::Node(j.get<int64_t>());
        }
        if (j.is_number_float())
        {
            return YAML::Node(j.get<double>());
        }
        if (j.is_string())
        {
            return YAML::Node(j.get<std::string>());
        }
        throw std::runtime_error("Unsupported JSON type");
    }

    // 公开接口：YAML 字符串 -> JSON 对象
    nlohmann::json yaml_to_json(const std::string &yaml_str)
    {
        try
        {
            YAML::Node root = YAML::Load(yaml_str);
            return yaml_node_to_json(root);
        }
        catch (const YAML::Exception &e)
        {
            throw std::runtime_error(std::string("YAML parse error: ") + e.what());
        }
    }

    // 公开接口：JSON 对象 -> YAML 字符串
    std::string json_to_yaml(const nlohmann::json &j)
    {
        try
        {
            YAML::Emitter emitter;
            emitter << json_to_yaml_node(j);
            if (!emitter.good())
            {
                throw std::runtime_error("YAML emit error");
            }
            return emitter.c_str();
        }
        catch (const YAML::Exception &e)
        {
            throw std::runtime_error(std::string("YAML emit error: ") + e.what());
        }
    }

    //========== YamJSON 实现 ==========

    // 构造函数
    YamJSON::YamJSON() : json_data_(nlohmann::json::object()) {}

    YamJSON::YamJSON(const std::string &yaml_content) : original_yaml_(yaml_content)
    {
        // 解析YAML为JSON
        try
        {
            // 先尝试验证YAML是否可以解析
            try
            {
                YAML::Node testNode = YAML::Load(yaml_content);
                // 如果能解析，继续正常流程
            }
            catch (const YAML::Exception &e)
            {
                std::cerr << "YAML解析警告（构造函数）: " << e.what() << std::endl;
                // 为了避免段错误，使用一个空的JSON对象初始化
                json_data_ = nlohmann::json::object();
                return; // 提前退出构造函数
            }

            // 正常流程：解析为JSON
            json_data_ = yaml_to_json(yaml_content);
        }
        catch (const std::exception &e)
        {
            std::cerr << "初始化错误: " << e.what() << std::endl;
            // 使用空JSON对象初始化，避免未初始化的对象
            json_data_ = nlohmann::json::object();
        }
        catch (...)
        {
            std::cerr << "未知初始化错误" << std::endl;
            json_data_ = nlohmann::json::object();
        }
    }

    YamJSON::YamJSON(const nlohmann::json &json_data) : json_data_(json_data) {}

    // 静态工厂方法
    YamJSON YamJSON::load(const std::string &file_path)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            throw std::runtime_error("无法打开YAML文件: " + file_path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        YamJSON result(buffer.str());
        result.file_path_ = file_path;
        return result;
    }

    YamJSON YamJSON::parse(const std::string &yaml_content)
    {
        return YamJSON(yaml_content);
    }

    // 转换方法
    YamJSON YamJSON::from_json(const nlohmann::json &json_data)
    {
        return YamJSON(json_data);
    }

    YamJSON YamJSON::from_yaml(const std::string &yaml_content)
    {
        return YamJSON(yaml_content);
    }

    // 重载版本 - 从YAML::Node创建
    YamJSON YamJSON::from_yaml(const YAML::Node &node)
    {
        nlohmann::json j = yaml_node_to_json(node);
        return YamJSON(j);
    }

    YAML::Node YamJSON::to_yaml_node() const
    {
        return json_to_yaml_node(json_data_);
    }

    // 输出YAML（保留注释）
    std::string YamJSON::to_yaml() const
    {
        if (original_yaml_.empty())
        {
            // 如果没有原始YAML，直接转换为YAML
            return json_to_yaml(json_data_);
        }

        // 安全解析原始YAML文档结构
        YAML::Node original_node;
        try
        {
            original_node = YAML::Load(original_yaml_);
        }
        catch (const YAML::Exception &e)
        {
            std::cerr << "YAML解析警告（to_yaml方法）: " << e.what() << std::endl;
            // 如果原始YAML解析失败，回退到无注释版本
            return json_to_yaml(json_data_);
        }
        catch (...)
        {
            std::cerr << "YAML解析未知错误（to_yaml方法）" << std::endl;
            return json_to_yaml(json_data_);
        }

        // 创建YAML解析器保留原始注释
        try
        {
            // 这里我们需要手动操作YAML内容，保留注释
            // 由于yaml-cpp库不直接支持操作注释，我们需要使用文本处理方法

            // 将JSON数据应用到YAML文档
            std::string result = original_yaml_;

            // 将修改后的JSON转为YAML（不含注释）
            std::string modified_yaml = json_to_yaml(json_data_);

            // 逐行处理，保留注释但更新值
            std::istringstream original_stream(original_yaml_);
            std::istringstream modified_stream(modified_yaml);

            std::vector<std::string> original_lines;
            std::string line;
            while (std::getline(original_stream, line))
            {
                original_lines.push_back(line);
            }

            std::map<std::string, std::string> modified_values;
            while (std::getline(modified_stream, line))
            {
                // 找出键值对
                std::regex key_value_regex(R"((\s*)([\w\-\.]+)(\s*):(\s*)(.+))");
                std::smatch matches;
                if (std::regex_match(line, matches, key_value_regex))
                {
                    std::string key = matches[2].str();
                    std::string value = matches[5].str();
                    modified_values[key] = value;
                }
            }

            // 处理原始YAML，保留注释并更新值
            std::vector<std::string> result_lines;
            for (const auto &original_line : original_lines)
            {
                std::string processed_line = original_line;

                // 检查是否是注释行
                if (original_line.find('#') != std::string::npos)
                {
                    result_lines.push_back(original_line); // 保留注释行
                    continue;
                }

                // 检查是否是键值对
                std::regex key_value_regex(R"((\s*)([\w\-\.]+)(\s*):(\s*)(.+))");
                std::smatch matches;
                if (std::regex_match(original_line, matches, key_value_regex))
                {
                    std::string indent = matches[1].str();
                    std::string key = matches[2].str();
                    std::string mid_space1 = matches[3].str();
                    std::string mid_space2 = matches[4].str();

                    // 检查这个键是否有更新值
                    auto it = modified_values.find(key);
                    if (it != modified_values.end())
                    {
                        processed_line = indent + key + mid_space1 + ":" + mid_space2 + it->second;
                    }
                }

                result_lines.push_back(processed_line);
            }

            // 将处理后的行组合回字符串
            std::string final_result;
            for (const auto &line : result_lines)
            {
                final_result += line + "\n";
            }

            return final_result;
        }
        catch (const YAML::Exception &e)
        {
            std::cerr << "YAML处理警告: " << e.what() << std::endl;
            // 如果出错，回退到不保留注释的方法
            return json_to_yaml(json_data_);
        }
        catch (const std::exception &e)
        {
            std::cerr << "处理错误: " << e.what() << std::endl;
            return json_to_yaml(json_data_);
        }
        catch (...)
        {
            std::cerr << "未知错误" << std::endl;
            return json_to_yaml(json_data_);
        }
    }

    // 美观输出方法
    std::string YamJSON::dump(int indent, bool as_json) const
    {
        if (as_json)
        {
            // 输出为JSON格式
            return json_data_.dump(indent);
        }
        else
        {
            // 输出为YAML格式
            if (indent > 0)
            {
                // 使用设置的缩进
                YAML::Emitter emitter;
                emitter.SetIndent(indent);
                emitter << to_yaml_node();
                if (emitter.good())
                {
                    return emitter.c_str();
                }
                // 如果设置缩进失败，则回退到普通输出
            }

            // 如果有原始YAML（含注释），使用to_yaml保留注释
            return to_yaml();
        }
    }

    // 保存方法
    bool YamJSON::save() const
    {
        if (file_path_.empty())
        {
            return false;
        }
        return save_to(file_path_);
    }

    bool YamJSON::save_to(const std::string &file_path) const
    {
        std::ofstream file(file_path);
        if (!file.is_open())
        {
            return false;
        }
        file << to_yaml();
        return true;
    }

    // 重新加载
    bool YamJSON::reload()
    {
        if (file_path_.empty())
        {
            return false;
        }
        try
        {
            std::ifstream file(file_path_);
            if (!file.is_open())
            {
                return false;
            }
            std::stringstream buffer;
            buffer << file.rdbuf();

            // 保存原始YAML内容并重新解析
            original_yaml_ = buffer.str();
            json_data_ = yaml_to_json(original_yaml_);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // 路径更新
    bool YamJSON::update_value(const std::vector<std::string> &path, const nlohmann::json &value)
    {
        try
        {
            // 创建指向json_data_的指针
            nlohmann::json *current = &json_data_;

            // 遍历路径
            for (size_t i = 0; i < path.size(); ++i)
            {
                const std::string &key = path[i];

                // 最后一个路径节点
                if (i == path.size() - 1)
                {
                    (*current)[key] = value;
                    return true;
                }

                // 检查中间节点是否存在
                if (!current->contains(key) || !(*current)[key].is_object())
                {
                    (*current)[key] = nlohmann::json::object();
                }

                // 移动到下一级
                current = &(*current)[key];
            }

            return false; // 空路径
        }
        catch (const std::exception &e)
        {
            std::cerr << "exception in update_value: " << e.what() << std::endl;
            return false;
        }
    }

    // 赋值操作符
    YamJSON &YamJSON::operator=(const nlohmann::json &json_data)
    {
        json_data_ = json_data;
        return *this;
    }

    YamJSON &YamJSON::operator=(const std::string &yaml_content)
    {
        original_yaml_ = yaml_content;
        try
        {
            json_data_ = yaml_to_json(yaml_content);
        }
        catch (const std::exception &e)
        {
            std::cerr << "赋值操作错误: " << e.what() << std::endl;
            // 出错时保持JSON数据不变
        }
        return *this;
    }

    YamJSON &YamJSON::operator=(const YAML::Node &node)
    {
        try
        {
            json_data_ = yaml_node_to_json(node);
            // 原始YAML内容无法保留，因为YAML::Node不包含注释信息
            original_yaml_ = "";
        }
        catch (const std::exception &e)
        {
            std::cerr << "从YAML::Node赋值错误: " << e.what() << std::endl;
        }
        return *this;
    }

    // 为 YamJSON 提供序列化支持
    void to_json(nlohmann::json &j, const YamJSON &yam)
    {
        j = yam.to_json();
    }

    void from_json(const nlohmann::json &j, YamJSON &yam)
    {
        yam = YamJSON::from_json(j);
    }

} // namespace plib::core::utils