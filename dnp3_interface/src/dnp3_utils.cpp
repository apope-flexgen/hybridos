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

#include "dnp3_utils.h"
#include <map>
#include <string>
#include <cstring>
#include "opendnp3/app/AnalogOutput.h"
#include "opendnp3/app/ControlRelayOutputBlock.h"
#include "opendnp3/gen/CommandStatus.h"
#include "opendnp3/gen/OperationType.h"

using namespace opendnp3;

using namespace std;

// So we have to discover the groups 
// these things are outputs 

// ControlRelayOutputBlock>& StartHeaderCROB();    Group12Var1
// AnalogOutputInt32>& StartHeaderAOInt32();       Group41Var1
// AnalogOutputInt16>& StartHeaderAOInt16();       Group41Var2
// AnalogOutputFloat32>& StartHeaderAOFloat32();   Group41Var3
// AnalogOutputDouble64>& StartHeaderAODouble64(); Group41Var4



// binary and analog
// if it is not in the soe handler we'll have to use configs
// // Binary Input - Any Variation             Group1Var0
// // Binary Input - Packed Format             Group1Var1
// // Binary Input - With Flags                Group1Var2

// // Binary Input Event - Any Variation       Group2Var0
// // Binary Input Event - Without Time        Group2Var1
// // Binary Input Event - With Absolute Time  Group2Var2
// // Binary Input Event - With Relative Time  Group2Var3

// group 3 & 4 DoubleBit binaries
// group 10 & 11 Binary Output
// group 20 & 21  counters
// Binary Input - Any Variation
// Binary Output - Any Variation  Group10Var0
// Binary Output - Packed Format  Group10Var1


// Internal Indications -

// uses unsigned int to extend the range for int
bool isNumber (const char *line)
{
    if (isdigit(atoi(line)))
        return true;
    return false;
}

const char *getVersion()
{
    return  DNP3_UTILS_VERSION;
}

std::string ToUTCString(const DNPTime& dnptime)
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

UTCTimestamp Now()
{
    uint64_t time
        = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
              .count();
    return {time};
}

double getF32Val(DbVar* db)
{
    double dval = db->valuedouble;

    if ((db->scale > 0.0) || (db->scale < 0.0))
    {
        dval *= db->scale;
    }
    return dval;
}
// uses unsigned int to extend the range for int
int32_t getInt32Val(DbVar* db)
{
    int32_t ival; 
    double dval = getF32Val(db);
    
    ival = static_cast<int32_t>(dval); 
    if(db->sign == 1)
    {
        if (dval > INT_MAX)
        {
            ival = INT_MAX;
        }
        else if (dval < INT_MIN)
        {
            ival = INT_MIN;
        }
    }
    else
    {      
        if (dval >= UINT_MAX)
        {
            ival = INT_MIN;  // this results in USHRT_MAX being applied to the output
        }
        else if (dval > INT_MAX)
        {
            ival = -(dval - INT_MAX);  // gives us a negative but corret value
        }
        else if (dval < 0.0)
        {
            ival = 0;
        }
    //    if (dval > UINT_MAX)
    //     {
    //         ival = UINT_MAX;
    //     }
    //     else if (dval < 0)
    //     {
    //          ival = 0;
    //     }
    }
    return ival;
}
// uses unsigned int to extend the range for int
int16_t getInt16Val(DbVar* db)
{
    int16_t ival; 
    double dval = getF32Val(db);
    
    ival = static_cast<int16_t>(dval); 
    if(db->sign == 1)
    {
        if (dval > SHRT_MAX)
        {
            ival = SHRT_MAX;
        }
        else if (dval < SHRT_MIN)
        {
            ival = SHRT_MIN;
        }
    }
    else
    {
        // USHRT
        if (dval >= USHRT_MAX)
        {
            ival = SHRT_MIN;  // this results in USHRT_MAX being applied to the output
        }
        else if (dval > SHRT_MAX)
        {
            ival = -(dval - SHRT_MAX);  // gives us a negative but corret value
        }
        else if (dval < 0.0)
        {
            ival = 0;
        }
    }
    if(0)FPS_ERROR_PRINT(" %s dval %f  sign %d int16_t %d  ival %d \n",
             __FUNCTION__ ,dval, db->sign, static_cast<int16_t>(dval), ival);
    return ival;
}

bool extractInt32Val(double &dval, DbVar* db)
{
    bool flag = false;
    dval = db->valuedouble;
    if(db->sign == 0)
    {
        flag = true;
        if  (dval < 0.0)
        {
            //dval = -(static_cast<double>(INT_MIN) + db->valuedouble);
            dval = 0;
        }
        else
        {
            if(dval > UINT_MAX)
                dval = UINT_MAX;
        }
    }
    else
    {
        if  (dval < INT_MIN)
        {
            dval = INT_MIN;
        }
        else if (dval > INT_MAX)
        {
            dval = INT_MAX;
        }
    }
    return flag;
}

bool extractInt16Val(double &dval, DbVar* db)
{
    bool flag = false;
    dval = db->valuedouble;
    if(db->sign == 0)
    {
        flag = true;
        if  (dval < 0.0)
        {
            //dval = -(static_cast<double>(INT_MIN) + db->valuedouble);
            dval = 0;
        }
        else
        {
            if(dval > USHRT_MAX)
                dval = USHRT_MAX;
        }        
    }
    else
    {
        if  (dval < SHRT_MIN)
        {
            dval = SHRT_MIN;
        }
        else if (dval > SHRT_MAX)
        {
            dval = SHRT_MAX;
        }
    }
    return flag;
}


// bypass message filter for opendpn3 messages
void emit_event(sysCfg* sys, const char* source, const char* message, int severity)
{
    fims*pFims =  sys->p_fims;
    cJSON* cj = cJSON_CreateObject();
    if(cj)
    {

        if (source == nullptr) 
        {
            char sysid[1024];
            snprintf(sysid, sizeof(sysid), "DNP3_%s:%s"
                    , sys->master ? "client":"server", sys->id 
                    );
            cJSON_AddStringToObject(cj, "source", sysid);
        }
        else
        {
            cJSON_AddStringToObject(cj, "source", source);
        }

        cJSON_AddStringToObject(cj, "message", message);
        cJSON_AddNumberToObject(cj, "severity", severity);
        char* body = cJSON_PrintUnformatted(cj);
        if(body && pFims)
        {                
            pFims->Send("post", "/events", NULL, body);
            free(body);
        }
        cJSON_Delete(cj);
    }
}

// this is used to  filter the flood of messages from openDnp3 
void emit_event_filt(sysCfg* sys, const char* source, const char* message, int severity)
{
    if (
            (strstr(message,"[CLOSED]") != NULL) 
        || (strstr(message,"[OPENING]") != NULL)
        || (strstr(message,"[OPEN]") != NULL)
        || (strstr(message,"[SHUTDOWN]") != NULL)
        || (strstr(message,"- End of file") != NULL)
        || (sys->debug > 2)
    ) 
    {
        emit_event(sys, source, message, severity);
    }
}

DbVar* getDbVar(sysCfg* sys, const char* uri, const char* name)
{
    return sys->getDbVar(uri, name);
}

//uses the ev field
void pubWithTimeStamp(cJSON* cj, sysCfg* sys, const char* ev)
{
    if(cj)
    {    
        addCjTimestamp(cj, "Timestamp");
        char* out = cJSON_PrintUnformatted(cj);
    
        if (out) 
        {
            char tmp[1024];
            if(ev == NULL) 
            {
                snprintf(tmp,1024,"/%s/%s", "/components", sys->id);

            }
            else
            {
                snprintf(tmp,1024,"/%s/%s", ev, sys->id);
            }

            if(sys->p_fims)
            {
               sys->p_fims->Send("pub", tmp, NULL, out);
            }
            else
            {
                std::cout << __FUNCTION__ << " Error in sys->p_fims\n";
            }       
            free(out);
        }
    }
}

void addCjTimestamp(cJSON* cj, const char* ts)
{
    char buffer[64],time_str[256];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm local_tm;
    tzset();
    strftime(buffer,sizeof(buffer),"%m-%d-%Y %T.", localtime_r(static_cast<time_t*>(&tv.tv_sec), &local_tm));
    snprintf(time_str, sizeof (time_str),"%s%ld",buffer,tv.tv_usec);
    cJSON_AddStringToObject(cj, ts, time_str);
}

cJSON* get_config_file(char* fname);

cJSON* get_config_json(int argc, char* argv[], int num)
{
    if(argc <= 1)
    {
        FPS_ERROR_PRINT("Need to pass argument for config file name.\n");
        return NULL;
    }
    if(argc <= num+1)
    {
        FPS_ERROR_PRINT("Done with configs at num %d\n", num);
         return NULL;
     }

    if (argv[num+1] == NULL)
    {
        FPS_ERROR_PRINT(" Failed to get the path of the test file. \n");
        return NULL;
    }
    FPS_ERROR_PRINT(" getting arg %d . \n", num+1);
    return get_config_file(argv[num+1]);
}


cJSON* get_config_file(char* fname)
{
    std::ifstream t(fname);
    if(!t)
    {
        FPS_ERROR_PRINT(" failed to read file  %s . \n", fname);
        return nullptr;
    }

    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buff(size, ' ');
    t.seekg(0);
    t.read(&buff[0], size); 

    cJSON* config = cJSON_Parse(buff.c_str());
    if(config == NULL)
        fprintf(stderr, "Invalid JSON object in file\n");
    return config;
}

// new parser for sample2
cJSON* parseJSONConfig(char* file_path)
{
    //Open a file//
    if (file_path == NULL)
    {
        FPS_ERROR_PRINT(" Failed to get the path of the test file. \n"); //a check to make sure that args[1] is not NULL//
        return NULL;
    }
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        FPS_ERROR_PRINT("Failed to open file %s\n", file_path);
        return NULL;
    }
    else FPS_DEBUG_PRINT("Opened file %s\n", file_path);
    // obtain file size

    fseek(fp, 0, SEEK_END);
    long unsigned file_size = ftell(fp);
    rewind(fp);
    // create configuration file

    char* configFile = new char[file_size];
    if (configFile == NULL)
    {
        FPS_ERROR_PRINT("Memory allocation error config file\n");
        fclose(fp);
        return NULL;
    }
    size_t bytes_read = fread(configFile, 1, file_size, fp);
    if (bytes_read != file_size)
    {
        FPS_ERROR_PRINT("Read error: read: %lu, file size: %lu .\n", (unsigned long)bytes_read, (unsigned long)file_size);
        fclose(fp);
        delete[] configFile;
        return NULL;
    }
    else
       FPS_DEBUG_PRINT("File size %lu\n", file_size);

    fclose(fp);
    cJSON* pJsonRoot = cJSON_Parse(configFile);
    delete[] configFile;
    if (pJsonRoot == NULL)
        FPS_ERROR_PRINT("cJSON_Parse returned NULL.\n");
    return(pJsonRoot);
}


OperationType TypeToOperationType(uint8_t arg)
{
    return static_cast<OperationType>(arg);
}

void addFullVal(cJSON* cji, DbVar* db)
{
    cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
    cJSON_AddNumberToObject(cji, "flags", db->flags);
    if (db->stime.length() > 0)
    {
        cJSON_AddStringToObject(cji, "stime", db->stime.c_str());
    }    
    if (db->ltime.length() > 0)
    {
        cJSON_AddStringToObject(cji, "ltime", db->ltime.c_str());
    }
    if (db->etime.length() > 0)
    {
        cJSON_AddStringToObject(cji, "etime", db->etime.c_str());
    }

}

