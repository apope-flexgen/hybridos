/* this is the outline of a master dnp3 controller
* the system can communicate with a number outstations
* The confg file will determine the address info on each outstation.
* each outsation will have a number of data points.
* a feature of dnp3 is the need to povide a shadow  stack to hold the user defined data points
* iniitally a copy of the modbus stuff
* note that modbus_client only responds to gets and sets on the base_url
* it pubs the result of the query_registers
*base_uri is set by /components + sys_cfg.name
*/
/*
 * dnp3_client.cpp
 *
 *  Created on: Sep 28, 2020
 *      Author: jcalcagni - modbus
 *              pwilshire - dnp3 version
 */

#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <signal.h>
#include <climits>

#include <arpa/inet.h>
#include <stdint.h>
#include <cjson/cJSON.h>
#include <fims/libfims.h>

#include <iostream>
//#include <asiopal/UTCTimeSource.h> 
#include <opendnp3/logging/LogLevels.h> 
#include <opendnp3/ConsoleLogger.h> 

//#include <asiodnp3/DefaultMasterApplication.h> 
#include <opendnp3/channel/PrintingChannelListener.h> 
//#include <asiodnp3/PrintingCommandCallback.h> 
#include <opendnp3/channel/ChannelRetry.h>
#include <opendnp3/master/PrintingSOEHandler.h>

#include "opendnp3/master/PrintingCommandResultCallback.h"



#include "fpsSOEHandler.h"
#include "fpsMasterApplication.h"
#include "fpsLogger.h"
#include "fpsChannelListener.h"
#include "fpsCommandCallback.h"
#include "dnp3_utils.h"

using namespace std; 
//using namespace openpal; 
//using namespace asiopal; 
//using namespace asiodnp3; 
using namespace opendnp3; 
// #define MICROSECOND_TO_MILLISECOND 1000
// #define NANOSECOND_TO_MILLISECOND  1000000
// #define MAX_FIMS_CONNECT 5

volatile bool running = true;
int num_configs = 0;
#define MAX_CONFIGS 16
sysCfg *sys_cfg[MAX_CONFIGS];

void signal_handler (int sig)
{
    running = false;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

class TestSOEHandler : public ISOEHandler
{

    virtual void BeginFragment(const ResponseInfo& info){};
    virtual void EndFragment(const ResponseInfo& info){};

    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values);
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<DoubleBitBinary>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values);
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<Counter>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<FrozenCounter>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<OctetString>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<TimeAndInterval>>& values);
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryCommandEvent>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogCommandEvent>>& values) {};
    virtual void Process(const HeaderInfo& info, const ICollection<DNPTime>& values) {};
public:
    TestSOEHandler(sysCfg* fpsDB) { sysdb = fpsDB;};
    static std::shared_ptr<ISOEHandler> Create(sysCfg* fpsDB)
    {
        return std::make_shared<TestSOEHandler>(fpsDB);
    }
    sysCfg* sysdb;
};

void TestSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<TimeAndInterval>>& values) {
    //static sysCfg *static_sysdb = sysdb;
    sysCfg * sys = sysdb; 
    //static int items = 0;
    if(sys->debug)
        FPS_ERROR_PRINT(">> ******************************TimeAndInterval:  enum [%04x] \n", (int)info.gv);

}

void TestSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) {
    //static sysCfg *static_sysdb = sysdb;
    sysCfg * sys =  sysdb; 
    auto ts  = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = ts - sys->ts_base;
    sys->tNow = diff.count();
    UTCTimestamp Now();
    //auto now = Now();

    static int items = 0;
    if(sysdb->debug)
        FPS_ERROR_PRINT(">> ******************************Bin:  enum [%04x] ->[%s] fmt [%d] events [%s]  tnow [%2.3f]\n"
                    , (int)info.gv, variation_encode((int)info.gv), sys->fmt,sys->events?"true":"false", sys->tNow);
    auto print = [info, sys](const Indexed<Binary>& pair) {
        //DbVar* db = static_sysdb->getDbVarId(Type_Binary, pair.index);
        auto now = Now();

        DbVar* db = sys->getDbVarId(Type_Binary, pair.index);
        if(db == NULL)
        {
            if(sys->debug) 
            {
                FPS_ERROR_PRINT ("****************** ERROR Binary Data entry not found for index [%d] \n", pair.index);
            }
        }
        if (db != NULL) 
        {
            auto etype = (int)info.gv;
            int ptype = 0;
            const char* vname = db->name.c_str();// static_sysdb->getBinary(pair.index);
            int flags = pair.value.flags.value;
            if(sys->debug)
                FPS_ERROR_PRINT("***************************** binary event  for idx [%d] name [%s] etype 0x%04x\n"
                                    , pair.index, db->name.c_str(), etype);

            if(etype != 0x103)
            {
                db->flags = flags;
                //inline static Binary From(uint8_t flags, uint16_t time){return From(flags, DNPTime(time));}
                //auto q = pair.value.flags; 129 = STATE | ONLINE
                // ONLINE(0x1) RESTART(0x2) COMM_LOST(0x4) REMOTE_FORCED(0x8) LOCAL_FORCED(0x10) CHATTER_FILTER(0x20) RESERVED(0x40),  STATE (0x80)
                BinaryQuality q = BinaryQuality::RESTART;
                try 
                {
                    db->sflags = "";
                    bool add_comma = false;
                    for (int ix = 8 ;ix >=0  ; ix--)
                    {
                        if(pair.value.flags.value & (1<<ix))
                        {
                            if(add_comma)
                                db->sflags += ",";
                            q = BinaryQualitySpec::from_type(pair.value.flags.value&(1<<ix));
                            //q = AnalogQuality::ONLINE;
                            db->sflags += BinaryQualitySpec::to_human_string(q);
                            add_comma = true;
                        }
                    }
                    //q =BinaryQualitySpec::from_type(pair.value.flags.value & 0x7f);
                    //db->sflags = BinaryQualitySpec::to_human_string(q);
                }
                catch (...)
                {
                    FPS_ERROR_PRINT ("****************** ERROR Bad Quality Flag [%d] forcing ONLINE\n", flags);
                    q = BinaryQuality::ONLINE;
                }
            }
            //Group2Var1 Binary Input Event - Without Time
            //Group2Var2 Binary Input Event - With Absolute Time
            //Group2Var3 Binary Input Event - With Relative Time
            if(etype == 0x0202)
            {
                db->etime = ToUTCString(pair.value.time);
            }
            else if(etype == 0x0203)
            {
                db->etime = ToUTCString(pair.value.time);
            }
            else if(etype == 0x0201)
            {
                db->ltime = ToUTCString(DNPTime(now.msSinceEpoch));
            }
            else
            {
                db->etime = ToUTCString(DNPTime(now.msSinceEpoch));
            }
            db->stime = ToUTCString(DNPTime(now.msSinceEpoch));
            if(sys->debug)
                FPS_ERROR_PRINT("***************************** bin idx %d name [%s] value [%d] flags [%d] q [%s] stime [%s] db_events [%s]\n"
                                     , pair.index, db->name.c_str(), pair.value.value, flags, db->sflags.c_str(), 
                                          db->stime.c_str(), db->events?"true":"false" );

            if(strcmp(vname,"Unknown")!= 0) 
            {
                bool val = true;
                if (pair.value.value == 0)
                    val = false;
                if (db->scale < 0.0)
                {
                    val = (val != true);
                }
                // if info.gv is 
                // 0203 etc this is an event with flags and time
                // 0201 etc this is an event with flags and no time
                // we send pub immediately on events
                ptype = sys->fmt;  // = 0 naked 1 = clothed
                int reject = 0;
                if((etype != 0x0203) && (etype != 0x0201))
                {
                    //ptype = sys->fmt;  // = 0 naked 1 = clothed
                    if (db->format)
                    {
                        ptype = db->fmt;
                    }
                }
                else 
                {
                    if(sys->debug)
                        FPS_ERROR_PRINT("***************************** creating event for idx [%d] name [%s] etype 0x%04x\n"
                                     , pair.index, db->name.c_str(), etype);
                    reject = HandleEvent(sys, db , ptype);
                }
                if(reject>=0)
                {
                    db->last_seen = sys->tNow;
                    if(sys->event_pub)
                    {
                        sys->addPubVar(db, val, ptype);  //publishwithtimestamp .. needs to be sent immediately
                    }
                    sys->setDbVar(db->uri, vname, val);
                    items++;
                }
            }
        }
        else
        {
            if(sys->debug)
                FPS_ERROR_PRINT("***************************** bin idx %d No Var Found\n", pair.index);
        }        
    };
    values.ForeachItem(print);
    if(sysdb->cj)
    {
        if(items > 0)
            sysdb->cjloaded++;
    }
    if(sysdb->debug)
        FPS_ERROR_PRINT("<< ******************************Bin:\n" );
}
void TestSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values) {
    // no more magic static
    sysCfg *sys = sysdb;

    //'std::chrono::_V2::system_clock::time_point'
    auto ts  = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = ts - sys->ts_base;
    sys->tNow = diff.count();

    UTCTimestamp Now();

    static int items = 0;
    if(sysdb->debug)
        FPS_ERROR_PRINT(">> ******************************An:  enum [%04x] ->[%s]  fmt [%d] events [%s]\n"
                , (int)info.gv
                , variation_encode((int)info.gv)
                , sys->fmt
                , sys->events?"true":"false"
                );
    //if(sys->debug)
    //    FPS_ERROR_PRINT(">> ******************************An:\n");
    auto print = [info, sys](const Indexed<Analog>& pair) {

        auto now = Now();
        auto etype = (int)info.gv;
        int ptype = 0;
        double dval = pair.value.value;
        int flags = /*AnalogQualitySpec::to_type(*/
                pair.value.flags.value;

        DbVar* db = sys->getDbVarId(Type_Analog, pair.index);
        if(db == NULL)
        {
            if(sys->debug) 
            {
                FPS_ERROR_PRINT ("****************** ERROR Analog Data entry not found for index [%d] \n", pair.index);
            }
        }
        if(db != NULL)
        {
            if(sys->debug) 
            {
                FPS_ERROR_PRINT("***************************** analog event  for idx [%d] name [%s] etype 0x%04x\n"
                                    , pair.index, db->name.c_str(), etype);
            }


            //
            // pair.value.flags.value

                // Group30Var1  Analog Input 32 bit with flags
                // Group30Var2  Analog Input 16 bit with flags
                // Group30Var3  Analog Input 32 bit without flags
                // Group30Var4  Analog Input 16 bit without flags
                // Group30Var5  Analog Input single precision with flags (Float) << these allow floating point numbers
                // Group30Var6  Analog Input double precision with flags (Double)
            if(
                (etype != 0x1e03 /*Group30Var3*/)
                && (etype != 0x1e04 /*Group30Var4*/)
            )
            {
                db->flags = flags;
                AnalogQuality q = AnalogQuality::RESTART;
                try 
                {
                    db->sflags = "";
                    bool add_comma = false;
                    for (int ix = 8 ;ix >=0  ; ix--)
                    {
                        if(pair.value.flags.value & (1<<ix))
                        {
                            if(add_comma)
                                db->sflags += ",";
                            q = AnalogQualitySpec::from_type(pair.value.flags.value&(1<<ix));
                            //q = AnalogQuality::ONLINE;
                            db->sflags += AnalogQualitySpec::to_human_string(q);
                            add_comma = true;
                        }
                    }
                }
                catch (...)
                {
                    FPS_ERROR_PRINT ("****************** ERROR Bad Quality Flag [%d] forcing ONLINE\n", flags);
                    q = AnalogQuality::ONLINE;
                }
                //auto q = AnalogQualitySpec::from_type(pair.value.flags.value & 0x7f);
                //db->sflags = AnalogQualitySpec::to_human_string(q);
            }
        }
        if (db != NULL) 
        {
            if ((db->scale > 0.0) || (db->scale < 0.0))
            {
                dval /= db->scale;
            }
            const char* vname = db->name.c_str();// static_sysdb->getBinary(pair.index);
            if(etype == 0x2003 ) // Group32Var3)    //0x2003)
            {
                db->etime = ToUTCString(pair.value.time);
            }
            else if(etype == 0x2001 ) //Group32Var1)  // 0x2001)
            {
                // this event did not include time !!! so we use local time
                db->ltime = ToUTCString(DNPTime(now.msSinceEpoch));
            }
            else
            {
                db->etime = ToUTCString(DNPTime(now.msSinceEpoch));
            }
            db->stime = ToUTCString(DNPTime(now.msSinceEpoch));
            //db->stime = ToUTCString(pair.value.time);
            if(sys->debug)
                FPS_ERROR_PRINT("***************************** analog pair idx %d var idx %d flags %d q [%s] name [%s] uri [%s] value [%f] stime [%s] info [%04x] db_events [%s]\n"
                                            , pair.index, db->idx, flags, db->sflags.c_str(), db->name.c_str(), db->uri, dval, db->stime.c_str()
                                            , (int)info.gv, db->events?"true":"false");
            if(strcmp(vname,"Unknown") != 0) 
            {
                //0x2001
                //0x2003
                ptype = sys->fmt;  // = 0 naked 1 = clothed
                int reject = 0;
                if((etype != 0x2003) && (etype != 0x2001))
                {
                    if (db->format)
                    {
                        ptype = db->fmt; // allow override 
                    }
                    //etype = 1;
                }
                else 
                {
                    reject = HandleEvent(sys, db , etype); //etype < 0 for a reject
                }
                if(reject>=0)
                {
                    db->last_seen = sys->tNow;
                    sys->addPubVar(db, dval, ptype);  // TODO we need to capture each event duplicate events will be lost
                    sys->setDbVar(db->uri, vname, dval);
                    items++;
                }
            }
        }
        else
        {
            if(sys->debug>1)
                FPS_ERROR_PRINT("***************************** analog pair idx %d No Var Found\n", pair.index);
        }
    };
    values.ForeachItem(print);
    if(sysdb->cj)
    {
        if(items > 0)
            sysdb->cjloaded++;
    }
    if(sysdb->debug)
        FPS_ERROR_PRINT("<< ******************************An:\n");
    return;
}
DNP3Manager* setupDNP3Manager(sysCfg* ourDB)
{
    auto manager = new DNP3Manager(1, fpsLogger::Create(ourDB));
    return manager;
}


