/*
 * dnp3_utils.cpp
 *
 *  Created on: Oct 2, 2018 - modbus
 *              May 4, 2020 - dp3
 *              Jan 1, 2022 - flah=gs and time
 *      Author: jcalcagni - modbus
 *             pwilshire - dnp3
 */
#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <signal.h>
#include <arpa/inet.h>
#include <climits>
#include <ctime>
#include <iomanip>
#include <string>
#include <fstream>
#include <streambuf>

#include <fims/libfims.h>

#include "gcom_dnp3_utils.h"
#include <map>
#include <string>
#include <cstring>
#include "gcom_dnp3_system_structs.h"
#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_watchdog.h"
#include "gcom_dnp3_heartbeat.h"
#include "gcom_dnp3_flags.h"
#include "shared_utils.hpp"
#include "gcom_dnp3_point_utils.h"

using namespace std;

void toLowercase(char* str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        str[i] = std::tolower(str[i]);
    }
}

bool spam_limit(GcomSystem* sys, int& max_errors)
{
    sys->error_mutex.lock();
    max_errors++;

    if (max_errors > SYS_SPAM_LIMIT)
    {
        sys->error_mutex.unlock();
        return true;
    }
    sys->error_mutex.unlock();
    return false;
}

// these are the types decoded from the config file
// must match the typedef sequence
const char* dreg_types[] = { "AnOPInt16", "AnOPInt32", "AnOPF32",  "CROB",    "analog",
                             "binary",    "analogOS",  "binaryOS", "counter", 0 };
/// @brief
/// @param fp
void deleteFlexPoint(void* fp)
{
    if (fp != nullptr)
    {
        if (((FlexPoint*)fp)->timeout_timer.pCallbackParam != nullptr)
        {
            tmwtimer_cancel(&((FlexPoint*)fp)->timeout_timer);
            if ((PointTimeoutStruct*)((FlexPoint*)fp)->timeout_timer.pCallbackParam != nullptr)
            {
                delete (PointTimeoutStruct*)((FlexPoint*)fp)->timeout_timer.pCallbackParam;
            }
        }
        if (((FlexPoint*)fp)->set_timer.active)
        {
            tmwtimer_cancel(&((FlexPoint*)fp)->set_timer);
        }
        if (((FlexPoint*)fp)->pub_timer.active)
        {
            tmwtimer_cancel(&((FlexPoint*)fp)->pub_timer);
        }
        delete (FlexPoint*)fp;
    }
}
/// @brief
/// @param sys
/// @param iname
/// @param type
/// @param offset
/// @param uri
/// @param variation
/// @return
TMWSIM_POINT* newDbVar(GcomSystem& sys, const char* iname, int type, int offset, char* uri, char* variation)
{
    if (sys.protocol_dependencies->who == DNP3_OUTSTATION)
    {
        TMWSIM_POINT* dbPoint;
        FlexPoint* flexPoint = new FlexPoint(&sys, iname, uri);
        void* dbHandle = ((SDNPSESN*)(sys.protocol_dependencies->dnp3.pSession))->pDbHandle;
        if (type == AnIn16)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var2;
            dbPoint->defaultEventVariation = Group42Var2;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt16;
            flexPoint->is_output_point = true;
        }
        else if (type == AnIn32)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var1;
            dbPoint->defaultEventVariation = Group42Var1;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt32;
            flexPoint->is_output_point = true;
        }
        else if (type == AnF32)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0.0);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPF32;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_AnalogOS)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0.0);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnalogOS;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_Analog)
        {
            while (sdnpsim_anlgInGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addAnalogInput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0, 0.0);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgInGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group30Var5;
            dbPoint->defaultEventVariation = Group32Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart++;
            flexPoint->type = Register_Types::Analog;
        }
        else if (type == Type_BinaryOS)
        {
            while (sdnpsim_binOutGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addBinaryOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0, 15);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_binOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group10Var1;
            dbPoint->defaultEventVariation = CROB;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::BinaryOS;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_Binary)
        {
            while (sdnpsim_binInGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addBinaryInput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, false);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_binInGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group1Var1;
            dbPoint->defaultEventVariation = Group2Var1;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart++;
            flexPoint->type = Register_Types::Binary;
        }
        else if (type == Type_Crob)
        {
            while (sdnpsim_binOutGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addBinaryOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0, 15);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_binOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::CROB;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_Counter)
        {
            // I have no clue why it's called a "binary counter" even though it does uints...
            while (sdnpsim_binaryCounterGetPoint(dbHandle, offset) ==
                   TMWDEFS_NULL)  // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT*)sdnpsim_addBinaryCounter(dbHandle, 0, 0, DNPDEFS_DBAS_FLAG_RESTART, 0);
                dbPoint->enabled = false;  // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT*)sdnpsim_binaryCounterGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group20Var1;
            dbPoint->defaultEventVariation = Group22Var1;
            sys.protocol_dependencies->dnp3.point_status_info->num_counters++;
            sys.protocol_dependencies->dnp3.point_status_info->num_counters_restart++;
            flexPoint->type = Register_Types::Counter;
        }
        else
        {
            delete flexPoint;
            return nullptr;
        }

        dbPoint->flexPointHandle = flexPoint;
        dbPoint->deleteFlexPoint = deleteFlexPoint;
        auto var = static_variation_decode(variation, type);
        if (var > 0)
        {
            dbPoint->defaultStaticVariation = var;
        }
        var = event_variation_decode(variation, type);
        if (var > 0)
        {
            dbPoint->defaultEventVariation = var;
        }
        return dbPoint;
    }
    else
    {
        TMWSIM_POINT* dbPoint = nullptr;
        FlexPoint* flexPoint = new FlexPoint(&sys, iname, uri);
        void* dbHandle = ((MDNPSESN*)(sys.protocol_dependencies->dnp3.pSession))->pDbHandle;
        if (type == AnIn16)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var2;
            dbPoint->defaultEventVariation = Group42Var0;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt16;
            flexPoint->is_output_point = true;
        }
        else if (type == AnIn32)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var1;
            dbPoint->defaultEventVariation = Group42Var0;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt32;
            flexPoint->is_output_point = true;
        }
        else if (type == AnF32)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPF32;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_AnalogOS)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnalogOS;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_Analog)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_analogInputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Input %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group30Var5;
            dbPoint->defaultEventVariation = Group32Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart++;
            flexPoint->type = Register_Types::Analog;
        }
        else if (type == Type_BinaryOS)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Binary Output %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::BinaryOS;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_Binary)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_binaryInputAddPoint(dbHandle, offset);
            }
            dbPoint->defaultStaticVariation = Group1Var1;
            dbPoint->defaultEventVariation = Group2Var0;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart++;
            flexPoint->type = Register_Types::Binary;
        }
        else if (type == Type_Crob)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_binaryOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for CROB %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::CROB;
            flexPoint->is_output_point = true;
        }
        else if (type == Type_Counter)
        {
            dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT*)mdnpsim_binaryCounterAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Counter %d", offset);
                delete (FlexPoint*)(dbPoint->flexPointHandle);
            }
            sys.protocol_dependencies->dnp3.point_status_info->num_counters++;
            sys.protocol_dependencies->dnp3.point_status_info->num_counters_restart++;
            flexPoint->type = Register_Types::Counter;
            dbPoint->defaultStaticVariation = Group20Var1;
            dbPoint->defaultEventVariation = Group22Var1;
        }
        else
        {
            delete flexPoint;
            return nullptr;
        }
        dbPoint->flexPointHandle = flexPoint;
        dbPoint->deleteFlexPoint = deleteFlexPoint;
        auto var = static_variation_decode(variation, type);
        if (var > 0)
        {
            dbPoint->defaultStaticVariation = var;
        }
        var = event_variation_decode(variation, type);
        if (var > 0)
        {
            dbPoint->defaultEventVariation = var;
        }

        return dbPoint;
    }
};

/// @brief given a uri and a dbPoint name recover the FlexPoint if we have one
/// @param sys
/// @param uri
/// @param name
/// @return
TMWSIM_POINT* getDbVar(GcomSystem& sys, std::string_view uri, std::string_view name)
{
    std::string suri = { uri.begin(), uri.end() };
    std::map<std::string, varList*>::iterator it = sys.dburiMap.find(suri);
    if (it != sys.dburiMap.end())
    {
        // dbvar_map
        auto dvar = it->second;
        auto dbm = dvar->dbmap;

        if (dvar->multi && name.size() > 0)
        {
            std::map<std::string, TMWSIM_POINT*>::iterator itd = dbm.find({ name.begin(), name.end() });

            if (itd != dbm.end())
            {
                return itd->second;
            }
        }
        else
        {
            auto point = dbm.begin();
            return point->second;
        }
    }
    it = sys.outputStatusUriMap.find(suri);
    if (it != sys.outputStatusUriMap.end())
    {
        // dbvar_map
        auto dvar = it->second;
        auto dbm = dvar->dbmap;

        if (dvar->multi && name.size() > 0)
        {
            std::map<std::string, TMWSIM_POINT*>::iterator itd = dbm.find({ name.begin(), name.end() });

            if (itd != dbm.end())
            {
                return itd->second;
            }
        }
        else
        {
            auto point = dbm.begin();
            return point->second;
        }
    }
    return NULL;
};

/// @brief
/// @param sys
void showNewUris(GcomSystem& sys)
{
    FPS_INFO_LOG(" %s uris===> \n\n", __FUNCTION__);

    std::map<std::string, varList*>::iterator it;
    std::map<std::string, TMWSIM_POINT*>::iterator itd;
    for (it = sys.dburiMap.begin(); it != sys.dburiMap.end(); ++it)
    {
        FPS_INFO_LOG(" .. uri [%s] num vars %d\n", it->first.c_str(), static_cast<int32_t>(it->second->dbmap.size()));

        // dbvar_map
        auto dvar = it->second;
        auto dbm = dvar->dbmap;
        for (itd = dbm.begin(); itd != dbm.end(); ++itd)
        {
            FPS_INFO_LOG(" ..                            [%s] \n", itd->first.c_str());
        }
    }
    FPS_INFO_LOG(" %s<=== New uris \n\n", __FUNCTION__);
}

