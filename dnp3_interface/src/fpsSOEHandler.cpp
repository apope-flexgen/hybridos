/*
 * author: pwilshire
 *     11 May, 2020
 * This code provides the details for a pub from the master of the componets returned in an outstation scan.
 * This pub is returned to the customer facing Fleet Manager Outstation
 * NOTE only analogs and binaries are handled at this stage.
 * 
 */
#include <cjson/cJSON.h>
#include <sstream>
#include <iomanip>

#include "fpsSOEHandler.h"
#include "dnp3_utils.h"

using namespace std; 
using namespace opendnp3; 

string ToUTCString(const DNPTime& dnptime)
{
    auto seconds = static_cast<time_t>(dnptime.value / 1000);
    auto milliseconds = static_cast<uint16_t>(dnptime.value % 1000);

#ifdef WIN32
    tm t;
    if (gmtime_s(&t, &seconds) != 0)
    {
        return "BAD TIME";
    }
#else
    tm t;
    if (!gmtime_r(&seconds, &t))
    {
        return "BAD TIME";
    }
#endif

    std::ostringstream oss;
    oss << (1900 + t.tm_year);
    oss << "-" << std::setfill('0') << std::setw(2) << (1 + t.tm_mon);
    oss << "-" << std::setfill('0') << std::setw(2) << t.tm_mday;
    oss << " " << std::setfill('0') << std::setw(2) << t.tm_hour;
    oss << ":" << std::setfill('0') << std::setw(2) << t.tm_min;
    oss << ":" << std::setfill('0') << std::setw(2) << t.tm_sec;
    oss << "." << std::setfill('0') << std::setw(3) << milliseconds;
    return oss.str();
}
//namespace asiodnp3 { 

void fpsSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) {
    //static sysCfg *static_sysdb = sysdb;
    sysCfg * sys =  sysdb; 
    static int items = 0;
    if(sysdb->debug)
        FPS_DEBUG_PRINT(">> ******************************Bin:\n");
    auto print = [sys](const Indexed<Binary>& pair) {
        //DbVar* db = static_sysdb->getDbVarId(Type_Binary, pair.index);
        DbVar* db = sys->getDbVarId(Type_Binary, pair.index);
        if (db != NULL) 
        {
            const char* vname = db->name.c_str();// static_sysdb->getBinary(pair.index);
            if(sys->debug)
                FPS_DEBUG_PRINT("***************************** bin idx %d name [%s] value [%d]\n"
                                     , pair.index, db->name.c_str(), pair.value.value);

            if(strcmp(vname,"Unknown")!= 0) 
            {
                bool val = true;
                if (pair.value.value == 0)
                    val = false;
                if (db->scale < 0.0)
                {
                    val = (val != true);
                }

                sys->addPubVar(db, val);
                sys->setDbVar(db->uri, vname, val);
                items++;
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
        FPS_DEBUG_PRINT("<< ******************************Bin:\n" );
}

void fpsSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<DoubleBitBinary>>& values) {
    if(sysdb->debug)
        FPS_DEBUG_PRINT("******************************DBin: \n");
    return PrintAll(info, values);
}

void fpsSOEHandler::Process(const HeaderInfo& /*info*/, const ICollection<Indexed<BinaryCommandEvent>>& values) {
    auto print = [](const Indexed<BinaryCommandEvent>& pair) {
        std::cout << "BinaryCommandEvent: "
                  << "[" << pair.index << "] : " <<  ToUTCString(pair.value.time) << " : " << pair.value.value << " : "
                  << CommandStatusSpec::to_string(pair.value.status) << std::endl;
    };
    values.ForeachItem(print);
}
// these are all scaled off the wire
void fpsSOEHandler::Process(const HeaderInfo & /* info*/, const ICollection<Indexed<Analog>>& values) {
    // no more magic static
    sysCfg *sys = sysdb;
    static int items = 0;
    if(sys->debug)
        FPS_DEBUG_PRINT(">> ******************************An:\n");
    auto print = [sys](const Indexed<Analog>& pair) {
        DbVar* db = sys->getDbVarId(Type_Analog, pair.index);
        double dval = pair.value.value;

        if (db != NULL) 
        {
            if ((db->scale > 0.0) || (db->scale < 0.0))
            {
                dval /= db->scale;
            }
            const char* vname = db->name.c_str();// static_sysdb->getBinary(pair.index);
            if(sys->debug)
                FPS_ERROR_PRINT("***************************** analog pair idx %d var idx %d name [%s] uri [%s] value [%f]\n"
                                            , pair.index, db->idx, db->name.c_str(), db->uri, dval);
            if(strcmp(vname,"Unknown") != 0) 
            {
                sys->addPubVar(db, dval);
                sys->setDbVar(db->uri, vname, dval);
                items++;
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
        FPS_DEBUG_PRINT("<< ******************************An:\n");
    return;
}
void fpsSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<Counter>>& values) {
    FPS_DEBUG_PRINT("******************************Cnt: \n");
    return PrintAll(info, values);
}
void fpsSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<FrozenCounter>>& values) {
    FPS_DEBUG_PRINT("******************************CntFz: \n");
    return PrintAll(info, values);
}
void fpsSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) {
    // FPS_DEBUG_PRINT("******************************BiOpSta: \n");
    // return PrintAll(info, values);
}
void fpsSOEHandler::Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) {
    // FPS_DEBUG_PRINT("******************************AnOpSta: \n"); 
    // std::cout << "AnalogOutputStatus: ";
    // auto print = [](const Indexed<AnalogOutputStatus>& pair) {
    //         std::cout << "index [" << pair.index << "] : value ["  << pair.value.value << " ] "
    //                 //<< CommandStatusToString(pair.value.status) 
    //                 << std::endl;
    // };
    // values.ForeachItem(print);
}

//     return PrintAll(info, values);
// }
void fpsSOEHandler::Process(const HeaderInfo& /*info*/, const ICollection<Indexed<OctetString>>& values) {
    auto print = [](const Indexed<OctetString>& pair) {
        std::cout << "OctetString "
                  << " [" << pair.index << "] : Size : " /*<< pair.value.ToSlice().Size()*/ << std::endl;
    };
    values.ForeachItem(print);
}
void fpsSOEHandler::Process(const HeaderInfo& /*info*/, const ICollection<Indexed<TimeAndInterval>>& values) {
    auto print = [](const Indexed<TimeAndInterval>& pair) {
        std::cout << "TimeAndInterval: "
                  << "[" << pair.index << "] : " << ToUTCString(pair.value.time) << " : " << pair.value.interval << " : "
                  << IntervalUnitsSpec::to_string(pair.value.GetUnitsEnum()) << std::endl;
    };
    values.ForeachItem(print);
}

void fpsSOEHandler::Process(const HeaderInfo& /*info*/, const ICollection<Indexed<AnalogCommandEvent>>& values) {
    auto print = [](const Indexed<AnalogCommandEvent>& pair) {
        std::cout << "AnalogCommandEvent: "
                  << "[" << pair.index << "] : " << ToUTCString(pair.value.time) << " : " << pair.value.value << " : "
                  << CommandStatusSpec::to_string(pair.value.status) << std::endl;
    };
    values.ForeachItem(print);
}
// void fpsSOEHandler::Process(const HeaderInfo& /*info*/, const ICollection<Indexed<SecurityStat>>& values) {
//     auto print = [](const Indexed<SecurityStat>& pair) {
//         std::cout << "SecurityStat: "
//                   << "[" << pair.index << "] : " << ToUTCString(pair.value.time) << " : " << pair.value.value.count << " : "
//                   << static_cast<int>(pair.value.quality) << " : " << pair.value.value.assocId << std::endl;
//     };
//     values.ForeachItem(print);
// }

void fpsSOEHandler::Process(const opendnp3::HeaderInfo& /*info*/,
                                 const opendnp3::ICollection<opendnp3::DNPTime>& values) {
    auto print = [](const DNPTime& value) { std::cout << "DNPTime: " << ToUTCString(value) << std::endl; };
    values.ForeachItem(print);
}
//} // namespace asiodnp3
