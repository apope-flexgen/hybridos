#include "gcom_config.h"
#include "gcom_timer.h"
#include "logger/logger.h"
#include "gcom_fims.h"
#include "load_to_dbi_client.h"

void load_points_from_dbi_client(cfg& myCfg)
{
    fims& fims_gateway = myCfg.fims_gateway;

    std::string dbi_update_prefix = "/dbi/" + myCfg.client_name + "/saved_registers";

    for (auto component_shared_ptr : myCfg.components)
    {
        cfg::component_struct* component = component_shared_ptr.get();
        if (component && component->dbi_update_frequency > 0)
        {
            std::string uri = "/" + component->component_id + "/" + component->id;
            std::string dbi_uri = dbi_update_prefix + uri;

            if (!fims_gateway.Send("get", dbi_uri.c_str(), uri.c_str(), nullptr, nullptr))
            {
                FPS_ERROR_LOG("[%s] failed to send a fims get message", myCfg.client_name);
                fims_gateway.Close();
                return;
            }
            // standard fims listen thread should take care of the rest
        }
    }
}

void send_to_dbi_client(cfg& myCfg, std::string& uri, std::any value)
{
    std::shared_ptr<cfg::dbi_update_struct> dbi_struct_shared_ptr = myCfg.dbi_update_map[uri];
    cfg::dbi_update_struct* dbi_struct = dbi_struct_shared_ptr.get();
    if (dbi_struct)
    {
        if (dbi_struct->time_to_update)
        {
            std::stringstream ss;
            any_to_stringstream(ss, value);
            send_set(myCfg.fims_gateway, dbi_struct->dbi_update_uri, ss.str());
            dbi_struct->time_to_update = false;
        }
    }
}

void load_to_dbi_client(cfg& myCfg, std::shared_ptr<cfg::io_point_struct> io_point_shared_ptr, std::any value)
{
    cfg::io_point_struct* io_point = io_point_shared_ptr.get();
    if (io_point)
    {
        std::string uri = "/" + io_point->component->component_id + "/" + io_point->component->id + "/" + io_point->id;
        if (myCfg.dbi_update_map.find(uri) != myCfg.dbi_update_map.end())
        {
            send_to_dbi_client(myCfg, uri, value);
        }
        else
        {
            start_send_to_dbi_timer(myCfg, uri, io_point->component->dbi_update_frequency);
            // don't send to dbi...there's a good chance this is coming from dbi!!
        }
    }
}

void send_to_dbi_callback(std::shared_ptr<TimeObject> t, void* pDbiStruct)
{
    cfg::dbi_update_struct* dbi_struct = (cfg::dbi_update_struct*)pDbiStruct;
    if (dbi_struct)
    {
        dbi_struct->time_to_update = true;
    }
}

void start_send_to_dbi_timer(struct cfg& myCfg, std::string point_uri, int dbi_update_frequency)
{
    std::string dbi_update_prefix = "/dbi/" + myCfg.client_name + "/saved_registers";
    std::shared_ptr<cfg::dbi_update_struct> dbi_struct = std::make_shared<cfg::dbi_update_struct>();
    dbi_struct->dbi_update_uri = dbi_update_prefix + point_uri;
    myCfg.dbi_update_map[point_uri] = dbi_struct;

    std::shared_ptr<TimeObject> obj1 = createTimeObject("dbi_update_" + point_uri,  // name
                                                        0,                          // start time (initial startup time)
                                                        0,                          // stop time - 0 = don't stop
                                                        dbi_update_frequency,       // how often to repeat
                                                        0,                          // count - 0 = don't stop
                                                        send_to_dbi_callback,       // callback
                                                        (void*)dbi_struct.get());   // callback params
    addTimeObject(obj1, 0, false);
}