//auto channel = setupDNP3channel(manager, sys_cfg.id, &sys_cfg, sys_cfg.ip_address, sys_cfg.port);
// I think we can have several channels under one manager
std::shared_ptr<IChannel> setupDNP3channel(DNP3Manager* manager, sysCfg* sys)
{

     // Specify what log levels to use. NORMAL is warning and above
    // You can add all the comms logging by uncommenting below
    auto FILTERS = levels::NORMAL;
    if (sys->debug > 2) 
        FILTERS |= levels::ALL_APP_COMMS;
    // This is the main point of interaction with the stack
    // Connect via a TCPClient socket to a outstation
    // repeat for each outstation
    //std::error_code ec;

    std::shared_ptr<IChannel> channel;
    const char* tls_folder_path = 
        "/usr/local/etc/config/dnp3_interface"; // default file path where we put our certificates
    // TODO setup our own PrintingChannelListener
    if (sys->conn_type && strcmp(sys->conn_type, "TLS") == 0) // TLS client type
    {
        if (sys->tls_folder_path) // if they have a custom path to tls certificates then use that
        {
            tls_folder_path = sys->tls_folder_path;
        }
        if (!sys->peerCertificate)
        {
            std::cout << "tls is missing file name for  peerCertificate  please provide it\n";
            return nullptr;
        }
        if (!sys->privateKey)
        {
            std::cout << "tls is missing file name forr  privateKey please provide it\n";
            return nullptr;
        }
        std::string localCertificate = std::string{tls_folder_path} + '/' + std::string{sys->localCertificate};
        std::string peerCertificate  = std::string{tls_folder_path} + '/' + std::string{sys->peerCertificate};
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


        // check for files existences:
        // if (!std::filesystem::exists(peerCertificate))
        // {
        //     std::cout << "peerCertificate: " << peerCertificate << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }
        // if (!std::filesystem::exists(privateKey))
        // {
        //     std::cout << "privateKey: " << privateKey << " doesn't exist. Please provide one\n";
        //     return nullptr;
        // }

        // TLSConfig cfg1(cert2, cert1, key1);
        // TLSConfig cfg2(cert1, cert2, key2);
        
        // Connect via a TCPClient socket to a outstation
        channel = manager->AddTLSClient(sys->id, 
                                        FILTERS, 
                                        ChannelRetry::Default(), 
                                        {IPEndpoint(sys->ip_address, sys->port)}, 
                                        "0.0.0.0",
                                        TLSConfig(peerCertificate, localCertificate, privateKey),
                                        fpsChannelListener::Create(sys) 
                                        //PrintingChannelListener::Create()
                                        );

        if (!channel)
        {
            std::cout << "Unable to create tls client: " <<  std::endl;
            return nullptr;
        }
    }
    else if (sys->conn_type && strcmp(sys->conn_type, "RTU") == 0) // serial/RTU channel
    {
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
        if (sys->asyncOpenDelay != 0) // time delay before opening, defaults to 500mS
        {
            if (sys->asyncOpenDelay < 0)
            {
                std::cout << "asyncOpenDelay: " << sys->asyncOpenDelay << " is negative, must be a postitive value (this is in milliseconds) for serial RTU communications\n";
                return nullptr;
            }
            rtu_settings.asyncOpenDelay = (TimeDuration::Milliseconds(sys->asyncOpenDelay));
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
        //   asyncOpenDelay(opendnp3::TimeDuration::Milliseconds(500))
        if (!checkDeviceName(sys->deviceName))
        {
            std::cout << "Serial Device : " << sys->deviceName << " doesn't exist. Please correct config\n";
            return nullptr;
        }
        channel = manager->AddSerial(sys->id
                                        , FILTERS
                                        , ChannelRetry::Default()
                                        , rtu_settings
                                        //, PrintingChannelListener::Create()
                                        , fpsChannelListener::Create(sys)
                                        );
    }
    else // tcp client type for default strcmp fail (or no conn_type):
    {
    //std::shared_ptr<IChannel> 
        channel = manager->AddTCPClient(sys->id 
                                        , FILTERS 
                                        , ChannelRetry::Default() 
                                        , {IPEndpoint(sys->ip_address, sys->port)}
                                        ,"0.0.0.0" 
                                        , fpsChannelListener::Create(sys)
                                        );
        if (!channel)
        {
            std::cout << "Unable to create TCP client: " <<  std::endl;
            return nullptr;
        }
    // The master config object for a master. The default are
    // useable, but understanding the options are important.
    }
    return channel;
}

//  this my need to be out of the scope of the function.
// TODO add this to sys.
//MasterStackConfig stackConfig;

std::shared_ptr<IMaster> setupDNP3master (std::shared_ptr<IChannel> channel, const char* mname, sysCfg* sys)
{
    //sys->stackConfig = &stackConfig;
    sys->stackConfig = new MasterStackConfig();
    // you can override application layer settings for the master here
    // in this example, we've change the application layer timeout to 2 seconds
    sys->stackConfig->master.responseTimeout = TimeDuration::Milliseconds(sys->respTime);
    if(sys->unsol)
        sys->stackConfig->master.disableUnsolOnStartup = false;
    else
        sys->stackConfig->master.disableUnsolOnStartup = true;
    // You can override the default link layer settings here
    // in this example we've changed the default link layer addressing
    sys->stackConfig->link.LocalAddr = sys->master_address;//localAddr; // 1
    sys->stackConfig->link.RemoteAddr = sys->station_address;//remoteAddr; //10;
    // Create a new master on a previously declared port, with a
    // name, log level, command acceptor, and config info. This
    // returns a thread-safe interface used for sending commands.
    char tmp [1024];
    snprintf(tmp,1024,"master_%s", sys->id);

    ///cpp/lib/include/opendnp3/channel/IChannel.h:    virtual std::shared_ptr<IMaster> AddMaster(const std::string& id,
    auto master = channel->AddMaster(/*"master"*/ tmp, // id for logging
                                     fpsSOEHandler::Create(sys), // callback for data processing  this generates the pub elements when we get data
                                     fpsMasterApplication::Create(sys), // master application instance this manages the collection of all the pub elements 
                                     *sys->stackConfig // stack configuration
                                    );

    if(master == NULL)
    {
        FPS_ERROR_PRINT(" Error creating [%s] client %d server %d\n"
            ,tmp, sys->master_address, sys->station_address);
        return NULL;
    }
    FPS_DEBUG_PRINT(" OK created  [%s]  client %d server %d \n"
            , tmp, sys->master_address, sys->station_address);

    sys->master = master;
    auto test_soe_handler = std::make_shared<TestSOEHandler>(sys);

    // do an integrity poll (Class 3/2/1/0) once per minute
    //auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Minutes(1));
    if (sys->frequency > 0)
    //auto integrityScan = 
        sys->master->AddClassScan(ClassField::AllClasses(), TimeDuration::Milliseconds(sys->frequency), test_soe_handler);
    //void Scan(const HeaderBuilderT& builder, TaskConfig config = TaskConfig::Default());

    //void ScanClasses(const opendnp3::ClassField& field, const opendnp3::TaskConfig& config) override;
    // do a exception poll scans as requested
    if (sys->freq1 > 0)
    //auto exceptionScan = 
        sys->master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Milliseconds(sys->freq1), test_soe_handler);
    if (sys->freq2 > 0)
    //auto exceptionScan = 
        sys->master->AddClassScan(ClassField(ClassField::CLASS_2), TimeDuration::Milliseconds(sys->freq2), test_soe_handler);
    if (sys->freq3 > 0)
    //auto exceptionScan = 
        sys->master->AddClassScan(ClassField(ClassField::CLASS_3), TimeDuration::Milliseconds(sys->freq3), test_soe_handler);
    // virtual std::shared_ptr<IMasterScan> AddRangeScan(opendnp3::GroupVariationID gvId,
    //                                                   uint16_t start,
    //                                                   uint16_t stop,
    //                                                   opendnp3::TimeDuration period,
    //                                                   const opendnp3::TaskConfig& config
    //                                                   = opendnp3::TaskConfig::Default())
    //     = 0; (edited) 
    if(sys->rangeFreq > 0)
    {
        //auto rangeScan = 
        if(sys->rangeAStop > 0)
        {
            sys->master->AddRangeScan(GroupVariationID(30,0),
                                    sys->rangeAStart,
                                    sys->rangeAStop,
                                    TimeDuration::Milliseconds(sys->rangeFreq), test_soe_handler);
        }
        //TODO binaries
        if(sys->rangeBStop > 0)
        {
            sys->master->AddRangeScan(GroupVariationID(1,0),
                                    sys->rangeBStart,
                                    sys->rangeBStop,
                                    TimeDuration::Milliseconds(sys->rangeFreq), test_soe_handler);
        }
    }
    //auto exceptionScan = master->Scan(ClassField(ClassField::CLASS_1);
    
    //auto binscan = 
    // 1,2 is group 1 without flags
    //fps Logger ms(1640868183288) <-AL--  master_test_dnp3_client - 001,001 Binary Input - Packed Format, 8-bit start stop [0, 7]
    //sys->master->AddAllObjectsScan(GroupVariationID(1,1), TimeDuration::Seconds(5), test_soe_handler);
    //sys->master->AddAllObjectsScan(GroupVariationID(1,1),  TimeDuration::Milliseconds(sys->frequency), test_soe_handler);
    // 1,2 is group 1 with flags
    //fps Logger ms(1640868183245) <-AL--  master_test_dnp3_client - 001,002 Binary Input - With Flags, 8-bit start stop [0, 7]
    ////sys->master->AddAllObjectsScan(GroupVariationID(1,2),  TimeDuration::Milliseconds(sys->frequency), test_soe_handler);
    //auto objscan = master->AddAllObjectsScan(GroupVariationID(30,1),
    //                                                               TimeDuration::Seconds(5));
    if(0)
        auto objscan2 = sys->master->AddAllObjectsScan(GroupVariationID(32,7),
                                                                   TimeDuration::Seconds(10), test_soe_handler);
    // Enable the master. This will start communications.
    sys->master->Enable();
    bool channelCommsLoggingEnabled = true;
    bool masterCommsLoggingEnabled = true;
    //bool channelCommsLoggingEnabled = false;
    //bool masterCommsLoggingEnabled = false;

    auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
    channel->SetLogFilters(levels);
    levels = masterCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
    sys->master->SetLogFilters(levels);
    // other master controls are :
    //
    return master;
}