// decode format but let uri flags override
int getDbFmt(sysCfg* sys, DbVar* db, int flag)
{
    auto fmt = sys->fmt;
    if (db->format)
    {
        fmt = db->fmt;
    }
    if ((flag & URI_FLAG_FULL) == URI_FLAG_FULL)
    {
        fmt = 2;
    }
    if ((flag & URI_FLAG_CLOTHED) == URI_FLAG_CLOTHED)
    {
        fmt = 1;
    }
    if ((flag & URI_FLAG_NAKED) == URI_FLAG_NAKED)
    {
        fmt = 0;
    }
    return fmt;

}
void addSysCjVal(sysCfg* sys, cJSON* cj, DbVar* db, int flag, double val)
{
    auto fmt = getDbFmt(sys,db,flag);

    if (fmt == 0)
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
    else if (fmt > 0)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

void addSysCjVal(sysCfg* sys, cJSON* cj, DbVar* db,  int flag, uint32_t val)
{
    auto fmt = getDbFmt(sys,db,flag);

    if (fmt == 0)
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
    else if(fmt > 0)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

void addSysCjVal(sysCfg* sys,cJSON* cj, DbVar* db,  int flag, int32_t val)
{
    auto fmt = getDbFmt(sys,db,flag);

    if (fmt == 0)
    {

        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
    else if (fmt > 0)
    {

        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

void addSysCjVal(sysCfg* sys, cJSON* cj, DbVar* db,  int flag, uint16_t val)
{
    auto fmt = getDbFmt(sys,db,flag);

    if (fmt == 0)
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
    else if (fmt > 0)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

void addSysCjVal(sysCfg* sys, cJSON* cj, DbVar* db,  int flag, int16_t val)
{
    auto fmt = getDbFmt(sys,db,flag);

    if (fmt == 0)
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
    else if (fmt > 0)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

void addSysCjVal(sysCfg* sys, cJSON* cj, DbVar* db, int flag, bool val)
{
    auto fmt = getDbFmt(sys,db,flag);

    if (fmt == 0)
    {
        cJSON_AddBoolToObject(cj, db->name.c_str(), val);

    }
    if(fmt>0)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddBoolToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

void addSysCjVal(sysCfg* sys, cJSON* cj, DbVar* db, int flag, const char* val)
{
   auto fmt = getDbFmt(sys,db,flag);
   if (fmt == 0)
    {
        cJSON_AddStringToObject(cj, db->name.c_str(), val);
    }
    if(fmt > 0)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddStringToObject(cji, "value", val);
        if (fmt > 1)
        {
            addFullVal(cji, db);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
}

// This is the root function
int addVarToCj(sysCfg*sys, cJSON* cj,  DbVar* db, int flag)
{
    int rc = 0;
    // always use the value from the actual compoent 
    // const char* dname = db->name.c_str();
    // if((flag & PRINT_PARENT) &&  (db->parent != NULL))
    //     dname = db->parent->name.c_str();
    if(flag & PRINT_URI)
    {
        cJSON* cju = cJSON_GetObjectItem(cj, db->uri);
        if(cju == NULL)
        {
            cju = cJSON_CreateObject();
            if(cju == NULL) return 1;
            cJSON_AddItemToObject(cj,db->uri, cju);
        }
        cj = cju;
    }
    switch (db->type)
    {
        case Type_AnalogOS:
        case Type_Analog:
        {
            if((db->variation == Group30Var5) || (db->variation == Group30Var6))
            {
                addSysCjVal(sys, cj, db, flag, db->valuedouble);
                
            }
            else
            {
                if(db->sign)
                {
                    addSysCjVal(sys, cj, db, flag, static_cast<double>(static_cast<int32_t>(db->valuedouble)));
                }
                else
                {
                    addSysCjVal(sys, cj, db, flag, db->valuedouble);        
                }
            }

            break;
        }

        case Type_BinaryOS:
        case Type_Binary:
        {
            bool bval = false;
            if (db->valuedouble > 0.0) bval = true;
            addSysCjVal(sys, cj, db, flag, bval);
            break;
        }
        case Type_Crob:
        {
            if (db->crob_input == CROB_STRING)
            {
                const char* cmd = OperationTypeSpec::to_string(OperationTypeSpec::from_type(db->crob));
                addSysCjVal(sys, cj, db, flag, cmd);
            }
            if (db->crob_input == CROB_INT)
            {
                addSysCjVal(sys, cj, db, flag, db->crob);
            }
            if (db->crob_input == CROB_BOOL)
            {
                if(db->crob_true == db->crob)
                {
                    addSysCjVal(sys, cj, db, flag, true);
                }
                else if(db->crob_false == db->crob)
                {
                    addSysCjVal(sys, cj, db, flag, false);
                }
                else if(0x03 == db->crob)
                {
                    addSysCjVal(sys, cj, db, flag, true);
                }
                else
                {
                    addSysCjVal(sys, cj, db, flag, false);
                }
            }
            if(0)
            {
                FPS_ERROR_PRINT("*** %s Found variable [%s] type  %d crob %u [%s] \n"
                    , __FUNCTION__, db->name.c_str(), db->type, db->crob, OperationTypeSpec::to_string(TypeToOperationType(db->crob)));
            }
            break;
        }
        case AnIn16:
        {
            double dval = 0.0;
            if (extractInt16Val(dval, db) == true)
            {
                // this can still be an int
                //addCjVal(cj, dname, flag, static_cast<int32_t>(dval));
                addSysCjVal(sys, cj, db, flag, static_cast<uint16_t>(dval));
            }
            else
            {
                addSysCjVal(sys, cj, db, flag, static_cast<int16_t>(dval));
            }
            break;
        }
        case AnIn32:
        {
            double dval = 0.0;
            if (extractInt32Val(dval, db) == true)
            {                
                FPS_DEBUG_PRINT("*** %s Found variable [%s] type  %d sign %d usng dval %f \n"
                                , __FUNCTION__, db->name.c_str(), db->type, db->sign, dval);
                addSysCjVal(sys, cj, db, flag, static_cast<uint32_t>(dval));
                    
            }
            else
            {
                FPS_DEBUG_PRINT("*** %s Found variable [%s] type  %d sign %d using valuedouble %f\n"
                                , __FUNCTION__, db->name.c_str(), db->type, db->sign, db->valuedouble);
                addSysCjVal(sys, cj, db, flag, static_cast<int32_t>(dval));
            }            
            break;
        }
        case AnF32:
        {
            addSysCjVal(sys, cj, db, flag, db->valuedouble);
            break;
        }
        default:
        {
            rc = -1;
            FPS_ERROR_PRINT("%s  unknown db_->type : %d\n"
                            , __FUNCTION__
                            , db->type
                            );
            break;
        }
    }
    return rc;
}

int addVarToCj(sysCfg* sys, cJSON* cj, std::pair<DbVar*,int>dbp)
{   
    DbVar* db = dbp.first;
    int flag = dbp.second;
    return addVarToCj(sys, cj, db, flag);
}

int addVarToCj(sysCfg* sys, cJSON* cj,  DbVar* db)
{
    return addVarToCj(sys, cj, db, 0);
}

int addVarToCj(sysCfg* sys, cJSON* cj, const char* uri, const char* dname)
{
    DbVar* db = sys->getDbVar(uri, dname);

    return addVarToCj(sys, cj, db, 0);
}
const char* dreg_types[] = { "AnOPInt16", "AnOPInt32", "AnOPF32", "CROB", "analog", "binary", "counter", "analogOS","binaryOS", 0 };

const char* iotypToStr (int t)
{
//    if (t < Type_of_Var::NumTypes)
    if (t < static_cast<int32_t> (sizeof(dreg_types))/static_cast<int32_t>(sizeof(dreg_types[0])))
    {
        return dreg_types[t];
    }
    return "Unknown";
}

int iotypToId (const char* t)
{
    int i;
    for (i = 0; i < Type_of_Var::NumTypes; i++)
    {
        if (strcmp(t , dreg_types[i] ) == 0 )
        return i;
    }
    return -1;
}

bool checkFileName(const char* fname)
{
    struct stat sb;
    memset(&sb, 0, sizeof(struct stat));

    stat(fname, &sb);
    if ((sb.st_mode & S_IFMT) == S_IFREG) {
        return true; // = strdup(fstr.c_str());
    }
    return false;
}

bool checkDeviceName(const char* fname)
{
    struct stat sb;
    memset(&sb, 0, sizeof(struct stat));

    stat(fname, &sb);
    std::cout << "Device st_mode " << sb.st_mode << std::endl;
    //if ((sb.st_mode & S_IFMT) == S_IFREG) {
    if (sb.st_mode != 0) {

        return true; // = strdup(fstr.c_str());
    }
    return false;
}

const char* variation_encode(int var)
{
    switch (var)
    {
        case 0x0101:
            return "Group1Var1";
        case 0x0102:
            return "Group1Var2";
        case 0x0a02:
            return "Group10Var2";
        case 0x0b01:
            return "Group11Var1";
        case 0x0b02:
            return "Group11Var2";
        case 0x1400:
            return "Group20Var0";
        case 0x1401:
            return "Group20Var1";
        case 0x1402:
            return "Group20Var2";
        case 0x1405:
            return "Group20Var5";
        case 0x1406:
            return "Group20Var6";
        case 0x1600:
            return "Group22Var0";
        case 0x1601:
            return "Group22Var1";
        case 0x1602:
            return "Group22Var2";
        case 0x1605:
            return "Group22Var5";
        case 0x1606:
            return "Group22Var6";
        case 0x1e01:
            return "Group30Var1";
        case 0x1e02:
            return "Group30Var2";
        case 0x1e03:
            return "Group30Var3";
        case 0x1e04:
            return "Group30Var4";
        case 0x1e05:
            return "Group30Var5";
        case 0x1e06:
            return "Group30Var6";
        case 0x2001:
            return "Group32Var1";
        case 0x2003:
            return "Group32Var3";
        case 0x0201:
            return "Group2Var1";
        case 0x0202:
            return "Group2Var2";
        case 0x0203:
            return "Group2Var3";
        default:
            return "Undecoded";
    }
    return "Undecoded";
}
int variation_decode(const char* ivar)
{
    if(ivar)
    {
        if (strcmp(ivar, "Group1Var0") == 0)
            return Group1Var0;
        else if (strcmp(ivar, "Group1Var1") == 0)
            return Group1Var1;
        else if (strcmp(ivar, "Group1Var2") == 0)
            return Group1Var2;
        if (strcmp(ivar, "Group2Var0") == 0)
            return Group2Var0;
        else if (strcmp(ivar, "Group2Var1") == 0)
            return Group2Var1;
        if (strcmp(ivar, "Group2Var2") == 0)
            return Group2Var2;
        else if (strcmp(ivar, "Group2Var3") == 0)
            return Group2Var3;
        else if (strcmp(ivar, "Group10Var2") == 0)
            return Group10Var2;
        else if (strcmp(ivar, "Group11Var1") == 0)
            return Group11Var1;
        else if (strcmp(ivar, "Group11Var2") == 0)
            return Group11Var2;
        else if (strcmp(ivar, "Group20Var0") == 0)
            return Group20Var0;
        else if (strcmp(ivar, "Group20Var1") == 0)
            return Group20Var1;
        else if (strcmp(ivar, "Group20Var2") == 0)
            return Group20Var2;
        else if (strcmp(ivar, "Group20Var5") == 0)
            return Group20Var5;
        else if (strcmp(ivar, "Group20Var6") == 0)
            return Group20Var6;
        else if (strcmp(ivar, "Group22Var0") == 0)
            return Group22Var0;
        else if (strcmp(ivar, "Group22Var1") == 0)
            return Group22Var1;
        else if (strcmp(ivar, "Group22Var2") == 0)
            return Group22Var2;
        else if (strcmp(ivar, "Group22Var5") == 0)
            return Group22Var5;
        else if (strcmp(ivar, "Group22Var6") == 0)
            return Group22Var6;
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
        else if (strcmp(ivar, "Group32Var0") == 0)
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
        else if (strcmp(ivar, "Group40Var0") == 0)
            return Group40Var0;
        else if (strcmp(ivar, "Group40Var1") == 0)
            return Group40Var1;
        else if (strcmp(ivar, "Group40Var2") == 0)
            return Group40Var2;
        else if (strcmp(ivar, "Group40Var3") == 0)
            return Group40Var3;
        else if (strcmp(ivar, "Group40Var4") == 0)
            return Group40Var4;
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
    return GroupUndef;
}

bool getCJint (cJSON* cj, const char* name, int& val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        val = cji->valueint;
        ok = true;
    }
    return ok;
}

bool getCJdouble (cJSON* cj, const char* name, double& val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        val = cji->valuedouble;
        ok = true;
    }
    return ok;
}

bool getCJbool (cJSON* cj, const char* name, bool& val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        val = cJSON_IsTrue(cji);
        ok = true;
    }
    return ok;
}

bool getCJstr (cJSON* cj, const char* name, char* &val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        if(val) free(val);
        val = strdup(cji->valuestring);
        ok = true;
    }
    return ok;
}

bool getCJcj (cJSON* cj, const char* name, cJSON* &val, bool required)
{
    bool ok = !required;
    cJSON *cji = cJSON_GetObjectItem(cj, name);
    if (cji) {
        if(val) cJSON_Delete(val);
        val = cJSON_Duplicate(cji->child, true);
        ok = true;
    }
    return ok;
}

cJSON* parse_files(cJSON* cji)
{
    return cJSON_GetObjectItem(cji, "files");
}

bool parse_system(cJSON* cji, sysCfg* sys, int who)
{
    bool ret = true;
    cJSON* cj = cJSON_GetObjectItem(cji, "system");

    if (cj == NULL)
    {
        FPS_ERROR_PRINT("system  missing from file! \n");
        ret = false;
    }
    sys->who = who;  // DNP3_MASTER or DNP3_OUTSTATION
    sys->master_address = 1;
    sys->station_address = 10;
     
    // todo add different frequencies for each zone
    sys->frequency  = 1000; // default once a second
    sys->port = 20000;
    sys->deadband = 0.01;
    sys->respTime = 2000; // default response time 2 Sec
    sys->maxElapsed = 100; // default max task elapsed time before we print an error in the log
    sys->max_pub_delay = 0;

    sys->baud = 9600;
    sys->dataBits = 8;
    sys->stopBits = 1;

    sys->parity = strdup("None");// opendnp3::Parity::None;
    sys->flowType = strdup("None");//opendnp3::FlowControl::None;
    sys->asyncOpenDelay=500; //(opendnp3::TimeDuration::Milliseconds(500));
    
    char* tmp_uri = NULL;
    sys->event_pub = true;
    if(ret) ret = getCJint(cj,"version",         sys->version,        false );
    if(ret) ret = getCJint(cj,"frequency",       sys->frequency,      false);
    if(ret) ret = getCJint(cj,"freq1",           sys->freq1,          false);
    if(ret) ret = getCJint(cj,"freq2",           sys->freq2,          false);
    if(ret) ret = getCJint(cj,"freq3",           sys->freq3,          false);
    if(ret) ret = getCJint(cj,"rangeAStart",     sys->rangeAStart,     false);
    if(ret) ret = getCJint(cj,"rangeAStop",      sys->rangeAStop,      false);
    if(ret) ret = getCJint(cj,"rangeBStart",     sys->rangeBStart,     false);
    if(ret) ret = getCJint(cj,"rangeBStop",      sys->rangeBStop,      false);
    if(ret) ret = getCJint(cj,"rangefreq",       sys->rangeFreq,       false);
    if(ret) ret = getCJint(cj,"port",            sys->port,            false);
    if(ret) ret = getCJint(cj,"master_address",  sys->master_address,  false);
    if(ret) ret = getCJint(cj,"station_address", sys->station_address, false);
    if(ret) ret = getCJstr(cj,"id",              sys->id,              true);
    if(ret) ret = getCJstr(cj,"protocol",        sys->protocol,        false);
    if(ret) ret = getCJstr(cj,"ip_address",      sys->ip_address,      true);
    if(ret) ret = getCJstr(cj,"name",            sys->name,            false);
    if(ret) ret = getCJint(cj,"debug",           sys->debug,           false);
    if(ret) ret = getCJstr(cj,"base_uri",        tmp_uri,             false);
    if(ret) ret = getCJstr(cj,"local_uri",       sys->local_uri,      false);
    if(ret) ret = getCJint(cj,"unsol",           sys->unsol,          false);
    if(ret) ret = getCJbool(cj,"useOffset",      sys->useOffset,      false);
    if(ret) ret = getCJbool(cj,"useVindex",      sys->useVindex,      false);
    if(ret) ret = getCJdouble(cj,"deadband",     sys->deadband,       false);
    // conn type:
    if(ret) ret = getCJstr(cj,"conn_type",       sys->conn_type,       false);
    // TLS stuff:
    if(ret) ret = getCJstr(cj,"tls_folder_path", sys->tls_folder_path,  false);
    if(ret) ret = getCJstr(cj,"privateKey",      sys->privateKey,       false);
    if(ret) ret = getCJstr(cj,"peerCertificate", sys->peerCertificate,  false);
    if(ret) ret = getCJstr(cj,"localCertificate",sys->localCertificate, false);
    //deprecated
    if(ret) ret = getCJstr(cj,"certificateChain",sys->certificateChain, false);
    if(ret) ret = getCJstr(cj,"caCertificate",   sys->caCertificate,    false);
    // serial/RTU stuff:
    if(ret) ret = getCJint(cj,"baud",            sys->baud,             false);
    if(ret) ret = getCJint(cj,"dataBits",        sys->dataBits,         false);
    if(ret) ret = getCJdouble(cj,"stopBits",     sys->stopBits,         false);
    if(ret) ret = getCJstr(cj,"parity",          sys->parity,           false);
    if(ret) ret = getCJstr(cj,"flowType",        sys->flowType,         false);
    if(ret) ret = getCJint(cj,"asyncOpenDelay",  sys->asyncOpenDelay,   false);
    if(ret) ret = getCJstr(cj,"deviceName",      sys->deviceName,       false);
    if(ret) ret = getCJstr(cj,"pubType",         sys->pubTypestr,       false);
    if(ret) ret = getCJdouble(cj,"timeout",      sys->timeout,          false);
    if(ret) ret = getCJdouble(cj,"respTime",     sys->respTime,         false);
    if(ret) ret = getCJint   (cj,"maxElapsed",   sys->maxElapsed,       false);
    if(ret) ret = getCJstr   (cj,"format",       sys->format,           false);
    if(ret) ret = getCJbool  (cj,"events",       sys->events,           false);
    if(ret) ret = getCJdouble(cj,"event_rate",   sys->event_rate,       false);
    if(ret) ret = getCJbool  (cj,"useGets",      sys->useGets,          false);
    if(ret) ret = getCJbool  (cj,"event_pub",    sys->event_pub,        false);
    if(ret) ret = getCJbool  (cj,"pub_outputs",  sys->pubOutputs,       false);
    if(ret) ret = getCJint   (cj,"event_buffer", sys->event_buffer,     false);
    if(ret) ret = getCJint   (cj,"batchPubDebug",sys->batch_pub_debug,  false);
    if(ret) ret = getCJbool  (cj,"enable_state_events",    sys->enable_state_events,        false);

    if (sys->event_buffer < 0 || sys->event_buffer > 512)
    {
        FPS_ERROR_PRINT("Invalid  event buffer  %d  must be from 0 to 512 ,setting to default 100 \n", sys->event_buffer);
        sys->event_buffer = 100;
    }

    if(getCJdouble(cj,"max_pub_delay", sys->max_pub_delay,     false)){
        sys->max_pub_delay /=1000.0;
    }

    if(getCJdouble(cj,"batchSets",    sys->batch_sets_in,      false)){
        sys->batch_sets_in /=1000.0;
        sys->batch_sets = sys->batch_sets_in;
    }
    if(getCJdouble(cj,"batchPubs",    sys->batch_pubs,      false)){
        sys->batch_pubs /=1000.0;
    }

    if(getCJdouble(cj,"batchSetsMax",    sys->batch_sets_max,      false)){
        sys->batch_sets_max /=1000.0;
    }
    if(getCJdouble(cj,"maxPubDroop",    sys->max_pub_droop,      false)){
        if (sys->max_pub_droop > 0.0)
        {
            // filter out bad values
            if ((sys->max_pub_droop < 0.2) || (sys->max_pub_droop > 0.98))
            {
                FPS_ERROR_PRINT("Invalid maxPubDroop   %2.3f  must be from 0.2 to 0.98 ,setting to 0.0 \n", sys->max_pub_droop);
                sys->max_pub_droop = 0.0;
            }

        }
    }

    if (getCJdouble(cj,"timeoutmS",      sys->timeout,          false)) {
        sys->timeout /=1000.0;
    }
 
    
    // fixup base_uri
    char tmp[1024];
    const char* sys_id = sys->id;
    if(sys->format)
    {
        std::string fmt = sys->format;
        if ( fmt == "naked") 
        {
            sys->pubType = 0;
            sys->fmt = 0;
        }
        else if ( fmt == "clothed") 
        {
            sys->pubType = 1;
            sys->fmt = 1;
        }
        else if ( fmt == "full") 
        {
            sys->pubType = 2;
            sys->fmt = 2;
        }
        // else if ( fmt == "flags") 
        // {
        //     sys->pubType = 2;
        // }
        // else if ( fmt == "time") 
        // {
        //     sys->pubType = 4;
        // }
        // else if ( fmt == "flags+time") 
        // {
        //     sys->pubType = 6;
        //}
    }
    // fix this 
    sys->pubType = 0;
    // now we use format

    if(sys->base_uri)
        free((void *) sys->base_uri);

    if(tmp_uri)
    {
        snprintf(tmp, sizeof(tmp),"%s/%s", tmp_uri, sys_id);
        free(tmp_uri);
    }
    else
    {
        if (who == DNP3_MASTER)
        {
            snprintf(tmp, sizeof(tmp),"%s/%s", "/components", sys_id);
        }
        else
        {
            snprintf(tmp, sizeof(tmp),"%s/%s", "/interfaces", sys_id);
        }
    }
    sys->base_uri = strdup(tmp);

    return ret;
}


// parse a map of items
int parse_items(sysCfg* sys, cJSON* objs, int type, int who)
{
    cJSON* obj;
    int mytype = type;

    cJSON_ArrayForEach(obj, objs)
    {
        cJSON *id, *offset, *uri, *bf, *bits, *variation;
        cJSON *evariation, *linkback, *clazz, *rsize, *sign;
        cJSON *scale, *cjidx, *opvar, *cjfloat, *linkuri, *deadband;
        cJSON *cjfmt, *cjevt, *cjtout, *cjevtr; 
        cJSON *cjcstring, *cjctrue, *cjcfalse, *cjcint, *cjbatched;

        if(obj == NULL)
        {
            FPS_ERROR_PRINT("Invalid or NULL obj\n");
            continue;
        }

        // note that id translates to name
        id         = cJSON_GetObjectItem(obj, "id");
        cjidx      = cJSON_GetObjectItem(obj, "idx");
        offset     = cJSON_GetObjectItem(obj, "offset");
        rsize      = cJSON_GetObjectItem(obj, "size");
        variation  = cJSON_GetObjectItem(obj, "variation");
        evariation = cJSON_GetObjectItem(obj, "evariation");
        uri        = cJSON_GetObjectItem(obj, "uri");
        bf         = cJSON_GetObjectItem(obj, "bit_field");
        bits       = cJSON_GetObjectItem(obj, "bit_strings");
        linkback   = cJSON_GetObjectItem(obj, "linkback");
        linkuri    = cJSON_GetObjectItem(obj, "linkuri");
        clazz      = cJSON_GetObjectItem(obj, "clazz");
        sign       = cJSON_GetObjectItem(obj, "signed");
        scale      = cJSON_GetObjectItem(obj, "scale");
        deadband   = cJSON_GetObjectItem(obj, "deadband");
        opvar      = cJSON_GetObjectItem(obj, "OPvar");  //1 = 16 bit , 2 = 32 bit , 3 float32
        cjfloat    = cJSON_GetObjectItem(obj, "float");
        cjfmt      = cJSON_GetObjectItem(obj, "format");
        cjtout     = cJSON_GetObjectItem(obj, "timeout");
        cjevt      = cJSON_GetObjectItem(obj, "events");
        cjevtr     = cJSON_GetObjectItem(obj, "event_rate");
        // allows more detailed manipulation of the crob type
        cjcstring  = cJSON_GetObjectItem(obj, "crob_string");  // true / false
        cjctrue    = cJSON_GetObjectItem(obj, "crob_true");  // string like POWER_ON
        cjcfalse   = cJSON_GetObjectItem(obj, "crob_false");  // string like POWER_ON
        cjcint     = cJSON_GetObjectItem(obj, "crob_int");  // true / false
        cjbatched  = cJSON_GetObjectItem(obj, "notBatched");  // true / false
        

        if (id == NULL || id->valuestring == NULL)
        {
            FPS_ERROR_PRINT("NULL variables or component_id \n");
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

        int vidx = -1;
        if(offset)
        {
            vidx = offset->valueint;
        }
        if(cjidx)
        {
            vidx = cjidx->valueint;
        }
        if (mytype == AnF32)
        {
            if(sys->debug)
                FPS_ERROR_PRINT("****** AF32 variable [%s] vidx %d\n",
                    id->valuestring, vidx);
        }
        if(sys->debug)
            FPS_ERROR_PRINT("****** mtype %d xxvariable [%s] vidx %d debug %d\n",
                        mytype, id->valuestring, vidx, sys->debug);

        // set up name uri
        char* nuri = sys->getDefUri(who);
        if (uri && uri->valuestring) 
        {
            nuri = uri->valuestring;
        }        
        
        DbVar* db = sys->newDbVar(id->valuestring, mytype, vidx,
                                    nuri, variation?variation->valuestring:NULL);

        // flag to allow events to be published
        if (sys->format)
        {
            db->format = strdup(sys->format);
            db->fmt = sys->fmt;
        }
        if(cjbatched &&(db != NULL))
        {
            db->not_batched = cJSON_IsTrue(cjbatched);
        }
        if(cjevt &&(db != NULL))
        {
            db->events = cJSON_IsTrue(cjevt);
        }
        if(cjevtr &&(db != NULL))
        {
            db->event_rate = cjevtr->valuedouble;
        }
        if(cjfmt &&(db != NULL))
        {
            if(cjfmt->valuestring)
            {
                std::string fmt = cjfmt->valuestring;
                if(fmt=="clothed")
                {
                    if(db->format)
                    {
                        free(db->format);
                    }
                    db->format = strdup(fmt.c_str());
                    db->fmt = 1;
                }
                else if(fmt=="full")
                {
                    if(db->format)
                    {
                        free(db->format);
                    }
                    db->format = strdup(fmt.c_str());
                    db->fmt = 2;
                }
                else  // defaults to naked
                {
                    db->format = strdup((char *)"naked");
                    db->fmt = 0;
                }
                // else if(fmt=="flags")
                // {
                //     db->format = 2;
                // }
                // else if(fmt=="time")
                // {
                //     db->format = 4;
                // }
                // else if(fmt=="flags+time")
                // {
                //     db->format = 6;
                // }
            }
        }
        if(cjcstring && (db!=NULL))
        {
            db->crob_string = (cjcstring->type == cJSON_True);
        }

        if(cjcint && (db!=NULL))
        {
            db->crob_int = (cjcint->type == cJSON_True);
        }

        if(cjctrue && (db!=NULL))
        {
            if(cjctrue->valuestring) 
            {
                db->crob_true = OperationTypeSpec::to_type(OperationTypeSpec::from_string(cjctrue->valuestring));
            }
            else
            {
                db->crob_true = (int)cjctrue->valuedouble;
            }
        }

        if(cjcfalse && (db!=NULL))
        {
            if(cjcfalse->valuestring) 
            {
                db->crob_false = OperationTypeSpec::to_type(OperationTypeSpec::from_string(cjcfalse->valuestring));
            }
            else
            {
                db->crob_false = (int)cjcfalse->valuedouble;
            }
        }

        if(db != NULL)
        {
            if(evariation)
            {
                db->setEvar(evariation->valuestring);
            }
            if(deadband)
            {
                db->deadband = deadband->valuedouble;
            }

            if(cjtout)
            {
                db->timeout = cjtout->valuedouble;
            }
            else
            {
                db->timeout = sys->timeout;
            }
            if (db->timeout == 0)
            {
                db->state = STATE_INIT;
            }
            else
            {
                db->state = STATE_RESET;
            }

            if(cjfloat)
            {
                db->setFloat(cJSON_IsTrue(cjfloat));
            }
            
            if(clazz)
            {
                db->setClazz(clazz->valueint);
                if(sys->debug)
                    FPS_ERROR_PRINT("****** variable [%s] set to clazz %d\n", db->name.c_str(), db->clazz);
            }

            if(sign)
            {
                db->sign = (sign->type == cJSON_True);
                if(sys->debug)
                    FPS_ERROR_PRINT("****** variable [%s] set to signed %d\n", db->name.c_str(), db->sign);
            }
            
            if(scale)
            {
                db->scale = (scale->valuedouble);
                if(sys->debug)
                    FPS_ERROR_PRINT("****** variable [%s] scale set %f\n", db->name.c_str(), db->scale);
            }
            if (bf && bits && (bits->type == cJSON_Array))
            {
                //TODO bits need a value use make_pair there too 
                FPS_ERROR_PRINT("*****************Adding bitfields for %s\n", db->name.c_str());
                sys->addBits(db, bits);

            }

            if(sys->debug)
                FPS_ERROR_PRINT(" config adding name [%s] idx [%d]\n", id->valuestring, vidx);
            sys->numObjs[mytype]++; 

            db->idx = vidx;

            if(cjidx)
            {
                db->idx = cjidx->valueint;
            }
            else
            {
                if (sys->useOffset) 
                {
                    if(offset)
                    {
                        db->idx = offset->valueint;
                    }
                }            
            }

            sys->setDbIdxMap(db, 1);
            if(sys->debug )
                FPS_ERROR_PRINT("*********** %s after setDbIdxMap name %s idx %d \n", __FUNCTION__, db->name.c_str(), db->idx);
            if (sys->dbMapIxs[db->type].find(db->idx) == sys->dbMapIxs[db->type].end()) 
            {
                FPS_ERROR_PRINT("***********   name %s type %d idx %d NOT in dbMapIxs  \n",  db->name.c_str(), db->type, db->idx);
            }
            else
            {
                if(sys->debug)
                    FPS_ERROR_PRINT("***********  name %s type %d idx %d Already in dbMapIxs  \n", db->name.c_str(), db->type, db->idx);
            }

            sys->addDbUri(db->uri, db);

            if((mytype == Type_Analog) || (mytype == Type_Binary)) 
            {
                if (who == DNP3_MASTER)
                {
                    if(linkuri)
                    {
                        if (linkuri->valuestring != NULL)
                        {
                            FPS_ERROR_PRINT(" Setting linkuri  name to [%s]\n", linkuri->valuestring);
                            db->linkuri = strdup(linkuri->valuestring);
                        }
                    }
                    if(linkback)
                    {
                        if (linkback->valuestring != NULL)
                        {
                            FPS_ERROR_PRINT(" Setting linkback variable name to [%s]\n", linkback->valuestring);
                            db->linkback = strdup(linkback->valuestring);
                            const char* luri = db->linkuri;
                            if (luri == NULL)
                                luri = db->uri;
                            db->linkb = sys->getDbVar(luri, linkback->valuestring);
                        }
                    }
                }
            }
            else
            {
                if (who == DNP3_OUTSTATION)
                {
                    if(linkuri)
                    {
                        if (linkuri->valuestring != NULL)
                        {
                            FPS_ERROR_PRINT(" Setting linkuri  name to [%s]\n", linkuri->valuestring);
                            db->linkuri = strdup(linkuri->valuestring);
                        }
                    }

                    if(linkback)
                    {
                        if (linkback->valuestring != NULL)
                        {
                            FPS_ERROR_PRINT(" Setting linkback variable name to [%s]\n", linkback->valuestring);
                            const char* luri = db->linkuri;
                            if (luri == NULL)
                                luri = db->uri;
                            db->linkback = strdup(linkback->valuestring);
                            db->linkb = sys->getDbVar(luri, linkback->valuestring);
                        }
                    }
                }
            }
        }
    }
    return  sys->numObjs[mytype]; 
}

int  parse_object(sysCfg* sys, cJSON* objs, int idx, int who)
{
    cJSON* cjlist = cJSON_GetObjectItem(objs, iotypToStr(idx));
    if (cjlist == NULL)
    {
        FPS_ERROR_PRINT("[%s] objects missing from config! \n",iotypToStr(idx));
        return -1;
    }
    return parse_items(sys, cjlist, idx, who);
}

bool parse_modbus(cJSON* cj, sysCfg* sys, int who)
{
    cJSON* cji;
    cJSON_ArrayForEach(cji, cj)
    {
        cJSON* cjmap = cJSON_GetObjectItem(cji, "map");
        cJSON* cjdtype = cJSON_GetObjectItem(cji, "dnp3_type");
        cJSON* cjtype = cJSON_GetObjectItem(cji, "type");

        if ((cjmap == NULL) || (cjmap->type != cJSON_Array))
        {
            FPS_ERROR_PRINT("modbus registers map object is not an array ! \n");
            return false;
        }

        int itype = -1;
        if ((cjdtype != NULL) && (cjdtype->type == cJSON_String))
        {
            itype = iotypToId (cjdtype->valuestring);
            if (itype < 0)
            {
                FPS_ERROR_PRINT("modbus dnp3_type [%s] not recognised! \n", cjdtype->valuestring);
                return false;
            }
        }
        if (itype < 0)
        {
            if ((cjtype != NULL) && (cjtype->type == cJSON_String))
            {
                itype = iotypToId (cjtype->valuestring);
                if (itype < 0)
                {
                    FPS_ERROR_PRINT("modbus type [%s] not recognised! \n", cjtype->valuestring);
                    return false;
                }
            }
        }
        if (itype < 0)
        {
            FPS_ERROR_PRINT(" register type missing or  not recognised! \n");
            return false;
        }
        parse_items(sys, cjmap, itype, who);
    }
    sys->assignIdx();

    return true;
}

bool parse_variables(cJSON* object, sysCfg* sys, int who)
{
    for (int idx = 0; idx< Type_of_Var::NumTypes; idx++)
        sys->numObjs[idx] = 0;

    cJSON* cjregs = cJSON_GetObjectItem(object, "registers");
    if (cjregs != NULL)
    {
        return parse_modbus(cjregs, sys, who);
    }

    cJSON *JSON_objects = cJSON_GetObjectItem(object, "objects");
    if (JSON_objects == NULL)
    {
        FPS_ERROR_PRINT("objects object is missing from file! \n");
        return false;
    }
    for (int itype = 0; itype< Type_of_Var::NumTypes; itype++)
        parse_object(sys, JSON_objects, itype, who);

    sys->assignIdx();

    return true;
}

int getSysUris(sysCfg *sys[], int who, const char** &subs, bool* &bpubs, int nums)
{
    int num = 0;
    int i;
    for (i = 0 ; i< nums; i++)
    {
        sys[i]->num_subs = sys[i]->getSubs(NULL, 0, who);
        num += sys[i]->num_subs;
    }

    subs = (const char **) malloc((num+3) * sizeof(char *));
    if(subs == NULL)
    {
        FPS_ERROR_PRINT("Failed to creae subs array.\n");
        return -1;
    }
    bpubs = (bool *) malloc((num+3) * sizeof(bool));
    memset(bpubs, false, (num+3) * sizeof(bool)); // all false we hope
    num = 0;
    for (i = 0 ; i< nums; i++)
    {
        num += sys[i]->getSubs(&subs[num], sys[i]->num_subs, who);
    }
    return num;
}

bool checkWho(sysCfg*sys, DbVar* db, int who)
{
    if(db == NULL) return false;
    if (who == DNP3_OUTSTATION)
    {
        if((db->type == Type_Analog ) || (db->type == Type_Binary)) 
            return true;
    }
    else
    {
       if(     (db->type == AnIn16 ) 
            || (db->type == AnIn32)
            || (db->type == AnF32)
            || (db->type == Type_Crob)
            ) 
            return true;
    }
    return false;    
}

bool checkWho(sysCfg*sys, const char* uri, const char *name, int who)
{
    DbVar* db = sys->getDbVar(uri, name);
    return checkWho(sys, db, who);
}

cJSON* parseValues(dbs_type& dbs, sysCfg*sys, fims_message*msg, int who, cJSON* body_JSON)
{
    cJSON* itypeValues = body_JSON;
    cJSON* cjit = NULL;
    if(sys->debug)
        FPS_ERROR_PRINT("Found variable list or array uri [%s] \n", msg->uri);
    if (itypeValues)
    { 
        if (cJSON_IsArray(itypeValues)) 
        {
            FPS_ERROR_PRINT("Error Found array of variables need simple list instead \n");
            return body_JSON;//return dbs.size();

        }
        else
        {
            // process a simple list
            cjit = itypeValues->child;
            if(sys->debug)
                FPS_DEBUG_PRINT("****** Start with variable list iterator->type %d\n\n", cjit?cjit->type:-1);
            char* mcuri = msg->uri;
            mcuri = strstr(msg->uri,"/reply/");
            if (mcuri != NULL)
            {
                mcuri += strlen("reply/");
            }
            else
            {
                mcuri = msg->uri;
                if(sys->local_uri)
                {
                    if(strncmp(msg->uri,sys->local_uri, strlen(sys->local_uri))== 0)
                    {
                        mcuri = (char *)msg->uri + strlen(sys->local_uri);
                    }
                    else
                    {
                        mcuri = msg->uri;
                    } 
                }       
            }

            while(cjit != NULL)
            {
                int flag = 0;
                if(sys->debug>1)
                    FPS_ERROR_PRINT("Found variable name  [%s] child %p \n"
                                            , cjit->string
                                            , (void *)cjit->child
                                            );
                if (!checkWho(sys, mcuri, cjit->string, who))
                {
                    if(sys->debug>1)
                        FPS_ERROR_PRINT("variable [%s] uri [%s] NOT set ON %d\n"
                                        , cjit->string
                                        , mcuri
                                        , who
                                        );
                }
                else
                {
                    if(sys->debug)
                        FPS_ERROR_PRINT("variable [%s] type %d uri [%s] OK set ON %d\n"
                                        , cjit->string
                                        , cjit->type
                                        , msg->uri
                                        , who
                                        );

                    addValueToVec(dbs, sys, mcuri, cjit->string, cjit, flag);
                }
                cjit = cjit->next;
            }
            if(sys->debug>1)
                FPS_ERROR_PRINT("***** Done with variable list \n\n");
        }
    }
    return body_JSON;//return dbs.size();
}

int countUris(const char* uri)
{
    int nfrags = 0;
    for (int i = 0; i < (int)strlen(uri); i++ )
    {
        if (uri[i] == '/')
            nfrags++;
    }
    return nfrags;
}


cJSON* parseBody(dbs_type& dbs, sysCfg*sys, fims_message*msg, int who)
{
    cJSON* body_JSON = cJSON_Parse(msg->body);
    cJSON* itypeValues = NULL;
    cJSON* itypeFlag = NULL;
    cJSON* itimeFlag = NULL;
    DbVar* db = NULL;
 
    if (body_JSON == NULL)
    {
        if((strcmp(msg->method,"set") == 0) || (strcmp(msg->method,"post") == 0))
        {
            if (sys->debug> 1)
            {
                FPS_ERROR_PRINT("%s fims message body error  method (%s) uri (%s) nfrags %d \n"
                    ,__FUNCTION__, msg->method, msg->uri, (int)msg->nfrags);
            }
            if(msg->body != NULL && msg->nfrags > 2)  
            {
                if (sys->debug> 1)
                   FPS_ERROR_PRINT("fims message body (%s) \n", msg->body);
            }
            else
            {
               return NULL;
            }
        }
    }

    if (msg->nfrags < 2)
    {
        FPS_ERROR_PRINT("fims message uri [%s] error not enough pfrags [%d] id [%s] \n"
                , msg->uri, msg->nfrags, sys->id);
        return body_JSON;
    }
    if(sys->debug)
        FPS_ERROR_PRINT("fims message uri [%s] PASSED test for enough pfrags [%d] id [%s] \n", msg->uri, msg->nfrags, sys->id);

    char* name = NULL;
    int flags = 0;

    char* newUri = sys->confirmUri(db, msg->uri, who, name, flags);
    if(sys->debug>1)
        FPS_ERROR_PRINT("fims message first test msg->uri [%s]  newUri [%s]  flags 0X%04x   URI_REPLY 0X%04x\n", 
                    msg->uri, newUri, flags, URI_FLAG_REPLY);

    if((flags & URI_FLAG_REPLY) == 0)
    {
        if((flags & URI_FLAG_URIOK) == 0)
        {
            if(sys->debug)
                FPS_ERROR_PRINT("fims message msg->uri [%s] frag [%s] Not ACCEPTED \n", msg->uri, sys->id);
            return body_JSON;
        }
        if(sys->debug)
            FPS_ERROR_PRINT(" %s Running with uri: [%s] flags 0x%04x  \n", __FUNCTION__, newUri, (unsigned int) flags);


        if((strcmp(msg->method,"set") != 0) && (strcmp(msg->method,"get") != 0) && (strcmp(msg->method,"pub") != 0) && (strcmp(msg->method,"post") != 0))
        {
            if(sys->debug)
                FPS_ERROR_PRINT("fims unsupported method [%s] uri [%s] \n", msg->method, msg->uri);
            return body_JSON;
        }
    }

    if((strcmp(msg->method, "set") == 0)||(strcmp(msg->method, "post") == 0))
    {
        if(sys->debug)
            FPS_ERROR_PRINT("fims method [%s] supported for [%s]\n", msg->method, (who==DNP3_MASTER)?"client":"server");

        if(who != DNP3_MASTER)
        {

            if((flags & URI_FLAG_REPLY) == URI_FLAG_REPLY)
            {
                if(sys->debug)
                    FPS_ERROR_PRINT("Server Set accepted  for [%s]\n"
                        , db?db->name.c_str():"No Dbvar"); 
            }
            else
            {

                FPS_ERROR_PRINT("Server Set only accepted for reply  [%s] uri [%s] \n"
                        , db?db->name.c_str():"No Dbvar", msg->uri); 
                return body_JSON;
            }
        }
        else
        {
            if(sys->debug)
                FPS_ERROR_PRINT("Client Set 2 accepted  for [%s]\n"
                        , db?db->name.c_str():"No Dbvar"); 
        }
    }
    // allow sets and gets on master
    if(strcmp(msg->method, "get") == 0)
        {
        int flag = 0;
        if(sys->debug)
            FPS_ERROR_PRINT("fims method [%s] supported for [%d] flags [%08x]\n", msg->method, who, flags);

        if(((flags & URI_FLAG_GET) == 0) && (who != DNP3_MASTER))
        {
            if(sys->debug)
            {
                if(db)
                    FPS_ERROR_PRINT("Get not supported for name [%s] on server  \n", db->name.c_str()); 
                else
                    FPS_ERROR_PRINT("Get not supported on server \n"); 
            }
            return body_JSON;
        }

        if((flags & URI_FLAG_NAKED) == URI_FLAG_NAKED)
        {
            flag |= URI_FLAG_NAKED;
        }
        if((flags & URI_FLAG_CLOTHED) == URI_FLAG_CLOTHED)
        {
            flag |= URI_FLAG_CLOTHED;
        }
        if((flags & URI_FLAG_FULL) == URI_FLAG_FULL)
        {
            flag |= URI_FLAG_FULL;
        }
        if((flags & URI_FLAG_FLAGS) == URI_FLAG_FLAGS)
        {
            flag |= PRINT_VALUE;
            flag |= PRINT_FLAGS;
        }
        if((flags & URI_FLAG_TIME) == URI_FLAG_TIME)
        {
            flag |= PRINT_VALUE;
            flag |= PRINT_TIME;
        }
        if((flags & URI_FLAG_SINGLE) == URI_FLAG_SINGLE)
        {
            
            if(sys->debug)
                FPS_ERROR_PRINT("Found SINGLE variable [%s] type  %d  flag %08x \n", db->name.c_str(), db->type, flag); 
            dbs.push_back(std::make_pair(db, flag));
        }
        else
        {
            if((flags & URI_FLAG_DUMP) == URI_FLAG_DUMP)
            {
                sys->addUrisToVec(dbs);
            }
            else
            {
                sys->addVarsToVec(dbs, newUri, flag);
            }
            //return body_JSON;
        }
        return body_JSON;
    }

    if(strcmp(msg->method,"set") == 0 || (strcmp(msg->method,"pub") == 0 && (who == DNP3_OUTSTATION)))
    {
        if(msg->body == NULL)
        {
            FPS_ERROR_PRINT("fims message uri [%s]  no BODY in Message\n", msg->uri);
            return body_JSON;
        }

        if((flags & URI_FLAG_SINGLE) == URI_FLAG_SINGLE)
        {
            if (db != NULL)
            {
                if(sys->debug)
                {
                    FPS_ERROR_PRINT("fims method [%s] uri [%s]  supported on [%d] single name [%s]\n"
                                        , msg->method
                                        , msg->uri
                                        , who
                                        , db->name.c_str()
                                        );
                }
            }
            else
            {
                if(sys->debug)
                {
                    FPS_ERROR_PRINT("fims method [%s] uri [%s]  single selected  [%d] NO DBVAR\n"
                                        , msg->method
                                        , msg->uri
                                        , who
                                        );
                }
                return body_JSON;
            }
        }
        
        if((flags & URI_FLAG_SYSTEM) == URI_FLAG_SYSTEM)
        {
            FPS_ERROR_PRINT("fims system command [%s] body [%s]\n", msg->uri, msg->body);
            cJSON* cjsys = cJSON_GetObjectItem(body_JSON, "debug");
            if (cjsys != NULL)
            {
                sys->debug = cjsys->valueint;
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "scan");
            if (cjsys != NULL)
            {
                sys->scanreq = cjsys->valueint;
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "unsol");
            if (cjsys != NULL)
            {
                sys->unsol = cjsys->valueint;
                sys->unsolUpdate = true;
 
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "class");
            if (cjsys != NULL)
            {
                sys->cjclass = cjsys;
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "max_pub_delay");
            if (cjsys != NULL)
            {
                double val = cjsys->valuedouble/1000.0;
                if (val < 5000.0)
                    sys->max_pub_delay = val; 
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "batchSets");
            if (cjsys != NULL)
            {
                double val = cjsys->valuedouble/1000.0;
                if (val>=0 && val < 5000.0)
                {
                    if (sys->batch_sets_in == sys->batch_sets)
                    {
                        sys->next_batch_time = sys->tNow + val;
                    }
                    sys->batch_sets_in = val;
                    sys->batch_sets = val;
                }
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "batchPubs");
            if (cjsys != NULL)
            {
                double val = cjsys->valuedouble/1000.0;
                if (val>=0 && val < 10.0)
                {
                    sys->next_batch_pub_time = sys->tNow + val;
                    sys->batch_pubs = val;
                }
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "batchPubDebug");
            if (cjsys != NULL)
            {
                double val = cjsys->valuedouble;
                sys->batch_pub_debug = (int)val;
                
            }

            cjsys = cJSON_GetObjectItem(body_JSON, "batchSetsMax");
            if (cjsys != NULL)
            {
                double val = cjsys->valuedouble/1000.0;
                if (val>=0 && val < 10.0)
                    sys->batch_sets_max = val;
            }
            cjsys = cJSON_GetObjectItem(body_JSON, "maxPubDroop");
            if (cjsys != NULL)
            {
                double val = cjsys->valuedouble/1000.0;
                if (val>=0 && val < 0.98)
                    sys->max_pub_droop = val;
            }

            return body_JSON;

        }
        if((flags & URI_FLAG_REPLY) == URI_FLAG_REPLY)
        {
            if(sys->debug )
                FPS_ERROR_PRINT("fims message reply uri ACCEPTED Body  [%s] \n", msg->body);
        }            
        
        if((flags & URI_FLAG_SINGLE) == URI_FLAG_SINGLE)
        {
            if(sys->debug)
                FPS_ERROR_PRINT("Found Single variable [%s] type  %d  body [%s] run set who = %d json [%p]\n"
                                        , db->name.c_str()
                                        , db->type
                                        , msg->body?msg->body:"nobody"
                                        , who
                                        , (void *)body_JSON
                                        );    
            int flag = 0;
            itypeValues = body_JSON;
            uint8_t cval = 255;
            itypeFlag = nullptr;
            char *vstring = nullptr;

            if(!itypeValues)
            {
                if(db->type == Type_Crob)
                {
                    cval = OperationTypeSpec::to_type(StringToOperationType(msg->body));
                    if (sys->debug)
                        FPS_ERROR_PRINT(" %s  body [%s] looks like a string  cval %d \n",msg->method,msg->body, cval);


                    if (cval != 255)
                    {
                        db->crob_input = CROB_STRING;
                        vstring = msg->body;
                    }
                    else
                    {
                        FPS_ERROR_PRINT(" ERROR %s  body [%s] not a valid json object  cval %d  db->type Type_Crob\n",msg->method,msg->body, cval);
                        return body_JSON;
                    }

                }
                else
                {
                    if (sys->debug)
                        FPS_ERROR_PRINT(" type [%d] meth %s  body [%s] not decoded \n",(int)db->type, msg->method,msg->body);
                    return body_JSON;
                }

            }
            if(itypeValues)
            {
                if(itypeValues->type == cJSON_Object)
                {
                    if(cJSON_GetObjectItem(itypeValues, "flag"))
                    {
                        if(sys->debug)
                            FPS_ERROR_PRINT("%s >>  found flag data\n",__func__);
                        itypeFlag = itypeValues;
                    }
                    itypeValues = cJSON_GetObjectItem(itypeValues, "value");
                    if(!itypeValues)
                    {
                        FPS_ERROR_PRINT(" %s  Value data missing\n", msg->method);
                        return body_JSON;                   
                    }
                    flag |= PRINT_VALUE;
                }
                if(sys->debug)
                {
                    FPS_ERROR_PRINT("Found variable [%s] type  %d  sign %d body [%s]  json type %d flag %d \n"
                                            , db->name.c_str()
                                            , db->type
                                            , db->sign
                                            , msg->body
                                            , (int)itypeValues->type
                                            , flag);
                }
                if (itypeValues->type == cJSON_String)
                {
                    vstring =itypeValues->valuestring;
                    if(db->type == Type_Crob)
                    {
                        cval = OperationTypeSpec::to_type(StringToOperationType(vstring));
                        db->crob_input = CROB_STRING;
                    }
                }
                else if (itypeValues->type == cJSON_Number)
                {
                    vstring = msg->body;
                    if(db->type == Type_Crob)
                    {
                        cval = (uint8_t) itypeValues->valuedouble ;//OperationTypeSpec::to_type(StringToOperationType(vstring));
                        db->crob_input = CROB_INT;
                    }
                }
                else if (itypeValues->type == cJSON_True)
                {
                    vstring = msg->body;
                    if(db->type == Type_Crob)
                    {
                        // decode the meaning of true
                        if (db->crob_true >= 0)
                        {
                            cval = db->crob_true;
                        }
                        else
                        {
                            cval = (uint8_t) 3 ;//OperationTypeSpec::to_type(StringToOperationType(vstring));
                        }
                        db->crob_input = CROB_INT;
                    }
                }
                else if (itypeValues->type == cJSON_False)
                {
                    vstring = msg->body;
                    if(db->type == Type_Crob)
                    {
                        // decode the meaning of false
                        if (db->crob_false >= 0)
                        {
                            cval = db->crob_false;
                        }
                        else
                        {
                            cval = (uint8_t) 4 ;//OperationTypeSpec::to_type(StringToOperationType(vstring));
                        }
                        db->crob_input = CROB_INT;
                    }
                }
            }
            if(db->type == Type_Crob)
            {
                CommandStatus status = (CommandStatus)0;
                sys->setDbVarIx(Type_Crob, db->idx, cval, status, itypeFlag);
                db->init_set = 1;
                if (sys->batch_sets > 0.0 )
                {
                    if (db->not_batched == false)
                    {
                        db->value_set++;
                        if (sys->debug)
                            FPS_ERROR_PRINT(" ***** %s batching item [%s] val [%s:%d]\n", __func__, db->name.c_str(),vstring, db->crob);
                    } else {
                        if (sys->debug)
                            FPS_ERROR_PRINT(" ***** %s not batching  item [%s] val [%s:%d]\n", __func__, db->name.c_str(), vstring, db->crob);
                        dbs.push_back(std::make_pair(db, flag));
                    }
                }
                else
                {
                    if (sys->debug)
                            FPS_ERROR_PRINT(" ***** %s setting  item [%s] val [%s:%d]\n", __func__, db->name.c_str(), vstring, db->crob);

                    dbs.push_back(std::make_pair(db, flag));
                }
                if(sys->debug)
                {
                    FPS_ERROR_PRINT(" ***** %s Adding Direct CROB value %s offset %d idx %d uint8 cval 0x%02x\n"
                                , __FUNCTION__, vstring, db->offset, db->idx
                                , cval
                                );
                }
            }
            else
            {
                if (itypeValues)  // recheck here to stop crash
                {

                    sys->setDbVar(db, itypeValues, itypeFlag, itimeFlag);
                    if (
                        (db->type == AnIn16) ||
                        (db->type == AnIn32) ||
                        (db->type == AnF32)
                        ) 
                        {
                            if(0)FPS_ERROR_PRINT(" ***** %s  setting value [%s] batchedSets [%2.3f]\n"
                                , db->name.c_str(), msg->body, sys->batch_sets
                                );

                            // add to dbs for main function to process
                            if (sys->batch_sets > 0.0 )
                            {
                                if ( db->not_batched == false)
                                {
                                    db->value_set++;
                                } else {
                                    dbs.push_back(std::make_pair(db, flag));
                                }
                            } else {
                                dbs.push_back(std::make_pair(db, flag));
                            }
                        }
                    if (
                        (db->type == Type_Analog) ||
                        (db->type == Type_Binary)
                        ) 
                        {
                            if(0)FPS_ERROR_PRINT(" ***** %s  setting value single [%s] batchedPubs [%2.3f]\n"
                                    , db->name.c_str(), msg->body, sys->batch_pubs
                                    );
                           // add to dbs for main function to process
                            if (sys->batch_pubs > 0.0 )
                            {
                                if ( db->not_batched == false)
                                {
                                    db->value_set++;
                                    if(sys->batch_pub_debug>0)
                                    {
                                        FPS_ERROR_PRINT(" ***** %s  batching value single [%s] batchedPubs [%2.3f]\n"
                                            , db->name.c_str(), msg->body, sys->batch_pubs
                                        );
                                    }
                                } else {
                                    if(sys->batch_pub_debug>0)
                                    {
                                        FPS_ERROR_PRINT(" ***** %s  setting notBatched value single [%s] batchedPubs [%2.3f]\n"
                                            , db->name.c_str(), msg->body, sys->batch_pubs
                                        );
                                    }

                                    dbs.push_back(std::make_pair(db, flag));
                                }

                            } else {

                                if(sys->batch_pub_debug>0)
                                {
                                    FPS_ERROR_PRINT(" ***** %s  setting value single [%s] batchedPubs [%2.3f]\n"
                                        , db->name.c_str(), msg->body, sys->batch_pubs
                                    );
                                }

                                dbs.push_back(std::make_pair(db, flag));
                            }
                        }
                }
            }
            return body_JSON;
        }
        if(sys->debug)
            FPS_ERROR_PRINT("fims message running parseValues \n");
        return parseValues(dbs, sys, msg, who, body_JSON);
    }
    return body_JSON;//return dbs.size();
}


int addValueToVec(dbs_type& dbs, sysCfg*sys, char* curi, const char* name, cJSON *cjvalue, int flag)
{
    cJSON *cjFlags =  nullptr;
    cJSON *cjTime =  nullptr;
    DbVar* db = getDbVar(sys, (const char*)curi, name); 
    if (db == NULL)
    {
        FPS_ERROR_PRINT( " ************* %s Var [%s] not found in dburiMap\n", __FUNCTION__, name);
        return -1;
    }
    
    if(sys->debug)
        FPS_ERROR_PRINT(" ************* %s Var [%s] found in dburiMap\n", __FUNCTION__, name);

    if (cjvalue->type == cJSON_Object)
    {
        flag |= PRINT_VALUE;

        cjFlags = cJSON_GetObjectItem(cjvalue, "flags");
        cjTime = cJSON_GetObjectItem(cjvalue, "time");
        cjvalue = cJSON_GetObjectItem(cjvalue, "value");

        if(sys->debug)
            FPS_ERROR_PRINT(" ************* %s Var type OBJECT [%s] set value flag to 1 cjv %p cjf %p\n"
                    , __FUNCTION__
                    , name
                    , (void*)cjvalue
                    , (void*)cjFlags
                    );
    }

    if (!cjvalue)
    {
        FPS_ERROR_PRINT(" ************** %s value object not found\n",__FUNCTION__);
        return -1;
    }

    if (db->type == Type_Crob) 
    {
        if(cjvalue->valuestring)
        {
            int cval = static_cast<int32_t>(StringToOperationType(cjvalue->valuestring));
            if(sys->debug)
                FPS_ERROR_PRINT(" ************* %s Var [%s] CROB setting string value [%s]  to %d  [%s] \n"
                                                    , __FUNCTION__
                                                    , db->name.c_str()
                                                    , name
                                                    , cval //static_cast<int32_t>(StringToOperationType(cjvalue->valuestring))
                                                    , cjvalue->valuestring
                                                    );
            db->crob_input = CROB_STRING;
            sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
        }
        else if((cjvalue->type == cJSON_True) || (cjvalue->type == cJSON_False))
        {
            bool cval = (cjvalue->type == cJSON_True);
            if(sys->debug)
                FPS_ERROR_PRINT(" ************* %s Var [%s] CROB setting bool value [%s]  to [%s] \n"
                                                    , __FUNCTION__
                                                    , db->name.c_str()
                                                    , name
                                                    , cval ? "true":"false"
                                                    );
            db->crob_input = CROB_BOOL;
            sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
        }
        else
        {
            int cval = (int)(cjvalue->valuedouble);
            if(sys->debug)
                FPS_ERROR_PRINT(" ************* %s Var [%s] CROB setting int value [%s]  to [%d] \n"
                                                    , __FUNCTION__
                                                    , db->name.c_str()
                                                    , name
                                                    , cval 
                                                    );

            db->crob_input = CROB_INT;
            sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
            
        }
        if (sys->batch_sets > 0.0 )
        {
            if ( db->not_batched == false)
            {
                db->value_set++;
            } else {
                dbs.push_back(std::make_pair(db, flag));
            }
        } else {
                dbs.push_back(std::make_pair(db, flag));
        }
    }
    else if (
            (db->type == AnIn16) ||
            (db->type == AnIn32) ||
            (db->type == AnF32)
            )
    {
        if(sys->debug)
            FPS_ERROR_PRINT( " *************** %s Var [%s] processed flags %p \n",__FUNCTION__, name, (void *) cjFlags);  
        sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
        if (sys->batch_sets > 0.0 )
        {
            if ( db->not_batched == false)
            {
                db->value_set++;
            } else {
                dbs.push_back(std::make_pair(db, flag));
            }
        } else {
            dbs.push_back(std::make_pair(db, flag));
        }
    }
    else if (
            (db->type == Type_Analog) ||
            (db->type == Type_Binary) 
            )
    {
        if(sys->debug)
            FPS_ERROR_PRINT( " *************** %s Var [%s] processed flags %p \n",__FUNCTION__, name, (void *) cjFlags);  
        sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
       if(0)FPS_ERROR_PRINT(" ***** %s  setting cjvalue batchedPubs [%2.3f]\n"
                                , db->name.c_str(), sys->batch_pubs
                            );

        if (sys->batch_pubs > 0.0 )
        {
            if ( db->not_batched == false)
            {
                if(sys->batch_pub_debug>0)
                {
                    FPS_ERROR_PRINT(" ***** %s  batching value  batchedPubs [%2.3f]\n"
                                    , db->name.c_str(),  sys->batch_pubs
                                    );
                }

                db->value_set++;
            } else {
                if(sys->batch_pub_debug>0)
                {
                    FPS_ERROR_PRINT(" ***** %s  setting notBatched value  batchedPubs [%2.3f]\n"
                                    , db->name.c_str(), sys->batch_pubs
                                    );
                }

                dbs.push_back(std::make_pair(db, flag));
            }
        } else {

            if(sys->batch_pub_debug>0)
            {
                FPS_ERROR_PRINT(" ***** %s  setting value  batchedPubs [%2.3f]\n"
                                    , db->name.c_str(), sys->batch_pubs
                                    );
            }
            
            dbs.push_back(std::make_pair(db, flag));
        }
    }
    else
    {
        FPS_ERROR_PRINT( " *************** %s Var [%s] not processed \n",__FUNCTION__, name);  
        return -1;
    }

    if(sys->debug)
        FPS_ERROR_PRINT( " *************** %s All Vars processed  size %d\n",__FUNCTION__, (int) dbs.size());  

    return dbs.size();   
}

int addValueToVec(dbs_type& dbs, sysCfg*sys, fims_message* msg, const char* name, cJSON *cjvalue, int flag)
{
    int ret;
    char* curi = strdup(msg->uri);
    char* mcuri = strstr(curi, name);
    if(mcuri != NULL)
    {
        mcuri[-1] = 0;
    }

    if (name == NULL)
    {
        FPS_ERROR_PRINT(" ************** %s offset is not  string\n",__FUNCTION__);
        free((void*)curi);
        return -1;
    }
    ret = addValueToVec(dbs, sys, curi, name, cjvalue, flag);
    free((void*)curi);
    return ret;
}

cJSON* sysdbFindAddArray(sysCfg* sys, const char* field)
{
    // look for cJSON Object called field
    cJSON* cjf = cJSON_GetObjectItem(sys->cj, field);
    if(!cjf)
    {
        cJSON* cja =cJSON_CreateArray();
        cJSON_AddItemToObject(sys->cj, field, cja);
        cjf = cJSON_GetObjectItem(sys->cj, field);
    }
    return cjf;
}

void sendCmdSet(sysCfg* sys, DbVar*db, cJSON* cj)
{
    const char *uri;
    char turi[1024];

    if (db->uri)
    {
        uri = db->uri;
        if (uri[0] == '/')
        {
            uri = &db->uri[1];
        }
    }
    else
    {
        snprintf(turi, sizeof(turi), "components/%s", sys->id );
        uri = (const char *)turi;
    }
    if (sys->batch_sets > 0.0 && db->not_batched == false)
    {
        sys->setLock.lock();
        if (sys->cjOut == NULL)
        {
            sys->cjOut = cJSON_CreateObject();

        }
        cJSON *cjo= cJSON_GetObjectItem(sys->cjOut, uri);
        if (cjo == NULL)
        {
            cjo = cJSON_CreateObject();
            cJSON_AddItemToObject(sys->cjOut,uri,cjo);
        }
        cJSON *cjj= cJSON_Duplicate(cj, true);
        if(cJSON_HasObjectItem(cjo, db->name.c_str()))
        {
            cJSON_DeleteItemFromObject(cjo,db->name.c_str());
        }
        cJSON_AddItemToObject(cjo, db->name.c_str(), cjj);

        if (sys->debug > 1)
        {
            char *out2 = cJSON_PrintUnformatted(sys->cjOut);
            if (out2) 
            {
                FPS_ERROR_PRINT("sys->cjOut batch  [%s] \n", out2);
                free(out2);
            }
        }
        sys->setLock.unlock();
    }
    
    if (sys->batch_sets == 0.0 || db->not_batched == true)
    {
        char *out = cJSON_PrintUnformatted(cj);
        if (out) 
        {
            char tmp[2048];
            snprintf(tmp, sizeof(tmp), "/%s/%s", uri, db->name.c_str() );

            if(sys->p_fims)
            {
                sys->p_fims->Send("set", tmp, NULL, out);
            }
            else
            {
                FPS_ERROR_PRINT("%s Error in sys->p_fims\n", __FUNCTION__ );
            }    
            free(out);
        }
        else
        {
            FPS_ERROR_PRINT("%s Error in cJSON object\n", __FUNCTION__ );
        }
    }
}

double dbOpScale(DbVar* db, double dval)
{
    if  ((db->scale > 0.0) || (db->scale < 0.0))
    {
        dval /= db->scale;
    }

    return dval;
}

void sysdbAddtoRecord(sysCfg* sys, const char* field, const opendnp3::AnalogOutputInt16& cmd, uint16_t index)
{
    DbVar* db = sys->getDbVarId(AnIn16 , index);
    if (db)
    {
        double dval = static_cast<double>(cmd.value);
        uint16_t u16val = static_cast<uint16_t>(dval);
        if(sys->debug)
            FPS_ERROR_PRINT("%s signed AnIn16  dval %2.3f  at index %d status [%d]-> [%s]\n", __FUNCTION__
                          , dval, static_cast<int32_t>(index), (int)cmd.status, CommandStatusSpec::to_human_string(cmd.status));
        if(cmd.status != CommandStatus::SUCCESS)
        {
            FPS_ERROR_PRINT("%s signed AnIn16  dval %2.3f  at index %d bad command status  [%s]\n", __FUNCTION__
                          , dval, static_cast<int32_t>(index),  CommandStatusSpec::to_human_string(cmd.status));
            return;
        }

        cJSON* cjv = cJSON_CreateObject();

        if(db->sign == 1)
        {
            double dval_save = dval;
            if(dval > SHRT_MAX)
            {
                dval = SHRT_MAX;
            }
            if(dval < SHRT_MIN)
            {
                dval = SHRT_MIN;                
            }
            if(sys->debug)
                FPS_ERROR_PRINT("%s signed AnIn16 dval save %f dval %f  at index %d\n", __FUNCTION__
                          , dval_save,  dval,  static_cast<int16_t>(index) );
            dval = dbOpScale(db, dval);
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(dval);
            } else {
               cJSON_AddNumberToObject(cjv, "value", static_cast<int16_t>(dval));         
            } 
        }
        else
        {
            if(sys->debug)
                FPS_ERROR_PRINT("%s unsigned AnIn16 dval %f  val %u at index %d\n", __FUNCTION__
                          , dval, u16val, static_cast<int16_t>(index) );
            if(dval < 0)
            {
                u16val = (SHRT_MAX) + (dval * -1.0);
                FPS_ERROR_PRINT("%s unsigned adjustment AnIn16 dval %f  new val %u at index %d\n", __FUNCTION__
                              , dval, u16val, static_cast<int16_t>(index) );
            }
            dval = dbOpScale(db, static_cast<double>(u16val));
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(dval);
            } else {
               cJSON_AddNumberToObject(cjv, "value", static_cast<uint16_t>(dval));         
            } 
        }
        sendCmdSet(sys, db, cjv);
        cJSON_Delete(cjv);
    }
    else
    {
        FPS_ERROR_PRINT("%s unable to find AnIn16 at index %d\n", __FUNCTION__, static_cast<int16_t>(index) );
    }    
}

void sysdbAddtoRecord(sysCfg* sys, const char* field, const opendnp3::AnalogOutputInt32& cmd, uint16_t index)
{
    DbVar* db = sys->getDbVarId(AnIn32 , index);
    if (db)
    {
        double dval = static_cast<double>(cmd.value);
        uint32_t u32val = static_cast<uint32_t>(dval);
        cJSON* cjv = cJSON_CreateObject();
        if(db->sign == 1)
        {
            dval = dbOpScale(db, dval);
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(dval);
            } else {
               cJSON_AddNumberToObject(cjv, "value", static_cast<int32_t>(dval));         
            } 
        }
        else
        {
            if(sys->debug)
                FPS_ERROR_PRINT("%s unsigned AnIn32 dval %f val %u at index %d\n", __FUNCTION__
                        , dval, u32val, static_cast<int32_t>(index) );

            if(dval < 0)
            {
                u32val = (INT_MAX) + (dval * -1.0);
                FPS_ERROR_PRINT("%s unsigned adjustment AnIn32 dval %f  new val %u at index %d\n", __FUNCTION__
                              , dval, u32val, static_cast<int32_t>(index) );
            } 
            dval = dbOpScale(db, static_cast<double>(u32val));
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(dval);
            } else {
                cJSON_AddNumberToObject(cjv, "value", static_cast<int32_t>(dval));         
            } 
            
        }        
        sendCmdSet(sys, db, cjv);
        cJSON_Delete(cjv);
    }
    else
    {
        FPS_ERROR_PRINT("%s unable to find AnIn32 at index %d\n", __FUNCTION__, static_cast<int32_t>(index) );
    }
}

void sysdbAddtoRecord(sysCfg* sys, const char* field, const opendnp3::AnalogOutputFloat32& cmd, uint16_t index)
{
    DbVar* db = sys->getDbVarId(AnF32 , index);
    if (db)
    {
        double dval = dbOpScale(db, static_cast<double>(cmd.value));
        cJSON* cjv = cJSON_CreateObject();
        if (db->fmt == 0) 
        {
            cJSON_Delete(cjv);
            cjv = cJSON_CreateNumber(dval);
        } else {
            cJSON_AddNumberToObject(cjv, "value", dval);         
        }
        sendCmdSet(sys, db, cjv);
        cJSON_Delete(cjv);
    }
    else
    {
        FPS_ERROR_PRINT("%s unable to find AnF32 (%d) at index %d\n", __FUNCTION__, AnF32, static_cast<int32_t>(index) );
    }

}

void sysdbAddtoRecord(sysCfg* sys, const char* field, const char* cmd, uint16_t index)
{
    DbVar* db = sys->getDbVarId(Type_Crob , index);
    if (db)
    {
        cJSON* cjv = cJSON_CreateObject();
        bool sendOK = true;
        //   NUL = 0x0,
        //   PULSE_ON = 0x1,
        //   PULSE_OFF = 0x2,
        //   LATCH_ON = 0x3,
        //   LATCH_OFF = 0x4,
        //   Undefined = 0xFF
        const char* cmd = OperationTypeSpec::to_string(OperationTypeSpec::from_type(db->crob));
        if(db->crob_int)
        {
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(db->crob);
            } else {
                cJSON_AddNumberToObject(cjv, "value", db->crob);         
            } 
        }
        // Fixed up the crob_string error
        else if(db->crob_string)
        {
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateString (cmd);
            } else {
                cJSON_AddStringToObject(cjv, "value", cmd);
            }
        }
        // default to crob_bool
        else
        {
            if ((db->crob_true >= 0) && (db->crob == db->crob_true))
            {
                if (db->fmt == 0) 
                {
                    cJSON_Delete(cjv);
                    cjv = cJSON_CreateBool (true);
                } else {
                    cJSON_AddBoolToObject(cjv, "value", true);
                }
            }
            else if ((db->crob_false>=0)&&(db->crob == db->crob_false))
            {
                if (db->fmt == 0) 
                {
                    cJSON_Delete(cjv);
                    cjv = cJSON_CreateBool (false);
                } else {
                    cJSON_AddBoolToObject(cjv, "value", false);
                }
            }
            else if ((db->crob_true<0)&&(db->crob == 3))
            {
                if (db->fmt == 0) 
                {
                    cJSON_Delete(cjv);
                    cjv = cJSON_CreateBool (true);
                } else {
                    cJSON_AddBoolToObject(cjv, "value", true);
                }
            }
            else if ((db->crob_false <0)&&(db->crob == 4))
            {
                if (db->fmt == 0) 
                {
                    cJSON_Delete(cjv);
                    cjv = cJSON_CreateBool (false);
                } else {
                    cJSON_AddBoolToObject(cjv, "value", false);
                }
            }
            else
            {
                sendOK = false;
            }
        }
        if(sendOK)
            sendCmdSet(sys, db, cjv);
        cJSON_Delete(cjv);
    }
    else
    {
        FPS_ERROR_PRINT("%s unable to find CROB at index %d\n", __FUNCTION__, static_cast<int32_t>(index) );
    }

}

const char* cfgGetSOEName(sysCfg* sys, const char* fname)
{
    return fname;
}
const OperationType StringToOperationType(const char* codeWord)
{
    if (strcmp("NUL", codeWord) == 0)
        return OperationType::NUL;
    if (strcmp("PULSE_ON", codeWord) == 0)
        return OperationType::PULSE_ON;
    if (strcmp("PULSE_OFF", codeWord) == 0)
        return OperationType::PULSE_OFF;
    if (strcmp("LATCH_ON", codeWord) == 0 || strcmp("true", codeWord) == 0)
        return OperationType::LATCH_ON;
    if (strcmp("LATCH_OFF", codeWord) == 0 || strcmp("false", codeWord) == 0)
        return OperationType::LATCH_OFF;
    return OperationType::Undefined;
}

bool run_config(cJSON* config, sysCfg** sys, int &num_configs, int who, fims* p_fims)
{
    bool ret = false;
    sys[num_configs] = new sysCfg();

    if(!parse_system(config, sys[num_configs], who)) 
    {
        FPS_ERROR_PRINT("Error reading system from config file.\n");
        cJSON_Delete(config);
        return ret;
    }
    if(!parse_variables(config, sys[num_configs], who)) 
    {
        FPS_ERROR_PRINT("Error reading variabled from config file.\n");
        cJSON_Delete(config);
        return ret;
    }
    ret = true;
    cJSON_Delete(config);

    //   _cfg.setupReadb(DNP3_MASTER);
    sys[num_configs]->showDbMap();
    //sys_cfg.showUris();
    sys[num_configs]->showNewUris();
    // TODO check max configs
    sys[num_configs]->p_fims = p_fims;// = new fims();

    num_configs += 1;
    return ret;
}

int getConfigs(int argc, char *argv[], sysCfg**sys, int who, fims *p_fims)
{
    int num_configs  = 0;    
    while (true)
    {

        FPS_DEBUG_PRINT("Reading config file(s) and starting setup.\n");
        cJSON* config = get_config_json(argc, argv, num_configs);
        if(config == NULL)
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
        cJSON* cjfiles = parse_files(config);
        if(cjfiles)
        {
            cJSON* cjfile;
            cJSON_ArrayForEach(cjfile, cjfiles)
            {
                FPS_ERROR_PRINT(" NOTE config file [%s]\n", cjfile->valuestring);
                config = get_config_file(cjfile->valuestring);
                if(!run_config(config,sys, num_configs, who, p_fims))
                {
                    return -1;
                }
            }
        }
        else
        {
            if(!run_config(config, sys, num_configs, who, p_fims))
                return -1;
        }
    }
    return num_configs;
}

// used in the outstation.
DatabaseConfig ConfigureDatabase(sysCfg* sys)
{ 
    DatabaseConfig config(0); // 10 of each type with default settings
  
    auto dsize = sys->getTypeSize(Type_Analog);
    FPS_ERROR_PRINT(" %s >>  analog size = [%d] \n", __func__, (int)dsize);
    for (int i = 0; i < static_cast<int32_t>(dsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_Analog, i);
        if(db != NULL)
        {
            config.analog_input[i].clazz = PointClass::Class1;
            if(db->variation == Group30Var5)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var5;
            }
            else if(db->variation == Group30Var6)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var6;
            }
            else if(db->variation == Group30Var1)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var1;
            }
            else if(db->variation == Group30Var2)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var2;
            }
            else if(db->variation == Group30Var3)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var3;
            }
            else if(db->variation == Group30Var4)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var4;
            }
            else if(db->variation == Group30Var5)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var5;
            }
            else if(db->variation == Group30Var6)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var6;
            }

            if(db->evariation == Group32Var0)
            {
                //TODO config.analog_input[i].evariation = EventAnalogVariation::Group32Var0;
            }
            else if(db->evariation == Group32Var1)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var1;
            }
            else if(db->evariation == Group32Var2)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var2;
            }
            else if(db->evariation == Group32Var3)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var3;
            }
            else if(db->evariation == Group32Var4)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var4;
            }
            else if(db->evariation == Group32Var5)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var5;
            }
            else if(db->evariation == Group32Var6)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var6;
            }
            else if(db->evariation == Group32Var7)
            {
                config.analog_input[i].evariation = EventAnalogVariation::Group32Var7;
            }
            if (db->deadband > 0.0) {
                config.analog_input[i].deadband = db->deadband;
            }
            else
            {
                config.analog_input[i].deadband = sys->deadband;
            }
            
    
            if(db->clazz != 0)
            {
                switch (db->clazz ) 
                {
                    case 1:
                    {
                        config.analog_input[i].clazz = PointClass::Class1;
                        break;
                    }
                    case 2:
                    {
                        config.analog_input[i].clazz = PointClass::Class2;
                        break;
                    }
                    case 3:
                    {
                        config.analog_input[i].clazz = PointClass::Class3;
                        break;
                    }
                    default:
                        break;
                }
            }

        }
    }
    dsize = sys->getTypeSize(Type_AnalogOS);
    FPS_ERROR_PRINT(" %s >>  analogOS size = [%d] \n", __func__, (int)dsize);

    for (int i = 0; i < static_cast<int32_t>(dsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_AnalogOS, i);
        if(db != NULL)
        {
            config.analog_output_status[i].clazz = PointClass::Class1;
            if(db->variation == Group40Var1)
            {
                config.analog_output_status[i].svariation = StaticAnalogOutputStatusVariation::Group40Var1;
            }
            else if(db->variation == Group40Var2)
            {
                config.analog_output_status[i].svariation = StaticAnalogOutputStatusVariation::Group40Var2;
            }
            else if(db->variation == Group40Var3)
            {
                config.analog_output_status[i].svariation = StaticAnalogOutputStatusVariation::Group40Var3;
            }
            else if(db->variation == Group40Var4)
            {
                config.analog_output_status[i].svariation = StaticAnalogOutputStatusVariation::Group40Var4;
            }

            if(db->evariation == Group42Var1)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var1;
            }
            else if(db->evariation == Group42Var2)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var2;
            }
            else if(db->evariation == Group42Var3)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var3;
            }
            else if(db->evariation == Group42Var4)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var4;
            }
            else if(db->evariation == Group42Var5)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var5;
            }
            else if(db->evariation == Group42Var6)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var6;
            }
            else if(db->evariation == Group42Var7)
            {
                config.analog_output_status[i].evariation = EventAnalogOutputStatusVariation::Group42Var7;
            }
            if (db->deadband > 0.0) {
                config.analog_output_status[i].deadband = db->deadband;
            }
            else
            {
                config.analog_output_status[i].deadband = sys->deadband;
            }
            
    
            if(db->clazz != 0)
            {
                switch (db->clazz ) 
                {
                    case 1:
                    {
                        config.analog_output_status[i].clazz = PointClass::Class1;
                        break;
                    }
                    case 2:
                    {
                        config.analog_output_status[i].clazz = PointClass::Class2;
                        break;
                    }
                    case 3:
                    {
                        config.analog_output_status[i].clazz = PointClass::Class3;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    dsize = sys->getTypeSize(Type_Counter);
    FPS_ERROR_PRINT(" %s >>  counter size = [%d] \n", __func__, (int)dsize);

    for (int i = 0; i < static_cast<int32_t>(dsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_Counter, i);
        if(db != NULL)
        {
            config.counter[i].clazz = PointClass::Class1;
            if(db->variation == Group20Var1)
            {
                config.counter[i].svariation = StaticCounterVariation::Group20Var1;
            }
            else if(db->variation == Group20Var2)
            {
                config.counter[i].svariation = StaticCounterVariation::Group20Var2;
            }
            else if(db->variation == Group20Var5)
            {
                config.counter[i].svariation = StaticCounterVariation::Group20Var5;
            }
            else if(db->variation == Group20Var6)
            {
                config.counter[i].svariation = StaticCounterVariation::Group20Var6;
            }

            if(db->evariation == Group22Var0)
            {
                //TODO config.analog_input[i].evariation = EventAnalogVariation::Group32Var0;
            }
            else if(db->evariation == Group22Var1)
            {
                config.counter[i].evariation = EventCounterVariation::Group22Var1;
            }
            else if(db->evariation == Group22Var2)
            {
                config.counter[i].evariation = EventCounterVariation::Group22Var2;
            }
            else if(db->evariation == Group22Var5)
            {
                config.counter[i].evariation = EventCounterVariation::Group22Var5;
            }
            else if(db->evariation == Group22Var6)
            {
                config.counter[i].evariation = EventCounterVariation::Group22Var6;
            }
    
            if(db->clazz != 0)
            {
                switch (db->clazz ) 
                {
                    case 1:
                    {
                        config.counter[i].clazz = PointClass::Class1;
                        break;
                    }
                    case 2:
                    {
                        config.counter[i].clazz = PointClass::Class2;
                        break;
                    }
                    case 3:
                    {
                        config.counter[i].clazz = PointClass::Class3;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    auto bsize = sys->getTypeSize(Type_Binary);
    FPS_ERROR_PRINT(" %s >>  binary size = [%d] \n", __func__, (int)bsize);

    for (int i = 0; i < static_cast<int32_t>(bsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_Binary , i);
        if(db != NULL)
        {
            // force the instance 
            config.binary_input[i].clazz = PointClass::Class1;
            config.binary_input[i].svariation = StaticBinaryVariation::Group1Var2;
            if(db->variation == Group1Var1)
            {
                config.binary_input[i].svariation = StaticBinaryVariation::Group1Var1;
            }

            if(db->evariation == Group2Var1)
            {
                config.binary_input[i].evariation = EventBinaryVariation::Group2Var1;
            }
            else if(db->evariation == Group2Var2)
            {
                config.binary_input[i].evariation = EventBinaryVariation::Group2Var2;
            }
            else if(db->evariation == Group2Var3)
            {
                config.binary_input[i].evariation = EventBinaryVariation::Group2Var3;
            }
            // 309 allow class designation for binaries
            if(db->clazz != 0)
            {
                switch (db->clazz ) 
                {
                    case 1:
                    {
                        config.counter[i].clazz = PointClass::Class1;
                        break;
                    }
                    case 2:
                    {
                        config.counter[i].clazz = PointClass::Class2;
                        break;
                    }
                    case 3:
                    {
                        config.counter[i].clazz = PointClass::Class3;
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    return config;
}

int HandleEvent(sysCfg *sys,  DbVar* db , int etype)
{

    auto event_rate = sys->event_rate;
    if(db->event_rate)
    {
        event_rate = db->event_rate;
    }
    if(event_rate)
    {
        if ((sys->tNow - db->last_seen) < event_rate)
        {
            etype = -1;
        }
    }
    if(!db->events)
    {
        etype = -1;
    }
    // db->events can override sys->events 
    if(!sys->events && !db->events)
    {
        etype = -1;
    }
    return etype;
}

