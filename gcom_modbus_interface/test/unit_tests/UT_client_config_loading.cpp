#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <filesystem>
#include <fstream>

#include "config_loaders/client_config_loader.hpp"
#include "fims/defer.hpp"

static const std::string pass_configs_path = "configs/test/client/should_pass/";
static const std::string fail_configs_path = "configs/test/client/should_fail/";
static const std::string extension = ".json";
static const std::string error_output_file = "UT_client_config_loading_errors.txt";

TEST_SUITE("config loading tests")
{
    config_loader::Error_Location err_loc;

    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;

    std::ofstream error_file(error_output_file);

    TEST_CASE("client configs")
    {
        config_loader::Client_Configs_Array client_configs;

        SUBCASE("should pass")
        {
            std::vector<std::string> pass_configs;

            REQUIRE(std::filesystem::exists(pass_configs_path));

            for (const auto& entry : std::filesystem::directory_iterator(pass_configs_path))
            {
                // Go into at least one directory (not recursive -> only one level deep):
                if (entry.is_directory())
                {
                    for (const auto& new_entry : std::filesystem::directory_iterator(entry))
                    {
                        if (new_entry.is_regular_file() && new_entry.path().extension() == extension)
                        {
                            pass_configs.emplace_back(entry.path().filename().string() +  "/" +  new_entry.path().filename().string());
                        }
                    }
                }
                else if (entry.is_regular_file() && entry.path().extension() == extension)
                {
                    pass_configs.emplace_back(entry.path().filename().string());
                }
            }

            for (const auto& config_file : pass_configs)
            {
                DOCTEST_SUBCASE(config_file.c_str())
                {
                    const auto json = simdjson::padded_string::load(pass_configs_path + config_file);
                    simdjson::ondemand::object config_obj;

                    REQUIRE(parser.iterate(json).get(doc) == simdjson::error_code::SUCCESS);
                    REQUIRE(doc.get(config_obj) == simdjson::error_code::SUCCESS);
                    err_loc.reset();
                    const auto res = client_configs.load(config_obj, err_loc);
                    if (res != true)
                    {
                        const auto err_str = fmt::format(R"(File that did not pass:      "{}"{})", config_file, err_loc);
                        error_file << err_str;
                    }
                    CHECK(res == true);
                }
            }
        }

        SUBCASE("should fail")
        {
            std::vector<std::string> fail_configs;

            REQUIRE(std::filesystem::exists(fail_configs_path));

            for (const auto& entry : std::filesystem::directory_iterator(fail_configs_path))
            {
                // Go into at least one directory (not recursive -> only one level deep):
                if (entry.is_directory())
                {
                    for (const auto& new_entry : std::filesystem::directory_iterator(entry))
                    {
                        if (new_entry.is_regular_file() && new_entry.path().extension() == extension)
                        {
                            fail_configs.emplace_back(entry.path().filename().string() +  "/" +  new_entry.path().filename().string());
                        }
                    }
                }
                else if (entry.is_regular_file() && entry.path().extension() == extension)
                {
                    fail_configs.emplace_back(entry.path().filename().string());
                }
            }

            for (const auto& config_file : fail_configs)
            {
                DOCTEST_SUBCASE(config_file.c_str())
                {
                    const auto json = simdjson::padded_string::load(fail_configs_path + config_file);
                    simdjson::ondemand::object config_obj;

                    REQUIRE(parser.iterate(json).get(doc) == simdjson::error_code::SUCCESS);
                    REQUIRE(doc.get(config_obj) == simdjson::error_code::SUCCESS);
                    err_loc.reset();
                    const auto res = client_configs.load(config_obj, err_loc);
                    if (res != false)
                    {
                        const auto err_str = fmt::format(R"(
File that did not fail:      "{}"
)", config_file);
                        error_file << err_str;
                    }
                    CHECK(res == false);
                }
            }
        }
    }
}
