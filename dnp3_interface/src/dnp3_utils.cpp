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

// we only really want these events
//{"source":"DNP3","message":"DNP3  hybridos message [INFO   ] --[fps Logger ms(1592449037729) INFO    server - Listening on: 127.0.0.1:12502]\n","severity":1}
//{"source":"DNP3","message":"DNP3  xxhybridos message [INFO   ] --[fps Logger ms(1592449067320) INFO    xxhybridos - Connecting to: 127.0.0.1, port 12502]\n","severity":1}
//{"source":"DNP3","message":"DNP3  xxhybridos message [INFO   ] --[fps Logger ms(1592449067321) INFO    xxhybridos - Connected to: 127.0.0.1, port 12502]\n","severity":1}
//{"source":"DNP3","message":"DNP3  xxhybridos message [WARN   ] --[fps Logger ms(1592449007315) WARN    xxhybridos - Error Connecting: Connection refused]\n","severity":1}
//{"source":"DNP3","message":"DNP3  hybridos message [WARN   ] --[fps Logger ms(1592482027367) WARN    server - End of file]\n","severity":1}

// these are the only event states we now get...
void emit_event(sysCfg* sys, const char* source, const char* message, int severity)
{
    fims*pFims =  sys->p_fims;
    if (
        (strstr(message,"[CLOSED]") != NULL) 
        || (strstr(message,"[OPENING]") != NULL)
        || (strstr(message,"[OPEN]") != NULL)
        || (strstr(message,"[SHUTDOWN]") != NULL)
        || (strstr(message,"- End of file") != NULL)
        || (sys->debug > 2)
    ) 

    {
        cJSON* cj = cJSON_CreateObject();
        if(cj)
        {
            cJSON_AddStringToObject(cj, "source", source);
            cJSON_AddStringToObject(cj, "message", message);
            cJSON_AddNumberToObject(cj, "severity", severity);
            char* body = cJSON_PrintUnformatted(cj);
            if(body)
            {                
                pFims->Send("post", "/events", NULL, body);
                free(body);
            }
            cJSON_Delete(cj);
        }
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

#include <string>
#include <fstream>
#include <streambuf>

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

// "system": {
//        "protocol": "DNP3",
//        "version": 1,
//        "id": "dnp3_outstation",
//        "ip_address": "192.168.1.50",
//        "port": 502,
//        "master_address": 1,
//		  "station_address": 10
//    },const char *iotypToStr (int t)

OperationType TypeToOperationType(uint8_t arg)
{
    return static_cast<OperationType>(arg);
}

void addCjVal(cJSON* cj, DbVar* db, int flag, double val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
}

void addCjVal(cJSON* cj, DbVar* db,  int flag, uint32_t val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
}
void addCjVal(cJSON* cj, DbVar* db,  int flag, int32_t val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
}

void addCjVal(cJSON* cj, DbVar* db,  int flag, uint16_t val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
}

void addCjVal(cJSON* cj, DbVar* db,  int flag, int16_t val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddNumberToObject(cji, "value", val);
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddNumberToObject(cj, db->name.c_str(), val);
    }
}

// void addCjVal(cJSON* cj, const char* dname, int flag, int val)
// {
//     if((flag & PRINT_VALUE)|| (format > 0))
//     {
//         cJSON* cji = cJSON_CreateObject();
//         cJSON_AddNumberToObject(cji, "value", val);
//         cJSON_AddItemToObject(cj, dname, cji);
//     }
//     else
//     {
//         cJSON_AddNumberToObject(cj, dname, val);
//     }
// }

void addCjVal(cJSON* cj, DbVar* db, int flag, bool val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddBoolToObject(cji, "value", val);
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);
        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddBoolToObject(cj, db->name.c_str(), val);
    }
}

void addCjVal(cJSON* cj, DbVar* db, int flag, const char* val)
{
    if(flag & PRINT_VALUE)
    {
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddStringToObject(cji, "value", val);
        //if((format & 2) && (db->sflags.length() > 0))
        if(flag & PRINT_FLAGS)
        {
            cJSON_AddStringToObject(cji, "sflags", db->sflags.c_str());
            cJSON_AddNumberToObject(cji, "flags", db->flags);

        }
        cJSON_AddItemToObject(cj, db->name.c_str(), cji);
    }
    else
    {
        cJSON_AddStringToObject(cj, db->name.c_str(), val);
    }
}

