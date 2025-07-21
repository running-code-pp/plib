#ifndef _core_Config_Conf_hpp_
#define _core_Config_Conf_hpp_

#include<string>
#include<vector>
#include<filesystem>
#include <map>
#include <stdexcept>
#include <sstream>


namespace plib::core::config {
	class  parse_conf_error : public std::runtime_error
	{
	public:
		parse_conf_error(const std::string& message) : std::runtime_error(message) {}
	};

	template <class...> struct False : std::bool_constant<false>
	{
	};

	class  ValueType
	{
	private:
		std::string _val;
		bool _is_empty;

	public:
		ValueType();
		ValueType(const ValueType&);
		ValueType(ValueType&&) noexcept;
		ValueType& operator=(const ValueType&);
		ValueType& operator=(ValueType&&) noexcept;
		~ValueType();

		template <typename T> ValueType(const T& value) : _is_empty{ false }
		{
			if constexpr (std::is_same<T, bool>::value)
			{
				_val = (value) ? "True" : "False";
			}
			else if constexpr (std::is_same<std::decay_t<T>, std::string>::value ||
				std::is_same<std::decay_t<T>,
				const char*>::value ||
				std::is_same<std::decay_t<std::remove_extent_t<T>>,
				char>::value)
			{
				_val = value;
			}
			else if constexpr (std::is_arithmetic<T>::value)
			{
				_val = std::to_string(value);
			}
			else
			{
				std::ostringstream oss;
				oss << value;
				_val = oss.str();
			}
		}

		bool is_empty() const { return _is_empty; }

		std::string as_string() const { return _val; }

		template <typename T>
		T parse() const
		{
			if constexpr (std::is_same<T, bool>::value)
			{
				if (_val == "True" || _val == "true" || _val == "1")
				{
					return true;
				}
				if (_val == "False" || _val == "false" || _val == "0")
				{
					return false;
				}
				else
				{
					throw parse_conf_error("Parse error: Invalid literal for bool\n    | " + _val);
				}
			}
			std::istringstream iss(_val);
			T parsed_val;
			if ((!(iss >> parsed_val)) || !(iss.eof()))
			{
				throw parse_conf_error("Parse error: Invalid literal during parse()\n    | " + _val);
			}
			return parsed_val;
		}

		template <typename T> T try_parse(T default_value) const
		{
			if constexpr (std::is_same<T, bool>::value)
			{
				if (_val == "True" || _val == "true" || _val == "1")
				{
					return true;
				}
				if (_val == "False" || _val == "false" || _val == "0")
				{
					return false;
				}
				else
				{
					return default_value;
				}
			}
			std::istringstream iss(_val);
			T parsed_val;
			if ((!(iss >> parsed_val)) || !(iss.eof()))
			{
				return default_value;
			}
			return parsed_val;
		}

		template <typename T> bool is() const
		{
			if constexpr (std::is_same<T, bool>::value)
			{
				if (_val == "True" || _val == "true" || _val == "1" || _val == "False" || _val == "false" ||
					_val == "0")
					return true;
				else
					return false;
			}
			std::istringstream iss(_val);
			T parsed_val;
			if ((!(iss >> parsed_val)) || !(iss.eof()))
			{
				return false;
			}
			return true;
		}
	};

	/**
	 * @brief This class represents a Confuration object which is a collection of key-value pairs,
	 * where key is a string and value is an integer, string, or a real number
	 */
	class  Conf
	{
	public:
		Conf();
		Conf(const Conf&);
		Conf(Conf&&) noexcept;
		Conf& operator=(const Conf&);
		Conf& operator=(Conf&&) noexcept;
		~Conf();
		const ValueType& get(const std::string& key) const
		{
			auto iter = cfg_map.find(key);
			if (iter == cfg_map.end())
			{
				return empty_value;
			}
			return iter->second;
		}

		template <typename T> void set(const std::string& key, const T& value)
		{
			cfg_map[key] = ValueType{ value };
		}

		size_t size() const { return cfg_map.size(); }

		ValueType& operator[](const std::string& key);

		void erase(const std::string& key);

		auto begin() { return cfg_map.begin(); }

		auto end() { return cfg_map.end(); }

		auto cbegin() { return cfg_map.cbegin(); }

		auto cend() { return cfg_map.cend(); }

	private:
		ValueType empty_value;
		std::map<std::string, ValueType> cfg_map;
	};

	struct ConfParserOptions
	{
		std::string whitespace_characters = " \t\r";
		std::string delimiters = "=";
		std::string single_line_comments = "#;";
		char escape_character = '\\';
		bool should_allow_empty_lines = true;
		bool should_allow_empty_values = true;
		bool should_empty_lines_be_skipped = true;
		bool should_lines_be_left_trimmed = true;
		bool should_lines_be_right_trimmed = true;
		bool should_keys_be_trimmed = true;
		bool should_values_be_trimmed = true;
		bool should_allow_comments = true;
	};

	class  ConfParser
	{
	private:
		// Line number of current line being processed, useful in error messages
		size_t line_no;
		// Also store current line, useful for error reporting
		std::string current_line;

		// Trim the string by removing spaces at both ends, inplace
		std::string& ltrim(std::string& s) const;
		std::string& rtrim(std::string& s) const;
		void throw_error(const std::string& message);
		std::string preprocess_line(std::string line);

		// Returns the index of the delimiter in the line, if the delimiter is not found, or is at an
		// invalid position, raises an error
		size_t find_delimiter(const std::string& line);
		std::pair<std::string, ValueType> parse_line(const std::string& line, size_t delimiter_index);

	public:
		ConfParserOptions options;
		ConfParser() = default;
		~ConfParser() = default;
		Conf from_stream(std::istream& is);
		Conf from_str(const std::string& s);
		Conf from_file(const std::filesystem::path& filepath);
	};
} // namespace plib::core::config

#endif  // _core_Config_Conf_hpp_