#include "doctest/doctest.h"
#include "add_points.h"

#include "../../include/gcom_dnp3_fims.h"
#include "../../include/gcom_dnp3_system_structs.h"
#include "../../include/gcom_dnp3_utils.h"
#include "../../include/gcom_dnp3_flags.h"
#include "../../include/gcom_dnp3_tmw_utils.h"
#include "../../include/gcom_dnp3_stats.h"

void parseHeader_condition_check(GcomSystem& sys, FimsMethod method, std::string uri, std::string reply_to,
                                 std::string process_name, std::string username)
{
    CHECK(sys.fims_dependencies->method == method);
    CHECK(sys.fims_dependencies->uri_view.compare(uri) == 0);
    CHECK(sys.fims_dependencies->replyto_view.compare(reply_to) == 0);
    CHECK(sys.fims_dependencies->process_name_view.compare(process_name) == 0);
    CHECK(sys.fims_dependencies->username_view.compare(username) == 0);
}

TEST_SUITE("dnp3_fims")
{
    TEST_CASE("init_fims")
    {
        printf("Testing dnp3_fims.cpp...\n");
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->who = DNP3_MASTER;
        sys.fims_dependencies->data_buf_len = 12345;
        sys.config_file_name = "test";
        sys.id = strdup("test_id");
        init_fims(sys);
        CHECK(sys.fims_dependencies->name.compare("dnp3_client@test") == 0);
        CHECK(sys.fims_dependencies->receiver_bufs.data_buf_len == 12345);
        REQUIRE(sys.fims_dependencies->subs.size() > 0);
        CHECK(sys.fims_dependencies->subs.at(0).compare("/test_id") == 0);
        free(sys.fims_dependencies->receiver_bufs.data_buf);
        free(sys.id);

        sys.protocol_dependencies->who = DNP3_OUTSTATION;
        sys.fims_dependencies->data_buf_len = 8675309;
        sys.config_file_name = "test2";
        sys.id = strdup("test_id2");
        init_fims(sys);
        CHECK(sys.fims_dependencies->name.compare("dnp3_server@test2") == 0);
        CHECK(sys.fims_dependencies->receiver_bufs.data_buf_len == 8675309);
        REQUIRE(sys.fims_dependencies->subs.size() > 0);
        CHECK(sys.fims_dependencies->subs.at(0).compare("/test_id") == 0);
        CHECK(sys.fims_dependencies->subs.at(1).compare("/test_id2") == 0);
    }

    TEST_CASE("add_fims_sub")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        std::string sub1 = "/sub1";
        add_fims_sub(sys, sub1);
        REQUIRE(sys.fims_dependencies->subs.size() == 1);
        CHECK(sys.fims_dependencies->subs.at(0).compare(sub1) == 0);
        std::string sub2 = "/sub2";
        add_fims_sub(sys, sub2);
        REQUIRE(sys.fims_dependencies->subs.size() == 2);
        CHECK(sys.fims_dependencies->subs.at(0).compare(sub1) == 0);
        CHECK(sys.fims_dependencies->subs.at(1).compare(sub2) == 0);
    }

    TEST_CASE("extractValueMulti - Valid JSON value extraction")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        simdjson::ondemand::parser parser;
        simdjson::ondemand::document doc;
        simdjson::ondemand::object json_obj;
        simdjson::ondemand::value json_val;
        Jval_buif to_set;

        char* json_str = strdup(R"({"some_key": 42})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_uint() == 42);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": -42})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_int() == -42);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": true})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_uint() == 1);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": 33.3})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_float() == 33.3);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": {"value":42}})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_uint() == 42);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": {"value":-42}})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_int() == -42);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": {"value":true}})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_uint() == 1);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": {"value":33.3}})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == true);
            CHECK(to_set.get_float() == 33.3);
        }
        free(json_str);
    }

    TEST_CASE("extractValueMulti - Invalid JSON value extraction")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        simdjson::ondemand::parser parser;
        simdjson::ondemand::document doc;
        simdjson::ondemand::object json_obj;
        simdjson::ondemand::value json_val;
        Jval_buif to_set;

        char* json_str = strdup(R"({"some_key": {"not_value":42}})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == false);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": [33,22,11]})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == false);
        }
        free(json_str);

        json_str = strdup(R"({"some_key": "string"})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        doc.get(json_obj);
        for (auto pair : json_obj)
        {
            auto val = pair.value();
            if (const auto err = val.error(); err)
            {
                REQUIRE(!err);
            }

            auto json_val = val.value_unsafe();
            auto val_clothed = json_val.get_object();
            bool result = extractValueMulti(sys, val_clothed, json_val, to_set);
            CHECK(result == false);
        }
        free(json_str);
    }

    TEST_CASE("extractValueSingle - valid json extraction")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        simdjson::ondemand::parser parser;
        simdjson::ondemand::document& doc = sys.fims_dependencies->doc;
        simdjson::ondemand::value json_val;
        Jval_buif to_set;

        char* json_str = strdup(R"(42)");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        auto val_clothed = doc.get_object();
        bool result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_uint() == 42);
        free(json_str);

        json_str = strdup(R"(-42)");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_int() == -42);
        free(json_str);

        json_str = strdup(R"(true)");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_uint() == 1);
        free(json_str);

        json_str = strdup(R"(33.3)");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_float() == 33.3);
        free(json_str);

        json_str = strdup(R"({"value":42})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_uint() == 42);
        free(json_str);

        json_str = strdup(R"({"value":-42})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_int() == -42);
        free(json_str);

        json_str = strdup(R"({"value":true})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_uint() == 1);
        free(json_str);

        json_str = strdup(R"({"value":33.3})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == true);
        CHECK(to_set.get_float() == 33.3);
        free(json_str);
    }
    TEST_CASE("extractValueSingle - Invalid JSON value extraction")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        simdjson::ondemand::parser parser;
        simdjson::ondemand::document& doc = sys.fims_dependencies->doc;
        simdjson::ondemand::value json_val;
        Jval_buif to_set;

        char* json_str = strdup(R"({"not_value":42})");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        auto val_clothed = doc.get_object();
        bool result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == false);
        free(json_str);

        json_str = strdup(R"([1,2,3,4])");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == false);
        free(json_str);

        json_str = strdup(R"("string")");
        parser.iterate(json_str, strlen(json_str), strlen(json_str) + simdjson::SIMDJSON_PADDING).get(doc);
        val_clothed = doc.get_object();
        result = extractValueSingle(sys, val_clothed, json_val, to_set);
        CHECK(result == false);
        free(json_str);
    }

    TEST_CASE("jval_to_double")
    {
        Jval_buif jval_bool = Jval_buif(true);
        Jval_buif jval_uint = Jval_buif(2UL);
        Jval_buif jval_int = Jval_buif(-5L);
        Jval_buif jval_float = Jval_buif(5.3f);

        double value = jval_to_double(jval_bool);
        CHECK(value == 1.0);

        value = jval_to_double(jval_uint);
        CHECK(value == 2.0);

        value = jval_to_double(jval_int);
        CHECK(value == -5.0);

        value = jval_to_double(jval_float);
        CHECK(value == 5.3f);
    }

    TEST_CASE("parseHeader - check requests")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->who = DNP3_MASTER;
        sys.base_uri = strdup("/components/dnp3_client");
        Meta_Data_Info meta_data;
        std::string method = "get";
        std::string uri = "/components/dnp3_client/_timings";
        std::string reply_to = "/me";
        std::string process_name = "dnp3_client@test";
        std::string username = "username";
        std::string message = "{}";
        meta_data.method_len = method.length();
        meta_data.uri_len = uri.length();
        meta_data.replyto_len = reply_to.length();
        meta_data.process_name_len = process_name.length();
        meta_data.username_len = username.length();
        std::string message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(sys.fims_dependencies->uri_requests.is_timings_request);
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        method = "set";
        uri = "/components/dnp3_client/_reset_timings";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_timings_request);
        CHECK(sys.fims_dependencies->uri_requests.is_reset_timings_request);
        parseHeader_condition_check(sys, FimsMethod::Set, uri, reply_to, process_name, username);

        uri = "/components/dnp3_client/_reload";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_reset_timings_request);
        CHECK(sys.fims_dependencies->uri_requests.is_reload_request);
        parseHeader_condition_check(sys, FimsMethod::Set, uri, reply_to, process_name, username);

        uri = "/components/dnp3_client/_config";
        method = "get";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_reload_request);
        CHECK(sys.fims_dependencies->uri_requests.is_config_request);
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        uri = "/components/dnp3_client/_raw";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_config_request);
        CHECK(sys.fims_dependencies->uri_requests.is_raw_request);
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        method = "set";
        uri = "/components/dnp3_client/_debug";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_raw_request);
        CHECK(sys.fims_dependencies->uri_requests.is_debug_request);
        parseHeader_condition_check(sys, FimsMethod::Set, uri, reply_to, process_name, username);

        uri = "/components/dnp3_client/_stats";
        method = "get";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_debug_request);
        CHECK(sys.fims_dependencies->uri_requests.is_stats_request);
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        uri = "/components/dnp3_client/_points";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_stats_request);
        CHECK(sys.fims_dependencies->uri_requests.is_points_request);
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        uri = "/components/dnp3_client/_stats/more";
        meta_data.uri_len = uri.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(!sys.fims_dependencies->uri_requests.is_points_request);
        CHECK(sys.fims_dependencies->uri_requests.is_stats_request);
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);
    }
    TEST_CASE("parseHeader - check methods")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->who = DNP3_OUTSTATION;
        sys.base_uri = strdup("/components/dnp3_server");
        sys.local_uri = strdup("/some_local_uri");
        Meta_Data_Info meta_data;
        std::string method = "set";
        std::string uri = "/components/dnp3_server";
        std::string reply_to = "/me";
        std::string process_name = "dnp3_server@test";
        std::string username = "username";
        std::string message = "{}";
        meta_data.method_len = method.length();
        meta_data.uri_len = uri.length();
        meta_data.replyto_len = reply_to.length();
        meta_data.process_name_len = process_name.length();
        meta_data.username_len = username.length();
        std::string message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Set, uri, reply_to, process_name, username);

        uri = "/components/dnp3_server/_reset_timings";
        meta_data.uri_len = uri.length();
        method = "set";
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        CHECK(sys.fims_dependencies->uri_requests.is_reset_timings_request);
        parseHeader_condition_check(sys, FimsMethod::Set, uri, reply_to, process_name, username);

        uri = "/components/dnp3_server/_config";
        meta_data.uri_len = uri.length();
        method = "set";
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Set, uri, reply_to, process_name, username);

        uri = "/some_local_uri";
        meta_data.uri_len = uri.length();
        method = "set";
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Set, "", reply_to, process_name, username);

        uri = "/components/dnp3_server";
        meta_data.uri_len = uri.length();
        method = "get";
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        method = "get";
        reply_to = "";
        meta_data.replyto_len = reply_to.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Get, uri, reply_to, process_name, username);

        sys.protocol_dependencies->who = DNP3_MASTER;
        uri = "/components/dnp3_client";
        meta_data.uri_len = uri.length();
        method = "pub";
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Pub, uri, reply_to, process_name, username);

        sys.protocol_dependencies->who = DNP3_OUTSTATION;
        uri = "/components/dnp3_server";
        meta_data.uri_len = uri.length();
        method = "pub";
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Pub, uri, reply_to, process_name, username);

        method = "aaa";
        reply_to = "";
        meta_data.replyto_len = reply_to.length();
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Unknown, uri, reply_to, process_name, username);

        method = "aaa";
        reply_to = "/me";
        meta_data.replyto_len = reply_to.length();
        sys.fims_dependencies->fims_gateway.Connect("dnp3_test");
        message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
        parseHeader_condition_check(sys, FimsMethod::Unknown, uri, reply_to, process_name, username);
    }
    TEST_CASE("parseHeader - too much data (but not really)")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        sys.protocol_dependencies->who = DNP3_OUTSTATION;
        sys.base_uri = strdup("/components/dnp3_server");
        sys.local_uri = strdup("/some_local_uri");
        Meta_Data_Info meta_data;
        std::string method = "set";
        std::string uri = "/components/dnp3_server";
        std::string reply_to = "/me";
        std::string process_name = "dnp3_server@test";
        std::string username = "username";
        std::string message = "{}";
        meta_data.method_len = method.length();
        meta_data.uri_len = uri.length();
        meta_data.replyto_len = reply_to.length();
        meta_data.process_name_len = process_name.length();
        meta_data.username_len = username.length();
        meta_data.data_len = 100000;
        sys.fims_dependencies->data_buf_len = 5;
        std::string message_str = method + uri + reply_to + process_name + username + message;
        CHECK(!parseHeader(sys, meta_data, message_str.data(), message_str.length()));
    }
    TEST_CASE("processCmds - forced request client")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        Meta_Data_Info meta_data;

        // Set up dependencies
        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/_force" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_force_request = true;
        sys.protocol_dependencies->who = DNP3_MASTER;

        // create a few mock points
        TMWSIM_POINT dbPoint_analog_out;
        FlexPoint fp_an_out(&sys, "analog_out_0", "/components/test");
        dbPoint_analog_out.flexPointHandle = &fp_an_out;
        dbPoint_analog_out.type = TMWSIM_TYPE_ANALOG;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_output_point = true;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value = 5;
        addDbUri(sys, "/components/test", &dbPoint_analog_out);

        TMWSIM_POINT dbPoint_analog_in;
        FlexPoint fp_an_in(&sys, "analog_in_0", "/components/test");
        dbPoint_analog_in.flexPointHandle = &fp_an_in;
        dbPoint_analog_in.type = TMWSIM_TYPE_ANALOG;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_output_point = false;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->operate_value = 5;
        addDbUri(sys, "/components/test", &dbPoint_analog_in);

        TMWSIM_POINT dbPoint_binary_out;
        FlexPoint fp_bin_out(&sys, "binary_out_0", "/components/test");
        dbPoint_binary_out.flexPointHandle = &fp_bin_out;
        dbPoint_binary_out.type = TMWSIM_TYPE_BINARY;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_output_point = true;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->operate_value = 1;
        addDbUri(sys, "/components/test", &dbPoint_binary_out);

        TMWSIM_POINT dbPoint_binary_in;
        FlexPoint fp_bin_in(&sys, "binary_in_0", "/components/test");
        dbPoint_binary_in.flexPointHandle = &fp_bin_in;
        dbPoint_binary_in.type = TMWSIM_TYPE_BINARY;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_output_point = false;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->operate_value = 1;
        addDbUri(sys, "/components/test", &dbPoint_binary_in);

        TMWSIM_POINT dbPoint_counter;
        FlexPoint fp_counter(&sys, "counter_0", "/components/test");
        dbPoint_counter.flexPointHandle = &fp_counter;
        dbPoint_counter.type = TMWSIM_TYPE_COUNTER;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_output_point = false;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->operate_value = 5;
        addDbUri(sys, "/components/test", &dbPoint_counter);

        // call function
        bool result = processCmds(sys, meta_data);

        // Assert
        CHECK(result);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value == 5);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->standby_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->standby_value == 1);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->standby_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->standby_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate_before_or_during_force);

        // make sure the result is the same when everything is already forced
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value = 8;

        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->operate_value = 8;

        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->operate_value = 0;

        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->operate_value = 1;

        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate = true;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->operate_value = 8;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value == 5);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->standby_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->standby_value == 1);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->standby_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate_before_or_during_force);

        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->standby_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate_before_or_during_force);

        // now let's unforce
        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/_unforce" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_unforce_request = true;

        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value = 7;

        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->operate_value = 7;

        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->operate_value = 0;

        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->operate_value = 0;

        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->operate_value = 7;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(!((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value == 5);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->operate_value == 7);
        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->operate_value == 1);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->operate_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->operate_value == 7);
        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate);

        // make sure running it again doesn't break everything
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate_before_or_during_force = true;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value = 3;

        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate_before_or_during_force = true;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->standby_value = 3;

        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate_before_or_during_force = true;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->standby_value = 0;

        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate_before_or_during_force = true;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->standby_value = 1;

        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate_before_or_during_force = true;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->standby_value = 3;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(!((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value == 5);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->operate_value == 7);
        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->operate_value == 1);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->operate_value == 0);
        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->sent_operate);

        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->operate_value == 7);
        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->sent_operate);

        // now check individual points work
        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/analog_out_0/_force" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_force_request = true;

        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value = 20;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate_before_or_during_force = false;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate = true;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value == 20);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate_before_or_during_force);

        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/analog_out_0/_unforce" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_unforce_request = true;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(!((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->operate_value == 20);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->sent_operate);
    }
    TEST_CASE("processCmds - forced request server")
    {
        GcomSystem sys = GcomSystem(Protocol::DNP3);
        Meta_Data_Info meta_data;

        // Set up dependencies
        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/_force" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_force_request = true;
        sys.protocol_dependencies->who = DNP3_OUTSTATION;

        // create a few mock points
        TMWSIM_POINT dbPoint_analog_out;
        FlexPoint fp_an_out(&sys, "analog_out_0", "/components/test");
        dbPoint_analog_out.flexPointHandle = &fp_an_out;
        dbPoint_analog_out.type = TMWSIM_TYPE_ANALOG;
        dbPoint_analog_out.flags = DNPDEFS_DBAS_FLAG_ON_LINE;
        dbPoint_analog_out.data.analog.value = 5;
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_output_point = true;
        addDbUri(sys, "/components/test", &dbPoint_analog_out);

        TMWSIM_POINT dbPoint_analog_in;
        FlexPoint fp_an_in(&sys, "analog_in_0", "/components/test");
        dbPoint_analog_in.flexPointHandle = &fp_an_in;
        dbPoint_analog_in.type = TMWSIM_TYPE_ANALOG;
        dbPoint_analog_in.flags = DNPDEFS_DBAS_FLAG_ON_LINE;
        dbPoint_analog_in.data.analog.value = 5;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_output_point = false;
        addDbUri(sys, "/components/test", &dbPoint_analog_in);

        TMWSIM_POINT dbPoint_binary_out;
        FlexPoint fp_bin_out(&sys, "binary_out_0", "/components/test");
        dbPoint_binary_out.flexPointHandle = &fp_bin_out;
        dbPoint_binary_out.type = TMWSIM_TYPE_BINARY;
        dbPoint_binary_out.flags = DNPDEFS_DBAS_FLAG_ON_LINE;
        dbPoint_binary_out.data.binary.value = TMWDEFS_TRUE;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_output_point = true;
        addDbUri(sys, "/components/test", &dbPoint_binary_out);

        TMWSIM_POINT dbPoint_binary_in;
        FlexPoint fp_bin_in(&sys, "binary_in_0", "/components/test");
        dbPoint_binary_in.flexPointHandle = &fp_bin_in;
        dbPoint_binary_in.type = TMWSIM_TYPE_BINARY;
        dbPoint_binary_in.flags = DNPDEFS_DBAS_FLAG_ON_LINE;
        dbPoint_binary_in.data.binary.value = TMWDEFS_TRUE;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_output_point = false;
        addDbUri(sys, "/components/test", &dbPoint_binary_in);

        TMWSIM_POINT dbPoint_counter;
        FlexPoint fp_counter(&sys, "counter_0", "/components/test");
        dbPoint_counter.flexPointHandle = &fp_counter;
        dbPoint_counter.type = TMWSIM_TYPE_COUNTER;
        dbPoint_counter.flags = DNPDEFS_DBAS_FLAG_ON_LINE;
        dbPoint_counter.data.counter.value = 5;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_output_point = false;
        addDbUri(sys, "/components/test", &dbPoint_counter);

        // call function
        bool result = processCmds(sys, meta_data);

        // Assert
        CHECK(result);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value == 5);

        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->standby_value == 5);

        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->standby_value == 1);

        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->standby_value == 1);

        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK((dbPoint_counter.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->standby_value == 5);

        // make sure the result is the same when everything is already forced
        dbPoint_analog_out.data.analog.value = 8;
        dbPoint_analog_in.data.analog.value = 8;
        dbPoint_binary_out.data.binary.value = TMWDEFS_FALSE;
        dbPoint_binary_in.data.binary.value = TMWDEFS_FALSE;
        dbPoint_counter.data.counter.value = 8;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value == 5);

        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->standby_value == 5);

        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->standby_value == 1);

        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->standby_value == 1);

        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK((dbPoint_counter.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_counter.flexPointHandle))->standby_value == 5);

        // now let's unforce
        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/_unforce" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_unforce_request = true;

        dbPoint_analog_out.data.analog.value = 7;
        dbPoint_analog_in.data.analog.value = 7;
        dbPoint_binary_out.data.binary.value = TMWDEFS_FALSE;
        dbPoint_binary_in.data.binary.value = TMWDEFS_FALSE;
        dbPoint_counter.data.counter.value = 7;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(!((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_analog_out.data.analog.value == 5);

        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_analog_in.data.analog.value == 5);

        CHECK(!((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_binary_out.data.binary.value == 1);

        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_binary_in.data.binary.value == 1);

        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK((dbPoint_counter.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_counter.data.counter.value == 5);

        // make sure running it again doesn't break everything
        ((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value = 7;
        ((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->standby_value = 7;
        ((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->standby_value = TMWDEFS_FALSE;
        ((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->standby_value = TMWDEFS_FALSE;
        ((FlexPoint*)(dbPoint_counter.flexPointHandle))->standby_value = 7;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(!((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_analog_out.data.analog.value == 5);

        CHECK(!((FlexPoint*)(dbPoint_analog_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_analog_in.data.analog.value == 5);

        CHECK(!((FlexPoint*)(dbPoint_binary_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_binary_out.data.binary.value == 1);

        CHECK(!((FlexPoint*)(dbPoint_binary_in.flexPointHandle))->is_forced);
        CHECK((dbPoint_binary_in.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_binary_in.data.binary.value == 1);

        CHECK(!((FlexPoint*)(dbPoint_counter.flexPointHandle))->is_forced);
        CHECK((dbPoint_counter.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_counter.data.binary.value == 5);

        // now check individual points work
        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/analog_out_0/_force" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_force_request = true;

        dbPoint_analog_out.data.analog.value = 20;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) != 0);
        CHECK(((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->standby_value == 20);

        sys.fims_dependencies->uri_view = std::string_view{ "/components/test/analog_out_0/_unforce" };
        sys.fims_dependencies->uri_requests.clear_uri();
        sys.fims_dependencies->uri_requests.is_unforce_request = true;

        dbPoint_analog_out.data.analog.value = 13;

        result = processCmds(sys, meta_data);

        CHECK(result);
        CHECK(!((FlexPoint*)(dbPoint_analog_out.flexPointHandle))->is_forced);
        CHECK((dbPoint_analog_out.flags & DNPDEFS_DBAS_FLAG_LOCAL_FORCED) == 0);
        CHECK(dbPoint_analog_out.data.analog.value == 20);
    }
    TEST_CASE("uriIsMultiOrSingle") {}
    TEST_CASE("replyToFullGet") {}
    TEST_CASE("formatPointValue") {}
    TEST_CASE("replyToGet") {}
}