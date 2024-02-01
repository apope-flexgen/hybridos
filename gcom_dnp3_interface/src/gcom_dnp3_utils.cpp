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

using namespace std;


bool spam_limit(GcomSystem *sys, int &max_errors)
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
const char *dreg_types[] = {"AnOPInt16", "AnOPInt32", "AnOPF32", "CROB", "analog", "binary", "analogOS", "binaryOS", 0};
/// @brief 
/// @param fp 
void deleteFlexPoint(void *fp)
{
    if (fp != nullptr)
    {
        if (((FlexPoint *)fp)->timeout_timer.pCallbackParam != nullptr)
        {
            tmwtimer_cancel(&((FlexPoint *)fp)->timeout_timer);
            if ((PointTimeoutStruct *)((FlexPoint *)fp)->timeout_timer.pCallbackParam != nullptr)
            {
                delete (PointTimeoutStruct *)((FlexPoint *)fp)->timeout_timer.pCallbackParam;
            }
        }
        if (((FlexPoint *)fp)->set_timer.active)
        {
            tmwtimer_cancel(&((FlexPoint *)fp)->set_timer);
        }
        if (((FlexPoint *)fp)->pub_timer.active)
        {
            tmwtimer_cancel(&((FlexPoint *)fp)->pub_timer);
        }
        delete (FlexPoint *)fp;
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
TMWSIM_POINT *newDbVar(GcomSystem &sys, const char *iname, int type, int offset, char *uri, char *variation)
{
    if (sys.protocol_dependencies->who == DNP3_OUTSTATION)
    {
        TMWSIM_POINT *dbPoint;
        FlexPoint *flexPoint = new FlexPoint(&sys, iname, uri);
        void *dbHandle = ((SDNPSESN *)(sys.protocol_dependencies->dnp3.pSession))->pDbHandle;
        if (type == AnIn16)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var2;
            dbPoint->defaultEventVariation = Group42Var2;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt16;
        }
        else if (type == AnIn32)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var1;
            dbPoint->defaultEventVariation = Group42Var1;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt32;
        }
        else if (type == AnF32)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0.0);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPF32;
        }
        else if (type == Type_AnalogOS)
        {
            while (sdnpsim_anlgOutGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addAnalogOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0.0);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnalogOS;
        }
        else if (type == Type_Analog)
        {
            while (sdnpsim_anlgInGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addAnalogInput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0, 0.0);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group30Var5;
            dbPoint->defaultEventVariation = Group32Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart++;
            flexPoint->type = Register_Types::Analog;
        }
        else if (type == Type_BinaryOS)
        {
            while (sdnpsim_binOutGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addBinaryOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0, 15);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group10Var1;
            dbPoint->defaultEventVariation = CROB;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::BinaryOS;
        }
        else if (type == Type_Binary)
        {
            while (sdnpsim_binInGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addBinaryInput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, false);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            dbPoint->defaultStaticVariation = Group1Var1;
            dbPoint->defaultEventVariation = Group2Var1;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart++;
            flexPoint->type = Register_Types::Binary;
        }
        else if (type == Type_Crob)
        {
            while (sdnpsim_binOutGetPoint(dbHandle, offset) == TMWDEFS_NULL) // if the point offset doesn't exist, add a new point
            {
                dbPoint = (TMWSIM_POINT *)sdnpsim_addBinaryOutput(dbHandle, 0, DNPDEFS_DBAS_FLAG_RESTART, 0, 15);
                dbPoint->enabled = false; // do this by default until we find the point we want
            }
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetPoint(dbHandle, offset);
            dbPoint->enabled = true;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::CROB;
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
        TMWSIM_POINT *dbPoint = nullptr;
        FlexPoint *flexPoint = new FlexPoint(&sys, iname, uri);
        void *dbHandle = ((MDNPSESN *)(sys.protocol_dependencies->dnp3.pSession))->pDbHandle;
        if (type == AnIn16)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var2;
            dbPoint->defaultEventVariation = Group42Var0;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt16;
        }
        else if (type == AnIn32)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var1;
            dbPoint->defaultEventVariation = Group42Var0;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPInt32;
        }
        else if (type == AnF32)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnOPF32;
        }
        else if (type == Type_AnalogOS)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_analogOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Output %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group40Var3;
            dbPoint->defaultEventVariation = Group42Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_outputs_restart++;
            flexPoint->type = Register_Types::AnalogOS;
        }
        else if (type == Type_Analog)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_analogInputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Analog Input %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            dbPoint->defaultStaticVariation = Group30Var5;
            dbPoint->defaultEventVariation = Group32Var5;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_analog_inputs_restart++;
            flexPoint->type = Register_Types::Analog;
        }
        else if (type == Type_BinaryOS)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for Binary Output %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::BinaryOS;
        }
        else if (type == Type_Binary)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_binaryInputAddPoint(dbHandle, offset);
            }
            dbPoint->defaultStaticVariation = Group1Var1;
            dbPoint->defaultEventVariation = Group2Var0;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_inputs_restart++;
            flexPoint->type = Register_Types::Binary;
        }
        else if (type == Type_Crob)
        {
            dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputLookupPoint(dbHandle, offset);
            if (dbPoint == TMWDEFS_NULL)
            {
                dbPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputAddPoint(dbHandle, offset);
            }
            if (dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                FPS_ERROR_LOG("Duplicate point offset detected for CROB %d", offset);
                delete (FlexPoint *)(dbPoint->flexPointHandle);
            }
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs++;
            sys.protocol_dependencies->dnp3.point_status_info->num_binary_outputs_restart++;
            flexPoint->type = Register_Types::CROB;
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
TMWSIM_POINT *getDbVar(GcomSystem &sys, std::string_view uri, std::string_view name)
{
    std::string suri = {uri.begin(), uri.end()};
    std::map<std::string, varList *>::iterator it = sys.dburiMap.find(suri);
    if (it != sys.dburiMap.end())
    {
        // dbvar_map
        auto dvar = it->second;
        auto dbm = dvar->dbmap;

        if (dvar->multi && name.size() > 0)
        {
            std::map<std::string, TMWSIM_POINT *>::iterator itd = dbm.find({name.begin(), name.end()});

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
            std::map<std::string, TMWSIM_POINT *>::iterator itd = dbm.find({name.begin(), name.end()});

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
void showNewUris(GcomSystem &sys)
{
    FPS_INFO_LOG(" %s uris===> \n\n", __FUNCTION__);

    std::map<std::string, varList *>::iterator it;
    std::map<std::string, TMWSIM_POINT *>::iterator itd;
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
int getSubs(GcomSystem &sys, const char **subs, int num, int who)
{
    if (num < (static_cast<int32_t>(sys.dburiMap.size()) + static_cast<int32_t>(sys.outputStatusUriMap.size())))
    {
        return (sys.dburiMap.size() + static_cast<int32_t>(sys.outputStatusUriMap.size()));
    }
    int idx = 0;
    if (subs)
    {
        std::map<std::string, varList *>::iterator it;
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
void addDbUri(GcomSystem &sys, const char *uri, TMWSIM_POINT *dbPoint)
{
    if (sys.dburiMap.find(uri) == sys.dburiMap.end())
    {
        sys.dburiMap[uri] = new varList_t(uri);
    }

    varList_t *vl = sys.dburiMap[uri];
    vl->multi = true;
    vl->addDb(dbPoint);

    std::string full_uri(uri);
    full_uri.append("/").append(((FlexPoint *)(dbPoint->flexPointHandle))->name);
    if (sys.dburiMap.find(full_uri.c_str()) == sys.dburiMap.end())
    {
        sys.dburiMap[full_uri.c_str()] = new varList_t(full_uri.c_str());
    }
    vl = sys.dburiMap[full_uri.c_str()];
    vl->multi = false;
    vl->addDb(dbPoint);
};

void addOutputStatusUri(GcomSystem &sys, const char *uri, TMWSIM_POINT *dbPoint)
{
    if (sys.outputStatusUriMap.find(uri) == sys.outputStatusUriMap.end())
    {
        sys.outputStatusUriMap[uri] = new varList_t(uri);
    }

    varList_t *vl = sys.outputStatusUriMap[uri];
    vl->multi = true;
    vl->addDb(dbPoint);

    std::string full_uri(uri);
    full_uri.append("/").append(((FlexPoint *)(dbPoint->flexPointHandle))->name);
    if (sys.outputStatusUriMap.find(full_uri.c_str()) == sys.outputStatusUriMap.end())
    {
        sys.outputStatusUriMap[full_uri.c_str()] = new varList_t(full_uri.c_str());
    }
    vl = sys.outputStatusUriMap[full_uri.c_str()];
    vl->multi = false;
    vl->addDb(dbPoint);
};
/// @brief 
/// @param sys 
/// @param who 
/// @return 
char *getDefUri(GcomSystem &sys, int who)
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
cJSON *get_config_json(int argc, char *argv[], int num)
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
cJSON *get_config_file(char *fname)
{
    char* newFname;
    if (std::strstr(fname, ".json") == nullptr) {
        // Calculate the length for the new string
        size_t fnameLen = std::strlen(fname);
        size_t extensionLen = std::strlen(".json");

        // Allocate memory for the new string (fname + ".json" + null terminator)
        newFname = new char[fnameLen + extensionLen + 1];

        // Copy the original fname to the new string
        std::strcpy(newFname, fname);

        // Append ".json" to the new string
        std::strcat(newFname, ".json");
    } else {
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

    cJSON *config = cJSON_Parse(buff.c_str());
    if (config == NULL)
        FPS_ERROR_LOG("Invalid JSON object in file");

    delete[] newFname;

    return config;
}
/// @brief 
/// @param t 
/// @return 
const char *iotypToStr(int t)
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
int iotypToId(const char *t)
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
int static_variation_decode(const char *ivar, int type)
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
int event_variation_decode(const char *ivar, int type)
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
cJSON *parse_files(cJSON *cji)
{
    return cJSON_GetObjectItem(cji, "files");
}
/// @brief 
/// @param cj 
/// @param name 
/// @param val 
/// @param required 
/// @return 
bool getCJint(cJSON *cj, const char *name, int &val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
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
bool getCJdouble(cJSON *cj, const char *name, double &val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
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
bool getCJbool(cJSON *cj, const char *name, bool &val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
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
bool getCJstr(cJSON *cj, const char *name, char *&val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
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
char *parseRegisterName(char *input)
{
    char *lastSlash = nullptr;
    char *current = input;

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
char *parseFullUri(char *input)
{
    char *firstSlash = strchr(input, '/');
    char *lastSlash = strrchr(input, '/'); // finds last occurrence of character

    if (firstSlash != nullptr && lastSlash != nullptr && lastSlash > firstSlash)
    {
        size_t length = lastSlash - firstSlash;
        char *result = new char[length + 1];
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
bool parse_system(cJSON *cji, GcomSystem &sys, int who)
{
    bool ret = true;
    bool final_ret = true;
    cJSON *cj = cJSON_GetObjectItem(cji, "system");

    if (cj == NULL)
    {
        FPS_ERROR_LOG("system  missing from file!");
        ret = false;
    }
    sys.protocol_dependencies->who = who; // DNP3_MASTER or DNP3_OUTSTATION

    // todo add different frequencies for each zone -- not there by default, but can be configured
    sys.protocol_dependencies->frequency = 1000; // default once a second
    sys.protocol_dependencies->port = 20000;
    sys.protocol_dependencies->deadband = 0.01;
    sys.protocol_dependencies->respTime = 30000; // default response time 30 Sec (TMW default)
    sys.protocol_dependencies->maxElapsed = 100; // default max task elapsed time before we print an error in the log
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

    char *tmp_uri = NULL;

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
        if (sys.protocol_dependencies->dnp3.freq1 == 0){
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
        if (sys.protocol_dependencies->dnp3.freq2 == 0){
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
        if (sys.protocol_dependencies->dnp3.freq3 == 0){
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
        if(sys.protocol_dependencies->timeout > 0){
            sys.protocol_dependencies->timeout *= 1000; // convert to milliseconds (e.g. timeout = 5 s ----> 5000 ms)
        }
        if(sys.protocol_dependencies->timeout == 0){
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
        char *tmp_fmt = nullptr;
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
            else // defaults to naked
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
        FPS_ERROR_LOG("Invalid  event buffer  %d  setting to default 100", sys.protocol_dependencies->dnp3.event_buffer);
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
        char *protocol_str = nullptr;
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
        char *conn_str = nullptr;
        ret = getCJstr(cj, "conn_type", conn_str, false);
        if (conn_str != nullptr)
        {
            if (strcmp(conn_str, "TCP") == 0)
            {
                sys.protocol_dependencies->conn_type = Conn_Type::TCP;
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
        ret = getCJint(cj, "baud", sys.protocol_dependencies->baud, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'baud' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJint(cj, "dataBits", sys.protocol_dependencies->dataBits, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'dataBits' must be an integer.");
            final_ret = false;
            ret = true;
        }
    }

    if (ret)
    {
        ret = getCJdouble(cj, "stopBits", sys.protocol_dependencies->stopBits, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'stopBits' must be a double.");
            final_ret = false;
            ret = true;
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
        ret = getCJstr(cj, "deviceName", sys.protocol_dependencies->deviceName, false);
        if (!ret)
        {
            FPS_ERROR_LOG("Error: 'deviceName' must be a string.");
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
        if(!sys.heartbeat){
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
        if(!sys.heartbeat->enabled){
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
        }else {
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
        if(!sys.watchdog->enabled){
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

    // fixup base_uri
    char tmp[1024];
    const char *sys_id = sys.id;

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

/// @brief 
/// @param sys 
/// @param dbPoint 
/// @param bit 
/// @return 
int addBit(GcomSystem &sys, TMWSIM_POINT *dbPoint, const char *bit)
{
    ((FlexPoint *)(dbPoint->flexPointHandle))->dbBits.push_back(std::make_pair(bit, 0));
    return static_cast<int32_t>(((FlexPoint *)(dbPoint->flexPointHandle))->dbBits.size());
};
/// @brief 
/// @param sys 
/// @param dbPoint 
/// @param bits 
/// @return 
int addBits(GcomSystem &sys, TMWSIM_POINT *dbPoint, cJSON *bits)
{
    int asiz = cJSON_GetArraySize(bits);
    for (int i = 0; i < asiz; i++)
    {
        cJSON *cji = cJSON_GetArrayItem(bits, i);
        // just use one copy of valuestring clean up will fix it.
        const char *bs = strdup(cji->valuestring);
        addBit(sys, dbPoint, bs);
        sys.bitsMap[bs] == std::make_pair(dbPoint, i);
    }
    return asiz;
}


/// @brief parse a map of items
/// @param sys 
/// @param objs 
/// @param type 
/// @param who 
/// @return 
bool parse_items(GcomSystem &sys, cJSON *objs, int type, int who)
{
    cJSON *obj;
    int mytype = type;

    cJSON_ArrayForEach(obj, objs)
    {
        cJSON *id, *offset, *uri, *bf, *bits, *variation;
        cJSON *evariation, *class_num, *rsize;//, *sign;
        cJSON *scale, *cjidx, *cjcfalse, *opvar, *deadband;
        cJSON *cjfmt, *cjtout;
        cJSON *cjcstring, *cjctrue, *cjcint, *event_pub;
        cJSON *batch_pub_rate, *interval_pub_rate, *batch_set_rate, *interval_set_rate;
        cJSON *show_output_status, *resend_tolerance, *resend_rate_ms, *output_status_uri;

        // heartbeat stuff
        // cJSON *min, *max, *incr;

        if (obj == NULL)
        {
            FPS_ERROR_LOG("Invalid or NULL obj");
            continue;
        }

        // min = cJSON_GetObjectItem(obj, "min");
        // max = cJSON_GetObjectItem(obj, "max");
        // incr = cJSON_GetObjectItem(obj, "incr");
        // note that id translates to name
        id = cJSON_GetObjectItem(obj, "id");
        cjidx = cJSON_GetObjectItem(obj, "idx");
        offset = cJSON_GetObjectItem(obj, "offset");
        rsize = cJSON_GetObjectItem(obj, "size");
        variation = cJSON_GetObjectItem(obj, "variation");
        evariation = cJSON_GetObjectItem(obj, "evariation");
        uri = cJSON_GetObjectItem(obj, "uri");
        bf = cJSON_GetObjectItem(obj, "bit_field");
        bits = cJSON_GetObjectItem(obj, "bit_strings");
        // linkback = cJSON_GetObjectItem(obj, "linkback");
        // linkuri = cJSON_GetObjectItem(obj, "linkuri");
        class_num = cJSON_GetObjectItem(obj, "class");
        if(!class_num){
            class_num = cJSON_GetObjectItem(obj, "clazz");
        }
        //sign = cJSON_GetObjectItem(obj, "signed");
        scale = cJSON_GetObjectItem(obj, "scale");
        deadband = cJSON_GetObjectItem(obj, "deadband");
        opvar = cJSON_GetObjectItem(obj, "OPvar"); // 1 = 16 bit , 2 = 32 bit , 3 float32
        cjfmt = cJSON_GetObjectItem(obj, "format");
        cjtout = cJSON_GetObjectItem(obj, "timeout");
        // allows more detailed manipulation of the crob type
        cjcstring = cJSON_GetObjectItem(obj, "crob_string"); // true / false
        cjctrue = cJSON_GetObjectItem(obj, "crob_true");     // string like POWER_ON
        cjcfalse = cJSON_GetObjectItem(obj, "crob_false");   // string like POWER_OFF
        cjcint = cJSON_GetObjectItem(obj, "crob_int");       // true / false
        batch_set_rate = cJSON_GetObjectItem(obj, "batch_set_rate");
        interval_set_rate = cJSON_GetObjectItem(obj, "interval_set_rate");
        batch_pub_rate = cJSON_GetObjectItem(obj, "batch_pub_rate");
        interval_pub_rate = cJSON_GetObjectItem(obj, "interval_pub_rate");
        event_pub = cJSON_GetObjectItem(obj, "event_pub");
        show_output_status = cJSON_GetObjectItem(obj, "show_output_status");
        output_status_uri = cJSON_GetObjectItem(obj, "output_status_uri");
        resend_tolerance = cJSON_GetObjectItem(obj, "resend_tolerance");
        resend_rate_ms = cJSON_GetObjectItem(obj, "resend_rate");

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
        // The return path from outstation to master still has type an index. The system has changed to use the offset from the conig file here too.
        // so we can no longer use the vector position as an index.
        // items are added from the config file against a registered uri ot the default one.
        // pubs, gets and sets from FIMS ues a combination of URI + var name to find vars.
        // data incoming from master to outstation will use type and offset to find the variable.
        // each type will have a map <int,TMWSIM_POINT *> to search
        // Data base for the outstation vars (analog and binary) will no longer use the vector size but max offset as dbPoint size ????
        // the <int , TMWSIM_POINT *> map can also  be used for these vars
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
                FPS_DEBUG_LOG("****** AF32 variable [%s] vidx %d",
                              id->valuestring, vidx);
        }
        if (sys.debug)
            FPS_DEBUG_LOG("****** mtype %d xxvariable [%s] vidx %d debug %d",
                          mytype, id->valuestring, vidx, sys.debug);
        // set up name uri
        char *nuri = getDefUri(sys, who);
        if (uri && uri->valuestring)
        {
            nuri = uri->valuestring;
        }

        TMWSIM_POINT *dbPoint = newDbVar(sys, id->valuestring, mytype, vidx,
                                         nuri, variation ? variation->valuestring : NULL);
        if (dbPoint != NULL)
        {
            size_t id_len = strlen(id->valuestring);
            if (sys.watchdog && (id_len == strlen(sys.watchdog->id)) && (strcmp(id->valuestring, sys.watchdog->id) == 0)){
                sys.watchdog->io_point = dbPoint;
            }
            if (sys.heartbeat && (id_len == strlen(sys.heartbeat->heartbeat_read_uri)) && (strcmp(id->valuestring, sys.heartbeat->heartbeat_read_uri) == 0)){
                sys.heartbeat->heartbeat_read_point = dbPoint;
            }
            if (sys.heartbeat && (id_len == strlen(sys.heartbeat->heartbeat_write_uri)) && (strcmp(id->valuestring, sys.heartbeat->heartbeat_write_uri) == 0)){
                sys.heartbeat->heartbeat_write_point = dbPoint;
            }
            ((FlexPoint *)(dbPoint->flexPointHandle))->format = sys.fims_dependencies->format;

            if (event_pub)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub = cJSON_IsTrue(event_pub);
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub = sys.protocol_dependencies->dnp3.event_pub;
            }

            if (show_output_status && (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::CROB))
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->show_output_status = cJSON_IsTrue(show_output_status);
                if(output_status_uri) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->output_status_uri = strdup(output_status_uri->valuestring);
                } else if (sys.protocol_dependencies->dnp3.output_status_uri) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->output_status_uri = strdup(sys.protocol_dependencies->dnp3.output_status_uri);
                }
            } else if (sys.protocol_dependencies->dnp3.show_output_status && (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::CROB)){
                ((FlexPoint *)(dbPoint->flexPointHandle))->show_output_status = sys.protocol_dependencies->dnp3.show_output_status;
                if(output_status_uri) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->output_status_uri = strdup(output_status_uri->valuestring);
                } else if (sys.protocol_dependencies->dnp3.output_status_uri) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->output_status_uri = strdup(sys.protocol_dependencies->dnp3.output_status_uri);
                }
            }
            if(((FlexPoint *)(dbPoint->flexPointHandle))->output_status_uri){
                addOutputStatusUri(sys, ((FlexPoint *)(dbPoint->flexPointHandle))->output_status_uri, dbPoint);
            } else {
                sys.protocol_dependencies->dnp3.show_output_status = false;
            }

            if (resend_tolerance && (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::CROB))
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->resend_tolerance = resend_tolerance->valuedouble;
                if(resend_rate_ms) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->resend_rate_ms = resend_rate_ms->valuedouble;
                } else if (sys.protocol_dependencies->dnp3.resend_rate_ms) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->resend_rate_ms = sys.protocol_dependencies->dnp3.resend_rate_ms;
                }
            } else if (sys.protocol_dependencies->dnp3.show_output_status && (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
            ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::CROB)){
                ((FlexPoint *)(dbPoint->flexPointHandle))->resend_tolerance = sys.protocol_dependencies->dnp3.resend_tolerance;
                if(resend_rate_ms) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->resend_rate_ms = resend_rate_ms->valuedouble;
                } else if (sys.protocol_dependencies->dnp3.resend_rate_ms) {
                    ((FlexPoint *)(dbPoint->flexPointHandle))->resend_rate_ms = sys.protocol_dependencies->dnp3.resend_rate_ms;
                }
            }

            // batch set rate
            if (batch_set_rate && batch_set_rate->valueint > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate = batch_set_rate->valueint;
            }
            else if (sys.protocol_dependencies->dnp3.batch_set_rate > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate = sys.protocol_dependencies->dnp3.batch_set_rate;
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets = false;
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_set_rate = 0;
            }

            // interval set rate
            if (interval_set_rate && interval_set_rate->valueint > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate = interval_set_rate->valueint;
            }
            else if (sys.protocol_dependencies->dnp3.interval_set_rate > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate = sys.protocol_dependencies->dnp3.interval_set_rate;
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets = false;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_set_rate = 0;
            }

            // batch pub rate
            if (batch_pub_rate && batch_pub_rate->valueint > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate = batch_pub_rate->valueint;
            }
            else if (sys.protocol_dependencies->dnp3.batch_pub_rate > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate = sys.protocol_dependencies->dnp3.batch_pub_rate;
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs = false;
                ((FlexPoint *)(dbPoint->flexPointHandle))->batch_pub_rate = 0;
            }

            // interval pub rate
            if (interval_pub_rate && interval_pub_rate->valueint > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate = interval_pub_rate->valueint;
                if(sys.heartbeat && sys.heartbeat->heartbeat_read_point == dbPoint){
                    ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub = true;
                }
            }
            else if (sys.protocol_dependencies->dnp3.interval_pub_rate > 0)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate = sys.protocol_dependencies->dnp3.interval_pub_rate;
                if(sys.heartbeat && sys.heartbeat->heartbeat_read_point == dbPoint){
                    ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub = true;
                }
            }
            else if (sys.heartbeat && sys.heartbeat->frequency > 0 && sys.heartbeat->heartbeat_read_point == dbPoint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs = true;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate = sys.heartbeat->frequency;
                ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub = true;
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs = false;
                ((FlexPoint *)(dbPoint->flexPointHandle))->interval_pub_rate = 0;
            }

            if (!((FlexPoint *)(dbPoint->flexPointHandle))->interval_pubs && !((FlexPoint *)(dbPoint->flexPointHandle))->batch_pubs)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->direct_pubs = true;
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->direct_pubs = false;
            }

            if (!((FlexPoint *)(dbPoint->flexPointHandle))->interval_sets && !((FlexPoint *)(dbPoint->flexPointHandle))->batch_sets)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->direct_sets = true;
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->direct_sets = false;
            }

            if (cjfmt)
            {
                if (cjfmt->valuestring)
                {
                    std::string fmt = cjfmt->valuestring;
                    if (fmt == "clothed")
                    {
                        ((FlexPoint *)(dbPoint->flexPointHandle))->format = FimsFormat::Clothed;
                    }
                    else if (fmt == "full")
                    {
                        ((FlexPoint *)(dbPoint->flexPointHandle))->format = FimsFormat::Full;
                    }
                    else // defaults to naked
                    {
                        ((FlexPoint *)(dbPoint->flexPointHandle))->format = FimsFormat::Naked;
                    }
                }
            }
            if (cjcstring)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_string = (cjcstring->type == cJSON_True);
            }

            if (cjcint)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_int = (cjcint->type == cJSON_True);
            }

            if (cjctrue && cjctrue->valuestring)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_true = strdup(cjctrue->valuestring);
            }

            if (cjcfalse && cjcfalse->valuestring)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->crob_false = strdup(cjcfalse->valuestring);
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
            } else {
                dbPoint->data.analog.deadband = sys.protocol_dependencies->deadband;
            }

            if (cjtout)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->timeout = cjtout->valuedouble;
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->timeout = sys.protocol_dependencies->timeout;
            }
            tmwtimer_init(&((FlexPoint *)dbPoint->flexPointHandle)->timeout_timer);
            // if (((FlexPoint *)(dbPoint->flexPointHandle))->timeout == 0)
            // {
            //     dbPoint->flags = 0;
            // }
            // else
            // {
            //     dbPoint->flags = 1;
            // }

            // for batching pubs
            tmwtimer_init(&((FlexPoint *)(dbPoint->flexPointHandle))->pub_timer);

            // set work stuff
            tmwtimer_init(&((FlexPoint *)(dbPoint->flexPointHandle))->set_timer);
            ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_uri.clear();
            FORMAT_TO_BUF(((FlexPoint *)(dbPoint->flexPointHandle))->set_work.send_uri, R"({}/{})", ((FlexPoint *)(dbPoint->flexPointHandle))->uri, ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
            ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.dbPoint = dbPoint;
            ((FlexPoint *)(dbPoint->flexPointHandle))->set_work.value = 0.0;

            if (class_num)
            {
                if (class_num->valueint >= 1)
                    dbPoint->classMask = 1 << (class_num->valueint - 1); // can be any value from 1 to 3
                else
                    dbPoint->classMask = 0;
                if (sys.debug)
                    FPS_DEBUG_LOG("****** variable [%s] set to class_num %d", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), dbPoint->classMask);
            }

            // if (sign)
            // {
            //     ((FlexPoint *)(dbPoint->flexPointHandle))->sign = (sign->type == cJSON_True);
            //     if (sys.debug)
            //         FPS_DEBUG_LOG("****** variable [%s] set to signed %d", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), ((FlexPoint *)(dbPoint->flexPointHandle))->sign);
            // }
            // else
            // {
            //     ((FlexPoint *)(dbPoint->flexPointHandle))->sign = sys.protocol_dependencies->dnp3.sign;
            //     if (sys.debug)
            //         FPS_DEBUG_LOG("****** variable [%s] set to signed %d", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), ((FlexPoint *)(dbPoint->flexPointHandle))->sign);
            // }

            if (scale)
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->scale = (scale->valuedouble);
                if (sys.debug)

                    FPS_DEBUG_LOG("****** variable [%s] scale set %f", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str(), ((FlexPoint *)(dbPoint->flexPointHandle))->scale);
            }
            else
            {
                ((FlexPoint *)(dbPoint->flexPointHandle))->scale = 0;
            }
            if (bf && bits && (bits->type == cJSON_Array))
            {
                // TODO bits need a value use make_pair there too
                if (sys.debug)
                    FPS_DEBUG_LOG("*****************Adding bitfields for %s", ((FlexPoint *)(dbPoint->flexPointHandle))->name.c_str());
                addBits(sys, dbPoint, bits);
            }

            if (sys.debug)
                FPS_DEBUG_LOG(" config adding name [%s] idx [%d]", id->valuestring, vidx);
            // was nuri

            addDbUri(sys, ((FlexPoint *)(dbPoint->flexPointHandle))->uri, dbPoint);
        }
    }
    return true;
}

int parse_object(GcomSystem &sys, cJSON *objs, int idx, int who)
{
    cJSON *cjlist = cJSON_GetObjectItem(objs, iotypToStr(idx));
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
bool parse_modbus(cJSON *cj, GcomSystem &sys, int who)
{
    // config file has "objects" with children groups "binary" and "analog"
    // who is needd to stop cross referencing linkvars
    cJSON *cji;
    cJSON_ArrayForEach(cji, cj)
    {
        cJSON *cjmap = cJSON_GetObjectItem(cji, "map");
        cJSON *cjdtype = cJSON_GetObjectItem(cji, "dnp3_type");
        cJSON *cjtype = cJSON_GetObjectItem(cji, "type");
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
bool parse_variables(cJSON *object, GcomSystem &sys, int who)
{
    // config file has "objects" with children groups "binary" and "analog"
    // who is needd to stop cross referencing linkvars
    cJSON *cjregs = cJSON_GetObjectItem(object, "registers");
    if (cjregs != NULL)
    {
        return parse_modbus(cjregs, sys, who);
    }

    cJSON *JSON_objects = cJSON_GetObjectItem(object, "objects");
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
int getSysUris(GcomSystem &sys, int who, int nums)
{
    int num_subs = 0;
    int i;
    // sysCfg** ss = sys;
    for (i = 0; i < nums; i++)
    {
        num_subs += getSubs(sys, NULL, 0, who);
    }
    // int num = sys.protocol_dependencies->getSubs(NULL, 0, who);
    const char **subs = (const char **)malloc((num_subs + 3) * sizeof(char *));
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
bool run_config(cJSON *config, GcomSystem &sys, int &num_configs, int who)
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
void init_point_groups(GcomSystem &sys)
{
    TMWSIM_POINT *dbPoint;
    DNP3Dependencies *dnp3_sys = &(sys.protocol_dependencies->dnp3);

    if (sys.protocol_dependencies->who == DNP3_OUTSTATION)
    {
        dnp3_sys->point_group_map["/analog_outputs"] = PointGroup{};
        PointGroup &point_group_1 = dnp3_sys->point_group_map["/analog_outputs"];
        point_group_1.group_name = "analog_outputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgOutGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_1.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/binary_outputs"] = PointGroup{};
        PointGroup &point_group_2 = dnp3_sys->point_group_map["/binary_outputs"];
        point_group_2.group_name = "binary_outputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binOutGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_2.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/analog_inputs"] = PointGroup{};
        PointGroup &point_group_3 = dnp3_sys->point_group_map["/analog_inputs"];
        point_group_3.group_name = "analog_inputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_anlgInGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_3.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
        dnp3_sys->point_group_map["/binary_inputs"] = PointGroup{};
        PointGroup &point_group_4 = dnp3_sys->point_group_map["/binary_inputs"];
        point_group_4.group_name = "binary_inputs";
        for (uint i = 0; i < tmwsim_tableSize(&((SDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            dbPoint = (TMWSIM_POINT *)sdnpsim_binInGetPoint(dnp3_sys->dbHandle, i);
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_4.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
        }
    }
    else
    {
        dnp3_sys->point_group_map["/analog_outputs"] = PointGroup{};
        PointGroup &point_group_1 = dnp3_sys->point_group_map["/analog_outputs"];
        point_group_1.group_name = "analog_outputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_1.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogOutputs, dbPoint);
        }
        dnp3_sys->point_group_map["/binary_outputs"] = PointGroup{};
        PointGroup &point_group_2 = dnp3_sys->point_group_map["/binary_outputs"];
        point_group_2.group_name = "binary_outputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_2.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryOutputs, dbPoint);
        }
        dnp3_sys->point_group_map["/analog_inputs"] = PointGroup{};
        PointGroup &point_group_3 = dnp3_sys->point_group_map["/analog_inputs"];
        point_group_3.group_name = "analog_inputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_3.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->analogInputs, dbPoint);
        }
        dnp3_sys->point_group_map["/binary_inputs"] = PointGroup{};
        PointGroup &point_group_4 = dnp3_sys->point_group_map["/binary_inputs"];
        point_group_4.group_name = "binary_inputs";
        dbPoint = tmwsim_tableGetFirstPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs);
        for (uint i = 0; i < tmwsim_tableSize(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs); i++)
        {
            if (dbPoint && dbPoint->flexPointHandle != TMWDEFS_NULL)
            {
                point_group_4.points[((FlexPoint *)dbPoint->flexPointHandle)->name] = dbPoint;
            }
            dbPoint = tmwsim_tableGetNextPoint(&((MDNPSIM_DATABASE *)(dnp3_sys->dbHandle))->binaryInputs, dbPoint);
        }
    }
}
/// @brief 
/// @param sys 
/// @param who 
/// @return 
bool init_vars(GcomSystem &sys, int who)
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
/// @brief 
/// @param argc 
/// @param argv 
/// @param sys 
/// @param who 
/// @return 
int getConfigs(int argc, char *argv[], GcomSystem &sys, int who)
{
    int num_configs = 0;
    while (true)
    {

        cJSON *config = get_config_json(argc, argv, num_configs);
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
        cJSON *cjfiles = parse_files(config);
        if (cjfiles)
        {
            cJSON *cjfile;
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
    return num_configs;
}
/// @brief 
/// @param argc 
/// @param argv 
/// @return 
std::string getFileName(int argc, char *argv[])
{
    std::string file_name;
    if (argc > 1)
    {
        file_name = std::string(argv[1]);
        std::size_t last_slash = file_name.find_last_of("/\\");
        file_name = file_name.substr(last_slash + 1);
        std::size_t extension = file_name.find(".json");
        file_name = file_name.substr(0, extension); // not entirely sure why I had to do this in two steps, but trying to do it in one step didn't work
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
double get_time_double() {
    std::lock_guard<std::mutex> lock(timeDoubleMutex);
    if (!timeInit)
        baseTime = std::chrono::system_clock::now();
    timeInit =  true;
    std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = currentTime - baseTime;
    return duration.count();
}
/// @brief 
/// @param tmpPtr 
/// @return 
double get_time_double(TMWDTIME &tmpPtr){
    std::lock_guard<std::mutex> lock(timeDoubleMutex);
    if (!timeInit)
        baseTime = std::chrono::system_clock::now();
    timeInit =  true;
    std::tm tm = {};
    tm.tm_year = tmpPtr.year - 1900;
    tm.tm_mon = tmpPtr.month - 1;
    tm.tm_mday = tmpPtr.dayOfMonth;
    tm.tm_hour = tmpPtr.hour;
    tm.tm_min = tmpPtr.minutes;
    tm.tm_sec = tmpPtr.mSecsAndSecs / 1000;
    auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    std::chrono::duration<double> duration = time_point - baseTime;
    return duration.count() + (tmpPtr.mSecsAndSecs % 1000)/1000.0;
}