// this runs whn we get a "set" on a defined value
// the data is immediately sent to the outstation.
// these are all scaled and signed if needed onto the wire

void addVarToCommands (CommandSet & commands, std::pair<DbVar*,int>dbp)
{
    DbVar* db = dbp.first;
    //int flag = dbp.second;

    switch (db->type) 
    {
        case Type_Crob:
        {
            //FPS_ERROR_PRINT(" Crob   [%d] ", db->crob);
            if(db->crob == 0)
            {
                //FPS_ERROR_PRINT(" Crob   [NUL] ");
                ControlRelayOutputBlock crob(OperationType::NUL);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 1)
            {
                //FPS_ERROR_PRINT(" Crob   [PULSE_ON] ");
                ControlRelayOutputBlock crob(OperationType::PULSE_ON);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 2)
            {
                //FPS_ERROR_PRINT(" Crob   [PULSE_OFF]");
                ControlRelayOutputBlock crob(OperationType::PULSE_OFF);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 3)
            {
                //FPS_ERROR_PRINT(" Crob   [LATCH_ON]");
                ControlRelayOutputBlock crob(OperationType::LATCH_ON);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 4)
            {
                //FPS_ERROR_PRINT(" Crob   [LATCH_OFF]");
                ControlRelayOutputBlock crob(OperationType::LATCH_OFF);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else
            {
                FPS_ERROR_PRINT(" Crob   [Undefined] \n");
                ControlRelayOutputBlock crob(OperationType::Undefined);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            //FPS_ERROR_PRINT("\n");

            break;
        }
        case AnIn16:
        {
            int16_t anInt16  = getInt16Val(db);
            commands.Add<AnalogOutputInt16>({WithIndex(AnalogOutputInt16(anInt16), db->idx)});
            break;
        }
        case AnIn32:
        {
            int32_t anInt32  = getInt32Val(db);
            commands.Add<AnalogOutputInt32>({WithIndex(AnalogOutputInt32(anInt32), db->idx)});
            //commands.Add<AnalogOutputInt32>({WithIndex(AnalogOutputInt32(ival, db->idx)});
            break;
        }
        case AnF32:
        {
            double anF32  = getF32Val(db);
            commands.Add<AnalogOutputFloat32>({WithIndex(AnalogOutputFloat32(anF32), db->idx)});
            break;
        }
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    if (strcmp(DNP3_UTILS_VERSION, getVersion())!=0)
    {
        FPS_ERROR_PRINT("Error with installed DNP3_UTILS_VERSION\n");
        return 1;
    }

    fims* p_fims;
    p_fims = new fims();
    
    running = true;
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    int fims_connect = 0;
    num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_MASTER, p_fims);

    
    const char **subs = NULL;
    bool *bpubs = NULL;
    
     // TODO make this ppen to subs. Perhaps we need a vector
    // components is pulled by uri  repeat for exch config
    int num = getSysUris((sysCfg **)&sys_cfg, DNP3_MASTER, subs, bpubs, num_configs);
    if(num < 0)
    {
        FPS_ERROR_PRINT("Failed to create subs array.\n");
        return 1;
    }

    if (p_fims == NULL)
    {
        FPS_ERROR_PRINT("Failed to allocate connection to FIMS server.\n");
        //rc = 1;
        return 1;//goto cleanup;
    }
    // use the id for fims connect bt also add master designation 
    {
        char tmp[1024];
        snprintf(tmp, sizeof(tmp),"DNP3_M_%s", sys_cfg[0]->id);
        while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
        {
            fims_connect++;
            sleep(1);
        }
    }

    if(fims_connect >= MAX_FIMS_CONNECT)
    {
        FPS_ERROR_PRINT("Failed to establish connection to FIMS server.\n");
        return 1;
    }

    FPS_DEBUG_PRINT("Map configured: Initializing data.\n");
    //if(p_fims->Subscribe((const char**)sub_array, 3, (bool *)publish_only) == false)
    if(p_fims->Subscribe((const char**)subs, num, (bool *)bpubs) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }
    FPS_ERROR_PRINT("Number of subscriptions %d.\n", num);
    for (int is = 0 ; is < num ; is++)
    {
        FPS_ERROR_PRINT(" subscriptions %d is [%s]\n", is, subs[is]);
    }

    free((void *)bpubs);
    free((void *)subs);
// first sysconfig sets up Manager
    auto manager = setupDNP3Manager(sys_cfg[0]);
    if (!manager){
        FPS_ERROR_PRINT("Error in setupDNP3Manager.\n");
        return 1;
    }
    // now we use data from the config file
    //std::shared_ptr<IChannel> 
    //auto channel = setupDNP3channel(manager, "tcpclient1", "127.0.0.1", 20001);
    auto channel = setupDNP3channel(manager, sys_cfg[0]); //, sys_cfg.ip_address, sys_cfg.port);
    if (!channel){
        FPS_ERROR_PRINT("Error in setupDNP3channel.\n");
        delete manager;
        return 1;
    }

    // repeat for each config
    // put returned master into the config context
        //std::shared_ptr<IChannel> 
    //int cfg_num = 0;
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];

        auto master = setupDNP3master (channel, DNP3_MASTER, sys);

        if (!master){
            FPS_ERROR_PRINT("Error in setupDNP3master.\n");
            delete manager;
            //delete channel;
            return 1;
        }
        sys->master = master;

    }
    //we cant do this
    //auto master2 = setupDNP3master (channel2, "master2", ourDB , 2 /*localAddr*/ , 10 /*RemoteAddr*/);

    FPS_DEBUG_PRINT("DNP3 Setup complete: Entering main loop.\n");
    
    // send out intial gets
    // set max ticks
    
    // no need to do this for the master
    //sys_cfg.getUris(DNP3_MASTER);

    // set all values to inval  done at the start
    // start time to complete gets
    // TODO set for all the getURI responses as todo
    // done only get outstation vars 

    while(running && p_fims->Connected())
    {
        // once a second
        fims_message* msg = p_fims->Receive_Timeout(1000000);
        //sysCfg *sys = sys_cfg[0];
        if(msg != NULL)
        {
//            if (sys->debug)
//                FPS_ERROR_PRINT("****** Hey %s got a message uri [%s] \n", __FUNCTION__, msg->uri);
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];
            
                dbs_type dbs; // collect all the parsed vars here
                // We can use a single dbs 
                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_MASTER);
                //if (sys->debug && ((int)dbs.size() > 0))
                //if (((int)dbs.size() > 0))
                //    FPS_ERROR_PRINT("****** Hey %s got a message uri [%s] dbs.size [%d] \n", __FUNCTION__, msg->uri, (int)dbs.size());

                if(dbs.size() > 0)
                {
                    //int format = 0;
                    CommandSet commands;
                    int numCmds = 0;
                    cJSON*cj = NULL;                
                    if((msg->replyto != NULL) && (strcmp(msg->method,"pub") != 0))
                        cj = cJSON_CreateObject();

                    while (!dbs.empty())
                    {
                        std::pair<DbVar*,int>dbp = dbs.back();
                        // only do this on sets or posts
                        if ((strcmp(msg->method,"set") == 0) || (strcmp(msg->method,"post") == 0))
                        {
                            addVarToCommands(commands, dbp);
                            numCmds++;
                        }
                        if(cj) addVarToCj(cj, dbp);
                        dbs.pop_back();
                    }

                    if(cj)
                    {
                        const char* reply = cJSON_PrintUnformatted(cj);
                        cJSON_Delete(cj);
                        cj = NULL;
                        // TODO check that SET is the correct thing to do in MODBUS_CLIENT
                        // probably OK since this is a reply
                        if(msg->replyto)
                            p_fims->Send("set", msg->replyto, NULL, reply);

                        free((void* )reply);
                    }
                    if(numCmds > 0)
                    {
                        // we need the "correct" DirectOperate option
                        // to take a list of commands and process them...
                        // sigh  
                        //CommandCallbackQueue queue;
                        if(sys->debug) FPS_ERROR_PRINT("      *****Running  Direct Operate \n");
                        //cpp/lib/src/master/PrintingCommandResultCallback.cpp:CommandResultCallbackT
                        //void DirectOperate(CommandSet&& commands, const CommandResultCallbackT& callback, const TaskConfig& config);
                        //void MContext::DirectOperate(CommandSet&& commands, const CommandResultCallbackT& callback, const TaskConfig& config)
                        //cpp/examples/master-gprs/main.cpp:            session->DirectOperate(ControlRelayOutputBlock(OperationType::LATCH_ON), index,

                        // this is for a single type 
                        //master->SelectAndOperate(crob, 0, PrintingCommandResultCallback::Get());

                        // This is for a single command how do  do it for a list  
                        //ControlRelayOutputBlock crob(OperationType::LATCH_ON);
                        //master->SelectAndOperate(crob, 0, PrintingCommandResultCallback::Get());
                        //break;

                        
                        // /// --- command handlers ----

                        // void MContext::DirectOperate(CommandSet&& commands, const CommandResultCallbackT& callback, const TaskConfig& config)
                        // {
                        //     const auto timeout = Timestamp(this->executor->get_time()) + params.taskStartTimeout;
                        //     this->ScheduleAdhocTask(CommandTask::CreateDirectOperate(this->tasks.context, std::move(commands),
                        //                                                              this->params.controlQualifierMode, *application, callback,
                        //                                                              timeout, config, logger));
                        // }
                        //Do we need the MContext not the IMaster here !!!
                        // nope here it is running for a MasterStack

                        // void MasterStack::DirectOperate(CommandSet&& commands, const CommandResultCallbackT& callback, const TaskConfig& config)
                        // {
                        //     /// this is to work around the fact that c++11 doesn't have generic move capture
                        //     auto set = std::make_shared<CommandSet>(std::move(commands));

                        //     auto action = [self = this->shared_from_this(), set, config, callback]() {
                        //         self->mcontext.DirectOperate(std::move(*set), callback, config);
                        //     };

                        //     this->executor->post(action);
                        // }



                        //sys->master->DirectOperate(std::move(commands), PrintingCommandResultCallback::Get(), *sys->stackConfig);//fpsCommandCallback::Get());
                        //sys->master->DirectOperate(std::move(commands), PrintingCommandResultCallback::Get(), TaskConfig::Default());//fpsCommandCallback::Get());
                        sys->master->DirectOperate(std::move(commands), fpsCommandCallback::Get(sys), TaskConfig::Default());//fpsCommandCallback::Get());
                    }
                }
            
                // TODO master has to be assigned per config
                if (sys->scanreq > 0)
                {
                    switch (sys->scanreq)
                    {
                        case 1:
                        {
                            sys->master->ScanClasses(ClassField(ClassField::CLASS_1), PrintingSOEHandler::Create());
                            break;
                        }
                        case 2:
                        {
                            sys->master->ScanClasses(ClassField(ClassField::CLASS_2), PrintingSOEHandler::Create());
                            break;
                        }
                        case 3:
                        {
                            sys->master->ScanClasses(ClassField(ClassField::CLASS_3), PrintingSOEHandler::Create());
                            break;
                        }
                        default:
                            break;
                    }
                    FPS_ERROR_PRINT("****** master scanreq %d serviced\n", sys->scanreq);
                    sys->scanreq = 0;
                }

                if (cjb != NULL)
                {
                    cJSON_Delete(cjb);
                }
            }
            p_fims->free_message(msg);
        }
    }

    //cleanup:
    if (manager){
        delete manager;
        //delete channel;
    }
    if (p_fims) delete p_fims;
    // sys_cfg should clean itself up
 
    //if(sys_cfg.eth_dev       != NULL) free(sys_cfg.eth_dev);
    //if(sys_cfg.ip_address    != NULL) free(sys_cfg.ip_address);
    //if(sys_cfg.name          != NULL) free(sys_cfg.name);
    //if(sys_cfg.serial_device != NULL) free(sys_cfg.serial_device);
    //if(sys_cfg.mb != NULL)             modbus_free(sys_cfg.mb);
    // for(int fd = 0; fd < fd_max; fd++)
    //     if(FD_ISSET(fd, &all_connections))
    //         close(fd);
    for (int ix = 0; ix < num_configs; ix++)
    {
        FPS_ERROR_PRINT("****** client cleanup sys %d start\n", ix);

        sysCfg *sys = sys_cfg[ix];
        //if (sys->master)  delete sys->master;
        if (sys->stackConfig)  delete sys->stackConfig;
        delete sys;
        FPS_ERROR_PRINT("****** client cleanup sys %d done\n", ix);

    }

    return 0;
}


