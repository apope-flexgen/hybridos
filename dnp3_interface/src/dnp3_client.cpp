/* 
* this is the outline of a master  dnp3 controller
* the system can communicate with a number outstations
* The confg file will determine the address info on each outstation.
* each outsation will have a number of data points.
* a feature of dnp3 is the need to povide a shadow  stack to hold the user defined data points
* iniitally a copy of the modbus stuff
* note that modbus_client only responds to gets and sets on the base_url
* it pubs the result of the query_registers
* base_uri is set by /components + sys_cfg.name
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
#include <opendnp3/logging/LogLevels.h> 
#include <opendnp3/ConsoleLogger.h> 

#include <opendnp3/channel/PrintingChannelListener.h> 
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
using namespace opendnp3; 

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
    auto ts  = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = ts - sys->ts_base;
    sys->setLock.lock();
    sys->tNow = diff.count();
    sys->tLastMsg = sys->tNow;
    sys->setLock.unlock();
    UTCTimestamp Now;

    if(sys->debug)
        FPS_ERROR_PRINT(">> ******************************TimeAndInterval:  enum [%04x] \n", (int)info.gv);

}

void TestSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) {
    //static sysCfg *static_sysdb = sysdb;
    sysCfg * sys =  sysdb; 
    auto ts  = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = ts - sys->ts_base;
    sys->setLock.lock();
    sys->tNow = diff.count();
    sys->tLastMsg = sys->tNow;
    sys->setLock.unlock();
    UTCTimestamp Now;

    static int items = 0;
    if(sysdb->debug)
        FPS_ERROR_PRINT(">> ******************************Bin:  enum [%04x] ->[%s] fmt [%d] events [%s]  tnow [%2.3f]\n"
                    , (int)info.gv, variation_encode((int)info.gv), sys->fmt,sys->events?"true":"false", sys->tNow);
    auto print = [info, sys](const Indexed<Binary>& pair) {
        auto now = std::chrono::system_clock::now();
        // Convert time point to duration since epoch, then cast to milliseconds
        auto msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();


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
                            db->sflags += BinaryQualitySpec::to_human_string(q);
                            add_comma = true;
                        }
                    }
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
                db->ltime = ToUTCString(DNPTime(msSinceEpoch));
            }
            else
            {
                db->etime = ToUTCString(DNPTime(msSinceEpoch));
            }
            db->stime = ToUTCString(DNPTime(msSinceEpoch));
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
                ptype = sys->fmt;  // naked = 0  clothed =1 full = 2
                int reject = 0;
                auto fmt = sys->fmt;
                if (db->format)
                {
                    fmt = db->fmt;
                }

                if((etype != 0x0203) && (etype != 0x0201))
                {
                    //ptype = sys->fmt;  // = 0 naked 1 = clothed 2 = full
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
                    sys->setDbVar(db->uri, vname, val);
                    if(sys->event_pub)
                    {
                        sys->addPubVar(db, val, ptype, fmt);  //publishwithtimestamp .. needs to be sent immediately
                    }
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
    sysCfg *sys = sysdb;

    auto ts  = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = ts - sys->ts_base;
    sys->setLock.lock();
    sys->tNow = diff.count();
    sys->tLastMsg = sys->tNow;
    sys->setLock.unlock();
    
    UTCTimestamp Now;
 
    static int items = 0;
    if(sysdb->debug)
        FPS_ERROR_PRINT(">> ******************************An:  enum [%04x] ->[%s]  fmt [%d] events [%s]\n"
                , (int)info.gv
                , variation_encode((int)info.gv)
                , sys->fmt
                , sys->events?"true":"false"
                );
    auto print = [info, sys](const Indexed<Analog>& pair) {

        auto now = std::chrono::system_clock::now();
        // Convert time point to duration since epoch, then cast to milliseconds
        auto msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();


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
                db->ltime = ToUTCString(DNPTime(msSinceEpoch));
            }
            else
            {
                db->etime = ToUTCString(DNPTime(msSinceEpoch));
            }
            db->stime = ToUTCString(DNPTime(msSinceEpoch));
            //db->stime = ToUTCString(pair.value.time);
            if(sys->debug)
                FPS_ERROR_PRINT("***************************** analog pair idx %d var idx %d flags %d q [%s] name [%s] uri [%s] value [%f] stime [%s] info [%04x] db_events [%s]\n"
                                            , pair.index, db->idx, flags, db->sflags.c_str(), db->name.c_str(), db->uri, dval, db->stime.c_str()
                                            , (int)info.gv, db->events?"true":"false");
            if(strcmp(vname,"Unknown") != 0) 
            {
                //0x2001
                //0x2003
                auto fmt = sys->fmt;
                if (db->format)
                {
                    fmt = db->fmt;
                }
                ptype = sys->fmt;  // = 0 naked 1 = clothed 2 = full
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
                    sys->setDbVar(db->uri, vname, dval);
                    sys->addPubVar(db, dval, ptype, fmt);  
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


std::shared_ptr<IChannel> setupDNP3channel(DNP3Manager* manager, sysCfg* sys)
{

    auto FILTERS = levels::NORMAL;
    if (sys->debug > 2) 
        FILTERS |= levels::ALL_APP_COMMS;

    std::shared_ptr<IChannel> channel;
    const char* tls_folder_path = 
        "/usr/local/etc/config/dnp3_interface"; // default file path where we put our certificates
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
    }
    return channel;
}


std::shared_ptr<IMaster> setupDNP3master (std::shared_ptr<IChannel> channel, const char* mname, sysCfg* sys)
{
    sys->stackConfig = new MasterStackConfig();
    sys->stackConfig->master.responseTimeout = TimeDuration::Milliseconds(sys->respTime);
    if(sys->unsol)
        sys->stackConfig->master.disableUnsolOnStartup = false;
    else
        sys->stackConfig->master.disableUnsolOnStartup = true;
    sys->stackConfig->link.LocalAddr = sys->master_address;//localAddr; // 1
    sys->stackConfig->link.RemoteAddr = sys->station_address;//remoteAddr; //10;
    char tmp [1024];
    snprintf(tmp,1024,"client_%s", sys->id);

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

    if (sys->frequency > 0)
        sys->master->AddClassScan(ClassField::AllClasses(), TimeDuration::Milliseconds(sys->frequency), test_soe_handler);

    if (sys->freq1 > 0)
        sys->master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Milliseconds(sys->freq1), test_soe_handler);
    if (sys->freq2 > 0)
        sys->master->AddClassScan(ClassField(ClassField::CLASS_2), TimeDuration::Milliseconds(sys->freq2), test_soe_handler);
    if (sys->freq3 > 0)
        sys->master->AddClassScan(ClassField(ClassField::CLASS_3), TimeDuration::Milliseconds(sys->freq3), test_soe_handler);
    if(sys->rangeFreq > 0)
    {
        if(sys->rangeAStop > 0)
        {
            sys->master->AddRangeScan(GroupVariationID(30,0),
                                    sys->rangeAStart,
                                    sys->rangeAStop,
                                    TimeDuration::Milliseconds(sys->rangeFreq), test_soe_handler);
        }
        if(sys->rangeBStop > 0)
        {
            sys->master->AddRangeScan(GroupVariationID(1,0),
                                    sys->rangeBStart,
                                    sys->rangeBStop,
                                    TimeDuration::Milliseconds(sys->rangeFreq), test_soe_handler);
        }
    }
    sys->master->Enable();
    bool channelCommsLoggingEnabled = true;
    bool masterCommsLoggingEnabled = true;

    auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
    channel->SetLogFilters(levels);
    levels = masterCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
    sys->master->SetLogFilters(levels);
    return master;
}

// this runs whn we get a "set" on a defined value
// the data is immediately sent to the outstation.
// these are all scaled and signed if needed onto the wire
void addVarToCommands (CommandSet & commands, std::pair<DbVar*,int>dbp)
{
    DbVar* db = dbp.first;
    switch (db->type) 
    {
        case Type_Crob:
        {
            if(db->crob == 0)
            {
                ControlRelayOutputBlock crob(OperationType::NUL);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 1)
            {
                ControlRelayOutputBlock crob(OperationType::PULSE_ON);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 2)
            {
                ControlRelayOutputBlock crob(OperationType::PULSE_OFF);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 3)
            {
                ControlRelayOutputBlock crob(OperationType::LATCH_ON);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else if(db->crob == 4)
            {
                ControlRelayOutputBlock crob(OperationType::LATCH_OFF);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
            else
            {
                ControlRelayOutputBlock crob(OperationType::Undefined);
                commands.Add<ControlRelayOutputBlock>({WithIndex(crob, db->idx)});
            }
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
    auto ts_base = std::chrono::system_clock::now();
    auto ts3  = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = ts3 - ts_base;
    double tNow = diff.count();
    double master_timeout = 1000000;
    double next_check_time = tNow + master_timeout/1000000.0;

    double fims_sleep_time = 1.0;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    int fims_connect = 0;
    num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_MASTER, p_fims);
    if (num_configs < 0){
        return 1;
    }
    
    const char **subs = NULL;
    bool *bpubs = NULL;
    
    // components is pulled by uri  repeat for each config
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
    auto manager = setupDNP3Manager(sys_cfg[0]);
    if (!manager){
        FPS_ERROR_PRINT("Error in setupDNP3Manager.\n");
        return 1;
    }
    auto channel = setupDNP3channel(manager, sys_cfg[0]); //, sys_cfg.ip_address, sys_cfg.port);
    if (!channel){
        FPS_ERROR_PRINT("Error in setupDNP3channel.\n");
        delete manager;
        return 1;
    }

    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];

        auto master = setupDNP3master (channel, DNP3_MASTER, sys);

        if (!master){
            FPS_ERROR_PRINT("Error in setupDNP3master.\n");
            delete manager;
            return 1;
        }
        sys->master = master;

    }

    FPS_DEBUG_PRINT("DNP3 Setup complete: Entering main loop.\n");
    

    while(running && p_fims->Connected())
    {
        fims_message* msg = p_fims->Receive_Timeout((int)(fims_sleep_time *1000000));
        if(msg != NULL)
        {
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];
            
                dbs_type dbs; // collect all the parsed vars here

                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_MASTER);
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
                        if(cj) addVarToCj(sys, cj, dbp);
                        dbs.pop_back();
                    }

                    if(cj)
                    {
                        const char* reply = cJSON_PrintUnformatted(cj);
                        cJSON_Delete(cj);
                        cj = NULL;
                        if(msg->replyto)
                            p_fims->Send("set", msg->replyto, NULL, reply);

                        free((void* )reply);
                    }
                    if(numCmds > 0)
                    {
                        if(sys->debug) FPS_ERROR_PRINT("      *****Running  Direct Operate \n");
                        sys->master->DirectOperate(std::move(commands), fpsCommandCallback::Get(sys), TaskConfig::Default());//fpsCommandCallback::Get());
                    }
                }
            
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
        auto ts2  = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = ts2 - ts_base;
        tNow = diff.count();
        sysCfg *sys = sys_cfg[0];

        if(tNow > next_check_time)
        {
            sys->setLock.lock();
            auto tLastMsg = sys->tLastMsg;
            sys->setLock.unlock();

            next_check_time = tNow + sys->frequency/1000.0;
            auto pub_delay = tNow - tLastMsg;
            //if (0)
            if (sys->pub_timeout_detected)
            {
                FPS_ERROR_PRINT("****** pub timeout  detected   delay   [%2.3f]  max_delay [%2.3f]\n", pub_delay, 
                            sys->max_pub_delay* sys->max_pub_droop);

                // add a little hysteresis
                if (pub_delay < sys->max_pub_delay * sys->max_pub_droop)
                {
                    sys->pub_timeout_detected = false;
                    char message[1024];
                    snprintf(message, sizeof(message), "DNP3 Timeout recovered"
                            );
                    emit_event(sys, nullptr, message, 1);
                    sys->batch_sets = sys->batch_sets_in;
                }

            }

            if ((sys->max_pub_delay> 0) && (pub_delay > sys->max_pub_delay) && !sys->pub_timeout_detected)
            {
                sys->pub_timeout_detected = true;
                FPS_ERROR_PRINT("****** pub timeout   tNow [%2.3f]  delay   [%2.3f]  max_delay [%2.3f]\n", tNow, pub_delay, sys->max_pub_delay);
                char message[1024];
                snprintf(message, sizeof(message), "DNP3 Timeout [%2.3f] exceeded  [%2.3f]"
                        ,  pub_delay
                        , sys->max_pub_delay
                        );
                emit_event(sys, nullptr, message, 1);

                if (sys->batch_sets_max > 0)
                {
                    sys->batch_sets = sys->batch_sets_max;
                }
            }
            if (sys->debug > 0)
            {
                FPS_ERROR_PRINT("****** master time to check hb tNow [%2.3f]  period [%2.3f] \n", tNow, next_check_time);
                if (tNow > tLastMsg+10.0)
                    FPS_ERROR_PRINT("****** comms timeout last  [%2.3f]  tNow [%2.3f] \n", tLastMsg+10.0, tNow);
            }
        }
        
        double sleep_time = 1.0;

        if(sys->batch_sets > 0 && tNow > sys->next_batch_time)
        {
            sys->next_batch_time = tNow + sys->batch_sets;

            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];
                CommandSet commands;
                int numCmds = 0;

                for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
                {
                    for (auto db : sys->dbVec[i])
                    {
                        if (db->value_set)
                        {
                            addVarToCommands(commands, std::make_pair(db, db->flags));
                            numCmds++;
                            db->value_set = 0; 
                        }
                    }
                }
                if(numCmds > 0)
                {
                    sys->master->DirectOperate(std::move(commands), fpsCommandCallback::Get(sys), TaskConfig::Default());
                    if(sys->debug) 
                        FPS_ERROR_PRINT("      *****Done Running  Direct Operate \n");
                }   
            }
        }

        if (sys->batch_sets > 0) 
        {
            if (sys->next_batch_time > 0)
            {
                if ((sys->next_batch_time - tNow) < sleep_time)
                {
                    sleep_time = sys->next_batch_time - tNow;
                }
            }             
        }
        if (next_check_time > tNow) 
        {
            if ((next_check_time - tNow) < sleep_time)
            {
                sleep_time = next_check_time - tNow;
                //FPS_ERROR_PRINT("      next check sleep  [%2.3f] \n", sleep_time);
            }
        }             
        fims_sleep_time = sleep_time;
    }

    //cleanup:
    if (manager){
        delete manager;
    }
    if (p_fims) delete p_fims;

    for (int ix = 0; ix < num_configs; ix++)
    {
        FPS_ERROR_PRINT("****** client cleanup sys %d start\n", ix);

        sysCfg *sys = sys_cfg[ix];
        if (sys->stackConfig)  delete sys->stackConfig;
        delete sys;
        FPS_ERROR_PRINT("****** client cleanup sys %d done\n", ix);

    }

    return 0;
}
