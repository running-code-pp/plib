#include "config/yaml/yaml.hpp"
#include "fstream"

namespace plib::core::config{
    YamlReader read_yaml_file(const std::string& filename) {
        YAML::Node node;
        try {
            node = YAML::LoadFile(filename);
        }
        catch (const YAML::Exception& e) {
            throw std::runtime_error("Failed to read YAML file: " + std::string(e.what()));
        }
        return node;
    }
    
    YamlReader read_yaml_str(const std::string& content) {
        YAML::Node node;
        try {
            node = YAML::Load(content);
        }
        catch (const YAML::Exception& e) {
            throw std::runtime_error("Failed to parse YAML string: " + std::string(e.what()));
        }
        return node;
    }
    
    void write_yaml(const std::string& filename, const YamlWriter& writer) {
        try {
            std::ofstream fout(filename);
            fout << writer.c_str();
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Failed to write YAML file: " + std::string(e.what()));
        }
    }
}
