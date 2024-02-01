#pragma once
#ifndef DNP3_UTILS_H
#define DNP3_UTILS_H
/*
 * mapping shared library
 *      pwilshire      5/5/2020
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cjson/cJSON.h>
#include <fims/libfims.h>
#include <fims/fps_utils.h>

#include <map>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <stdio.h>

extern "C"
{
#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwpltmr.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpsim.h"
#include "tmwtargio.h"
}

#include "gcom_dnp3_fims.h"
#include "gcom_dnp3_system_structs.h"

#ifndef FPS_INFO_PRINT
#define FPS_INFO_PRINT printf
#endif
#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#endif
#ifndef FPS_DEBUG_PRINT
#define FPS_DEBUG_PRINT printf
#endif

// Max number of error logs since boot allowed to be published
#define SYS_SPAM_LIMIT 64

#define STATE_RESET 0
#define STATE_ONLINE 1
#define STATE_COMM_LOST 2
#define STATE_INIT -1

#define CROB_STRING  0
#define CROB_BOOL    1
#define CROB_INT     2

#define MAX_FIMS_CONNECT 5

#define DNP3_UTILS_VERSION "3.0.6"

const char *variation_encode(int var);

enum class DNP3FlowControl : uint8_t {
    None,
    Hardware,
    Software
};

struct char_cmp
{
    bool operator()(const char *a, const char *b) const
    {
        return strcmp(a, b) < 0;
    }
};

int static_variation_decode(const char *ivar, int type);
int event_variation_decode(const char *ivar, int type);
const char *iotypToStr(int t);
int iotypToId(const char *t);

// legacy specifications for register types
// my hope is we can eventually get rid of these
enum Type_of_Var
{
    AnIn16,        // Group 40 Variation 2: Analog Output Status
    AnIn32,        // Group 40 Variation 1: Analog Output Status
    AnF32,         // Group 40 Variation 3: Analog Output Status
    Type_Crob,     // Group 12 Variation 1: Binary Commands
    Type_Analog,   // Group 30 all variations: Analog Inputs
    Type_Binary,   // Group 1 both variations: Binary Inputs
    Type_AnalogOS, // Group 40 all variations: Analog Output Status
    Type_BinaryOS, // Group 10 all variations: Binary Output Status
    NumTypes
};


enum BinaryInputStaticVariations {
    Group1Var0, // non-existent placeholder
    Group1Var1, // binary input, packed format
    Group1Var2 // binary input with flags
};

enum BinaryInputEventVariations {
    Group2Var0, // non-existent placeholder
    Group2Var1, // binary input without time
    Group2Var2, // binary input with absolute time
    Group2Var3 // binary input with relative time
};

enum BinaryOutputStaticVariations {
    Group10Var0, // non-existent placeholder
    Group10Var1, // binary output, packed format
    Group10Var2 // binary output with flags
};

enum BinaryOutputEventVariations {
    Group11Var0, // non-existent placeholder
    Group11Var1, // binary input without time
    Group11Var2 // binary input with time
};

enum BinaryOutputCommandVariations {
    Group12Var0, // non-existent placeholder
    CROB, // control relay output blocks (CROBs)
    PCB, // pattern control blocks (PCBs)
    PatternMask // no idea what this does
};

enum AnalogOutputStaticVariations {
    Group40Var0, // non-existent placeholder
    Group40Var1, // 32-bit signed integer with flag
    Group40Var2, // 16-bit signed integer with flag
    Group40Var3, // 32-bit floating-point with flag
    Group40Var4, // 64-bit floating-point with flag    
};

enum AnalogOutputEventVariations {
    Group42Var0, // non-existent placeholder
    Group42Var1, // 32-bit signed integer without time
    Group42Var2,// 16-bit signed integer without time
    Group42Var3,// 32-bit signed integer with time
    Group42Var4,// 16-bit signed integer with time
    Group42Var5,// 32-bit float without time
    Group42Var6,// 64-bit float without time
    Group42Var7,// 32-bit float with time
    Group42Var8 // 64-bit float with time  
};

enum AnalogInputStaticVariations
{
    Group30Var0, // non-existent placeholder
    Group30Var1, // 32-bit signed integer with flag
    Group30Var2, // 16-bit signed integer with flag
    Group30Var3, // 32-bit signed integer without flag
    Group30Var4, // 16-bit signed integer without flag
    Group30Var5, // 32-bit floating-point with flag
    Group30Var6, // 64-bit floating-point with flag
};

enum AnalogInputEventVariations
{
    Group32Var0, // non-existent placeholder
    Group32Var1, // 32-bit signed integer without time
    Group32Var2,// 16-bit signed integer without time
    Group32Var3,// 32-bit signed integer with time
    Group32Var4,// 16-bit signed integer with time
    Group32Var5,// 32-bit float without time
    Group32Var6,// 64-bit float without time
    Group32Var7,// 32-bit float with time
    Group32Var8 // 64-bit float with time
};

enum
{
    URI_FLAG_REPLY = 1 << 1,
    URI_FLAG_GET = 1 << 2,
    URI_FLAG_SET = 1 << 3,
    URI_FLAG_URIOK = 1 << 4,
    URI_FLAG_NAMEOK = 1 << 5,
    URI_FLAG_SINGLE = 1 << 6,
    URI_FLAG_SYSTEM = 1 << 7,
    URI_FLAG_DUMP = 1 << 8,
    URI_FLAG_VALUE = 1 << 9,
    URI_FLAG_FLAGS = 1 << 10,
    URI_FLAG_TIME = 1 << 11

};

char *getDefUri(GcomSystem &sys, int who);
void addDbUri(GcomSystem &sys,const char *uri, TMWSIM_POINT *dbPoint);
int getUris(GcomSystem &sys,int who);
int getSubs(GcomSystem &sys,const char **subs, int num, int who);
bool checkUris(GcomSystem &sys,int who);
int addBits(GcomSystem &sys,TMWSIM_POINT *dbPoint, cJSON *bits);
int addBit(GcomSystem &sys,TMWSIM_POINT *dbPoint, const char *bit);
char *confirmUri(GcomSystem &sys,TMWSIM_POINT *&dbPoint, const char *uri, int who, char *&name, int &flags);
void showNewUris(GcomSystem &sys);
void cleanUpDbMaps(GcomSystem &sys);
TMWSIM_POINT *getDbVar(GcomSystem &sys, std::string_view uri, std::string_view name);
TMWSIM_POINT *newDbVar(GcomSystem &sys, const char *iname, int type, int offset, char *uri, char *variation);


TMWSIM_POINT *getDbVar(GcomSystem &sys, std::string_view name);
cJSON *get_config_json(int argc, char *argv[], int num);
cJSON *get_config_file(char *fname);
// new mapping
bool parse_system(cJSON *object, GcomSystem &sys, int who);
bool parse_variables(cJSON *object, GcomSystem &sys, int who);
cJSON *parseJSONConfig(char *file_path);

int addVarToCj(cJSON *cj, TMWSIM_POINT *dbPoint);
int addVarToCj(GcomSystem &sys, cJSON *cj, const char *dname);

int getSysUris(GcomSystem &sys, int who, int nums);

const char *getVersion();
int getConfigs(int argc, char *argv[], GcomSystem &sys, int who);
bool init_vars(GcomSystem &sys, int who);
std::string getFileName(int argc, char *argv[]);
void deleteFlexPoint(void *fp);
void signal_handler(int sig);
double get_time_double();
double get_time_double(TMWDTIME &tmpPtr);
bool spam_limit(GcomSystem *sys, int &max_errors);

#endif
