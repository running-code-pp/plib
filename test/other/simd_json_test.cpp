#include <simdjson.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

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

    nlohmann::json complex_json;
    complex_json["name"] = "Example";
    complex_json["values"] = {1, 2, 3, 4};
    complex_json["nested"] = {{"key1", "value1"}, {"key2", "value2"}};
    std::cout << complex_json.dump(4) << std::endl;
    return 0;
}