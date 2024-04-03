/*
    * dnp3_server (outstation)
    * p wilshire 
    *   july 29, 2020
    *       
    * 
*/

#include <fims/libfims.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>

#include <fims/fps_utils.h>
#include <cjson/cJSON.h>
#include <unistd.h>
#include "dnp3_utils.h"

#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>

#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/outstation/IUpdateHandler.h>

#include <opendnp3/DNP3Manager.h>
#include <opendnp3/outstation/UpdateBuilder.h>
#include "fpsCommandHandler.h"
#include "fpsLogger.h"
#include "fpsChannelListener.h"
#include "fpsOutstationApplication.h"


using namespace std;
using namespace opendnp3;


fims *p_fims;
int num_configs = 0;

#define MAX_CONFIGS 16

SysCfg* sys = nullptr;
bool running = true;
void signal_handler (int sig)
{
    running = false;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

DatabaseConfig ConfigureDatabase(sysCfg* sys);

DNP3Manager* setupDNP3Manager(sysCfg* sys)
{
    auto manager = new DNP3Manager(1, fpsLogger::Create(sys));
    return manager;
}

std::shared_ptr<IChannel> setupDNP3channel(DNP3Manager* manager, const char* cname, sysCfg* sys) {
    const auto FILTERS = levels::NORMAL | levels::ALL_COMMS;
    std::shared_ptr<IChannel> channel;
    const char* tls_folder_path = 
        "/usr/local/etc/config/dnp3_interface"; 
    if (sys->conn_type && strcmp(sys->conn_type, "TLS") == 0) // TLS client type
    {
        if (sys->tls_folder_path) // if they have a custom path to tls certificates then use that
        {
            tls_folder_path = sys->tls_folder_path;
        }
        if (!sys->peerCertificate )
        {
            std::cout << "tls is missing peerCertificate please provide it\n";
            return nullptr;
        }
        if (!sys->localCertificate )
        {
            std::cout << "tls is missing localCertificate please provide it\n";
            return nullptr;
        }
        if (!sys->privateKey)
        {
            std::cout << "tls is missing file names for privateKey please provide it\n";
            return nullptr;
        }
        std::string peerCertificate  = std::string{tls_folder_path} + '/' + std::string{sys->peerCertificate};
        std::string localCertificate = std::string{tls_folder_path} + '/' + std::string{sys->localCertificate};
        std::string privateKey       = std::string{tls_folder_path} + '/' + std::string{sys->privateKey};

        if (!checkFileName(privateKey.c_str()))
        {
            std::cout << "privateKey: " << privateKey << " doesn't exist. Please provide it\n";
            return nullptr;
        }

        if (!checkFileName(peerCertificate.c_str()))
        {
            std::cout << "peerCertificate: " << peerCertificate << " doesn't exist. Please provide it\n";
            return nullptr;
        }

        if (!checkFileName(localCertificate.c_str()))
        {
            std::cout << "localCertificate: " << localCertificate << " doesn't exist. Please provide it\n";
            return nullptr;
        }

        channel = manager->AddTLSServer(cname, 
                                        FILTERS, 
                                        ServerAcceptMode::CloseExisting, 
                                        IPEndpoint(sys->ip_address, sys->port), 
                                        TLSConfig(peerCertificate, localCertificate, privateKey),
                                        fpsChannelListener::Create(sys));

        if (!channel)
        {
           std::cout << "Unable to create tls server: " << std::endl;
           return nullptr;
        }
    }
    else if (sys->conn_type && strcmp(sys->conn_type, "RTU") == 0) // serial/RTU channel
    {
        std::cout << "Creating rtu server: " << std::endl;
        
        SerialSettings rtu_settings;
        if (sys->baud != 0)
        {
            rtu_settings.baud = sys->baud; // do error checking here (how?)
        }
        if (sys->dataBits != 0)
        {
            rtu_settings.dataBits = sys->dataBits; // do error checking here (how?)
        }
        if (sys->stopBits >= 0) // double/float
        {
            if (sys->stopBits == 0)
            {
                rtu_settings.stopBits = opendnp3::StopBits::None;
            }
            else if (sys->stopBits == 1)
            {
                rtu_settings.stopBits = opendnp3::StopBits::One;
            }
            else if (sys->stopBits == 1.5)
            {
                rtu_settings.stopBits = opendnp3::StopBits::OnePointFive;
            }
            else if (sys->stopBits == 2)
            {
                rtu_settings.stopBits = opendnp3::StopBits::Two;
            }
            else
            {
                std::cout << "a stopBits number of: " << sys->stopBits << " is not accepted for serial RTU communications\n";
                return nullptr;
            }
        }
        if (sys->parity)
        {
            if (strcmp(sys->parity, "None") == 0)
            {
                rtu_settings.parity = opendnp3::Parity::None;
            }
            else if (strcmp(sys->parity, "Even") == 0)
            {
                rtu_settings.parity = opendnp3::Parity::Even;
            }
            else if (strcmp(sys->parity, "Odd") == 0)
            {
                rtu_settings.parity = opendnp3::Parity::Odd;
            }
            else
            {
                std::cout << "parity: " << sys->parity << " is not accepted for serial RTU communications\n";
                return nullptr;
            }
        }
        if (sys->flowType)
        {
            if (strcmp(sys->flowType, "None") == 0)
            {
                rtu_settings.flowType = opendnp3::FlowControl::None;
            }
            else if (strcmp(sys->flowType, "Hardware") == 0)
            {
                rtu_settings.flowType = opendnp3::FlowControl::Hardware;
            }
            else if (strcmp(sys->flowType, "XonXoff") == 0)
            {
                rtu_settings.flowType = opendnp3::FlowControl::XONXOFF;
            }
            else
            {
                std::cout << "flowtype: " << sys->flowType << " is not accepted for serial RTU communications\n";
                return nullptr;
            }
        }
        if (sys->asyncOpenDelay != 0) // I don't know what this is supposed to be
        {
             if (sys->asyncOpenDelay < 0)
             {
                 std::cout << "asyncOpenDelay: " << sys->asyncOpenDelay << " is negative, must be a postitive value (this is in milliseconds) for serial RTU communications\n";
                 return nullptr;
             }
             rtu_settings.asyncOpenDelay= TimeDuration::Milliseconds(sys->asyncOpenDelay);
        }
        if (sys->deviceName)
        {
            rtu_settings.deviceName = sys->deviceName; 
        }

        if (!checkDeviceName(sys->deviceName))
        {
            std::cout << "Serial Device : " << sys->deviceName << " doesn't exist. Please correct config\n";
            return nullptr;
        }

        channel = manager->AddSerial(sys->id
                                        , FILTERS
                                        , ChannelRetry::Default()
                                        , rtu_settings
                                        , fpsChannelListener::Create(sys));
                                        // , PrintingChannelListener::Create());
        if (!channel)
        {
           std::cout << "Unable to create rtu server: " << std::endl;
           return nullptr;
        }
        std::cout << "Created rtu server:  ["<< sys->deviceName<< "]"  << std::endl;
        return channel;
    }
    else // tcp server type for default strcmp fail:
    {
        channel = manager->AddTCPServer(cname, 
                                        FILTERS, 
                                        ServerAcceptMode::CloseExisting, 
                                        IPEndpoint(sys->ip_address, sys->port),
                                        fpsChannelListener::Create(sys)
                                        );
    }
    return channel;
}

void setConfigUnsol(sysCfg* sys, OutstationStackConfig *config)
{
    if(sys->unsol == 0)
        config->outstation.params.allowUnsolicited = false;
    else
        config->outstation.params.allowUnsolicited = true;
}

std::shared_ptr<IOutstation> setupDNP3outstation (std::shared_ptr<IChannel> channel, const char* mname,
                                 sysCfg* sys, OutstationStackConfig *config)
{
    config->outstation.eventBufferConfig = EventBufferConfig::AllTypes(sys->event_buffer);

    setConfigUnsol(sys, config);

    config->link.LocalAddr = sys->station_address;   // was 10
    config->link.RemoteAddr = sys->master_address;//  was 1;

    config->link.KeepAliveTimeout = opendnp3::TimeDuration::Max();

    char tmp [1024];
    snprintf(tmp,1024,"outstation_%s", sys->id);
    auto outstation = channel->AddOutstation(tmp 
                                            , fpsCommandHandler::Create(sys)
                                            , fpsOutstationApplication::Create(sys)
                                            , *config);

    outstation->Enable();
    sys->outstation = outstation;
    return outstation;
}

void addVarToBuilder (UpdateBuilder& builder, DbVar* db, int debug, bool useVindex)
{
    int idx = db->idx;
    if(useVindex)
        idx = db->vindex;

    UTCTimestamp Now;
    auto now = Now;

    switch (db->type) 
    {
        case Type_Analog:
        {
            double dval = db->valuedouble;
            if(debug>0)
            {
                FPS_ERROR_PRINT("*****  %s  var name [%s] db->idx [%d] idx [%d] value [%f]\n"
                ,__FUNCTION__, db->name.c_str(), db->idx, idx, dval);
            }
            if((db->scale > 0.0) || (db->scale < 0.0))
            {
                dval *= db->scale;
                if(debug>0)
                {
                    FPS_ERROR_PRINT("*****  %s  var name [%s] idx [%d] vindex [%d] scaled value [%f]\n"
                    ,__FUNCTION__, db->name.c_str(), db->idx, db->vindex, dval);
                }
            }
            if(db->newFlags)
            {
                if(db->newTime)
                {
                    builder.Update(Analog(dval, Flags(db->flags), DNPTime(now.msSinceEpoch)), idx);
                }
                else
                {
                    builder.Update(Analog(dval, Flags(db->flags)), idx);
                }
                db->newFlags = false;
                db->newTime = false;
            }
            else
            {
                builder.Update(Analog(dval), idx);
            }
            break;
        }
        case Type_AnalogOS:
        {
            builder.Update(AnalogOutputStatus(db->valuedouble), idx);
            break;
        }
        case Type_Binary:
        {
            int32_t vint = static_cast<int32_t>(db->valuedouble);
            if(db->scale < 0.0) 
            {
                if(vint > 0)
                    vint = 0;
                else
                    vint = 1;
            }
            if(db->newFlags)
            {
                if(debug > 0)
                {
                    FPS_ERROR_PRINT("%s    setting Flags [%d]", db->name.c_str(), db->flags);
                }
                if(db->newTime)
                {
                    builder.Update(Binary(vint, Flags(db->flags), DNPTime(now.msSinceEpoch)), idx);
                }
                else
                {
                    builder.Update(Binary(vint, Flags(db->flags)), idx);
                }
                db->newFlags = false;
                db->newTime = false;

            }
            else
            {
                builder.Update(Binary(vint), idx);
            }
            break;
        }
        case Type_BinaryOS:
        {
            int32_t vint = static_cast<int32_t>(db->valuedouble);
            builder.Update(BinaryOutputStatus(vint), idx);
            break;
        }
        default:
            break;
    }
}

// Used to instansiate singleton
class SysCfg;
auto ts_base  = std::chrono::system_clock::now();
int main(int argc, char* argv[])
{
    sysCfg *sys_cfg[MAX_CONFIGS];
    
    sys = SysCfg::getInstance(); 

    int rc = 0;
    int fims_connect = 0;
    int ttick = 0; // timeout tick

    if (strcmp(DNP3_UTILS_VERSION, getVersion())!=0)
    {
        FPS_ERROR_PRINT("Error with installed DNP3_UTILS_VERSION\n");
        return 1;
    }

    running = true;
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    p_fims = new fims();

    if (p_fims == NULL)
    {
        FPS_ERROR_PRINT("Failed to allocate connection to FIMS server.\n");
        rc = 1;
        return 1;
    }
    
    num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_OUTSTATION, p_fims);
    if (num_configs < 0){
        return 1;
    }

    const char **subs = NULL;
    bool *bpubs = NULL;
    int num = getSysUris((sysCfg**)&sys_cfg, DNP3_OUTSTATION, subs, bpubs, num_configs);

    if(num < 0)
    {
        FPS_ERROR_PRINT("Failed to create subs array.\n");
        return 1;
    }

    FPS_ERROR_PRINT(">>>>>> Num Uris found %d .\n", num);
    for (int ix = 0; ix < num; ix++ )
    {
        FPS_ERROR_PRINT(">>>>>>>>>> Uri [%d] [%s] \n", ix, subs[ix]);    
    }

    char tmp[1024];
    snprintf(tmp, sizeof(tmp),"DNP3_O_%s", sys_cfg[0]->id);
    while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
    {
        fims_connect++;
        sleep(1);
    }

    if(fims_connect >= MAX_FIMS_CONNECT)
    {
        FPS_ERROR_PRINT("Failed to establish connection to FIMS server.\n");
        rc = 1;
        return 1;
        //goto cleanup;
    } 

    if(p_fims->Subscribe(subs, num, bpubs) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }

    free((void *)bpubs);
    free((void *)subs);


    auto manager = setupDNP3Manager(sys_cfg[0]);
    if (!manager)
    {
        FPS_ERROR_PRINT( "DNP3 Manger setup failed.\n");
        return 1;
    }

    auto channel = setupDNP3channel(manager, "server", sys_cfg[0]);

    if (!channel){
        FPS_ERROR_PRINT( "DNP3 Channel setup failed.\n");
        return 1;
    }

    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        if(sys->debug)
        {
            cout<<"*** Binaries: dbVec: "<<sys->dbVec[Type_Binary].size()
                << " indx :" << sys->getTypeSize(Type_Binary) << endl;

            cout<<"*** Analogs: dbVec: "<<sys->dbVec[Type_Analog].size()
                <<" indx :" << sys->getTypeSize(Type_Analog)+1 << endl;
        }

        sys->OSconfig = new OutstationStackConfig(ConfigureDatabase(sys));

        auto outstation = setupDNP3outstation(channel, "outstation", sys, sys->OSconfig);
        if (!outstation){
            FPS_ERROR_PRINT( "Outstation setup failed id [%s] master %d station %d\n"
                            , sys->id, sys->master_address, sys->station_address);
            return 1;
        }

        FPS_ERROR_PRINT( "Outstation setup OK id [%s] master %d station %d\n"
                                    , sys->id, sys->master_address, sys->station_address);
        sys->getUris(DNP3_OUTSTATION);
    }

    FPS_ERROR_PRINT( "DNP3 Server Setup complete: Entering main loop.\n");
    // issue gets if needed 
    // for (int ixs = 0 ; ixs < num_configs; ixs++ )
    // {
    //     sysCfg *sysi = sys_cfg[ixs];
    //     if(1||sysi->debug) FPS_ERROR_PRINT("[%d] useGets %s\n", ixs, sysi->useGets?"true":"false");
    //     if(1|| sysi->useGets) sysi->getUris(DNP3_OUTSTATION);
    // }
    auto fims_delay = 1000;

    while(running && p_fims->Connected())
    {

        fims_message* msg = p_fims->Receive_Timeout(fims_delay);
        auto ts  = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = ts - ts_base;
        auto tNow = diff.count();

        if(msg == NULL)
        { 
            ttick++;
            if(ttick%100 == 0)
            {
                for (int ixs = 0 ; ixs < num_configs; ixs++ )
                {
                    sysCfg *sys = sys_cfg[ixs];

                    if(sys->debug)FPS_ERROR_PRINT("Timeout tick %d\n", ttick);
                    bool ok = sys->checkUris(DNP3_OUTSTATION);
                    if(ok == false)
                    {
                        if (ttick > (MAX_SETUP_TICKS*10))
                        {
                            // just quit here
                            if (sys->debug)
                                 FPS_ERROR_PRINT("QUITTING TIME Timeout tick %d\n", ttick);
                        }
                    }
                }
            }
        }
        else
        {
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];

                if (sys->debug)
                    FPS_ERROR_PRINT("****** %s got a message uri [%s] \n", __FUNCTION__, msg->uri);
                dbs_type dbs; // collect all the parsed vars here
               
                if(sys->last_seen < 0)
                    sys->last_seen = tNow;
                sys->tNow = tNow;

                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_OUTSTATION);
                if(dbs.size() > 0)
                {
                    cJSON* cj = cJSON_CreateObject();
                    
                    UpdateBuilder builder;
                    int varcount = 0;
                    while (!dbs.empty())
                    {
                        std::pair<DbVar*,int>dbp = dbs.back();
                        DbVar* db = dbp.first;
                        if  (db->state==STATE_COMM_LOST)
                        {
                            FPS_ERROR_PRINT(" Time [%2.3f] Variable Update Restored; db [%s] last [%2.3f] timeout [%2.3f] \n"
                                                , sys->tNow, db->name.c_str(), db->last_seen,db->timeout);
                        }
                        db->state = STATE_ONLINE;

                        if (
                            (strcmp(msg->method, "set") == 0) || 
                            (strcmp(msg->method, "post") == 0) || 
                            (strcmp(msg->method, "pub") == 0)
                            )
                            {
                                varcount++;
                                db->flags = 0x01; // ONLINE
                                db->newFlags = true;
                                db->newTime = true;
                                db->time=(long int)ts.time_since_epoch().count();//sys->tNow;
                                db->last_seen = tNow;
                                addVarToBuilder(builder, db, sys->debug, sys->useVindex);
                            }
                        addVarToCj(sys, cj, dbp);  // include flag
                        addCjTimestamp(cj, "Timestamp");
                        dbs.pop_back();
                    }

                    //finalize set of updates
                    if(varcount > 0) 
                    {
                        auto updates = builder.Build();
                        if(sys->outstation)
                        {
                            sys->outstation->Apply(updates);
                        }
                    }
                    if(cj)
                    {
                        if((msg->replyto) && (strcmp(msg->method, "pub") != 0))
                        {
                            const char* reply = cJSON_PrintUnformatted(cj);
                            p_fims->Send("set", msg->replyto, NULL, reply);
                            free((void* )reply);
                        }
                        cJSON_Delete(cj);
                        cj = NULL;
                    }
                }
            
                if (sys->scanreq > 0)
                {
                    FPS_ERROR_PRINT("****** outstation scanreq %d ignored \n", sys->scanreq);
                    sys->scanreq = 0;
                }

                if (sys->unsolUpdate)
                {
                    FPS_ERROR_PRINT("****** outstation unsol %d handled \n", sys->unsol);
                    setConfigUnsol(sys, sys->OSconfig);
                    sys->unsolUpdate = false;
                }

                if (sys->cjclass != NULL)
                {
                    const char*tmp = cJSON_PrintUnformatted(sys->cjclass);
                    if(tmp != NULL)
                    {                
                        FPS_ERROR_PRINT("****** outstation class change [%s] handled \n", tmp);
                        free((void*)tmp);
                        sys->cjclass = NULL;
                    }
                }

                if (cjb != NULL)
                {
                    cJSON_Delete(cjb);
                    cjb = NULL;
                }
            }
            p_fims->free_message(msg);
        }

        for (int ixs = 0 ; ixs < num_configs; ixs++ )
        {
            bool checklost = false;
            sysCfg *sys = sys_cfg[ixs];
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                sys->tNow = tNow;
                if(sys->last_seen < 0)
                    sys->last_seen = sys->tNow;


                if (sys->timeout)
                {
                    if((sys->tNow - sys->last_seen) > sys->timeout)
                    {
                        checklost = true;
                        sys->last_seen = sys->tNow;
                    }
                }
                if(sys->debug>3)
                {
                    FPS_ERROR_PRINT(" Time [%2.3f] last [%2.3f] Checking dbvec cfg [%d] type: [%s] size [%d] timeout [%2.3f] checklost [%s]\n"
                        , (sys->tNow), sys->last_seen, ixs, iotypToStr(i), (int)sys->dbVec[i].size(), (double)sys->timeout, checklost?"true":"false");
                }
            }
        
            if(checklost)
            {

                UpdateBuilder builder;
                int varcount = 0;
                for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
                {

                    if(sys->debug>0)
                        FPS_ERROR_PRINT(" Time [%2.3f] Checking coms on dbvec cfg [%d] type: [%s] size [%d]\n"
                            , sys->tNow, ixs, iotypToStr(i), (int)sys->dbVec[i].size());
                    for (auto db : sys->dbVec[i])
                    {
                        if (db->timeout)
                        {
                            if(db->last_seen < 0)
                                db->last_seen = tNow;
                            if(0)FPS_ERROR_PRINT(" Time [%2.3f] Comms lost  db [%s] state [%d] ONLINE [%d] \n", sys->tNow, db->name.c_str(), db->state, STATE_ONLINE);
                            if (db->state==STATE_ONLINE)
                            {
                                if((tNow - db->last_seen) >= db->timeout)
                                {
                                    FPS_ERROR_PRINT(" Time [%2.3f] Variable Update Failed;  db [%s] last [%2.3f] timeout [%2.3f] \n", sys->tNow, db->name.c_str(), db->last_seen,db->timeout);

                                    db->state = STATE_COMM_LOST;
                                    db->flags=(int)BinaryQuality::COMM_LOST; // need the correct typ from [i]
                                    db->flags = 0x04;
                                    db->time = (long int)ts.time_since_epoch().count();//sys->tNow;
                                    db->newFlags = true;
                                    db->newTime = true;
                                    addVarToBuilder(builder, db, sys->debug, sys->useVindex);
                                    varcount++;
                                }
                            } 
                        }
                        // no timeout means go ONLINE automatically
                        else if ( db->state == STATE_INIT)
                        {
                            FPS_ERROR_PRINT(" Time [%2.3f] State Init  db [%s] last [%2.3f] timeout [%2.3f] \n", sys->tNow, db->name.c_str(), db->last_seen,db->timeout);
                            db->state = STATE_ONLINE;
                            db->flags = 0x01;
                            db->time = sys->tNow;
                            db->newFlags = true;
                            db->newTime = true;
                            addVarToBuilder(builder, db, sys->debug, sys->useVindex);
                            varcount++;
                        }
                    }    
                }
                if (varcount > 0)
                {
                    varcount = 0;
                    auto updates = builder.Build();
                    if(sys->outstation)
                    {
                        sys->outstation->Apply(updates);
                    }
                }
            }

            if (sys->batch_sets > 0) 
            {
                if (tNow > sys->next_batch_time) 
                {
                    sys->next_batch_time = tNow+sys->batch_sets;

                    if (sys->cjOut) {
                        sys->setLock.lock();
                        // need to lock this
                        cJSON *cj = sys->cjOut;
                        sys->cjOut = NULL;
                        sys->setLock.unlock();


                        if (sys->debug > 1)
                        {
                            char *out2 =cJSON_PrintUnformatted(cj);
                            if (out2)
                            {
                                FPS_ERROR_PRINT(" Batch out  tNow [%f] next [%f] [%s]\n", tNow, sys->next_batch_time, out2);
                                free(out2);
                            }
                        }
                        cJSON *cjn = cj->child;
                        while (cjn) {
                            char tmp[20480];
                            char *out3 = cJSON_PrintUnformatted(cjn);
                            if (sys->debug > 1)
                                FPS_ERROR_PRINT(" Batch out cjn [/%s] -> [%s]\n", cjn->string, out3);
                            snprintf(tmp, sizeof(tmp), "/%s", cjn->string);
                            p_fims->Send("set", tmp, NULL, out3);

                            free(out3);
                            cjn = cjn->next;
                        }
                        cJSON_Delete(cj);
                    }
                }
            }
            if (sys->batch_pubs > 0) 
            {
                if (0)FPS_ERROR_PRINT(" Time [%2.3f] batch_pubs [%2.3f] Checking next_batch_pub_time [%2.3f]\n"
                                    , sys->tNow, sys->batch_pubs, sys->next_batch_pub_time);
                if (tNow > sys->next_batch_pub_time) 
                {
                    sys->next_batch_pub_time = tNow+sys->batch_pubs;
                    if (sys->batch_pub_debug > 0)
                    {
                        FPS_ERROR_PRINT(" Time [%2.3f] batch_pubs [%2.3f] =======> running  next_batch_pub_time [%2.3f]\n"
                                        , sys->tNow, sys->batch_pubs, sys->next_batch_pub_time);
                    }
                    UpdateBuilder builder;
                    int varcount = 0;

                    for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
                    {
                        if ((i == Type_Analog) || (i == Type_Binary))
                        {
                            if(sys->batch_pub_debug>0)
                                FPS_ERROR_PRINT(" Time [%2.3f] Checking  value_set   cfg [%d] type: [%s] size [%d]\n"
                                    , sys->tNow, ixs, iotypToStr(i), (int)sys->dbVec[i].size());

                            for (auto db : sys->dbVec[i])
                            {
                                if (db->value_set)
                                {
                                    varcount++;
                                    if(sys->batch_pub_debug>0)
                                        FPS_ERROR_PRINT(" Time [%2.3f] ===> found   value_set %d  name [%s] varcount [%d]\n"
                                                , sys->tNow, db->value_set, db->name.c_str(), varcount);

                                    db->value_set = 0;
                                    // if not online make it so 
                                    if (db->state!=STATE_ONLINE)
                                    {
                                        db->state = STATE_ONLINE;
                                        db->flags = 0x01;
                                        db->time = (long int)ts.time_since_epoch().count();//sys->tNow;
                                        db->newFlags = true;
                                        db->newTime = true;
                                    }
                                    addVarToBuilder(builder, db, sys->debug, sys->useVindex);
                                }        
                            }
                        }
                    }
                    if (varcount > 0)
                    {
                        auto updates = builder.Build();
                        if(sys->outstation)
                        {
                            sys->outstation->Apply(updates);
                        }
                        varcount = 0;
                    }

                }
            }
        }
 
    }

    if (manager) delete manager;
    if (p_fims) delete p_fims;
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        if (sys->OSconfig) delete sys->OSconfig;
        delete sys;
    }
    return rc;
}