/// @brief get the list of subs
/// @param sys
/// @param subs
/// @param num
/// @param who
/// @return
int getSubs(GcomSystem& sys, const char** subs, int num, int who)
{
    if (num < (static_cast<int32_t>(sys.dburiMap.size()) + static_cast<int32_t>(sys.outputStatusUriMap.size())))
    {
        return (sys.dburiMap.size() + static_cast<int32_t>(sys.outputStatusUriMap.size()));
    }
    int idx = 0;
    if (subs)
    {
        std::map<std::string, varList*>::iterator it;
        it = sys.dburiMap.find(sys.base_uri);
        if (it == sys.dburiMap.end())
        {
            subs[idx++] = sys.base_uri;
            sys.fims_dependencies->subs.push_back(sys.base_uri);
        }
        if (sys.local_uri)
        {
            subs[idx++] = sys.local_uri;
            sys.fims_dependencies->subs.push_back(sys.local_uri);
        }
        for (it = sys.dburiMap.begin(); it != sys.dburiMap.end(); ++it)
        {
            subs[idx++] = it->first.c_str();
            if (it->second->multi)
            {
                sys.fims_dependencies->subs.push_back(it->first.c_str());
            }
        }
        for (it = sys.outputStatusUriMap.begin(); it != sys.outputStatusUriMap.end(); ++it)
        {
            subs[idx++] = it->first.c_str();
            if (it->second->multi)
            {
                sys.fims_dependencies->subs.push_back(it->first.c_str());
            }
        }
        if (sys.debug)
        {
            for (int i = 0; i < idx; i++)
            {
                FPS_INFO_LOG("      ===>[%s]<=== \n", subs[i]);
            }
        }
    }
    else
    {
        FPS_ERROR_LOG(" %s No Subs \n", __FUNCTION__);
    }

    return idx;
}
/// @brief
/// @param sys
/// @param uri
/// @param dbPoint
void addDbUri(GcomSystem& sys, const char* uri, TMWSIM_POINT* dbPoint)
{
    if (sys.dburiMap.find(uri) == sys.dburiMap.end())
    {
        sys.dburiMap[uri] = new varList_t(uri);
    }

    varList_t* vl = sys.dburiMap[uri];
    vl->multi = true;
    vl->bit = false;
    vl->addDb(dbPoint);

    {
        std::string full_uri(uri);
        full_uri.append("/").append(((FlexPoint*)(dbPoint->flexPointHandle))->name);
        if (sys.dburiMap.find(full_uri.c_str()) == sys.dburiMap.end())
        {
            sys.dburiMap[full_uri.c_str()] = new varList_t(full_uri.c_str());
        }
        vl = sys.dburiMap[full_uri.c_str()];
        vl->multi = false;
        vl->bit = false;
        vl->addDb(dbPoint);
    }

    if (((FlexPoint*)(dbPoint->flexPointHandle))->is_individual_bits)
    {
        for (auto bit : ((FlexPoint*)(dbPoint->flexPointHandle))->dbBits)
        {
            if (bit.first != "Unknown")
            {
                std::string full_uri(uri);
                full_uri.append("/").append(bit.first);
                if (sys.dburiMap.find(full_uri.c_str()) == sys.dburiMap.end())
                {
                    sys.dburiMap[full_uri.c_str()] = new varList_t(full_uri.c_str());
                }
                vl = sys.dburiMap[full_uri.c_str()];
                vl->multi = false;
                vl->bit = true;
                vl->addDb(dbPoint);
            }
        }
    }
};

void addOutputStatusUri(GcomSystem& sys, const char* uri, TMWSIM_POINT* dbPoint)
{
    if (sys.outputStatusUriMap.find(uri) == sys.outputStatusUriMap.end())
    {
        sys.outputStatusUriMap[uri] = new varList_t(uri);
    }

    varList_t* vl = sys.outputStatusUriMap[uri];
    vl->multi = true;
    vl->bit = false;
    vl->addDb(dbPoint);

    {
        std::string full_uri(uri);
        full_uri.append("/").append(((FlexPoint*)(dbPoint->flexPointHandle))->name);
        if (sys.outputStatusUriMap.find(full_uri.c_str()) == sys.outputStatusUriMap.end())
        {
            sys.outputStatusUriMap[full_uri.c_str()] = new varList_t(full_uri.c_str());
        }
        vl = sys.outputStatusUriMap[full_uri.c_str()];
        vl->multi = false;
        vl->bit = false;
        vl->addDb(dbPoint);
    }

    if (((FlexPoint*)(dbPoint->flexPointHandle))->is_individual_bits)
    {
        for (auto bit : ((FlexPoint*)(dbPoint->flexPointHandle))->dbBits)
        {
            if (bit.first != "Unknown")
            {
                std::string full_uri(uri);
                full_uri.append("/").append(bit.first);
                if (sys.outputStatusUriMap.find(full_uri.c_str()) == sys.outputStatusUriMap.end())
                {
                    sys.outputStatusUriMap[full_uri.c_str()] = new varList_t(full_uri.c_str());
                }
                vl = sys.outputStatusUriMap[full_uri.c_str()];
                vl->multi = false;
                vl->bit = true;
                vl->addDb(dbPoint);
            }
        }
    }
};
/// @brief
/// @param sys
/// @param who
/// @return
char* getDefUri(GcomSystem& sys, int who)
{
    if (sys.defUri == NULL)
    {
        if (sys.base_uri == NULL)
        {
            if (who == DNP3_MASTER)
            {
                asprintf(&(sys.defUri), "/components/%s", sys.id);
            }
            else
            {
                asprintf(&(sys.defUri), "/interfaces/%s", sys.id);
            }
        }
        else
        {
            asprintf(&(sys.defUri), "%s", sys.base_uri);
        }
    }
    return sys.defUri;
}
/// @brief
/// @param argc
/// @param argv
/// @param num
/// @return
cJSON* get_config_json(int argc, char* argv[], int num)
{
    if (argc <= 1)
    {
        FPS_ERROR_LOG("Need to pass argument for config file name.");
        return NULL;
    }
    if (argc <= num + 1)
    {
        return NULL;
    }

    if (argv[num + 1] == NULL)
    {
        FPS_ERROR_LOG(" Failed to get the path of the test file.");
        return NULL;
    }

    return get_config_file(argv[num + 1]);
}

