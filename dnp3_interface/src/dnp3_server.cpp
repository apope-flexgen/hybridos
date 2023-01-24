/*
    * dnp3_server (outstation)
    * pwilshire 
    *   july 29, 2020

    * new opendnp3
    *    Dec 19, 2021
    *       TODO / TOTEST scan config ( allow multis )  save sys in shared objct.
    *        Check Points loaded in local DB. 
    *    Jan 24, 2022
    *        Double Check Events and time
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
//using namespace openpal;
//using namespace asiopal;
//using namespace asiodnp3;


fims *p_fims;
int num_configs = 0;

#define MAX_CONFIGS 16
//sysCfg *sys_cfg[MAX_CONFIGS];

SysCfg* sys = nullptr;
bool running = true;
void signal_handler (int sig)
{
    running = false;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

//fill out from system config
DatabaseConfig ConfigureDatabase(sysCfg* sys);


DNP3Manager* setupDNP3Manager(sysCfg* sys)
{
    auto manager = new DNP3Manager(1, fpsLogger::Create(sys));
    return manager;
}

std::shared_ptr<IChannel> setupDNP3channel(DNP3Manager* manager, const char* cname, sysCfg* sys) {
    // Specify what log levels to use. NORMAL is warning and above
    // You can add all the comms logging by uncommenting below.
    const auto FILTERS = levels::NORMAL | levels::ALL_COMMS;
    //std::error_code ec;
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

        // check for files existences:
        // if (!std::filesystem::exists(caCertificate))
        // {
        //     std::cout << "caCertificate: " << caCertificate << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }
        // if (!std::filesystem::exists(certificateChain))
        // {
        //     std::cout << "certificateChain: " << certificateChain << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }
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

    // Create a TCP server (listener)
    //auto channel = manager.AddTLSServer("server", logLevels, ServerAcceptMode::CloseExisting, IPEndpoint("0.0.0.0", 20001),
    //                                    TLSConfig(peerCertificate, localCertificate, privateKey),
    //                                    PrintingChannelListener::Create());
        // Connect via a TCPClient socket to a outstation
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
        // default values if they do not specify in the config (deviceName is "" by default I believe):
        //   baud(9600),
        //   dataBits(8),
        //   stopBits(opendnp3::StopBits::One),
        //   parity(opendnp3::Parity::None),
        //   flowType(opendnp3::FlowControl::None),
        //   asyncOpenDelay(openpal::TimeDuration::Milliseconds(500))
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
        // default file path where we put our certificates
    // Create a TCP server (listener)
    //auto 
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
    // The main object for a outstation. The defaults are useable,
    // but understanding the options are important.

    // Specify the maximum size of the event buffers. Defaults to 0
    config->outstation.eventBufferConfig = EventBufferConfig::AllTypes(100);
    //config.outstation.eventBufferConfig.maxBinaryEvents = fpsDB->dbVec[Type_Binary].size(),
    //config.outstation.eventBufferConfig.maxAnalogEvents = fpsDB->dbVec[Type_Analog].size(),
    // Specify the maximum size of the event buffers
    //config.outstation.eventBufferConfig = EventBufferConfig::AllTypes(10);

    // you can override an default outstation parameters here
    // in this example, we've enabled the oustation to use unsolicted reporting
    // if the master enables it
    //config.outstation.params.allowUnsolicited = true;
    //if (sys->useVindex)
    //    config->outstation.params.indexMode = IndexMode::Discontiguous;

    // if(sys->unsol == 0)
    //     config.outstation.params.allowUnsolicited = false;
    // else
    //     config.outstation.params.allowUnsolicited = true;
    // allow sys to over rule
    setConfigUnsol(sys, config);

    //config.outstation.params.allowUnsolicited = false;

    // You can override the default link layer settings here
    // in this example we've changed the default link layer addressing
    // 
    config->link.LocalAddr = sys->station_address;   // was 10
    config->link.RemoteAddr = sys->master_address;//  was 1;

    config->link.KeepAliveTimeout = opendnp3::TimeDuration::Max();

    // You can optionally change the default reporting variations or class assignment prior to enabling the outstation

    // Create a new outstation with a log level, command handler, and
    // config info this	returns a thread-safe interface used for
    // updating the outstation's database.
    // TODO fpsOutStationApplication
    char tmp [1024];
    snprintf(tmp,1024,"outstation_%s", sys->id);
    auto outstation = channel->AddOutstation(tmp 
                                            , fpsCommandHandler::Create(sys)
                                            , fpsOutstationApplication::Create(sys)
                                            , *config);

    // Enable the outstation and start communications
    outstation->Enable();
    sys->outstation = outstation;
    return outstation;
}
// scaled onto the wire
void addVarToBuilder (UpdateBuilder& builder, DbVar* db, int debug, bool useVindex)
{
    int idx = db->idx;
    if(useVindex)
        idx = db->vindex;

    UTCTimestamp Now();
    auto now = Now();

    switch (db->type) 
    {
        case Type_Analog:
        {
            // lets add flags and time to the thing.
            //Analog(double value)
            //Analog(double value, Flags flags)
            //Analog(double value, Flags flags, DNPTime time)
            //
            //   if(arg == "ONLINE") return AnalogQuality::ONLINE;
            //   if(arg == "RESTART") return AnalogQuality::RESTART;
            //   if(arg == "COMM_LOST") return AnalogQuality::COMM_LOST;
            //   if(arg == "REMOTE_FORCED") return AnalogQuality::REMOTE_FORCED;
            //   if(arg == "LOCAL_FORCED") return AnalogQuality::LOCAL_FORCED;
            //   if(arg == "OVERRANGE") return AnalogQuality::OVERRANGE;
            //   if(arg == "REFERENCE_ERR") return AnalogQuality::REFERENCE_ERR;
            //   if(arg == "RESERVED") return AnalogQuality::RESERVED;
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
    //sysCfg sys_cfg;
    sysCfg *sys_cfg[MAX_CONFIGS];
    
    sys = SysCfg::getInstance(); 
    //sys->setSys(sys_cfg);

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
        //goto cleanup;
    }
    
    num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_OUTSTATION, p_fims);

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

    // use the id for fims connect but also add outstation designation 
    {
        char tmp[1024];
        snprintf(tmp, sizeof(tmp),"DNP3_O_%s", sys_cfg[0]->id);
        while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
        {
            fims_connect++;
            sleep(1);
        }
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

    // Main point of interaction with the stack. 1 thread in the pool for 1 outstation
    // Manager must be in main scope

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
    // repeat for each config
    // put returned outstation into the config context
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
        // sys->OSconfig = new OutstationStackConfig(DatabaseSizes( 
        //                                             sys->getTypeSize(Type_Binary)+1,
        //                                             0,                               // no double binaries
        //                                             sys->getTypeSize(Type_Analog)+1,
        //                                             0,                               // no counters
        //                                             0,                               // no frozen counters
        //                                             sys->getTypeSize(Type_BinaryOS),
        //                                             sys->getTypeSize(Type_AnalogOS),
        //                                             0,                               // no timers
        //                                             0                                // no octet streams
        //                                             ));
        sys->OSconfig = new OutstationStackConfig(ConfigureDatabase(sys));

        auto outstation = setupDNP3outstation(channel, "outstation", sys, sys->OSconfig);
        if (!outstation){
            FPS_ERROR_PRINT( "Outstation setup failed id [%s] master %d station %d\n"
                            , sys->id, sys->master_address, sys->station_address);
            return 1;
        }
        FPS_ERROR_PRINT( "Outstation setup OK id [%s] master %d station %d\n"
                                    , sys->id, sys->master_address, sys->station_address);

        //sys->outstation = outstation;
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

    while(running && p_fims->Connected())
    {

        // use a time out to detect init failure 
        fims_message* msg = p_fims->Receive_Timeout(1000);
        // regardless of message state need to check each config for 
        // lost messages
        // check lost flag
        auto ts  = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = ts - ts_base;
        auto tNow = diff.count();
        // handle no message...
        if(msg == NULL)
        { 
            // TODO check for all the getURI resposes
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
                //auto ts  = std::chrono::system_clock::now();

                sysCfg *sys = sys_cfg[ixs];

                if (sys->debug)
                    FPS_ERROR_PRINT("****** %s got a message uri [%s] \n", __FUNCTION__, msg->uri);
                dbs_type dbs; // collect all the parsed vars here
                //TODO checkTimeouts(dbs, sys); // add all the timeout vars to dbs ...
               
                if(sys->last_seen < 0)
                    sys->last_seen = tNow;
                sys->tNow = tNow;

                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_OUTSTATION);
                //if (sys->debug)
                //    FPS_ERROR_PRINT("****** %s got a message uri [%s]  cjb [%p] dbs.size %d \n", __FUNCTION__, msg->uri, (void*)cjb, (int)dbs.size());
                //int format  = 0;
                if(dbs.size() > 0)
                {
                    cJSON* cj = cJSON_CreateObject();
                    
                    UpdateBuilder builder;
                    int varcount = 0;
                    while (!dbs.empty())
                    {
                        std::pair<DbVar*,int>dbp = dbs.back();
                        DbVar* db = dbp.first;
                        db->state = STATE_ONLINE;
                        // only do this on sets pubs or  posts
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
                        addVarToCj(cj, dbp);  // include flag
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
                        if(msg->replyto)
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
        // add check here for things not updated.
        // auto ts  = std::chrono::system_clock::now();
        // double t1 = ts.time_since_epoch().count();
        //auto ts  = std::chrono::system_clock::now();
        for (int ixs = 0 ; ixs < num_configs; ixs++ )
        {
            bool checklost = false;
            sysCfg *sys = sys_cfg[ixs];
            // now check update time for each var 
            //FPS_ERROR_PRINT("\n dnp3 type [%s]\n", iotypToStr(i)); 
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                //dbVec[type].push_back(db);
                //std::chrono::duration<double> diff = ts - ts_base;
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
                dbs_type dbs; // collect all the timeout vars here
                //TODO checkTimeouts(dbs, sys); // add all the timeout vars to dbs ...

                for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
                {
                    //dbVec[type].push_back(db);

                    if(sys->debug>0)
                        FPS_ERROR_PRINT(" Time [%2.3f] Checking coms on dbvec cfg [%d] type: [%s] size [%d]\n"
                            , sys->tNow, ixs, iotypToStr(i), (int)sys->dbVec[i].size());
                    for (auto db : sys->dbVec[i])
                    {
                        if (db->timeout)
                        {
                            if(db->last_seen < 0)
                                db->last_seen = tNow;
                            if(1)FPS_ERROR_PRINT(" Time [%2.3f] Comms lost  db [%s] state [%d] ONLINE [%d] \n", sys->tNow, db->name.c_str(), db->state, STATE_ONLINE);
                            if (db->state==STATE_ONLINE)
                            {
                                if((tNow - db->last_seen) >= db->timeout)
                                {
                                    FPS_ERROR_PRINT(" Time [%2.3f] Comms lost  db [%s] last [%2.3f] timeout [%2.3f] \n", sys->tNow, db->name.c_str(), db->last_seen,db->timeout);

                                    db->state = STATE_COMM_LOST;
                                    db->flags=(int)BinaryQuality::COMM_LOST; // need the correct typ from [i]
                                    db->flags = 0x04;
                                    db->time = (long int)ts.time_since_epoch().count();//sys->tNow;
                                    db->newFlags = true;
                                    db->newTime = true;
                                    dbs.push_back(std::make_pair(db, db->flags));
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
                            dbs.push_back(std::make_pair(db, db->flags));
                        }
                        // else if ( db->state == STATE_COMM_LOST)
                        // {
                        //     if((sys->tNow - db->last_seen) < db->timeout)
                        //     {
                        //         db->state = STATE_ONLINE;
                        //         db->flags = 0x01;
                        //         db->time = sys->tNow;
                        //         db->newFlags = true;
                        //         db->newTime = true;
                        //         dbs->push_back(db);
                        //     }        
                        // }
                    }    
                    //dbVec[i].clear();
                }
                if(dbs.size() > 0)
                {
                    UpdateBuilder builder;
                    int varcount = 0;
                    while (!dbs.empty())
                    {;
                        std::pair<DbVar*,int>dbp = dbs.back();
                        DbVar* db = dbp.first;
                        varcount++;
                        db->last_seen = tNow;
                        if (db->state == STATE_COMM_LOST)
                        {
                            db->flags = 0x04;
                            db->newFlags = true;
                            db->newTime = true;
                            db->time=(long int)ts.time_since_epoch().count();//sys->tNow;
                        }        
                        addVarToBuilder(builder, db, sys->debug, sys->useVindex);
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
                        varcount = 0;
                    }
                }
            }
        }
 
    }

    //TODO 
    //cleanup:

    if (manager) delete manager;
    if (p_fims) delete p_fims;
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        if (sys->OSconfig) delete sys->OSconfig;
        //if (sys->outstation) delete sys->outstation;
        delete sys;
    }
    // if(sys_cfg[0]->ip_address    != NULL) free(sys_cfg[0]->ip_address);
    // if(sys_cfg[0]->name          != NULL) free(sys_cfg[0]->name);
    return rc;
}
