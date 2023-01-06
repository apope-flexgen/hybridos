#include "dbi_funcs.hpp"

// this function sends the dbi vars out if they have timed out then resets their timers
bool check_dbi_var_timeouts(dbi_var_list& var_list, fims* fims_gateway)
{
    std::vector<const assetVar*> update_list; // list of vars to update
    update_list.reserve(var_list.size()/2); // about half the size of dbi_var_list by default
    const auto now = std::chrono::steady_clock::now(); // base time to check when we need to update variables

    // loop over var_list to see if something needs to be updated in dbi
    for (auto& var : var_list)
    {
        if (!var.dbi_var.getbParam("EnableDbiUpdate"))
        {
            continue; // don't update var if it isn't allowed to be updated
        }

        const auto elapsed = now - var.update_timeout_start;
        const auto update_time = var.dbi_var.getiParam("UpdateTimeCfg");
        if (update_time <= 0) // it is a double
        {
            const auto dbl_update_time = var.dbi_var.getdParam("UpdateTimeCfg");
            if (elapsed >= flex::dbl_sec{dbl_update_time}) // time to update the var in dbi
            {
                update_list.emplace_back(&var.dbi_var); // put it onto the update list to be sent out
                var.update_timeout_start = now; // reset its update start_time for checking the next time
            }
        }
        else if (elapsed >= std::chrono::seconds{update_time}) // it is an int
        {
            update_list.emplace_back(&var.dbi_var);
            var.update_timeout_start = now;
        }
    }

    fmt::basic_memory_buffer<char, 100> uri_buf;
    fmt::basic_memory_buffer<char, 300> msg_buf;

    for (const auto& var : update_list)
    {
        fmt::format_to(std::back_inserter(uri_buf), "/dbi/ess_controller{}/{}", var->comp, var->name);
        fmt::format_to(std::back_inserter(msg_buf), "{:f}", *var);
        *uri_buf.end() = '\0'; // gets rid of final "junk" character at the end due to name for some reason
        *msg_buf.end() = '\0'; // for some reason this needs this - I don't know why we have a 'B' at the end

        if (!fims_gateway->Send("set", uri_buf.data(), nullptr, msg_buf.data()))
        {
            return false;
        }

        uri_buf.clear();
        msg_buf.clear();
    }

    return true;
}

// this adds a var to be monitored on the list for updates
// possible TODO: Make sure that the same assetVar isn't added twice
// probably just needs to check the address
bool add_dbi_var(dbi_var_list& var_list, assetVar& to_add)
{
    if (!to_add.gotParam("EnableDbiUpdate"))
    {
        to_add.setParam("EnableDbiUpdate", true); // set this variable up to update as a dbi_var
    }
    if (!to_add.gotParam("UpdateTimeCfg"))
    {
        to_add.setParam("UpdateTimeCfg", 5); // default to five second update time
    }
    if (!to_add.gotParam("dbiStatus")) // todo: handle init case once we get it from the configs, might be tricky
    {
        to_add.setParam("dbiStatus", (char*)"ok"); // default to saying "ok" to dbi being on update list? - should be correct
    }

    // might need to add update time call again
    var_list.emplace_back(dbi_var_ref{to_add, std::chrono::steady_clock::now()});
    return true;
}

// this inits all of their timeouts for synchronization purposes
bool init_dbi_var_list_timeouts(dbi_var_list& var_list)
{
    const auto now = std::chrono::steady_clock::now();
    for (auto& var : var_list)
    {
        var.update_timeout_start = now;
    }
    return true;
}

// this tells dbi to send the data to /ess/dbi/comp:name for processing so it gets back into our database
// we "check" it there for some reason - I have no idea
bool get_dbi_var(const assetVar& to_get, fims* p_fims)
{
    fmt::basic_memory_buffer<char, 100> uri_buf;
    fmt::basic_memory_buffer<char, 100> reply_buf;

    fmt::format_to(std::back_inserter(uri_buf), "/dbi/ess_controller{}/{}", to_get.comp, to_get.name);
    fmt::format_to(std::back_inserter(reply_buf), "/ess/dbi{}/{}", to_get.comp, to_get.name);
    *uri_buf.end() = '\0';
    *reply_buf.end() = '\0';

    if (!p_fims->Send("get", uri_buf.data(), reply_buf.data(), nullptr))
    {
        return false;
    }
    return true;
}

#if 0
// NOTE: This function is slow and should not be called more than once really
bool send_config_to_dbi(const std::string& file_path_and_name, fims* p_fims)
{
    std::ifstream file_stream{file_path_and_name};

    if (!file_stream.is_open()) {
        FPS_PRINT_ERROR("can't open file: {}, please check filepath and name", file_path_and_name);
        return false;
    }

    auto file_content = std::string{std::istreambuf_iterator<char>(file_stream), std::istreambuf_iterator<char>()};

    // need to put the file_content in quotes here so we store the whole thing to not conflict with dbi resolution
    // etc.
    // auto file_content_in_quotes = fmt::format(R"("{}")", file_content);

    // don't know whether I might need to check for npos or not - shouldn't be a problem
    // will have to put this into the main code later
    auto filename_split_index = file_path_and_name.find_last_of('/');
    auto filename = file_path_and_name.substr(filename_split_index + 1);
    auto split = spdlog::details::file_helper::split_by_extension(filename);

    auto filename_no_ext = std::get<0>(split); // gets filename

    fmt::basic_memory_buffer<char, 100> uri_buf;
    fmt::format_to(std::back_inserter(uri_buf), "/dbi/ess_controller/configs/{}", filename_no_ext);
    *uri_buf.end() = '\0';

    if(!p_fims->Send("set", uri_buf.data(), nullptr, file_content.data()))
    {
        FPS_PRINT_ERROR("could not send fims message to dbi to put config file in there");
        return false;
    }

    return true;
}
#endif
bool get_config_from_dbi(const char* filename_no_ext, fims* p_fims)
{
    fmt::basic_memory_buffer<char, 100> uri_buf;
    fmt::basic_memory_buffer<char, 100> reply_buf;

    fmt::format_to(std::back_inserter(uri_buf), "/dbi/ess_controller/configs/{}", filename_no_ext);
    // where to send it back when getting a config?
    fmt::format_to(std::back_inserter(reply_buf), "/ess/dbi/{}", filename_no_ext);
    *uri_buf.end() = '\0';
    *reply_buf.end() = '\0';

    // tell dbi to send config back to us appropriately
    if(!p_fims->Send("get", uri_buf.data(), reply_buf.data(), nullptr))
    {
        return false;
    }
    return true;
}
