#include"config/conf/conf.hpp"
#include <fstream>
#include <sstream>
// 确保使用 ::std::filesystem
using namespace plib::core::config;

std::string& ConfParser::rtrim(std::string& s) const
{
	s.erase(s.find_last_not_of(options.whitespace_characters) + 1);
	return s;
}

std::string& ConfParser::ltrim(std::string& s) const
{
	s.erase(0, s.find_first_not_of(options.whitespace_characters));
	return s;
}

void ConfParser::throw_error(const std::string& message)
{
	std::ostringstream oss;
	oss << "Syntax error: " << message << "\n";
	oss << "Line " << line_no << " | " << current_line << "\n";
	throw parse_conf_error(oss.str());
}

std::string ConfParser::preprocess_line(std::string line)
{
	if (options.should_lines_be_left_trimmed)
		ltrim(line);
	if (options.should_lines_be_right_trimmed)
		rtrim(line);

	if (line.empty())
	{
		if (!options.should_allow_empty_lines)
			throw_error("Empty line found");
		return line;
	}

	if (!options.should_allow_comments)
	{
		return line;
	}

	// If comments are allowed, this code should remove comment escape sequence
	// i.e. if the comment characters are ; and #
	// \# and \; should become just # and ;
	// It ignores all other escape sequences

	char previous_character = line[0];
	auto is_first_char_comment = options.single_line_comments.find(line[0]) != std::string::npos;
	if (is_first_char_comment)
	{
		// If the first character is itself a comment character, skip the entire process
		return "";
	}

	std::string line_cpy;
	line_cpy.reserve(line.size());

	line_cpy.push_back(previous_character);

	for (size_t i = 1; i < line.size(); ++i)
	{
		auto is_comment_char = options.single_line_comments.find(line[i]) != std::string::npos;
		if (is_comment_char)
		{
			if (previous_character == options.escape_character)
				line_cpy.pop_back();
			else
				break;
		}

		line_cpy.push_back(line[i]);
		previous_character = line[i];
	}
	return line_cpy;
}

// Returns the index of the delimiter in the line, if the delimiter is not found, or is at an
// invalid position, raises an error
size_t ConfParser::find_delimiter(const std::string& line)
{
	size_t index = line.find_first_of(options.delimiters);
	if (index == std::string::npos)
		throw_error("No delimiter found");

	// Check if the delimiter is at the beginning of the line or at the end
	if (index == 0)
		throw_error("Empty key, delimiter at beginning of line");
	if ((index + 1) == line.size() && !options.should_allow_empty_values)
		throw_error("Empty value, delimiter at end of line");
	return index;
}

std::pair<std::string, ValueType>ConfParser::parse_line(const std::string& line, size_t delimiter_index)
{
	// Split the line into key and value part
	auto key = line.substr(0, delimiter_index);
	auto value = line.substr(delimiter_index + 1);

	if (options.should_keys_be_trimmed)
	{
		rtrim(key);
		ltrim(key);
	}

	if (options.should_values_be_trimmed)
	{
		ltrim(value);
		rtrim(value);
	}

	return std::make_pair(key, value);
}

Conf ConfParser::from_stream(std::istream& is)
{
	Conf cfg;
	std::string line;
	line_no = 0;
	while (std::getline(is, line))
	{
		++line_no;
		current_line = line;
		line = preprocess_line(line);
		if (line.empty())
			continue;
		auto delimiter_index = find_delimiter(line);
		auto kv = parse_line(line, delimiter_index);
		cfg[kv.first] = kv.second;
	}
	if (is.bad())
	{
		std::ostringstream oss;
#if defined(_WIN32)
		char err_buf[256] = { 0 };
		strerror_s(err_buf, sizeof(err_buf), errno);
		oss << "Error while reading Conf file: " << err_buf;
#else
		oss << "Error while reading Conf file: " << strerror(errno);
#endif
		throw parse_conf_error(oss.str());
	}
	return cfg;
}

Conf ConfParser::from_str(const std::string& s)
{
	std::istringstream iss(s);
	return from_stream(iss);
}

Conf ConfParser::from_file(const ::std::filesystem::path& filepath)
{
	std::ifstream ifs(filepath);
	if (!ifs)
	{
		std::ostringstream oss;
#if defined(_WIN32)
		char err_buf[256] = { 0 };
		strerror_s(err_buf, sizeof(err_buf), errno);
		oss << "File error: Error while reading Conf file: " << err_buf;
#else
		oss << "File error: Error while reading Conf file: " << strerror(errno);
#endif
		throw parse_conf_error(oss.str());
	}
	return from_stream(ifs);
}

namespace plib::core::config {
	ValueType::ValueType() : _is_empty(true) {}
	ValueType::ValueType(const ValueType&) = default;
	ValueType::ValueType(ValueType&&) noexcept = default;
	ValueType& ValueType::operator=(const ValueType&) = default;
	ValueType& ValueType::operator=(ValueType&&) noexcept = default;
	ValueType::~ValueType() = default;

	Conf::Conf() = default;
	Conf::Conf(const Conf&) = default;
	Conf::Conf(Conf&&) noexcept = default;
	Conf& Conf::operator=(const Conf&) = default;
	Conf& Conf::operator=(Conf&&) noexcept = default;
	Conf::~Conf() = default;

	ValueType& Conf::operator[](const std::string& key) {
		return cfg_map[key];
	}
	void Conf::erase(const std::string& key)
	{
		auto iter = cfg_map.find(key);
		if (iter != cfg_map.end())
		{
			cfg_map.erase(iter);
		}
	}
}