// This is the root function
int addVarToCj(cJSON* cj,  DbVar* db, int flag)
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
            cJSON_AddItemToObject(cj,db->uri,cju);
        }
        cj = cju;
    }
    switch (db->type)
    {
        case Type_AnalogOS:
        case Type_Analog:
        {
            if(db->variation == Group30Var5)
            {
                addCjVal(cj, db, flag, db->valuedouble);
                
            }
            else
            {
                if(db->sign)
                {
                    addCjVal(cj, db, flag, static_cast<double>(static_cast<int32_t>(db->valuedouble)));
                }
                else
                {
                    addCjVal(cj, db, flag, db->valuedouble);        
                }
            }

            break;
        }

        case Type_BinaryOS:
        case Type_Binary:
        {
            bool bval = false;
            if (db->valuedouble > 0.0) bval = true;
            addCjVal(cj, db, flag, bval);
            break;
        }
        case Type_Crob:
        {
            // TODO switch response to suit the original data entry 
            // Int
            // flag (true/false)
            // string
            // need to reply with the same response.
            // LATCH_ON etc
            if (db->crob_input == CROB_STRING)
            {
                const char* cmd = OperationTypeSpec::to_string(OperationTypeSpec::from_type(db->crob));
                addCjVal(cj, db, flag, cmd);
            }
            if (db->crob_input == CROB_INT)
            {
                addCjVal(cj, db, flag, db->crob);
            }
            if (db->crob_input == CROB_BOOL)
            {
                if(db->crob_true == db->crob)
                {
                    addCjVal(cj, db, flag, true);
                }
                else if(db->crob_false == db->crob)
                {
                    addCjVal(cj, db, flag, false);
                }
                else if(0x03 == db->crob)
                {
                    addCjVal(cj, db, flag, true);
                }
                else 
                {
                    addCjVal(cj, db, flag, false);
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
                addCjVal(cj, db, flag, static_cast<uint16_t>(dval));
            }
            else
            {
                addCjVal(cj, db, flag, static_cast<int16_t>(dval));
                //addCjVal(cj, dname, flag, static_cast<int32_t>(db->valuedouble));                
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
                addCjVal(cj, db, flag, static_cast<uint32_t>(dval));
                    
            }
            else
            {
                FPS_DEBUG_PRINT("*** %s Found variable [%s] type  %d sign %d using valuedouble %f\n"
                                , __FUNCTION__, db->name.c_str(), db->type, db->sign, db->valuedouble);
                addCjVal(cj, db, flag, static_cast<int32_t>(dval));
               // addCjVal(cj, dname, flag, static_cast<int32_t>(db->valuedouble));                
            }            
            break;
        }
        case AnF32:
        {
            addCjVal(cj, db, flag, db->valuedouble);
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

int addVarToCj(cJSON* cj, std::pair<DbVar*,int>dbp)
{   
    DbVar* db = dbp.first;
    int flag = dbp.second;
    return addVarToCj(cj, db, flag);
}

int addVarToCj(cJSON* cj,  DbVar* db)
{
    return addVarToCj(cj, db, 0);
}

int addVarToCj(sysCfg* sys, cJSON* cj, const char* uri, const char* dname)
{
    DbVar* db = sys->getDbVar(uri, dname);

    return addVarToCj(cj, db, 0);
}
// these are the types decoded from the config file
// must match the typedef sequence
const char* dreg_types[] = { "AnOPInt16", "AnOPInt32", "AnOPF32", "CROB", "analog", "binary", "analogOS","binaryOS", 0 };

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
        case 0x0203:
            return "Group2Var3";
        default:
            return "Undecoded";
    }
    return "Undecoded";
}
// Group30Var1  Analog Input 32 bit with flags
// Group30Var2  Analog Input 16 bit with flags
// Group30Var3  Analog Input 32 bit without flags
// Group30Var4  Analog Input 16 bit without flags
// Group30Var5  Analog Input single precision with flags (Float) << these allow floating point numbers
// Group30Var6  Analog Input double precision with flags (Double)
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

// modbus system example
// "notes":" a collection of inputs from the system all using default uri",
// 		"id": "sel_735",
// 		"ip_address": "127.0.0.1",
// 		"port": 12502,
// 		"ip_address2": "192.168.0.136",
// 		"port2": 502,
// 		"frequency": 500,
// 		"freq1": 500,
// 		"freq2": 500,
// 		"freq3": 500,
//      "rangeFreq": 500,
//      "rangeAStart": 600,
//      "rangeAStop": 672,
//      "rangeBStart": 600,
//      "rangeBStop": 672,
//
//      "unsol":1/0,
// 		"byte_swap": false
//{
//   "files":[
//        "file1",
//        "file2",
//        "file3"
//    ]
//}
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

    sys->baud = 9600;
    sys->dataBits = 8;
    sys->stopBits = 1;

    sys->parity = strdup("None");// opendnp3::Parity::None;
    sys->flowType = strdup("None");//opendnp3::FlowControl::None;
    //sys->flowType = strdup("None");//opendnp3::FlowControl::None;
    sys->asyncOpenDelay=500; //(opendnp3::TimeDuration::Milliseconds(500));
     //baud(9600),
        //   dataBits(8),
        //   stopBits(opendnp3::StopBits::One),
        //   parity(opendnp3::Parity::None),
        //   flowType(opendnp3::FlowControl::None),
        //   asyncOpenDelay(opendnp3::TimeDuration::Milliseconds(500))
    
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

// big wrinkle here.. Lets parse the modbus file  
// "registers": 
// 	[
// 		{
// 			"type": "Input Registers",
// 			"dnp3_type": "Analog",
// 			"starting_offset": 350,
// 			"number_of_registers": 26,
// 			"map":
// 			[
// 				{
// 					"id": "current_a",
// 					"offset": 350,
// 					"size": 2,
// 					"scale":100,
// 					"signed": true
//               }
//          ]
//      }
//  ]

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
        cJSON *cjcstring, *cjctrue, *cjcfalse, *cjcint;

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
        

//        if (id == NULL || offset == NULL || id->valuestring == NULL)
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

        // rework
        // DbVars are sorted by uri's now.
        // we can have the same DbVar name for different uris.
        // incoming messages from master only know type (AnIn16,AnIn32,AF32,CROB) and offset.
        // for these items offset is still set by the config file.
        // The return path from outstation to master still has type an index. The system has changed to use the offset from the conig file here too.
        // so we can no longer use the vector position as an index.
        // items are added from the config file against a registered uri ot the default one.
        // pubs, gets and sets from FIMS ues a combination of URI + var name to find vars.
        // data incoming from master to outstation will use type and offset to find the variable.
        // each type will have a map <int,DbVar *> to search
        // Data base for the outstation vars (analog and binary) will no longer use the vector size but max offset as db size ????
        // the <int , dBVar *> map can also  be used for these vars
        //
        // Get this going as follows 
        //   1.. parse config file using the uri's.
        //   2.. get the uri/var name search  working for FIMS querues
        //   3.. get the int, DbVar* search / map  running to find vars for outstation builder.
        //   4.. get the  mapping working for the incoming commands.
        //    
        // 

        // the cjidx fields will ovride the auto idx.
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

        if(cjctrue && cjctrue->valuestring && (db!=NULL))
        {
            db->crob_true = OperationTypeSpec::to_type(OperationTypeSpec::from_string(cjctrue->valuestring));
        }

        if(cjcfalse && cjcfalse->valuestring && (db!=NULL))
        {
            db->crob_false = OperationTypeSpec::to_type(OperationTypeSpec::from_string(cjcfalse->valuestring));
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
            // keep a count of the objects.
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
            // do it first time in parsing
            // need to follow up after parse complete to pick up remaining allocations
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
            // register the uri 
            // was nuri
            sys->addDbUri(db->uri, db);

            // Deal with linkback option
            // the master SOEhandler will cause the linkback value to be updated.
            // we may mirror this in the outstation handler too.  
            // master and outstation have different linkback types
            if((mytype == Type_Analog) || (mytype == Type_Binary)) 
            {
                if (who == DNP3_MASTER)
                {
                    if(linkuri)
                    {
                        // copy the variable uri into the linkuri string.
                        // char* linkback; 
                        // DbVar* linkb
                        // the variable may not exist yet so it will be found later.
                        if (linkuri->valuestring != NULL)
                        {
                            FPS_ERROR_PRINT(" Setting linkuri  name to [%s]\n", linkuri->valuestring);
                            db->linkuri = strdup(linkuri->valuestring);
                            //db->linkb = sys->getDbVar(nuri, linkback->valuestring);
                        }
                    }
                    if(linkback)
                    {
                        // copy the variable name into the linkback string.
                        // char* linkback; 
                        // DbVar* linkb
                        // the variable may not exist yet so it will be found later.
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
                            //db->linkb = sys->getDbVar(nuri, linkback->valuestring);
                        }
                    }

                    if(linkback)
                    {
                        // copy the variable name into the linkback string.
                        // char* linkback; 
                        // DbVar* linkb
                        // the variable may not exist yet so it will be found later.
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
// "registers": 
// 	[
// 		{
// 			"type": "Input Registers",
// 			"dnp3_type": "Analog",
// 			"starting_offset": 350,
// 			"number_of_registers": 26,
// 			"map":
// 			[
// 				{
// 					"id": "current_a",
// 					"offset": 350,
// 					"size": 2,
// 					"scale":100,
// 					"signed": true
//               }
//          ]
//      }
//  ]

bool parse_modbus(cJSON* cj, sysCfg* sys, int who)
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
            FPS_ERROR_PRINT("modbus registers map object is not an array ! \n");
            return false;
        }

        // here are the tpe options
        // no dnp3_type then look at type
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

    // config file has "objects" with children groups "binary" and "analog"
    // who is needd to stop cross referencing linkvars
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

    // after this is done we auto assign the indexes here
    sys->assignIdx();

    return true;
}
// TODO check out the first blank one 
// TODO put these in the sys strucure
int getSysUris(sysCfg *sys[], int who, const char** &subs, bool* &bpubs, int nums)
{
    int num = 0;
    int i;
    //sysCfg** ss = sys;
    for (i = 0 ; i< nums; i++)
    {
        sys[i]->num_subs = sys[i]->getSubs(NULL, 0, who);
        num += sys[i]->num_subs;
        //sys++;
    }
    //int num = sys->getSubs(NULL, 0, who);
    subs = (const char **) malloc((num+3) * sizeof(char *));
    if(subs == NULL)
    {
        FPS_ERROR_PRINT("Failed to creae subs array.\n");
        //rc = 1;
        return -1;
    }
    bpubs = (bool *) malloc((num+3) * sizeof(bool));
    memset(bpubs, false, (num+3) * sizeof(bool)); // all false we hope
    num = 0;
    //sys = ss;
    for (i = 0 ; i< nums; i++)
    {
        num += sys[i]->getSubs(&subs[num], sys[i]->num_subs, who);
        //sys++;
    }
    //num = sys->getSubs(subs, num, who);

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

//std::vector<std::pair<DbVar*,int>>dbs; // collect all the parsed vars here
cJSON* parseValues(dbs_type& dbs, sysCfg*sys, fims_message*msg, int who, cJSON* body_JSON)
{
    cJSON* itypeValues = body_JSON;
    cJSON* cjit = NULL;
    if(sys->debug)
        FPS_ERROR_PRINT("Found variable list or array uri [%s] \n", msg->uri);
    // decode values may be in an array 
    // TODO arrays are DEPRECATED
    if (itypeValues)
    { 
        if (cJSON_IsArray(itypeValues)) 
        {
            //if(sys->debug)
            FPS_ERROR_PRINT("Error Found array of variables need simple list instead \n");
            return body_JSON;//return dbs.size();

            //cJSON_ArrayForEach(cjit, itypeValues) 
            //{
            //    cJSON* cjo = cJSON_GetObjectItem(cjit, "offset");
            //    cJSON* cjv = cJSON_GetObjectItem(cjit, "value");
            //    addValueToVec(dbs, sys, msg, cjo->valuestring, cjv, 0);
            //}
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
            // this may not work here
            // fims_send -m set -u /interfaces/dnp3_outstation/uline_flows_mv '{"value":0}'
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
                    // this simply adds the valu to the dbs vec
                    // perhaps we would like to set up some other things from this input

                    addValueToVec(dbs, sys, mcuri, cjit->string, cjit, flag);
                    //addValueToVec(dbs, sys, msg, cjit->string, cjit, flag);
                }
                cjit = cjit->next;
            }
            if(sys->debug>1)
                FPS_ERROR_PRINT("***** Done with variable list \n\n");
        }
    }
    return body_JSON;//return dbs.size();
}

// uri handling
// reate a list of uris from cofig data
// a var name can be partly a uri
//   gateway/system1/voltageSP
// with a uri /components/fleetmanager/
// this incoming uri in the fims request will be
// get  /components/fleetmanager/[gateway/system1/voltageSP]
// anything past /components/fleetmanager/  must be a name simple
// but what do we forward to /components/gateway1
// so we have a set uri , used for pubs
// and a get uri used to pick up pubs etc.
// so the test checkUri makes sure we have a name / uri match
//  
// si  
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

// we have an incoming uri
// outstation normal    
//       /<uri>             set values on pubs
// outstation reply
//       /interfaces/id/reply/<uri>    set reply flag , remove /<id>/reply from the uri
// outstation exception
//       /local/<uri>           allow gets and sets as well

// master normal    
//       /<uri>             get/ set values on sets
// master reply
//       /reply/id/<uri>    set reply flag , remove /reply from the uri
// master exception
//       
//
//std::vector<std::pair<DbVar*,int>>dbs; // collect all the parsed vars here
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
                FPS_ERROR_PRINT("%s fims message body error  method (%s) uri (%s) nfrags %d \n"
                    ,__FUNCTION__, msg->method, msg->uri, (int)msg->nfrags);
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

    // special case [/interfaces/hybridos/reply/interfaces/hybridos] 
    // valid uri
    // /assets/feeders/feed_6
    // 
    // possible inputs
    // /assets/feeders/feed_6/feeder_kW_slew_rate  ==> process single  
    // /assets/feeders/feed_6                      ==> process list
    // /assets/feeders/feed_6/test/feeder_kW_slew_rate    == reject unless we have a var called "test/feeder_kW_slew_rate"
    //
    char* name = NULL;
    int flags = 0;
    //DbVar* db = NULL;
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

    // get is OK codewise..
    // find a good looking uri and find the number of frags
    //
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
            FPS_ERROR_PRINT("fims method [%s] almost  supported for [%d]\n", msg->method, who);

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
                FPS_ERROR_PRINT("Found SINGLE variable [%s] type  %d \n", db->name.c_str(), db->type); 
            // send the var and flag back to the dnp3_outstation code
            dbs.push_back(std::make_pair(db, flag));
            //return body_JSON;
        }
        // else get them all
        // but only the ones associated with the uri
        // TODO detect flag here
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

    // Allow "pub" to "set" in the outstation
    if(strcmp(msg->method,"set") == 0 || (strcmp(msg->method,"pub") == 0 && (who == DNP3_OUTSTATION)))
    {
        // watch out for sets on /interfaces/outstation/dnp3_outstation/reply/dnp3_outstation
        // handle a single item set getting better 
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
        
        //uri = msg->pfrags[fragptr+2];  // TODO check for delim. //components/master/dnp3_outstation/line_voltage/stuff
        // look for '{"debug":"on"/"off"}' or '{"scan":1,2 or 3} {"unsol": 1 or 0} {"class" '{"<varname>":newclass}}
        if((flags & URI_FLAG_SYSTEM) == URI_FLAG_SYSTEM)
        {
            FPS_DEBUG_PRINT("fims system command [%s] body [%s]\n", msg->uri, msg->body);
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
            return body_JSON;

        }
        // TODO redundant
        if((flags & URI_FLAG_REPLY) == URI_FLAG_REPLY)
        {
            if(sys->debug )
                FPS_ERROR_PRINT("fims message reply uri ACCEPTED Body  [%s] \n", msg->body);
        }            
        
        if((flags & URI_FLAG_SINGLE) == URI_FLAG_SINGLE)
        {
            // process a single var
            if(sys->debug)
                FPS_ERROR_PRINT("Found Single variable [%s] type  %d run set %d\n"
                                        , db->name.c_str()
                                        , db->type
                                        , who
                                        );    
            int flag = 0;
            if(sys->debug)
                FPS_DEBUG_PRINT("Found variable type  %d \n", db->type);
            itypeValues = body_JSON;
            // allow '"string"' OR '{"value":"string"}'
            if(!itypeValues)
            {
                FPS_ERROR_PRINT(" pub not a valid json object\n");
                return body_JSON;                   
            }
            if(itypeValues->type == cJSON_Object)
            {

                // we can have flag and / or time
                // just deal with flag for now
                if(cJSON_GetObjectItem(itypeValues, "flag"))
                {
                    FPS_ERROR_PRINT("%s >>  found flag data\n",__func__);
                    itypeFlag = itypeValues;
                }
                else
                {
                    itypeFlag = nullptr;
                }

                itypeValues = cJSON_GetObjectItem(itypeValues, "value");
                if(!itypeValues)
                {
                   FPS_ERROR_PRINT(" pub Value data missing\n");
                   return body_JSON;                   
                }
                flag |= PRINT_VALUE;
            }
            if(sys->debug)
            {
                FPS_ERROR_PRINT("Found variable [%s] type  %d  sign %d body [%s]  flag %d \n"
                                        , db->name.c_str()
                                        , db->type
                                        , db->sign
                                        , msg->body
                                        , flag);
            }
            // Only Crob gets a string 
            if(itypeValues)
            {
                if (itypeValues->type == cJSON_String)
                {
                    if(db->type == Type_Crob)
                    {
                        CommandStatus status = (CommandStatus)0;
                        uint8_t cval = OperationTypeSpec::to_type(StringToOperationType(itypeValues->valuestring));
                        db->crob_input = CROB_STRING;
                        sys->setDbVarIx(Type_Crob, db->idx, cval, status, itypeFlag);
                        db->initSet = 1;
                        dbs.push_back(std::make_pair(db, flag));
                        if(sys->debug)
                        {
                            FPS_ERROR_PRINT(" ***** %s Adding Direct CROB value %s offset %d idx %d uint8 cval 0x%02x\n"
                                        , __FUNCTION__, itypeValues->valuestring, db->offset, db->idx
                                        , cval
                                        );
                        }
                    }
                // TODO any other strings
                }
                else
                {
                    sys->setDbVar(db, itypeValues, itypeFlag, itimeFlag);
                    // add to dbs for main function to process
                    dbs.push_back(std::make_pair(db, flag));
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
        // TODO pick out other things we may need to add/setup...
        // Also do we want to dynamically add new items nah the restart is painless.
        // {
        //             "id": "test_unsigned_int16",
        //             "offset": 2,
        //             "signed":false,
        //             "name": "test int signed int F 16",
        //             "uri": "/site/operation"
        // }

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
        //  if(sys->debug)
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
            // we have a string so turn it into a crob value , use crob_true / crob_false if we can
            db->crob_input = CROB_STRING;
            sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);

            //sys->setDbVar(curi, name, cval, cjFlags, cjTime);

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
            // we have a bool so turn it into a crob value , use crob_true / crob_false if we can
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

            // we have an int  crob value no conversion  needed
            db->crob_input = CROB_INT;
            sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
            //sys->setDbVar(curi, name, cval, cjFlags, cjTime);
            
        }
        dbs.push_back(std::make_pair(db, flag));
    }
    else if (
            (db->type == Type_Analog) ||
            (db->type == Type_Binary) ||
            (db->type == AnIn16) ||
            (db->type == AnIn32) ||
            (db->type == AnF32)
            )
    {
        if(sys->debug)
            FPS_ERROR_PRINT( " *************** %s Var [%s] processed flags %p \n",__FUNCTION__, name, (void *) cjFlags);  
        sys->setDbVar(curi, name, cjvalue, cjFlags, cjTime);
        dbs.push_back(std::make_pair(db, flag));
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
// TODO need no dbs option
// add who to sys
int addValueToVec(dbs_type& dbs, sysCfg*sys, fims_message* msg, const char* name, cJSON *cjvalue, int flag)
{
    int ret;
    //sys->confirmUri(db, msg->uri, who, name, flags);
    // cjoffset must be a name
    // cjvalue may be an object
    // msg->uri // just take it up to the name
    char* curi = strdup(msg->uri);
    char* mcuri = strstr(curi, name);
    if(mcuri != NULL)
    {
        // force a termination at the '/'
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

// this sends out sets for each command received.
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
    if (1||cJSON_IsObject(cj))
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
    else
    {
        cJSON*cjv = cJSON_CreateObject();
        cJSON_AddItemToObject(cjv, db->name.c_str(), cj);

        char *out = cJSON_PrintUnformatted(cjv);
        cJSON_Delete(cjv);
        if (out) 
        {
            char tmp[2048];
            snprintf(tmp, sizeof(tmp), "/%s", uri );

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

// handle output scaling
double dbOpScale(DbVar* db, double dval)
{
    if  ((db->scale > 0.0) || (db->scale < 0.0))
    {
        dval /= db->scale;
    }

    return dval;
}

// used in outstation comand handler to publish changes  scale after conversion
void sysdbAddtoRecord(sysCfg* sys, const char* field, const opendnp3::AnalogOutputInt16& cmd, uint16_t index)
{
    DbVar* db = sys->getDbVarId(AnIn16 , index);
    if (db)
    {
        // cmd.status has the following fields.
        //

        //double dval = dbOpScale(db, static_cast<double>(cmd.value));
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

        // cJSON* cjf = sysdbFindAddArray(sys, field);
        cJSON* cjv = cJSON_CreateObject();
        //cJSON* cji = cJSON_CreateObject();

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
                          , dval_save,  dval,  static_cast<int32_t>(index) );
            dval = dbOpScale(db, dval);
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(dval);
            } else {
               cJSON_AddNumberToObject(cjv, "value", static_cast<uint32_t>(dval));         
            } 
            //cJSON_AddNumberToObject(cjv,"value", static_cast<int16_t>(dval));
        }
        else
        {
            if(sys->debug)
                FPS_ERROR_PRINT("%s unsigned AnIn16 dval %f  val %u at index %d\n", __FUNCTION__
                          , dval, u16val, static_cast<int32_t>(index) );
            // remove -ve value for unsigned turn it into a + ve value between SHRT_MAX and USHRT_MAX 
            if(dval < 0)
            {
                u16val = (SHRT_MAX) + (dval * -1.0);
                FPS_ERROR_PRINT("%s unsigned adjustment AnIn16 dval %f  new val %u at index %d\n", __FUNCTION__
                              , dval, u16val, static_cast<int32_t>(index) );
            }
            dval = dbOpScale(db, static_cast<double>(u16val));
            if (db->fmt == 0) 
            {
                cJSON_Delete(cjv);
                cjv = cJSON_CreateNumber(dval);
            } else {
               cJSON_AddNumberToObject(cjv, "value", static_cast<uint32_t>(dval));         
            } 
            //cJSON_AddNumberToObject(cjv, "value", static_cast<uint16_t>(dval));            
        }
        sendCmdSet(sys, db, cjv);
        cJSON_Delete(cjv);
    }
    else
    {
        FPS_ERROR_PRINT("%s unable to find AnIn16 at index %d\n", __FUNCTION__, static_cast<int32_t>(index) );
    }    
}

// scale after conversion
void sysdbAddtoRecord(sysCfg* sys, const char* field, const opendnp3::AnalogOutputInt32& cmd, uint16_t index)
{
    DbVar* db = sys->getDbVarId(AnIn32 , index);
    if (db)
    {
        //double dval = dbOpScale(db, static_cast<double>(cmd.value));
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
               cJSON_AddNumberToObject(cjv, "value", static_cast<uint32_t>(dval));         
            } 
        }
        else
        {
            if(sys->debug)
                FPS_ERROR_PRINT("%s unsigned AnIn32 dval %f val %u at index %d\n", __FUNCTION__
                        , dval, u32val, static_cast<int32_t>(index) );

            // remove -ve value for unsigned turn it into a + ve value between INT_MAX and UINT_MAX 
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
                cJSON_AddNumberToObject(cjv, "value", static_cast<uint32_t>(dval));         
            } 
//          cJSON_AddNumberToObject(cjv, "value", u32val);            
            
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
        // cJSON* cjf = sysdbFindAddArray(sys, field);
        cJSON* cjv = cJSON_CreateObject();
        bool sendOK = true;
        //cJSON* cji = cJSON_CreateObject();
        //   NUL = 0x0,
        //   PULSE_ON = 0x1,
        //   PULSE_OFF = 0x2,
        //   LATCH_ON = 0x3,
        //   LATCH_OFF = 0x4,
        //   Undefined = 0xFF
        {
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
            else if(db->crob_string)
            {
                //cJSON_AddStringToObject(cjv, "value", cmd);
                if (db->crob == db->crob_true)
                {
                    if (db->fmt == 0) 
                    {
                        cJSON_Delete(cjv);
                        cjv = cJSON_CreateString (cmd);
                    } else {
                       cJSON_AddStringToObject(cjv, "value", cmd);
                    }
                }
                else if (db->crob == db->crob_false)
                {
                    if (db->fmt == 0) 
                    {
                        cJSON_Delete(cjv);
                        cjv = cJSON_CreateString (cmd);
                    } else {
                        cJSON_AddStringToObject(cjv, "value", cmd);
                    }
                }
            }
            else
            {
                if (db->crob == db->crob_true)
                {
                    if (db->fmt == 0) 
                    {
                        cJSON_Delete(cjv);
                        cjv = cJSON_CreateBool (true);
                    } else {
                        cJSON_AddBoolToObject(cjv, "value", true);
                    }
                    //cJSON_AddBoolToObject(cjv, "value", true);

                }
                else if (db->crob == db->crob_false)
                {
                    if (db->fmt == 0) 
                    {
                        cJSON_Delete(cjv);
                        cjv = cJSON_CreateBool (false);
                    } else {
                        cJSON_AddBoolToObject(cjv, "value", false);
                    }
                    //cJSON_AddBoolToObject(cjv, "value", false);
                }
                else if (db->crob == 3)
                {
                    if (db->fmt == 0) 
                    {
                        cJSON_Delete(cjv);
                        cjv = cJSON_CreateBool (true);
                    } else {
                        cJSON_AddBoolToObject(cjv, "value3", true);
                    }

                    //cJSON_AddBoolToObject(cjv, "value3", true);
                }
                else if (db->crob == 4)
                {
                    if (db->fmt == 0) 
                    {
                        cJSON_Delete(cjv);
                        cjv = cJSON_CreateBool (false);
                    } else {
                        cJSON_AddBoolToObject(cjv, "value4", false);
                    }

                    //cJSON_AddBoolToObject(cjv, "value4", false);
                }
                else
                {
                    sendOK = false;
                }
            }
        }
        // if(db->crob == 0x01)
        //     cJSON_AddBoolToObject(cjv, "ovalue", true);
        // else
        //     cJSON_AddBoolToObject(cjv, "ovalue", false);
        // cJSON_AddStringToObject(cjv, "value", cmd);
        //cJSON_AddItemToObject(cji, db->name.c_str(), cjv);
        // cJSON_AddItemToArray(cjf, cji);
        // sys->cjloaded++;
        if(sendOK)
            sendCmdSet(sys, db, cjv);
        cJSON_Delete(cjv);
    }
    else
    {
        FPS_ERROR_PRINT("%s unable to find CROB at index %d\n", __FUNCTION__, static_cast<int32_t>(index) );
    }

}

// TODO allow a setup option in the config file to supply the SOEname
const char* cfgGetSOEName(sysCfg* sys, const char* fname)
{
    return fname;
}
//IF it is commented out , it means that it no longer is in OperationType
const OperationType StringToOperationType(const char* codeWord)
{
//    if (strcmp("NUL_CANCEL", codeWord) == 0)
//        return OperationType::NUL_CANCEL;
    if (strcmp("NUL", codeWord) == 0)
        return OperationType::NUL;
//    if (strcmp("PULSE_ON_CANCEL", codeWord) == 0)
//        return OperationType::PULSE_ON_CANCEL;
    if (strcmp("PULSE_ON", codeWord) == 0)
        return OperationType::PULSE_ON;
//    if (strcmp("PULSE_OFF_CANCEL", codeWord) == 0)
//        return OperationType::PULSE_OFF_CANCEL;
    if (strcmp("PULSE_OFF", codeWord) == 0)
        return OperationType::PULSE_OFF;
//    if (strcmp("LATCH_ON_CANCEL", codeWord) == 0)
//        return OperationType::LATCH_ON_CANCEL;
    if (strcmp("LATCH_ON", codeWord) == 0 || strcmp("true", codeWord) == 0)
        return OperationType::LATCH_ON;
//    if (strcmp("LATCH_OFF_CANCEL", codeWord) == 0)
//        return OperationType::LATCH_OFF_CANCEL;
    if (strcmp("LATCH_OFF", codeWord) == 0 || strcmp("false", codeWord) == 0)
        return OperationType::LATCH_OFF;
//    if (strcmp("CLOSE_PULSE_ON_CANCEL", codeWord) == 0)
//        return OperationType::CLOSE_PULSE_ON_CANCEL;
//    if (strcmp("CLOSE_PULSE_ON", codeWord) == 0)
//        return OperationType::CLOSE_PULSE_ON;
//    if (strcmp("TRIP_PULSE_ON_CANCEL", codeWord) == 0)
//        return OperationType::TRIP_PULSE_ON_CANCEL;
//    if (strcmp("TRIP_PULSE_ON", codeWord) == 0)
//        return OperationType::TRIP_PULSE_ON;
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
// we either have a list in a file or a list of files
//

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
  
    // just deal with analog vars and Group30Var5, this allows floating point numbers through the system
    //auto dsize = sys->dbVec[Type_Analog].size();
    auto dsize = sys->getTypeSize(Type_Analog);
    for (int i = 0; i < static_cast<int32_t>(dsize); i++)
    {
        DbVar* db = sys->getDbVarId(Type_Analog, i);
        if(db != NULL)
        {
            //config.analog_input[i].value.value = 0;
            config.analog_input[i].clazz = PointClass::Class1;
            if(db->variation == Group30Var5)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var5;
            }
            else if(db->variation == Group30Var2)
            {
                config.analog_input[i].svariation = StaticAnalogVariation::Group30Var2;
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
                //if (sys->useVindex)
                //    config.analog_input[i].vIndex = db->idx;
            }

            // else if(db->variation == Group30Var2)
            // {
            //     config.analog_input[i].svariation = StaticAnalogVariation::Group30Var2;
            // }
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

            //if (sys->useVindex)
            //    config.binary_input[i].vIndex = db->idx;
        }
    }

    // example of configuring analog index 0 for Class2 with floating point variations by default
    //config.analog_input[0].clazz = PointClass::Class2;
    //config.analog_input[0].svariation = StaticAnalogVariation::Group30Var5;
    //config.analog_input[0].evariation = EventAnalogVariation::Group32Var7;
    //config.analog_input[0].deadband = 1.0; ///EventAnalogVariation::Group32Var7;   
    return config;
}

// Note does not log skipped events 
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

//
// MODBUS does a send on each variable 
// bool process_dnp3_message(int bytes_read, int header_length, datalog* data, system_config* config, server_data* server_map, bool serial, uint8_t * query)
//sprintf(uri, "%s/%s", reg->uri, reg->reg_name);
//                     cJSON_AddBoolToObject(send_body, "value", (value == 0xFF00) ? cJSON_True : cJSON_False);
//                     char* body_msg = cJSON_PrintUnformatted(send_body);
//                     server_map->p_fims->Send("set", uri, NULL, body_msg);

