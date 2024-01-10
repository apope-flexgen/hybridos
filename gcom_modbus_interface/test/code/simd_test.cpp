

#include <iostream>
#include <fstream>
#include <map>
#include <any>

#include <simdjson.h>

//g++ src/simd_test.cpp -o simd_test -lsimdjson
std::map<std::string, std::any> jsonToMap(simdjson::dom::object obj);
void printAny(std::any &value, int indent);

/**
 * @brief Convert a simdjson::dom::element to an std::any
 *
 * @param elem simdjson::dom::element JSON element created after parsing using simdjson::dom::parser
 */
std::any jsonToAny(simdjson::dom::element elem)
{
    switch (elem.type())
    {
    case simdjson::dom::element_type::INT64:
    {
        int64_t val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            if (val <= INT32_MAX && val >= INT32_MIN)
            {
                return static_cast<int32_t>(val);
            }
            else
            {
                return val;
            }
        }
        break;
    }
    case simdjson::dom::element_type::BOOL:
    {
        bool val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            return val;
        }
        break;
    }
    case simdjson::dom::element_type::UINT64:
    {
        uint64_t val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            if (val <= UINT32_MAX)
            {
                return static_cast<uint32_t>(val);
            }
            else
            {
                return val;
            }
        }
        break;
    }
    case simdjson::dom::element_type::DOUBLE:
    {
        double val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            return val;
        }
        break;
    }
    case simdjson::dom::element_type::STRING:
    {
        std::string_view sv;
        if (elem.get(sv) == simdjson::SUCCESS)
        {
            return std::string(sv);
        }
        break;
    }
    case simdjson::dom::element_type::ARRAY:
    {
        simdjson::dom::array arr = elem;
        std::vector<std::any> vec;
        for (simdjson::dom::element child : arr)
        {
            vec.push_back(jsonToAny(child));
        }
        return vec;
    }
    case simdjson::dom::element_type::OBJECT:
        return jsonToMap(elem);
    default:
        return std::any(); // Empty std::any
    }
    // If we reach here, something went wrong.
    //FPS_ERROR_LOG
    printf("Error processing JSON element.");
    return std::any(); // Return empty std::any
}
/**
 * @brief Convert a simdjson::dom::object to a map<string, any>.
 *
 * @param obj simdjson::dom::object JSON object created after parsing using simdjson::dom::parser
 */
std::map<std::string, std::any> jsonToMap(simdjson::dom::object obj)
{
    std::map<std::string, std::any> map;
    for (auto [key, value] : obj)
    {
        map[std::string(key)] = jsonToAny(value);
    }
    return map;
}
void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  "; // Indentation, 2 spaces per level
    }
}

void printArray(const std::vector<std::any> &array, int indent) {
    for (const auto &element : array) {
        printIndent(indent);
        std::cout << "- ";
        printAny(const_cast<std::any&>(element), indent);
    }
}

// Utility function to print nested maps
void printMap(const std::map<std::string, std::any> &baseMap, int indent = 0)
{
    for (const auto &[key, value] : baseMap)
    {
        printIndent(indent);
        // for (int i = 0; i < indent; ++i)
        // {
        //     std::cout << "  ";
        // }
        std::cout << key << ": ";
        if (value.type() == typeid(std::map<std::string, std::any>))
        {
            std::cout << "\n";
            printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
        }
        if (value.type() == typeid(std::vector<std::any>))
        {
            std::cout << "\n";
            printArray(std::any_cast<std::vector<std::any>>(value), indent + 1);
        }
        else if (value.type() == typeid(int))
        {
            std::cout << std::any_cast<int>(value) << "\n";
        }
        else if (value.type() == typeid(int64_t))
        {
            std::cout << std::any_cast<int64_t>(value) << "\n";
        }
        else if (value.type() == typeid(double))
        {
            std::cout << std::any_cast<double>(value) << "\n";
        }
        else if (value.type() == typeid(std::string))
        {
            std::cout << std::any_cast<std::string>(value) << "\n";
        }
        else if (value.type() == typeid(bool))
        {
            auto val = std::any_cast<bool>(value);
            if (val)
                std::cout << "true"
                          << "\n";
            else
                std::cout << "false"
                          << "\n";
        }
    }
}

// Utility function to print nested maps
void printAny(std::any &value, int indent)
{
    if (value.type() == typeid(std::map<std::string, std::any>))
    {
        std::cout << "\n";
        printMap(std::any_cast<std::map<std::string, std::any>>(value), indent + 1);
    }
    else if (value.type() == typeid(std::vector<std::any>))
    {
        std::cout << " array" << "\n";
        printArray(std::any_cast<std::vector<std::any>>(value), indent + 1);
    }
   else if (value.type() == typeid(int))
    {
        std::cout << std::any_cast<int>(value) << "\n";
    }
    else if (value.type() == typeid(int64_t))
    {
        std::cout << std::any_cast<int64_t>(value) << "\n";
    }
    else if (value.type() == typeid(double))
    {
        std::cout << std::any_cast<double>(value) << "\n";
    }
    else if (value.type() == typeid(std::string))
    {
        std::cout << std::any_cast<std::string>(value) << "\n";
    }
    else if (value.type() == typeid(bool))
    {
        auto val = std::any_cast<bool>(value);
        if (val)
            std::cout << "true"
                      << "\n";
        else
            std::cout << "false"
                      << "\n";
    }
    else
    {
        std::cout << "type unknown"
                  << "\n";
    }
}


int xmain() {
    simdjson::dom::parser parser;

    const char* json_data = "{\"key\": \"myvalue\",\"myarray\":[{\"myakey\":234}], \"mymap\":{\"mynapkey\":234}}";
    simdjson::padded_string p_json_data = simdjson::padded_string(std::string_view(json_data));
    auto result = parser.parse(p_json_data);

    if (result.error()) {
        std::cerr << "Parsing failed: " << simdjson::error_message(result.error()) << std::endl;
        return 1;
    }

    std::string value;
    simdjson::simdjson_result<std::string_view> result_string = result["key"].get_string();
    if (result_string.error()) {
        std::cerr << "Extraction failed: " << simdjson::error_message(result_string.error()) << std::endl;
        return 1;
    }
    auto jsonM = jsonToAny(result.value());
    printAny(jsonM, 0);

    value = std::string(result_string.value());
    std::cout << "Extracted value: " << value << std::endl;
    return 0;
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string filename = argv[1];

    // Open the file
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    // Read the contents into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_data = buffer.str();

    simdjson::dom::parser parser;
    simdjson::padded_string p_json_data = simdjson::padded_string(json_data);
    auto result = parser.parse(p_json_data);
    // ... rest of your parsing logic ...
    if (result.error()) {
        std::cerr << "Parsing failed: " << simdjson::error_message(result.error()) << std::endl;
        return 1;
    }

    // std::string value;
    // simdjson::simdjson_result<std::string_view> result_string = result["key"].get_string();
    // if (result_string.error()) {
    //     std::cerr << "Extraction failed: " << simdjson::error_message(result_string.error()) << std::endl;
    //     return 1;
    // }
    auto jsonM = jsonToAny(result.value());
    printAny(jsonM, 0);

    // value = std::string(result_string.value());
    // std::cout << "Extracted value: " << value << std::endl;
    return 0;
}

