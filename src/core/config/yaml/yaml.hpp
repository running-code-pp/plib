#ifndef _core_config_yaml_hpp_
#define _core_config_yaml_hpp_

#include <yaml-cpp/yaml.h>

namespace plib::core::config {
	using YamlReader = YAML::Node;
	using YamlWriter = YAML::Emitter;

	// must try-catch
	YamlReader read_yaml_file(const std::string& filename);
	// must try-catch
	YamlReader read_yaml_str(const std::string& content);
	// must try-catch
	void write_yaml(const std::string& filename, const YamlWriter& writer);
}

#endif // _core_config_yaml_hpp_