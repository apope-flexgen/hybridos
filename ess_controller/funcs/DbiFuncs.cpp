#include "asset.h"
#include "dbi_funcs.hpp" // so we can run the check_dbi_list_loop

extern "C++" {
    int UpdateDbiVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int GetDbiVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int InitDbiList(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
}

// call this function inside of onSet for configs if you want the dbi var to be updated constantly
// regular function conditions apply in the configs
// does NOT check if config time etc. are there, this is purely reactive like any other func
// to just put the variable and params into the dbi like normal upon set, nothing special
int UpdateDbiVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    // msg bufs should be ok, if they overflow we can check stuff to be sure.
    // if that happens then just switch to fmt::format (with std::string)
    fmt::basic_memory_buffer<char, 75> uri_buf;
    fmt::basic_memory_buffer<char, 300> msg_buf;

    fmt::format_to(std::back_inserter(uri_buf), "/dbi/ess_controller{}", Av->comp);
    fmt::format_to(std::back_inserter(msg_buf), "{{{:c}}}", *Av);
    msg_buf.push_back('\0'); 

    if(!p_fims->Send("set", uri_buf.data(), nullptr, msg_buf.data()))
    {
        return -1; // couldn't send fims_msg
    }
    return 0; // no error
}

// cheating for now - remove this later (just for testing purposes)
// int GetDbiVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
// {
//     VarMapUtils* vm = nullptr;
//     if (Av && Av->am)
//     {
//         vm = Av->am->vm;
//     }
//     if (vm)
//     {
//         assetVar* av = vm->getVar(vmap, "/status/ess:active_power");
//         if(av)
//         {
//             if(get_dbi_var(*av, nullptr), p_fims))
//             {
//                 return 0;
//             }
//         }
//     }

//     return -1;
// }

// todo: use this to get a sample config back to load back into the ess controller
int GetDbiConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    if(!get_config_from_dbi("temp", p_fims))
    {
        return -1;
    }
    return 0;
}

int InitDbiList(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    if (Av->am->vm->dbi_update_list.size() > 0) // after list is non-empty will refuse to run again
    {
        return -1;
    }

    // get enough space
    // Av->am->vm->dbi_update_list.reserve(Av->extras->baseDict->featMap.size());

    // todo: check this for correctness
    cJSON* dbi_vars;
    cJSON_ArrayForEach(dbi_vars, Av->extras->optVec->cjopts)
    {
        auto dbi_uris = cJSON_GetObjectItem(dbi_vars, "dbi_uris");
        cJSON* uri;
        cJSON_ArrayForEach(uri, dbi_uris)
        {
            auto some_uri = uri->valuestring;
            if (!some_uri) 
            {

                FPS_PRINT_CRIT("{}","no uri in dbi list, weird");
                continue;
            }
            FPS_PRINT_CRIT("some_uri = {}, type = {}", some_uri, uri->type);
            // auto var = Av->am->vm->getVar(vmap, some_uri, nullptr);
            // if (!var) { continue; }
            // add_dbi_var(Av->am->vm->dbi_update_list, *var);
        }
    }

    // shrink list down to minimum size
    // Av->am->vm->dbi_update_list.shrink_to_fit();

    // init list timeouts for synchronization
    // init_dbi_var_list_timeouts(Av->am->vm->dbi_update_list);   
    return 0;
}
