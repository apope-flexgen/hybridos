// #pragma once
#ifndef GCOM_CONFIG_TMPL_H
#define GCOM_CONFIG_TMPL_H

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <any>
#include <map>
#include <vector>
#include <simdjson.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <string>

#include <fstream>
#include <sstream>
#include <map>
#include <any>
#include <string>
#include <sstream>
#include <optional>
#include <typeinfo>

template <typename T>
std::optional<T> getMapValue(const std::map<std::string, std::any> &map, const std::string &query) {
    std::stringstream ss(query);
    std::string key;
    const std::map<std::string, std::any> *currentMap = &map;

    try {
        while (std::getline(ss, key, '.')) {
            auto it = currentMap->find(key);
            if (it == currentMap->end()) {
                return std::nullopt;
            }

            if (it->second.type() == typeid(std::map<std::string, std::any>)) {
                currentMap = &std::any_cast<const std::map<std::string, std::any> &>(it->second);
            }
            else if (it->second.type() == typeid(T)) {
                if (!std::getline(ss, key, '.')) {
                    return std::any_cast<T>(it->second);
                }
            }
            else if (typeid(T) == typeid(double) && it->second.type() == typeid(int)) {
                if (!std::getline(ss, key, '.')) {
                    return std::any_cast<T>(static_cast<double>((std::any_cast<int>(it->second))));
                    //return static_cast<double>(std::any_cast<int>(it->second));
                }
            }
            else {
                return std::nullopt;
            }
        }
    }
    catch (const std::bad_any_cast &e) {
        std::cout << __func__ << " did not work " << e.what()<< std::endl;
        // Handle or log the exception
        return std::nullopt;
    }

    return std::nullopt;
}


// template <typename T>
// std::optional<T> getMapValue(const std::map<std::string, std::any> &map, const std::string &query)
// {
//     std::stringstream ss(query);
//     std::string key;
//     const std::map<std::string, std::any> *currentMap = &map;

//     try
//     {
//         while (std::getline(ss, key, '.'))
//         {
//             auto it = currentMap->find(key);
//             if (it == currentMap->end())
//             {
//                 return std::nullopt;
//             }

//             if (it->second.type() == typeid(std::map<std::string, std::any>))
//             {
//                 currentMap = &std::any_cast<const std::map<std::string, std::any> &>(it->second);
//             }
//             else if (it->second.type() == typeid(T))
//             {
//                 if (!std::getline(ss, key, '.'))
//                 {
//                     return std::any_cast<T>(it->second);
//                 }
//             }
//             else if constexpr (std::is_same_v<T, double> && it->second.type() == typeid(int))
//             {
//                 if (!std::getline(ss, key, '.'))
//                 {
//                     return static_cast<double>(std::any_cast<int>(it->second));
//                 }
//             }
//             else
//             {
//                 return std::nullopt;
//             }
//         }
//     }
//     catch (const std::bad_any_cast &e)
//     {
//         // Handle or log the exception, for example:
//         // FPS_ERROR_LOG("Failed to cast value in map with query '%s': %s", query, e.what());
//         return std::nullopt;
//     }

//     return std::nullopt;
// }


/**
 * @brief Given a map<string, any>, lookup the query string in the map and convert it to type T
 *
 * @param map const std::map<std::string, std::any> reference to map to search into
 * @param query the key to look for in the map. Uses dot notation to represent subelements of a larger
 * map (e.g. "connection.ip_address" )
 *
 * @return the value (of type T) corresponding to the key found in the map. Returns std::nullopt if not
 * found.
 */
template <typename T>
std::optional<T> xgetMapValue(const std::map<std::string, std::any> &map, const std::string &query)
{
    std::stringstream ss(query);
    std::string key;
    const std::map<std::string, std::any> *currentMap = &map;

    try
    {
        while (std::getline(ss, key, '.'))
        {
            auto it = currentMap->find(key);
            if (it == currentMap->end())
            {
                return std::nullopt;
            }

            if (it->second.type() == typeid(std::map<std::string, std::any>))
            {
                currentMap = &std::any_cast<const std::map<std::string, std::any> &>(it->second);
            }
            else if (it->second.type() == typeid(T) && !std::getline(ss, key, '.'))
            {
                return std::any_cast<T>(it->second);
            }
            else
            {
                return std::nullopt;
            }
        }
    }
    catch (const std::bad_any_cast &e)
    {
        // Handle or log the exception, for example:
        FPS_ERROR_LOG("Failed to cast value in map with query '%s': %s", query, e.what());
        return std::nullopt;
    }

    return std::nullopt;
}

/**
 * @brief Given a map<string, any>, lookup the query string in the map, and convert it to type T (if found)
 * or give it a default value (if not found and a valid default is provided).
 *
 * @param map const std::map<std::string, std::any> reference to map to search into
 * @param query the key to look for in the map. Uses dot notation to represent subelements of a larger
 * map (e.g. "connection.ip_address" )
 * @param target where to store the value
 * @param defaultValue the default value to use if the target isn't found
 * @param useDefault true/false value to determine if you use the default when not found
 * @param required do you need a value?
 * @param debug bool value representing whether or not to print messages to help debug code
 *
 * @return true if found or default value was used; false if required and useDefault is false
 */
template <typename T>
bool getItemFromMap(const std::map<std::string, std::any> &map, const std::string &query, T &target, const T &defaultValue, bool useDefault, bool required, bool debug)
{

    auto result = getMapValue<T>(map, query);
    if (result.has_value())
    {
        if (debug)
            std::cerr << "Found " << query << " " << result.value() << std::endl;
        target = result.value();
        return true;
    }
    else if (useDefault && required)
    {
        if (debug)
            std::cerr << "Used Default " << query << " " << defaultValue << std::endl;
        target = defaultValue;
        return true;
    }
    else if (required)
    {
        return false;
    }
    target = defaultValue;
    return true;
}

#endif