/// @brief
/// @param fname
/// @return
cJSON* get_config_file(char* fname)
{
    char* newFname;
    if (std::strstr(fname, ".json") == nullptr)
    {
        // Calculate the length for the new string
        size_t fnameLen = std::strlen(fname);
        size_t extensionLen = std::strlen(".json");

        // Allocate memory for the new string (fname + ".json" + null terminator)
        newFname = new char[fnameLen + extensionLen + 1];

        // Copy the original fname to the new string
        std::strcpy(newFname, fname);

        // Append ".json" to the new string
        std::strcat(newFname, ".json");
    }
    else
    {
        size_t fnameLen = std::strlen(fname);
        newFname = new char[fnameLen + 1];
        std::strcpy(newFname, fname);
    }

    std::ifstream t(newFname);
    if (!t)
    {
        FPS_ERROR_LOG(" failed to read file  %s .", newFname);
        return nullptr;
    }

    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buff(size, ' ');
    t.seekg(0);
    t.read(&buff[0], size);

    cJSON* config = cJSON_Parse(buff.c_str());
    if (config == NULL)
        FPS_ERROR_LOG("Invalid JSON object in file");

    delete[] newFname;

    return config;
}
/// @brief
/// @param t
/// @return
const char* iotypToStr(int t)
{
    //    if (t < Type_of_Var::NumTypes)
    if (t < static_cast<int32_t>(sizeof(dreg_types)) / static_cast<int32_t>(sizeof(dreg_types[0])))
    {
        return dreg_types[t];
    }
    return "Unknown";
}
/// @brief
/// @param t
/// @return
int iotypToId(const char* t)
{
    int i;
    for (i = 0; i < Type_of_Var::NumTypes; i++)
    {
        if (strcmp(t, dreg_types[i]) == 0)
            return i;
    }
    return -1;
}
/// @brief
/// @param ivar
/// @param type
/// @return
int static_variation_decode(const char* ivar, int type)
{
    if (ivar)
    {
        if (strcmp(ivar, "Group1Var0") == 0)
            return Group1Var0;
        else if (strcmp(ivar, "Group1Var1") == 0)
            return Group1Var1;
        else if (strcmp(ivar, "Group1Var2") == 0)
            return Group1Var2;
        else if (strcmp(ivar, "Group2Var3") == 0)
            return Group2Var3;
        else if (strcmp(ivar, "Group30Var1") == 0)
            return Group30Var1;
        if (strcmp(ivar, "Group10Var0") == 0)
            return Group10Var0;
        else if (strcmp(ivar, "Group10Var1") == 0)
            return Group10Var1;
        else if (strcmp(ivar, "Group10Var2") == 0)
            return Group10Var2;
        else if (strcmp(ivar, "Group2Var3") == 0)
            return Group2Var3;
        else if (strcmp(ivar, "Group20Var1") == 0)
            return Group20Var1;
        else if (strcmp(ivar, "Group20Var2") == 0)
            return Group20Var2;
        else if (strcmp(ivar, "Group20Var3") == 0)
            return Group20Var3;
        else if (strcmp(ivar, "Group20Var4") == 0)
            return Group20Var4;
        else if (strcmp(ivar, "Group20Var5") == 0)
            return Group20Var5;
        else if (strcmp(ivar, "Group20Var6") == 0)
            return Group20Var6;
        else if (strcmp(ivar, "Group20Var7") == 0)
            return Group20Var7;
        else if (strcmp(ivar, "Group20Var8") == 0)
            return Group20Var8;
        else if (strcmp(ivar, "Group30Var1") == 0)
            return Group30Var1;
        else if (strcmp(ivar, "Group30Var2") == 0)
            return Group30Var2;
        else if (strcmp(ivar, "Group30Var3") == 0)
            return Group30Var3;
        else if (strcmp(ivar, "Group30Var4") == 0)
            return Group30Var4;
        else if (strcmp(ivar, "Group30Var5") == 0)
            return Group30Var5;
        else if (strcmp(ivar, "Group30Var6") == 0)
            return Group30Var6;
        else if (strcmp(ivar, "Group40Var1") == 0)
            return Group40Var1;
        else if (strcmp(ivar, "Group40Var2") == 0)
            return Group40Var2;
        else if (strcmp(ivar, "Group40Var3") == 0)
            return Group40Var3;
        else if (strcmp(ivar, "Group40Var4") == 0)
            return Group40Var4;
    }
    return -1;
}
/// @brief
/// @param ivar
/// @param type
/// @return
int event_variation_decode(const char* ivar, int type)
{
    if (ivar)
    {
        if (strcmp(ivar, "Group2Var0") == 0)
            return Group2Var0;
        else if (strcmp(ivar, "Group2Var1") == 0)
            return Group2Var1;
        if (strcmp(ivar, "Group2Var2") == 0)
            return Group2Var2;
        if (strcmp(ivar, "Group11Var1") == 0)
            return Group11Var1;
        if (strcmp(ivar, "Group11Var2") == 0)
            return Group11Var2;
        else if (strcmp(ivar, "Group22Var1") == 0)
            return Group22Var1;
        else if (strcmp(ivar, "Group22Var2") == 0)
            return Group22Var2;
        else if (strcmp(ivar, "Group22Var3") == 0)
            return Group22Var3;
        else if (strcmp(ivar, "Group22Var4") == 0)
            return Group22Var4;
        else if (strcmp(ivar, "Group22Var5") == 0)
            return Group22Var5;
        else if (strcmp(ivar, "Group22Var6") == 0)
            return Group22Var6;
        else if (strcmp(ivar, "Group22Var7") == 0)
            return Group22Var7;
        else if (strcmp(ivar, "Group22Var8") == 0)
            return Group22Var8;
        if (strcmp(ivar, "Group32Var0") == 0)
            return Group32Var0;
        else if (strcmp(ivar, "Group32Var1") == 0)
            return Group32Var1;
        else if (strcmp(ivar, "Group32Var2") == 0)
            return Group32Var2;
        else if (strcmp(ivar, "Group32Var3") == 0)
            return Group32Var3;
        else if (strcmp(ivar, "Group32Var4") == 0)
            return Group32Var4;
        else if (strcmp(ivar, "Group32Var5") == 0)
            return Group32Var5;
        else if (strcmp(ivar, "Group32Var6") == 0)
            return Group32Var6;
        else if (strcmp(ivar, "Group32Var7") == 0)
            return Group32Var7;
        else if (strcmp(ivar, "Group32Var8") == 0)
            return Group32Var8;
        else if (strcmp(ivar, "Group42Var0") == 0)
            return Group42Var0;
        else if (strcmp(ivar, "Group42Var1") == 0)
            return Group42Var1;
        else if (strcmp(ivar, "Group42Var2") == 0)
            return Group42Var2;
        else if (strcmp(ivar, "Group42Var3") == 0)
            return Group42Var3;
        else if (strcmp(ivar, "Group42Var4") == 0)
            return Group42Var4;
        else if (strcmp(ivar, "Group42Var5") == 0)
            return Group42Var5;
        else if (strcmp(ivar, "Group42Var6") == 0)
            return Group42Var6;
        else if (strcmp(ivar, "Group42Var7") == 0)
            return Group42Var7;
        else if (strcmp(ivar, "Group42Var8") == 0)
            return Group42Var8;
    }
    return -1;
}
/// @brief
/// @param cji
/// @return
cJSON* parse_files(cJSON* cji)
{
    return cJSON_GetObjectItem(cji, "files");
}
/// @brief
/// @param cj
/// @param name
/// @param val
/// @param required
/// @return
bool getCJint(cJSON* cj, const char* name, int& val, bool required)
{
    bool ok = !required;
    cJSON* cji = cJSON_GetObjectItem(cj, name);
    if (cji)
    {
        val = cji->valueint;
        ok = true;
    }
    return ok;
}
/// @brief
/// @param cj
/// @param name
/// @param val
/// @param required
/// @return
bool getCJdouble(cJSON* cj, const char* name, double& val, bool required)
{
    bool ok = !required;
    cJSON* cji = cJSON_GetObjectItem(cj, name);
    if (cji)
    {
        val = cji->valuedouble;
        ok = true;
    }
    return ok;
}
/// @brief
/// @param cj
/// @param name
/// @param val
/// @param required
/// @return
bool getCJbool(cJSON* cj, const char* name, bool& val, bool required)
{
    bool ok = !required;
    cJSON* cji = cJSON_GetObjectItem(cj, name);
    if (cji)
    {
        val = cJSON_IsTrue(cji);
        ok = true;
    }
    return ok;
}
/// @brief
/// @param cj
/// @param name
/// @param val
/// @param required
/// @return
bool getCJstr(cJSON* cj, const char* name, char*& val, bool required)
{
    bool ok = !required;
    cJSON* cji = cJSON_GetObjectItem(cj, name);
    if (cji)
    {
        val = strdup(cji->valuestring);
        ok = true;
    }
    return ok;
}
/// @brief
/// @param input
/// @return
char* parseRegisterName(char* input)
{
    char* lastSlash = nullptr;
    char* current = input;

    while (*current != '\0')
    {
        if (*current == '/')
            lastSlash = current;
        current++;
    }

    if (lastSlash != nullptr)
        return lastSlash + 1;
    else
        return nullptr;
}
/// @brief
/// @param input
/// @return
char* parseFullUri(char* input)
{
    char* firstSlash = strchr(input, '/');
    char* lastSlash = strrchr(input, '/');  // finds last occurrence of character

    if (firstSlash != nullptr && lastSlash != nullptr && lastSlash > firstSlash)
    {
        size_t length = lastSlash - firstSlash;
        char* result = new char[length + 1];
        strncpy(result, firstSlash, length + 1);
        result[length] = '\0';
        return result;
    }
    else
        return nullptr;
}
/// @brief
/// @param cji
/// @param sys
/// @param who
/// @return
bool parse_system(cJSON* cji, GcomSystem& sys, int who)
{
    bool ret = true;
    bool final_ret = true;
    cJSON* cj = cJSON_GetObjectItem(cji, "system");

    if (cj == NULL)
    {
        FPS_ERROR_LOG("system  missing from file!");
        ret = false;
    }
    sys.protocol_dependencies->who = who;  // DNP3_MASTER or DNP3_OUTSTATION

    // todo add different frequencies for each zone -- not there by default, but can be configured
    sys.protocol_dependencies->frequency = 1000;  // default once a second
    sys.protocol_dependencies->port = 20000;
    sys.protocol_dependencies->deadband = 0.01;
    sys.protocol_dependencies->respTime = 30000;  // default response time 30 Sec (TMW default)
    sys.protocol_dependencies->maxElapsed = 100;  // default max task elapsed time before we print an error in the log
    sys.fims_dependencies->max_pub_delay = 0;

    sys.protocol_dependencies->baud = 9600;
    sys.protocol_dependencies->dataBits = 8;
    sys.protocol_dependencies->stopBits = 1;

    sys.protocol_dependencies->parity = strdup("None");
    sys.protocol_dependencies->flowType = strdup("None");
    sys.protocol_dependencies->asyncOpenDelay = 500;
    sys.fims_dependencies->data_buf_len = 20000;
    sys.protocol_dependencies->master_address = 1;
    sys.protocol_dependencies->station_address = 10;

    sys.protocol_dependencies->dnp3.point_status_info = new PointStatusInfo();

    char* tmp_uri = NULL;

    if (ret)
    {
        ret = getCJstr(cj, "id", sys.id, true);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'id' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "port", sys.protocol_dependencies->port, false);
        if (!ret || sys.protocol_dependencies->port < 0)
        {
            FPS_ERROR_LOG("Error: 'port' must be an integer greater than 0.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "ip_address", sys.protocol_dependencies->ip_address, true);
        if (ret && strcmp(sys.protocol_dependencies->ip_address, "0.0.0.0") == 0)
        {
            strncpy(sys.protocol_dependencies->ip_address, "*.*.*.*", sizeof("*.*.*.*"));
        }
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'ip_address' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "master_address", sys.protocol_dependencies->master_address, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'master_address' must be an integer.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "station_address", sys.protocol_dependencies->station_address, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'station_address' must be an integer.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "base_uri", tmp_uri, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'base_uri' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "local_uri", sys.local_uri, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'local_uri' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "frequency", sys.protocol_dependencies->frequency, false);
        if (!ret || sys.protocol_dependencies->frequency < 0)
        {
            FPS_ERROR_LOG("Error: 'frequency' must be an integer greater than 0.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJbool(cj, "unsol", sys.protocol_dependencies->dnp3.unsolUpdate, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'unsol' must be a boolean.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "debug", sys.debug, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'debug' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "deadband", sys.protocol_dependencies->deadband, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'deadband' must be a double.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "class_1_freq", sys.protocol_dependencies->dnp3.freq1, false);
        if (sys.protocol_dependencies->dnp3.freq1 == 0)
        {
            ret = getCJint(cj, "freq1", sys.protocol_dependencies->dnp3.freq1, false);
        }
        if (!ret || sys.protocol_dependencies->dnp3.freq1 < 0)
        {
            FPS_ERROR_LOG("Error: 'class_1_freq' must be an integer greater than 0.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "class_2_freq", sys.protocol_dependencies->dnp3.freq2, false);
        if (sys.protocol_dependencies->dnp3.freq2 == 0)
        {
            ret = getCJint(cj, "freq2", sys.protocol_dependencies->dnp3.freq2, false);
        }
        if (!ret || sys.protocol_dependencies->dnp3.freq2 < 0)
        {
            FPS_ERROR_LOG("Error: 'class_2_freq' must be an integer greater than 0.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "class_3_freq", sys.protocol_dependencies->dnp3.freq3, false);
        if (sys.protocol_dependencies->dnp3.freq3 == 0)
        {
            ret = getCJint(cj, "freq3", sys.protocol_dependencies->dnp3.freq3, false);
        }
        if (!ret || sys.protocol_dependencies->dnp3.freq3 < 0)
        {
            FPS_ERROR_LOG("Error: 'class_3_freq' must be an integer greater than 0.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "timeout", sys.protocol_dependencies->timeout, false);
        if (sys.protocol_dependencies->timeout > 0)
        {
            sys.protocol_dependencies->timeout *= 1000;  // convert to milliseconds (e.g. timeout = 5 s ----> 5000 ms)
        }
        if (sys.protocol_dependencies->timeout == 0)
        {
            ret = getCJdouble(cj, "timeoutmS", sys.protocol_dependencies->timeout, false);
        }
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'timeout' must be a double.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "respTime", sys.protocol_dependencies->respTime, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'respTime' must be a double.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        char* tmp_fmt = nullptr;
        ret = getCJstr(cj, "format", tmp_fmt, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'format' must be a string (clothed, full, or naked).");
            final_ret = false;
            ret = true;
        }
        if (tmp_fmt != nullptr)
        {
            if (strcmp(tmp_fmt, "clothed") == 0)
            {
                sys.fims_dependencies->format = FimsFormat::Clothed;
            }
            else if (strcmp(tmp_fmt, "full") == 0)
            {
                sys.fims_dependencies->format = FimsFormat::Full;
            }
            else  // defaults to naked
            {
                sys.fims_dependencies->format = FimsFormat::Naked;
            }
        }
    }

    if (ret)
    {
        ret = getCJbool(cj, "event_pub", sys.protocol_dependencies->dnp3.event_pub, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'event_pub' must be a boolean.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "event_buffer", sys.protocol_dependencies->dnp3.event_buffer, false);
        if (sys.protocol_dependencies->dnp3.event_buffer <= 0)
        {
            FPS_ERROR_LOG("Invalid  event buffer  %d  setting to default 100",
                          sys.protocol_dependencies->dnp3.event_buffer);
            sys.protocol_dependencies->dnp3.event_buffer = 100;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "data_buf_len", sys.fims_dependencies->data_buf_len, false);
        if (!ret || sys.fims_dependencies->data_buf_len < 0)
        {
            FPS_ERROR_LOG("Error: 'data_buf_len' must be an integer greater than 0.\n");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "stats_pub_frequency", sys.protocol_dependencies->dnp3.stats_pub_frequency, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'stats_pub_frequency' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "batch_set_rate", sys.protocol_dependencies->dnp3.batch_set_rate, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'batch_set_rate' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "interval_set_rate", sys.protocol_dependencies->dnp3.interval_set_rate, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'interval_set_rate' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "batch_pub_rate", sys.protocol_dependencies->dnp3.batch_pub_rate, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'batch_pub_rate' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "interval_pub_rate", sys.protocol_dependencies->dnp3.interval_pub_rate, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'interval_pub_rate' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        char* protocol_str = nullptr;
        ret = getCJstr(cj, "protocol", protocol_str, false);
        if (protocol_str != nullptr)
        {
            if (strcmp(protocol_str, "DNP3") == 0 || strcmp(protocol_str, "dnp3") == 0)
            {
                sys.protocol_dependencies->protocol = Protocol::DNP3;
            }
            else
            {
                sys.protocol_dependencies->protocol = Protocol::Modbus;
            }
            free(protocol_str);
        }
    }

    // conn type:
    if (ret)
    {
        char* conn_str = nullptr;
        ret = getCJstr(cj, "conn_type", conn_str, false);
        if (conn_str != nullptr)
        {
            if (strcmp(conn_str, "TCP") == 0)
            {
                sys.protocol_dependencies->conn_type = Conn_Type::TCP;
            }
            if (strcmp(conn_str, "TLS") == 0)
            {
                sys.protocol_dependencies->conn_type = Conn_Type::TCP;
                sys.protocol_dependencies->use_tls = true;
            }
            if (strcmp(conn_str, "RTU") == 0)
            {
                sys.protocol_dependencies->conn_type = Conn_Type::RTU;
            }
        }
        else
        {
            sys.protocol_dependencies->conn_type = Conn_Type::TCP;
        }
    }

    // serial/RTU stuff:
    if (ret)
    {
        ret = getCJint(cj, "baud_rate", sys.protocol_dependencies->baud, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'baud' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "data_bits", sys.protocol_dependencies->dataBits, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'data_bits' must be an integer.");
            final_ret = false;
            ret = true;
        }
        else
        {
            if (sys.protocol_dependencies->dataBits == 7)
            {
                sys.protocol_dependencies->data_bits = TMWTARG232_DATA_BITS_7;
            }
            else if (sys.protocol_dependencies->dataBits == 8)
            {
                sys.protocol_dependencies->data_bits = TMWTARG232_DATA_BITS_8;
            }
            else
            {
                sys.protocol_dependencies->data_bits = TMWTARG232_DATA_BITS_8;
            }
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "stop_bits", sys.protocol_dependencies->stopBits, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'stop_bits' must be a double.");
            final_ret = false;
            ret = true;
        }
        else
        {
            if (sys.protocol_dependencies->stopBits == 1)
            {
                sys.protocol_dependencies->stop_bits = TMWTARG232_STOP_BITS_1;
            }
            else if (sys.protocol_dependencies->stopBits == 8)
            {
                sys.protocol_dependencies->stop_bits = TMWTARG232_STOP_BITS_2;
            }
            else
            {
                sys.protocol_dependencies->stop_bits = TMWTARG232_STOP_BITS_1;
            }
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "parity", sys.protocol_dependencies->parity, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'parity' must be a string.");
            final_ret = false;
            ret = true;
        }
        else
        {
            toLowercase(sys.protocol_dependencies->parity);
            if (strcmp(sys.protocol_dependencies->parity, "none") == 0)
            {
                sys.protocol_dependencies->parity_type = TMWTARG232_PARITY_NONE;
            }
            else if (strcmp(sys.protocol_dependencies->parity, "odd") == 0)
            {
                sys.protocol_dependencies->parity_type = TMWTARG232_PARITY_ODD;
            }
            else if (strcmp(sys.protocol_dependencies->parity, "even") == 0)
            {
                sys.protocol_dependencies->parity_type = TMWTARG232_PARITY_EVEN;
            }
            else
            {
                sys.protocol_dependencies->parity_type = TMWTARG232_PARITY_NONE;
            }
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "flowType", sys.protocol_dependencies->flowType, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'flowType' must be a string.");
            final_ret = false;
            ret = true;
        }
        else
        {
            toLowercase(sys.protocol_dependencies->flowType);
            if (strcmp(sys.protocol_dependencies->flowType, "none") == 0)
            {
                sys.protocol_dependencies->port_mode = TMWTARG232_MODE_NONE;
            }
            else if (strcmp(sys.protocol_dependencies->flowType, "hardware") == 0)
            {
                sys.protocol_dependencies->port_mode = TMWTARG232_MODE_HARDWARE;
            }
            else if ((strcmp(sys.protocol_dependencies->flowType, "software") == 0) ||
                     (strcmp(sys.protocol_dependencies->flowType, "xonxoff") == 0))
            {
                sys.protocol_dependencies->port_mode = TMWTARG232_MODE_WINDOWS;
            }
            else
            {
                sys.protocol_dependencies->port_mode = TMWTARG232_MODE_NONE;
            }
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "asyncOpenDelay", sys.protocol_dependencies->asyncOpenDelay, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'asyncOpenDelay' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "serial_device", sys.protocol_dependencies->deviceName, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'serial_device' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    // heartbeat stuff:
    if (ret)
    {
        sys.heartbeat = new Heartbeat();
        sys.heartbeat->sys = &sys;
        ret = getCJstr(cj, "component_heartbeat_read_uri", sys.heartbeat->heartbeat_read_uri, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'component_heartbeat_read_uri' must be a string.");
            final_ret = false;
            ret = true;
        }
    }
    if (ret)
    {
        if (!sys.heartbeat)
        {
            sys.heartbeat = new Heartbeat();
            sys.heartbeat->sys = &sys;
        }
        ret = getCJstr(cj, "component_heartbeat_write_uri", sys.heartbeat->heartbeat_write_uri, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'component_heartbeat_write_uri' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.heartbeat)
    {
        ret = getCJbool(cj, "heartbeat_enabled", sys.heartbeat->enabled, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'heartbeat_enabled' must be a bool.");
            final_ret = false;
            ret = true;
        }
        if (!sys.heartbeat->enabled)
        {
            delete sys.heartbeat;
            sys.heartbeat = nullptr;
        }
    }

    if (ret && sys.heartbeat)
    {
        ret = getCJint(cj, "component_heartbeat_max_value", sys.heartbeat->max_value_int, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'component_heartbeat_max_value' must be an integer.");
            final_ret = false;
            ret = true;
        }
        else
        {
            sys.heartbeat->max_value = static_cast<uint64_t>(sys.heartbeat->max_value_int);
        }
    }

    if (ret && sys.heartbeat)
    {
        ret = getCJint(cj, "component_heartbeat_timeout_ms", sys.heartbeat->timeout, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'component_heartbeat_timeout_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.heartbeat)
    {
        ret = getCJdouble(cj, "component_heartbeat_frequency_ms", sys.heartbeat->frequency, false);
        sys.heartbeat->frequency /= 1000;
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'component_heartbeat_frequency_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        sys.watchdog = new Watchdog();
        sys.watchdog->sys = &sys;
        ret = getCJstr(cj, "watchdog_uri", sys.watchdog->id, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_uri' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.watchdog)
    {
        ret = getCJbool(cj, "watchdog_enabled", sys.watchdog->enabled, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_enabled' must be a bool.");
            final_ret = false;
            ret = true;
        }
        if (!sys.watchdog->enabled)
        {
            delete sys.watchdog;
            sys.watchdog = nullptr;
        }
    }

    if (ret && sys.watchdog)
    {
        ret = getCJdouble(cj, "watchdog_alarm_timeout_ms", sys.watchdog->alarmTimeout, false);
        sys.watchdog->alarmTimeout /= 1000;
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_alarm_timeout_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.watchdog)
    {
        ret = getCJdouble(cj, "watchdog_fault_timeout_ms", sys.watchdog->faultTimeout, false);
        sys.watchdog->faultTimeout /= 1000;
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_fault_timeout_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.watchdog)
    {
        ret = getCJdouble(cj, "watchdog_recovery_timeout_ms", sys.watchdog->recoveryTimeout, false);
        sys.watchdog->recoveryTimeout /= 1000;
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_recovery_timeout_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.watchdog)
    {
        ret = getCJdouble(cj, "watchdog_recovery_time_ms", sys.watchdog->timeRequiredToRecover, false);
        sys.watchdog->timeRequiredToRecover /= 1000;
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_recovery_time_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret && sys.watchdog)
    {
        ret = getCJint(cj, "watchdog_frequency_ms", sys.watchdog->frequency, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'watchdog_frequency_ms' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJbool(cj, "pub_outputs", sys.protocol_dependencies->dnp3.pub_outputs, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'pub_outputs' must be a boolean.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJbool(cj, "show_output_status", sys.protocol_dependencies->dnp3.show_output_status, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'show_output_status' must be a boolean.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJstr(cj, "output_status_uri", sys.protocol_dependencies->dnp3.output_status_uri, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'output_status_uri' must be a string.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "resend_tolerance", sys.protocol_dependencies->dnp3.resend_tolerance, false);
        if (!ret || sys.protocol_dependencies->dnp3.resend_tolerance < 0)
        {
            FPS_ERROR_LOG("Error: 'resend_tolerance' must be an float greater than 0.");
            sys.protocol_dependencies->dnp3.resend_tolerance = 0;
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "resend_rate", sys.protocol_dependencies->dnp3.resend_rate_ms, false);
        if (!ret || sys.protocol_dependencies->dnp3.resend_rate_ms < 0)
        {
            FPS_ERROR_LOG("Error: 'resend_rate' must be an integer greater than 0.");
            sys.protocol_dependencies->dnp3.resend_rate_ms = 0;
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "dbi_save_frequency_seconds", sys.dbi_save_frequency_seconds, false);
        if (!ret || sys.dbi_save_frequency_seconds < 0)
        {
            FPS_ERROR_LOG("Error: 'dbi_save_frequency_seconds' must be a double greater than 0.");
            sys.dbi_save_frequency_seconds = 0;
            final_ret = false;
            ret = true;
        }
    }

    // fixup base_uri
    char tmp[1024];
    const char* sys_id = sys.id;

    if (sys.base_uri)
        free(sys.base_uri);

    if (tmp_uri)
    {
        snprintf(tmp, sizeof(tmp), "%s/%s", tmp_uri, sys_id);
        free(tmp_uri);
    }
    else
    {
        if (who == DNP3_MASTER)
        {
            snprintf(tmp, sizeof(tmp), "%s/%s", "/components", sys_id);
        }
        else
        {
            snprintf(tmp, sizeof(tmp), "%s/%s", "/interfaces", sys_id);
        }
    }
    sys.base_uri = strdup(tmp);

    if (ret && sys.protocol_dependencies->dnp3.stats_pub_frequency > 0)
    {
        ret = getCJstr(cj, "stats_pub_uri", sys.protocol_dependencies->dnp3.stats_pub_uri, true);
        if (!ret)
        {
            snprintf(tmp, sizeof(tmp), "%s/%s", sys.base_uri, "stats");
            sys.protocol_dependencies->dnp3.stats_pub_uri = strdup(tmp);
        }
    }

    return final_ret;
}

int getNumBits(TMWSIM_POINT* dbPoint)
{
    if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
    {
        if (dbPoint->defaultStaticVariation == 1)
        {
            return 31;  // technically these are signed integers
        }
        else if (dbPoint->defaultStaticVariation == 2)
        {
            return 16;  // technically these are signed integers
        }
        else if (dbPoint->defaultStaticVariation == 3)
        {
            return 31;  // technically these are signed integers
        }
        else if (dbPoint->defaultStaticVariation == 4)
        {
            return 16;  // technically these are signed integers
        }
        else if (dbPoint->defaultStaticVariation == 6)
        {
            return 32;  // floats
        }
        else
        {
            return 64;  // doubles
        }
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS)
    {
        if (dbPoint->defaultStaticVariation == 1)
        {
            return 31;  // technically these are signed integers
        }
        else if (dbPoint->defaultStaticVariation == 2)
        {
            return 16;  // technically these are signed integers
        }
        else if (dbPoint->defaultStaticVariation == 3)
        {
            return 32;  // floats
        }
        else
        {
            return 64;  // doubles
        }
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16)
    {
        return 15;  // technically these are signed integers
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32)
    {
        return 31;  // technically these are signed integers
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32)
    {
        return 32;  // this is kind of a weird situation, so I don't know if we would really use this...
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
    {
        if (dbPoint->defaultStaticVariation == 1 || dbPoint->defaultStaticVariation == 3 ||
            dbPoint->defaultStaticVariation == 5 || dbPoint->defaultStaticVariation == 7)
        {
            return 32;  // uint32
        }
        return 16;  // uint16
    }
    else if (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::Binary ||
             ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
             ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::CROB)
    {
        return 1;
    }
    return 0;
}

/// @brief
/// @param sys
/// @param dbPoint
/// @param bits
/// @return
int addBits(GcomSystem& sys, TMWSIM_POINT* dbPoint, cJSON* bits)
{
    if (dbPoint == nullptr || bits == nullptr || ((FlexPoint*)(dbPoint->flexPointHandle)) == nullptr){
        return 0;
    }
    int asiz = cJSON_GetArraySize(bits);
    int num_bits = getNumBits(dbPoint);
    if (num_bits > 0 && asiz > num_bits)
    {
        asiz = num_bits;
    }

    for (int i = 0; i < asiz; i++)
    {
        cJSON* cji = cJSON_GetArrayItem(bits, i);
        std::string bitstring = "Unknown";

        if (cJSON_IsString(cji))
        {
            bitstring = cji->valuestring;
        }

        if (bitstring == "IGNORE")
        {
            bitstring = "Unknown";
        }

        // add bit
        ((FlexPoint*)(dbPoint->flexPointHandle))->dbBits.push_back(std::pair<std::string, int>(bitstring, i));
    }
    return asiz;
}

int addEnum(GcomSystem& sys, TMWSIM_POINT* dbPoint, cJSON* bits)
{
    int asiz = cJSON_GetArraySize(bits);
    int num_bits = getNumBits(dbPoint);
    if (num_bits > 0 && num_bits < 31 && asiz > (1 << num_bits))
    {
        asiz = 1 << num_bits;
    }

    int bit_value = 0;
    for (int i = 0; i < asiz; i++)
    {
        cJSON* cji = cJSON_GetArrayItem(bits, i);

        std::string bitstring = "Unknown";
        if (cJSON_IsObject(cji))
        {
            auto cj_value = cJSON_GetObjectItem(cji, "value");
            auto cj_bitstring = cJSON_GetObjectItem(cji, "string");
            if (cj_value && cj_bitstring)
            {
                if (cJSON_IsNumber(cj_value))
                    bit_value = cj_value->valueint;
                if (cJSON_IsString(cj_bitstring))
                    bitstring = cj_bitstring->valuestring;
            }
        }
        else if (cJSON_IsString(cji))
        {
            bitstring = cji->valuestring;
        }
        if (bitstring == "IGNORE")
        {
            bitstring = "Unknown";
        }

        ((FlexPoint*)(dbPoint->flexPointHandle))->dbBits.push_back(std::pair<std::string, int>(bitstring, bit_value));

        bit_value++;
    }
    return asiz;
}

/// @brief parse a map of items
/// @param sys
/// @param objs
/// @param type
/// @param who
/// @return
bool parse_items(GcomSystem& sys, cJSON* objs, int type, int who)
{
    cJSON* obj;
    int mytype = type;

    cJSON_ArrayForEach(obj, objs)
    {
        cJSON *id, *offset, *uri, *variation;
        cJSON *evariation, *class_num, *rsize;  //, *sign;
        cJSON *scale, *cjidx, *cjcfalse, *opvar, *deadband;
        cJSON *cjfmt, *cjtout;
        cJSON *cjcstring, *cjctrue, *cjcint, *event_pub;
        cJSON *batch_pub_rate, *interval_pub_rate, *batch_set_rate, *interval_set_rate;
        cJSON *show_output_status, *resend_tolerance, *resend_rate_ms, *output_status_uri;
        cJSON *is_bitfield, *is_enum, *is_individual_bits, *bitstrings;
        cJSON *pulse_enabled, *on_time, *off_time, *num_pulses;

        // heartbeat stuff
        // cJSON *min, *max, *incr;

        if (obj == NULL)
        {
            FPS_ERROR_LOG("Invalid or NULL obj");
            continue;
        }

        // standard stuff
        id = cJSON_GetObjectItem(obj, "id");  // note that id translates to name
        cjidx = cJSON_GetObjectItem(obj, "idx");
        offset = cJSON_GetObjectItem(obj, "offset");
        rsize = cJSON_GetObjectItem(obj, "size");
        variation = cJSON_GetObjectItem(obj, "variation");
        evariation = cJSON_GetObjectItem(obj, "evariation");
        uri = cJSON_GetObjectItem(obj, "uri");
        // linkback = cJSON_GetObjectItem(obj, "linkback");
        // linkuri = cJSON_GetObjectItem(obj, "linkuri");
        class_num = cJSON_GetObjectItem(obj, "class");
        if (!class_num)
        {
            class_num = cJSON_GetObjectItem(obj, "clazz");
        }
        // sign = cJSON_GetObjectItem(obj, "signed");
        scale = cJSON_GetObjectItem(obj, "scale");
        deadband = cJSON_GetObjectItem(obj, "deadband");
        opvar = cJSON_GetObjectItem(obj, "OPvar");  // 1 = 16 bit , 2 = 32 bit , 3 float32
        cjfmt = cJSON_GetObjectItem(obj, "format");
        cjtout = cJSON_GetObjectItem(obj, "timeout");

        // allows more detailed manipulation of the crob type
        cjcstring = cJSON_GetObjectItem(obj, "crob_string");  // true / false
        cjctrue = cJSON_GetObjectItem(obj, "crob_true");      // string like POWER_ON
        cjcfalse = cJSON_GetObjectItem(obj, "crob_false");    // string like POWER_OFF
        cjcint = cJSON_GetObjectItem(obj, "crob_int");        // true / false
        pulse_enabled = cJSON_GetObjectItem(obj, "pulse_enabled");
        on_time = cJSON_GetObjectItem(obj, "on_time");
        off_time = cJSON_GetObjectItem(obj, "off_time");
        num_pulses = cJSON_GetObjectItem(obj, "num_pulses");

        // stuff for outputs
        batch_set_rate = cJSON_GetObjectItem(obj, "batch_set_rate");
        interval_set_rate = cJSON_GetObjectItem(obj, "interval_set_rate");
        batch_pub_rate = cJSON_GetObjectItem(obj, "batch_pub_rate");
        interval_pub_rate = cJSON_GetObjectItem(obj, "interval_pub_rate");
        event_pub = cJSON_GetObjectItem(obj, "event_pub");
        show_output_status = cJSON_GetObjectItem(obj, "show_output_status");
        output_status_uri = cJSON_GetObjectItem(obj, "output_status_uri");
        resend_tolerance = cJSON_GetObjectItem(obj, "resend_tolerance");
        resend_rate_ms = cJSON_GetObjectItem(obj, "resend_rate");

        // stuff for bit_fields
        is_bitfield = cJSON_GetObjectItem(obj, "bit_field");
        is_enum = cJSON_GetObjectItem(obj, "enum");
        is_individual_bits = cJSON_GetObjectItem(obj, "individual_bits");
        bitstrings = cJSON_GetObjectItem(obj, "bit_strings");

        //        if (id == NULL || offset == NULL || id->valuestring == NULL)
        if (id == NULL || id->valuestring == NULL)
        {
            FPS_ERROR_LOG("NULL variables or component_id");
            continue;
        }

        // allow 32 bit systems ints
        mytype = type;
        if (rsize != NULL)
        {
            if (type == AnIn16)
            {
                if (rsize->valueint > 1)
                {
                    mytype = AnIn32;
                }
            }
        }
        // use the SEL output variation designation
        if (opvar)
        {
            switch (opvar->valueint)
            {
                case 1:
                    mytype = AnIn16;
                    break;
                case 2:
                    mytype = AnIn32;
                    break;
                case 3:
                    mytype = AnF32;
                    break;
                default:
                    mytype = AnIn32;
                    break;
            }
        }

        // rework
        // DbVars are sorted by uri's now.
        // we can have the same TMWSIM_POINT name for different uris.
        // incoming messages from master only know type (AnIn16,AnIn32,AF32,CROB) and offset.
        // for these items offset is still set by the config file.
        // The return path from outstation to master still has type an index. The system has changed to use the offset
        // from the conig file here too. so we can no longer use the vector position as an index. items are added from
        // the config file against a registered uri ot the default one. pubs, gets and sets from FIMS ues a combination
        // of URI + var name to find vars. data incoming from master to outstation will use type and offset to find the
        // variable. each type will have a map <int,TMWSIM_POINT *> to search Data base for the outstation vars (analog
        // and binary) will no longer use the vector size but max offset as dbPoint size ???? the <int , TMWSIM_POINT *>
        // map can also  be used for these vars
        //
        // Get this going as follows
        //   1.. parse config file using the uri's.
        //   2.. get the uri/var name search  working for FIMS querues
        //   3.. get the int, TMWSIM_POINT* search / map  running to find vars for outstation builder.
        //   4.. get the  mapping working for the incoming commands.
        //
        //

        // the cjidx fields will ovride the auto idx.
        int vidx = -1;
        if (offset)
        {
            vidx = offset->valueint;
        }
        if (cjidx)
        {
            vidx = cjidx->valueint;
        }
        if (mytype == AnF32)
        {
            if (sys.debug)
                FPS_DEBUG_LOG("****** AF32 variable [%s] vidx %d", id->valuestring, vidx);
        }
        if (sys.debug)
            FPS_DEBUG_LOG("****** mtype %d xxvariable [%s] vidx %d debug %d", mytype, id->valuestring, vidx, sys.debug);
        // set up name uri
        char* nuri = getDefUri(sys, who);
        if (uri && uri->valuestring)
        {
            nuri = uri->valuestring;
        }

        TMWSIM_POINT* dbPoint = newDbVar(sys, id->valuestring, mytype, vidx, nuri,
                                         variation ? variation->valuestring : NULL);
        if (dbPoint != NULL)
        {
            size_t id_len = strlen(id->valuestring);
            if (sys.watchdog && (id_len == strlen(sys.watchdog->id)) &&
                (strcmp(id->valuestring, sys.watchdog->id) == 0))
            {
                sys.watchdog->io_point = dbPoint;
            }
            if (sys.heartbeat && (id_len == strlen(sys.heartbeat->heartbeat_read_uri)) &&
                (strcmp(id->valuestring, sys.heartbeat->heartbeat_read_uri) == 0))
            {
                sys.heartbeat->heartbeat_read_point = dbPoint;
            }
            if (sys.heartbeat && (id_len == strlen(sys.heartbeat->heartbeat_write_uri)) &&
                (strcmp(id->valuestring, sys.heartbeat->heartbeat_write_uri) == 0))
            {
                sys.heartbeat->heartbeat_write_point = dbPoint;
            }
            ((FlexPoint*)(dbPoint->flexPointHandle))->format = sys.fims_dependencies->format;

            if (event_pub)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = cJSON_IsTrue(event_pub);
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = sys.protocol_dependencies->dnp3.event_pub;
            }

            if (show_output_status && (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
                                       ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
                                       ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
                                       ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
                                       ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
                                       ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::CROB))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->show_output_status = cJSON_IsTrue(show_output_status);
                if (output_status_uri)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->output_status_uri = strdup(
                        output_status_uri->valuestring);
                }
                else if (sys.protocol_dependencies->dnp3.output_status_uri)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->output_status_uri = strdup(
                        sys.protocol_dependencies->dnp3.output_status_uri);
                }
            }
            else if (sys.protocol_dependencies->dnp3.show_output_status &&
                     (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::CROB))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->show_output_status = sys.protocol_dependencies->dnp3
                                                                                   .show_output_status;
                if (output_status_uri)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->output_status_uri = strdup(
                        output_status_uri->valuestring);
                }
                else if (sys.protocol_dependencies->dnp3.output_status_uri)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->output_status_uri = strdup(
                        sys.protocol_dependencies->dnp3.output_status_uri);
                }
            }
            if (((FlexPoint*)(dbPoint->flexPointHandle))->output_status_uri)
            {
                addOutputStatusUri(sys, ((FlexPoint*)(dbPoint->flexPointHandle))->output_status_uri, dbPoint);
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->show_output_status = false;
            }

            if (resend_tolerance && (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
                                     ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
                                     ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
                                     ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
                                     ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
                                     ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::CROB))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->resend_tolerance = resend_tolerance->valuedouble;
                if (resend_rate_ms)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->resend_rate_ms = resend_rate_ms->valuedouble;
                }
                else if (sys.protocol_dependencies->dnp3.resend_rate_ms)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->resend_rate_ms = sys.protocol_dependencies->dnp3
                                                                                   .resend_rate_ms;
                }
            }
            else if (sys.protocol_dependencies->dnp3.resend_tolerance &&
                     (((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
                      ((FlexPoint*)(dbPoint->flexPointHandle))->type == Register_Types::CROB))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->resend_tolerance = sys.protocol_dependencies->dnp3
                                                                                 .resend_tolerance;
                if (resend_rate_ms)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->resend_rate_ms = resend_rate_ms->valuedouble;
                }
                else if (sys.protocol_dependencies->dnp3.resend_rate_ms)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->resend_rate_ms = sys.protocol_dependencies->dnp3
                                                                                   .resend_rate_ms;
                }
            }

            // batch set rate
            if (batch_set_rate && batch_set_rate->valueint > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_set_rate = batch_set_rate->valueint;
            }
            else if (sys.protocol_dependencies->dnp3.batch_set_rate > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_set_rate = sys.protocol_dependencies->dnp3
                                                                               .batch_set_rate;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_set_rate = 0;
            }

            // interval set rate
            if (interval_set_rate && interval_set_rate->valueint > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate = interval_set_rate->valueint;
            }
            else if (sys.protocol_dependencies->dnp3.interval_set_rate > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate = sys.protocol_dependencies->dnp3
                                                                                  .interval_set_rate;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate = 0;
            }

            // batch pub rate
            if (batch_pub_rate && batch_pub_rate->valueint > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pub_rate = batch_pub_rate->valueint;
            }
            else if (sys.protocol_dependencies->dnp3.batch_pub_rate > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pub_rate = sys.protocol_dependencies->dnp3
                                                                               .batch_pub_rate;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pub_rate = 0;
            }

            // interval pub rate
            if (interval_pub_rate && interval_pub_rate->valueint > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pub_rate = interval_pub_rate->valueint;
                if (sys.heartbeat && sys.heartbeat->heartbeat_read_point == dbPoint)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = true;
                }
            }
            else if (sys.protocol_dependencies->dnp3.interval_pub_rate > 0)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pub_rate = sys.protocol_dependencies->dnp3
                                                                                  .interval_pub_rate;
                if (sys.heartbeat && sys.heartbeat->heartbeat_read_point == dbPoint)
                {
                    ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = true;
                }
            }
            else if (sys.heartbeat && sys.heartbeat->frequency > 0 && sys.heartbeat->heartbeat_read_point == dbPoint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = true;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pub_rate = sys.heartbeat->frequency;
                ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = true;
            }
            else if (((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = false;
                ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pub_rate = 0;
            }

            if (!((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs &&
                !((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->direct_pubs = true;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->direct_pubs = false;
            }

            if (!((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets &&
                !((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->direct_sets = true;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->direct_sets = false;
            }

            if (cjfmt)
            {
                if (cjfmt->valuestring)
                {
                    std::string fmt = cjfmt->valuestring;
                    if (fmt == "clothed")
                    {
                        ((FlexPoint*)(dbPoint->flexPointHandle))->format = FimsFormat::Clothed;
                    }
                    else if (fmt == "full")
                    {
                        ((FlexPoint*)(dbPoint->flexPointHandle))->format = FimsFormat::Full;
                    }
                    else  // defaults to naked
                    {
                        ((FlexPoint*)(dbPoint->flexPointHandle))->format = FimsFormat::Naked;
                    }
                }
            }
            if (cjcstring)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_string = (cjcstring->type == cJSON_True);
            }

            if (cjcint)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_int = (cjcint->type == cJSON_True);
            }

            if (cjctrue && cjctrue->valuestring)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_true = strdup(cjctrue->valuestring);
            }

            if (cjcfalse && cjcfalse->valuestring)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->crob_false = strdup(cjcfalse->valuestring);
            }

            if (pulse_enabled)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_crob = (pulse_enabled->type == cJSON_True);
                bool pulse_crob = ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_crob;
                if (pulse_crob && on_time && off_time && num_pulses)
                {
                    if (((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets ||
                        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets)
                    {
                        FPS_ERROR_LOG(
                            "Cannot implement pulsed CROB for point [%s]. Must choose among interval sets, batched sets, or pulsed CROB.",
                            ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                        pulse_crob = false;
                    }
                    ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_on_ms = on_time->valueint;
                    if (pulse_crob && ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_on_ms <= 0)
                    {
                        FPS_ERROR_LOG(
                            "Cannot implement pulsed CROB for point [%s]. on_time [%d]. Need value greater than 0.",
                            ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                            ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_on_ms);
                        ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_crob = false;
                    }
                    ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_off_ms = off_time->valueint;
                    if (pulse_crob && ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_off_ms <= 0)
                    {
                        FPS_ERROR_LOG(
                            "Cannot implement pulsed CROB for point [%s]. off_time [%d]. Need value greater than 0.",
                            ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                            ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_off_ms);
                        ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_crob = false;
                    }
                    ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_count = num_pulses->valueint;
                    if (pulse_crob && ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_count <= 0)
                    {
                        FPS_ERROR_LOG(
                            "Cannot implement pulsed CROB for point [%s]. num_pulses [%d]. Need value greater than 0.",
                            ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                            ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_count);
                        ((FlexPoint*)(dbPoint->flexPointHandle))->pulse_crob = false;
                    }
                }
            }
        }
        if (dbPoint != NULL)
        {
            if (evariation)
            {
                dbPoint->defaultEventVariation = event_variation_decode(evariation->valuestring, mytype);
            }
            if (deadband)
            {
                dbPoint->data.analog.deadband = deadband->valuedouble;
            }
            else
            {
                dbPoint->data.analog.deadband = sys.protocol_dependencies->deadband;
            }

            if (cjtout)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->timeout = cjtout->valuedouble;
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->timeout = sys.protocol_dependencies->timeout;
            }
            tmwtimer_init(&((FlexPoint*)dbPoint->flexPointHandle)->timeout_timer);
            // if (((FlexPoint *)(dbPoint->flexPointHandle))->timeout == 0)
            // {
            //     dbPoint->flags = 0;
            // }
            // else
            // {
            //     dbPoint->flags = 1;
            // }

            // for batching pubs
            tmwtimer_init(&((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer);

            // set work stuff
            tmwtimer_init(&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer);
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.send_uri.clear();
            FORMAT_TO_BUF(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.send_uri, R"({}/{})",
                          ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                          ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.dbPoint = dbPoint;
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value = 0.0;

            if (class_num)
            {
                if (class_num->valueint >= 1)
                    dbPoint->classMask = 1 << (class_num->valueint - 1);  // can be any value from 1 to 3
                else
                    dbPoint->classMask = 0;
                if (sys.debug)
                    FPS_DEBUG_LOG("****** variable [%s] set to class_num %d",
                                  ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(), dbPoint->classMask);
            }

            // if (sign)
            // {
            //     ((FlexPoint *)(dbPoint->flexPointHandle))->sign = (sign->type == cJSON_True);
            //     if (sys.debug)
            //         FPS_DEBUG_LOG("****** variable [%s] set to signed %d", ((FlexPoint
            //         *)(dbPoint->flexPointHandle))->name.c_str(), ((FlexPoint *)(dbPoint->flexPointHandle))->sign);
            // }
            // else
            // {
            //     ((FlexPoint *)(dbPoint->flexPointHandle))->sign = sys.protocol_dependencies->dnp3.sign;
            //     if (sys.debug)
            //         FPS_DEBUG_LOG("****** variable [%s] set to signed %d", ((FlexPoint
            //         *)(dbPoint->flexPointHandle))->name.c_str(), ((FlexPoint *)(dbPoint->flexPointHandle))->sign);
            // }

            if (scale)
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = (scale->valuedouble);
                if (sys.debug)

                    FPS_DEBUG_LOG("****** variable [%s] scale set %f",
                                  ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str(),
                                  ((FlexPoint*)(dbPoint->flexPointHandle))->scale);
            }
            else
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->scale = 0;
            }

            if (is_bitfield && bitstrings && (bitstrings->type == cJSON_Array))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->is_bitfield = (is_bitfield->type == cJSON_True);
                if (((FlexPoint*)(dbPoint->flexPointHandle))->is_bitfield)
                {
                    if (sys.debug)
                        FPS_DEBUG_LOG("*****************Adding bitfields for %s",
                                      ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                    addBits(sys, dbPoint, bitstrings);
                }
            }

            if (is_enum && bitstrings && (bitstrings->type == cJSON_Array))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->is_enum = (is_enum->type == cJSON_True);
                if (((FlexPoint*)(dbPoint->flexPointHandle))->is_enum)
                {
                    if (sys.debug)
                        FPS_DEBUG_LOG("*****************Adding enum for %s",
                                      ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                    addEnum(sys, dbPoint, bitstrings);
                }
            }

            if (is_individual_bits && bitstrings && (bitstrings->type == cJSON_Array))
            {
                ((FlexPoint*)(dbPoint->flexPointHandle))->is_individual_bits = (is_individual_bits->type == cJSON_True);
                if (((FlexPoint*)(dbPoint->flexPointHandle))->is_individual_bits)
                {
                    if (sys.debug)
                        FPS_DEBUG_LOG("*****************Adding individual bits for %s",
                                      ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
                    addBits(sys, dbPoint, bitstrings);
                }
            }

            if (sys.debug)
                FPS_DEBUG_LOG(" config adding name [%s] idx [%d]", id->valuestring, vidx);
            // was nuri

            addDbUri(sys, ((FlexPoint*)(dbPoint->flexPointHandle))->uri, dbPoint);
        }
    }
    return true;
}

int parse_object(GcomSystem& sys, cJSON* objs, int idx, int who)
{
    cJSON* cjlist = cJSON_GetObjectItem(objs, iotypToStr(idx));
    if (cjlist == NULL)
    {
        FPS_DEBUG_LOG("[%s] objects missing from config!", iotypToStr(idx));
        return -1;
    }
    return parse_items(sys, cjlist, idx, who);
}
/// @brief ???
/// @param cj
/// @param sys
/// @param who
/// @return
bool parse_modbus(cJSON* cj, GcomSystem& sys, int who)
{
    // config file has "objects" with children groups "binary" and "analog"
    // who is needd to stop cross referencing linkvars
    cJSON* cji;
    cJSON_ArrayForEach(cji, cj)
    {
        cJSON* cjmap = cJSON_GetObjectItem(cji, "map");
        cJSON* cjdtype = cJSON_GetObjectItem(cji, "dnp3_type");
        cJSON* cjtype = cJSON_GetObjectItem(cji, "type");
        // dnp3_type can be output, valuedouble holds it all
        if ((cjmap == NULL) || (cjmap->type != cJSON_Array))
        {
            FPS_ERROR_LOG("modbus registers map object is not an array !");
            return false;
        }

        // here are the tpe options
        // no dnp3_type then look at type
        int itype = -1;
        if ((cjdtype != NULL) && (cjdtype->type == cJSON_String))
        {
            itype = iotypToId(cjdtype->valuestring);
            if (itype < 0)
            {
                FPS_ERROR_LOG("modbus dnp3_type [%s] not recognised!", cjdtype->valuestring);
                return false;
            }
        }
        if (itype < 0)
        {
            if ((cjtype != NULL) && (cjtype->type == cJSON_String))
            {
                itype = iotypToId(cjtype->valuestring);
                if (itype < 0)
                {
                    FPS_ERROR_LOG("modbus type [%s] not recognised!", cjtype->valuestring);
                    return false;
                }
            }
        }
        if (itype < 0)
        {
            FPS_ERROR_LOG(" register type missing or  not recognised!");
            return false;
        }
        parse_items(sys, cjmap, itype, who);
    }
    // sys.protocol_dependencies->assignIdx();

    return true;
}
/// @brief
/// @param object
/// @param sys
/// @param who
/// @return
bool parse_variables(cJSON* object, GcomSystem& sys, int who)
{
    // config file has "objects" with children groups "binary" and "analog"
    // who is needd to stop cross referencing linkvars
    cJSON* cjregs = cJSON_GetObjectItem(object, "registers");
    if (cjregs != NULL)
    {
        return parse_modbus(cjregs, sys, who);
    }

    cJSON* JSON_objects = cJSON_GetObjectItem(object, "objects");
    if (JSON_objects == NULL)
    {
        FPS_ERROR_LOG("objects object is missing from file!");
        return false;
    }
    for (int itype = 0; itype < Type_of_Var::NumTypes; itype++)
        parse_object(sys, JSON_objects, itype, who);

    // after this is done we auto assign the indexes here
    // sys.protocol_dependencies->assignIdx();

    return true;
}
/// @brief
/// @param sys
/// @param who
/// @param nums
/// @return
int getSysUris(GcomSystem& sys, int who, int nums)
{
    int num_subs = 0;
    int i;
    // sysCfg** ss = sys;
    for (i = 0; i < nums; i++)
    {
        num_subs += getSubs(sys, NULL, 0, who);
    }
    // int num = sys.protocol_dependencies->getSubs(NULL, 0, who);
    const char** subs = (const char**)malloc((num_subs + 3) * sizeof(char*));
    if (subs == NULL)
    {
        FPS_ERROR_LOG("Failed to create subs array.");
        return -1;
    }

    int num = 0;
    for (i = 0; i < nums; i++)
    {
        num += getSubs(sys, &subs[num], num_subs, who);
    }
    free(subs);

    return num;
}
/// @brief
/// @param config
/// @param sys
/// @param num_configs
/// @param who
/// @return
bool run_config(cJSON* config, GcomSystem& sys, int& num_configs, int who)
{
    bool ret = false;
    sys.config = config;

    if (!parse_system(config, sys, who))
    {
        FPS_ERROR_LOG("Error reading system from config file.");
        cJSON_Delete(config);
        return ret;
    }

    ret = true;

    return ret;
}

/// @brief
/// @param sys
void init_point_groups(GcomSystem& sys)
{
    TMWSIM_POINT* dbPoint;
    DNP3Dependencies* dnp3_sys = &(sys.protocol_dependencies->dnp3);

    if (sys.protocol_dependencies->who == DNP3_OUTSTATION)
    {
        dnp3_sys->point_group_map["/analog_outputs"] = PointGroup{};
        PointGroup& point_group_1 = dnp3_sys->point_group_map["/analog_outputs"];
        point_group_1.group_name = "analog_outputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgOutGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_1.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/binary_outputs"] = PointGroup{};
        PointGroup& point_group_2 = dnp3_sys->point_group_map["/binary_outputs"];
        point_group_2.group_name = "binary_outputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)sdnpsim_binOutGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_2.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/analog_inputs"] = PointGroup{};
        PointGroup& point_group_3 = dnp3_sys->point_group_map["/analog_inputs"];
        point_group_3.group_name = "analog_inputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)sdnpsim_anlgInGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_3.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/binary_inputs"] = PointGroup{};
        PointGroup& point_group_4 = dnp3_sys->point_group_map["/binary_inputs"];
        point_group_4.group_name = "binary_inputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT*)sdnpsim_binInGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_4.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/counters"] = PointGroup{};
        PointGroup& point_group_5 = dnp3_sys->point_group_map["/counters"];
        point_group_5.group_name = "counters";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            dbPoint = (TMWSIM_POINT*)sdnpsim_binCntrGetEnabledPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_5.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
    }
    else
    {
        dnp3_sys->point_group_map["/analog_outputs"] = PointGroup{};
        PointGroup& point_group_1 = dnp3_sys->point_group_map["/analog_outputs"];
        point_group_1.group_name = "analog_outputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_1.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogOutputs, dbPoint);
        }
        dnp3_sys->point_group_map["/binary_outputs"] = PointGroup{};
        PointGroup& point_group_2 = dnp3_sys->point_group_map["/binary_outputs"];
        point_group_2.group_name = "binary_outputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_2.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryOutputs, dbPoint);
        }
        dnp3_sys->point_group_map["/analog_inputs"] = PointGroup{};
        PointGroup& point_group_3 = dnp3_sys->point_group_map["/analog_inputs"];
        point_group_3.group_name = "analog_inputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_3.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->analogInputs, dbPoint);
        }
        dnp3_sys->point_group_map["/binary_inputs"] = PointGroup{};
        PointGroup& point_group_4 = dnp3_sys->point_group_map["/binary_inputs"];
        point_group_4.group_name = "binary_inputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_4.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryInputs, dbPoint);
        }
        dnp3_sys->point_group_map["/counters"] = PointGroup{};
        PointGroup& point_group_5 = dnp3_sys->point_group_map["/counters"];
        point_group_5.group_name = "counters";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_5.points[((FlexPoint*)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE*)(dnp3_sys->dbHandle))->binaryCounters, dbPoint);
        }
    }
}
/// @brief
/// @param sys
/// @param who
/// @return
bool init_vars(GcomSystem& sys, int who)
{
    bool ret = false;
    if (!parse_variables(sys.config, sys, who))
    {
        FPS_ERROR_LOG("Error reading variabled from config file.");
        return ret;
    }
    ret = true;

    // initializeWatchdogAndHeartbeat(sys);

    init_point_groups(sys);

    if (sys.debug)
    {
        showNewUris(sys);
    }

    return ret;
}

const char* get_program_name(const char* uri)
{
    // Find the last occurrence of '/'
    const char* lastSlash = std::strrchr(uri, '/');

    // If '/' is found, return the part after it; otherwise, return the whole URI
    return (lastSlash != nullptr) ? (lastSlash + 1) : uri;
}

cJSON* load_from_dbi(int argc, char* argv[])
{
    if (argc <= 2)
    {  // it would have to be equal to 2 in order to get here, but just to be safe
        FPS_ERROR_LOG("Need URI to load from DBI");
        return nullptr;
    }

    std::string program_name(get_program_name(argv[0]));
    std::string fname(argv[2]);
    if (fname.front() != '/')
    {
        FPS_ERROR_LOG("For %s with init uri \"%s\": the uri does not begin with `/`", program_name.c_str(), argv[2]);
        return nullptr;
    }
    fims fims_gateway;
    const auto conn_id_str = program_name + "_uri_init@" + fname;
    if (!fims_gateway.Connect(conn_id_str.data()))
    {
        FPS_ERROR_LOG("For %s with init uri \"%s\": could not connect to fims_server", program_name.c_str(), argv[2]);
        return nullptr;
    }
    const auto sub_string = "/" + program_name + "_uri_init" + fname;
    if (!fims_gateway.Subscribe(std::vector<std::string>{ sub_string }))
    {
        FPS_ERROR_LOG("For %s with init uri \"%s\": failed to subscribe for uri init", program_name.c_str(), argv[2]);
        fims_gateway.Close();
        return nullptr;
    }
    if (!fims_gateway.Send(fims::str_view{ "get", sizeof("get") - 1 }, fims::str_view{ fname.data(), fname.size() },
                           fims::str_view{ sub_string.data(), sub_string.size() }, fims::str_view{ nullptr, 0 },
                           fims::str_view{ nullptr, 0 }))
    {
        FPS_ERROR_LOG("For %s with inti uri \"%s\": failed to send a fims get message", program_name.c_str(), argv[2]);
        fims_gateway.Close();
        return nullptr;
    }
    auto config_msg = fims_gateway.Receive_Timeout(5000000);  // give them 5 seconds to respond before erroring
    if (!config_msg)
    {
        FPS_ERROR_LOG("For %s with init uri \"%s\": failed to receive a message in 5 seconds", program_name.c_str(),
                      argv[2]);
        fims_gateway.Close();
        return nullptr;
    }
    if (!config_msg->body)
    {
        FPS_ERROR_LOG("For %s with init uri \"%s\": message was received, but body doesn't exist", program_name.c_str(),
                      argv[2]);
        fims_gateway.Close();
        return nullptr;
    }
    cJSON* config = cJSON_Parse(config_msg->body);
    if (config == NULL)
        fprintf(stderr, "Invalid JSON object in file\n");
    fims_gateway.Close();
    if (config_msg)
    {
        delete config_msg;
    }
    FPS_INFO_LOG("File loaded from DBI");
    return config;
}

/// @brief
/// @param argc
/// @param argv
/// @param sys
/// @param who
/// @return
int getConfigs(int argc, char* argv[], GcomSystem& sys, int who)
{
    int num_configs = 0;
    while (true)
    {
        if (argc >= 2 && std::strcmp(argv[1], "-u") == 0)
        {
            cJSON* config = load_from_dbi(argc, argv);
            if (config == NULL)
            {
                if (num_configs == 0)
                {
                    FPS_ERROR_PRINT("Error reading config file\n");
                    return -1;
                }
                else
                {
                    break;
                }
            }
            if (!run_config(config, sys, num_configs, who))
                return -1;
            return 1;
        }
        else
        {
            cJSON* config = get_config_json(argc, argv, num_configs);
            if (config == NULL)
            {
                if (num_configs == 0)
                {
                    FPS_ERROR_LOG("Error reading config file");
                    return -1;
                }
                else
                {
                    break;
                }
            }
            cJSON* cjfiles = parse_files(config);
            if (cjfiles)
            {
                cJSON* cjfile;
                cJSON_ArrayForEach(cjfile, cjfiles)
                {
                    FPS_ERROR_LOG(" NOTE config file [%s]", cjfile->valuestring);
                    config = get_config_file(cjfile->valuestring);
                    if (!run_config(config, sys, num_configs, who))
                    {
                        return -1;
                    }
                    num_configs += 1;
                }
            }
            else
            {
                if (!run_config(config, sys, num_configs, who))
                    return -1;
                num_configs += 1;
            }
        }
    }
    return num_configs;
}
/// @brief
/// @param argc
/// @param argv
/// @return
std::string getFileName(int argc, char* argv[])
{
    std::string file_name;
    if (argc > 1)
    {
        file_name = std::string(argv[1]);
        if (file_name == "-u" && argc > 2)
        {
            file_name = std::string(argv[2]);
        }
        std::size_t last_slash = file_name.find_last_of("/\\");
        file_name = file_name.substr(last_slash + 1);
        std::size_t extension = file_name.find(".json");
        file_name = file_name.substr(0, extension);  // not entirely sure why I had to do this in two steps, but trying
                                                     // to do it in one step didn't work
    }
    else if (argc == 1)
    {
        file_name = argv[0];
    }
    else
    {
        file_name = "dnp3_process";
    }
    return file_name;
}

bool timeInit = false;
std::mutex timeDoubleMutex;
std::chrono::system_clock::time_point baseTime;
/// @brief
/// @return
double get_time_double()
{
    std::lock_guard<std::mutex> lock(timeDoubleMutex);
    if (!timeInit)
        baseTime = std::chrono::system_clock::now();
    timeInit = true;
    std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = currentTime - baseTime;
    return duration.count();
}
/// @brief
/// @param tmpPtr
/// @return
double get_time_double(TMWDTIME& tmpPtr)
{
    std::lock_guard<std::mutex> lock(timeDoubleMutex);
    if (!timeInit)
        baseTime = std::chrono::system_clock::now();
    timeInit = true;
    std::tm tm = {};
    tm.tm_year = tmpPtr.year - 1900;
    tm.tm_mon = tmpPtr.month - 1;
    tm.tm_mday = tmpPtr.dayOfMonth;
    tm.tm_hour = tmpPtr.hour;
    tm.tm_min = tmpPtr.minutes;
    tm.tm_sec = tmpPtr.mSecsAndSecs / 1000;
    auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    std::chrono::duration<double> duration = time_point - baseTime;
    return duration.count() + (tmpPtr.mSecsAndSecs % 1000) / 1000.0;
}

bool parse_message_body(GcomSystem& sys, std::string uri, const char* message_body)
{
    auto& parser = sys.fims_dependencies->parser;
    auto& doc = sys.fims_dependencies->doc;
    Jval_buif to_set;

    // gets doc
    if (const auto err = parser
                             .iterate(message_body, strlen(message_body),
                                      strlen(message_body) + simdjson::SIMDJSON_PADDING)
                             .get(doc);
        err)
    {
        return false;
    }

    simdjson::ondemand::object set_obj;
    if (const auto err = doc.get(set_obj); err)
    {
        return false;
    }

    for (auto pair : set_obj)
    {
        const auto key = pair.unescaped_key();
        if (const auto err = key.error(); err)
        {
            return false;
        }
        const auto key_view = key.value_unsafe();
        auto val = pair.value();
        if (const auto err = val.error(); err)
        {
            return false;
        }

        TMWSIM_POINT* dbPoint = getDbVar(sys, uri, key_view);
        if (!dbPoint)
        {
            continue;
        }

        auto curr_val = val.value_unsafe();
        auto val_clothed = curr_val.get_object();
        auto success = extractValueMulti(sys, val_clothed, curr_val, to_set, key_view);
        if (!success)
        {
            continue;
        }

        

        double value = jval_to_double(to_set);

        if (dbPoint->type == TMWSIM_TYPE_ANALOG)
        {
            dbPoint->data.analog.value = value;
        }
        else if (dbPoint->type == TMWSIM_TYPE_COUNTER)
        {
            dbPoint->data.counter.value = static_cast<TMWTYPES_ULONG>(value);
        }
        else if (dbPoint->type == TMWSIM_TYPE_BINARY)
        {
            dbPoint->data.binary.value = static_cast<bool>(value);
        }
    }
    return true;
}

void load_points_from_dbi_server(GcomSystem& sys)
{
    fims& fims_gateway = sys.fims_dependencies->fims_gateway;

    for (auto uriPair : sys.dburiMap)
    {
        if (uriPair.second->multi)
        {
            std::string point_group_uri = uriPair.first;
            std::string dbi_uri = "/dbi/" + std::string(sys.id) + "/saved_registers" + point_group_uri;
            std::string replyto_uri = "/" + std::string(sys.id) + "/saved_registers" + point_group_uri;
            if (!fims_gateway.Send("get", dbi_uri.c_str(), replyto_uri.c_str(), nullptr, nullptr))
            {
                FPS_ERROR_LOG("[%s] failed to send a fims get message", sys.id);
                fims_gateway.Close();
                return;
            }

            auto point_group_message = fims_gateway.Receive_Timeout(
                5000000);  // give them 5 seconds to respond before erroring
            if (!point_group_message)
            {
                FPS_ERROR_LOG("Failed to retrieve saved register data for [%s] from DBI", point_group_uri.c_str());
                continue;
            }
            if (!point_group_message->body)
            {
                FPS_ERROR_LOG("Failed to retrieve saved register data for [%s] from DBI", point_group_uri.c_str());
                continue;
            }
            bool success = parse_message_body(sys, point_group_uri, point_group_message->body);
            if (!success)
            {
                FPS_ERROR_LOG("Error parsing saved register data for [%s]", point_group_uri.c_str());
                continue;
            }
        }
    }
}

void load_points_from_dbi_client(GcomSystem& sys)
{
    fims& fims_gateway = sys.fims_dependencies->fims_gateway;

    for (auto uriPair : sys.dburiMap)
    {
        if (uriPair.second->multi)
        {
            std::string point_group_uri = uriPair.first;
            std::string dbi_uri = "/dbi/" + std::string(sys.id) + "/saved_registers" + point_group_uri;
            std::string replyto_uri = point_group_uri;
            if (!fims_gateway.Send("get", dbi_uri.c_str(), replyto_uri.c_str(), nullptr, nullptr))
            {
                FPS_ERROR_LOG("[%s] failed to send a fims get message", sys.id);
                fims_gateway.Close();
                return;
            }
            // standard fims listen thread should take care of the rest
        }
    }
}

void write_points_to_dbi_server(void* pSys)
{
    GcomSystem* sys = (GcomSystem*)pSys;
    TMWSIM_POINT* dbPoint;
    std::string key;
    fims& fims_gateway = sys->fims_dependencies->fims_gateway;
    fmt::memory_buffer& send_buf = sys->fims_dependencies->send_buf;

    sys->db_mutex.lock_shared();
    for (auto uriPair : sys->dburiMap)
    {
        if (uriPair.second->multi)
        {
            std::string point_group_uri = uriPair.first;
            std::string dbi_uri = "/dbi/" + std::string(sys->id) + "/saved_registers" + point_group_uri;
            bool has_one_point = false;
            send_buf.clear();
            send_buf.push_back('{');  // begin object

            for (auto key_point_pair : uriPair.second->dbmap)
            {
                key = key_point_pair.first;
                dbPoint = key_point_pair.second;
                if (has_one_point)
                {
                    FORMAT_TO_BUF(send_buf, R"(,)");
                }
                else
                {
                    has_one_point = true;
                }

                FORMAT_TO_BUF(send_buf, R"("{}":)", ((FlexPoint*)(dbPoint->flexPointHandle))->name);
                if (dbPoint->type == TMWSIM_TYPE_ANALOG)
                {
                    FORMAT_TO_BUF(send_buf, R"({:.{}g})", dbPoint->data.analog.value,
                                  std::numeric_limits<double>::max_digits10 - 1);
                }
                else if (dbPoint->type == TMWSIM_TYPE_COUNTER)
                {
                    FORMAT_TO_BUF(send_buf, R"({})", dbPoint->data.counter.value);
                }
                else if (dbPoint->type == TMWSIM_TYPE_BINARY)
                {
                    FORMAT_TO_BUF(send_buf, R"({})", static_cast<bool>(dbPoint->data.binary.value) ? 1 : 0);
                }
                else
                {
                    FORMAT_TO_BUF(send_buf, R"({})", dbPoint->data.analog.value);
                }
            }

            send_buf.push_back('}');  // begin object

            if (has_one_point)
            {
                if (!send_set(fims_gateway, dbi_uri.c_str(), std::string_view{ send_buf.data(), send_buf.size() }))
                {
                    FPS_ERROR_LOG("[%s] failed to send a fims set to DBI", sys->id);
                    fims_gateway.Close();
                    return;
                }
            }
        }
    }
    sys->db_mutex.unlock_shared();
    tmwtimer_start(&sys->dbi_save_timer, sys->dbi_save_frequency_seconds * 1000,
                   sys->protocol_dependencies->dnp3.pChannel, write_points_to_dbi_server, sys);
}

void write_points_to_dbi_client(void* pSys)
{
    GcomSystem* sys = (GcomSystem*)pSys;
    TMWSIM_POINT* dbPoint;
    std::string key;
    fims& fims_gateway = sys->fims_dependencies->fims_gateway;
    fmt::memory_buffer& send_buf = sys->fims_dependencies->send_buf;

    sys->db_mutex.lock_shared();
    for (auto uriPair : sys->dburiMap)
    {
        if (uriPair.second->multi)
        {
            std::string point_group_uri = uriPair.first;
            std::string dbi_uri = "/dbi/" + std::string(sys->id) + "/saved_registers" + point_group_uri;
            bool has_one_point = false;
            send_buf.clear();
            send_buf.push_back('{');  // begin object

            for (auto key_point_pair : uriPair.second->dbmap)
            {
                key = key_point_pair.first;
                dbPoint = key_point_pair.second;
                if (((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate)
                {
                    if (has_one_point)
                    {
                        FORMAT_TO_BUF(send_buf, R"(,)");
                    }
                    else
                    {
                        has_one_point = true;
                    }

                    FORMAT_TO_BUF(send_buf, R"("{}":)", ((FlexPoint*)(dbPoint->flexPointHandle))->name);
                    format_point_value(send_buf, dbPoint, ((FlexPoint*)(dbPoint->flexPointHandle))->operate_value);
                }
            }

            send_buf.push_back('}');  // begin object

            if (!send_set(fims_gateway, dbi_uri.c_str(), std::string_view{ send_buf.data(), send_buf.size() }))
            {
                FPS_ERROR_LOG("[%s] failed to send a fims set to DBI", sys->id);
                fims_gateway.Close();
                return;
            }
        }
    }
    sys->db_mutex.unlock_shared();
    tmwtimer_start(&sys->dbi_save_timer, sys->dbi_save_frequency_seconds * 1000,
                   sys->protocol_dependencies->dnp3.pChannel, write_points_to_dbi_client, sys);
}