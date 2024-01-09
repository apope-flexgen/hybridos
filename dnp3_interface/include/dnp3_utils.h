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

#include "opendnp3/app/AnalogOutput.h"
#include "opendnp3/app/ControlRelayOutputBlock.h"
#include "opendnp3/DNP3Manager.h" 

using namespace opendnp3;
//using namespace asiodnp3; 

#ifndef FPS_ERROR_PRINT 
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif
#define MAX_SETUP_TICKS  50
#define DNP3_MASTER 0
#define DNP3_OUTSTATION 1

#define MICROSECOND_TO_MILLISECOND 1000
#define NANOSECOND_TO_MILLISECOND  1000000
#define MAX_FIMS_CONNECT 5

#define DNP3_UTILS_VERSION "3.0.6"

const OperationType StringToOperationType(const char* codeWord);
void addCjTimestamp(cJSON* cj, const char* ts);
const char* variation_encode(int var);

struct char_cmp 
{
    bool operator () (const char* a,const char* b) const
    {
        return strcmp(a,b)<0;
    }
};


typedef struct maps_t maps;
typedef std::map<const char*, std::pair<bool*, maps**>, char_cmp> body_map;
typedef std::map<const char*, body_map*, char_cmp> uri_map;

int variation_decode(const char* ivar);
const char *iotypToStr (int t);
int iotypToId (const char* t);
int countUris(const char* uri);

enum Type_of_Var{
    AnIn16,
    AnIn32,
    AnF32,
    Type_Crob,     // write master
    Type_Analog,  // write outstation
    Type_Binary,     //write outstation
    Type_Counter,  // auto write outstation
    Type_AnalogOS,  // auto write outstation
    Type_BinaryOS,  //write outstation
    NumTypes
};

// used to manage char* compares in maps
struct char_dcmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};

enum 
{
  Group1Var0 = 0x100,
  Group1Var1 = 0x101,
  Group1Var2 = 0x102,
  Group2Var0 = 0x200,
  Group2Var1 = 0x201,
  Group2Var2 = 0x202,
  Group2Var3 = 0x203,
  Group3Var0 = 0x300,
  Group3Var1 = 0x301,
  Group3Var2 = 0x302,
  Group4Var0 = 0x400,
  Group4Var1 = 0x401,
  Group4Var2 = 0x402,
  Group4Var3 = 0x403,
  Group10Var0 = 0xA00,
  Group10Var1 = 0xA01,
  Group10Var2 = 0xA02,
  Group11Var0 = 0xB00,
  Group11Var1 = 0xB01,
  Group11Var2 = 0xB02,
  Group12Var0 = 0xC00,
  Group12Var1 = 0xC01,
  Group13Var1 = 0xD01,
  Group13Var2 = 0xD02,
  Group20Var0 = 0x1400,
  Group20Var1 = 0x1401,
  Group20Var2 = 0x1402,
  Group20Var5 = 0x1405,
  Group20Var6 = 0x1406,
  Group21Var0 = 0x1500,
  Group21Var1 = 0x1501,
  Group21Var2 = 0x1502,
  Group21Var5 = 0x1505,
  Group21Var6 = 0x1506,
  Group21Var9 = 0x1509,
  Group21Var10 = 0x150A,
  Group22Var0 = 0x1600,
  Group22Var1 = 0x1601,
  Group22Var2 = 0x1602,
  Group22Var5 = 0x1605,
  Group22Var6 = 0x1606,
  Group23Var0 = 0x1700,
  Group23Var1 = 0x1701,
  Group23Var2 = 0x1702,
  Group23Var5 = 0x1705,
  Group23Var6 = 0x1706,
  Group30Var0 = 0x1E00,
  Group30Var1 = 0x1E01,
  Group30Var2 = 0x1E02,
  Group30Var3 = 0x1E03,
  Group30Var4 = 0x1E04,
  Group30Var5 = 0x1E05,
  Group30Var6 = 0x1E06,
  Group32Var0 = 0x2000,
  Group32Var1 = 0x2001,
  Group32Var2 = 0x2002,
  Group32Var3 = 0x2003,
  Group32Var4 = 0x2004,
  Group32Var5 = 0x2005,
  Group32Var6 = 0x2006,
  Group32Var7 = 0x2007,
  Group32Var8 = 0x2008,
  Group40Var0 = 0x2800,
  Group40Var1 = 0x2801,
  Group40Var2 = 0x2802,
  Group40Var3 = 0x2803,
  Group40Var4 = 0x2804,
  Group41Var0 = 0x2900,
  Group41Var1 = 0x2901,
  Group41Var2 = 0x2902,
  Group41Var3 = 0x2903,
  Group41Var4 = 0x2904,
  Group42Var0 = 0x2A00,
  Group42Var1 = 0x2A01,
  Group42Var2 = 0x2A02,
  Group42Var3 = 0x2A03,
  Group42Var4 = 0x2A04,
  Group42Var5 = 0x2A05,
  Group42Var6 = 0x2A06,
  Group42Var7 = 0x2A07,
  Group42Var8 = 0x2A08,
  Group43Var1 = 0x2B01,
  Group43Var2 = 0x2B02,
  Group43Var3 = 0x2B03,
  Group43Var4 = 0x2B04,
  Group43Var5 = 0x2B05,
  Group43Var6 = 0x2B06,
  Group43Var7 = 0x2B07,
  Group43Var8 = 0x2B08,
  Group50Var1 = 0x3201,
  Group50Var3 = 0x3203,
  Group50Var4 = 0x3204,
  Group51Var1 = 0x3301,
  Group51Var2 = 0x3302,
  Group52Var1 = 0x3401,
  Group52Var2 = 0x3402,
  Group60Var1 = 0x3C01,
  Group60Var2 = 0x3C02,
  Group60Var3 = 0x3C03,
  Group60Var4 = 0x3C04,
  Group70Var1 = 0x4601,
  Group70Var2 = 0x4602,
  Group70Var3 = 0x4603,
  Group70Var4 = 0x4604,
  Group70Var5 = 0x4605,
  Group70Var6 = 0x4606,
  Group70Var7 = 0x4607,
  Group70Var8 = 0x4608,
  Group80Var1 = 0x5001,
  Group110Var0 = 0x6E00,
  Group111Var0 = 0x6F00,
  Group112Var0 = 0x7000,
  Group113Var0 = 0x7100,
  GroupUndef = 0xFFFF
};


// flag options for reporting
//keep these out of the way of the uri_flags
#define PRINT_VALUE      1
#define PRINT_PARENT     2
#define PRINT_URI        4
#define PRINT_FLAGS      8
#define PRINT_TIME       16

// used in cell timeout detection
#define STATE_RESET 0
#define STATE_ONLINE 1
#define STATE_COMM_LOST 2
#define STATE_INIT -1

#define CROB_STRING  0
#define CROB_BOOL    1
#define CROB_INT     2


// local copy of all inputs and outputs
typedef struct DbVar_t {
    DbVar_t(const char *iname, int _type, int _offset, const char* iuri, const char*ivariation):site(NULL),type(_type), offset(_offset),site_offset(-1) {
        valuedouble = 0.0;
        flags = 1;
        newFlags=false;
        newTime=false;
        not_batched = false;
        value_set = 0;
        value_changed = 0;
        time = 0;
        
        sflags = "Init";
        crob = 0;
        bit = -1;
        parent = NULL;
        init_set = 0;
        //readb = NULL;
        linkb = NULL;  // used to link outstation responses to master vars
        linkback = NULL;
        linkuri = NULL;        
        clazz = 0;
        sign = false;
        scale = 0.0;
        pref = 0;
        deadband = 0.0;
        timeout = 0.0;
        last_seen = -1.0;
        state = 0;
        name = iname;
        variation = 0;         // space to flag different DNP3 variation like Group30var5
        evariation = 0;         // space to flag different DNP3 variation like Group30var5
        event_rate = 0.0;
        crob_true = -1;
        crob_false = -1;
        crob_string = false;
        crob_int = false;
        
        crob_input = CROB_BOOL;

        
        if(iuri)
        {
            uri = strdup(iuri);
            //free((void *)iuri);
        }
        else
        {
            uri = NULL;
        }
        variation = variation_decode(ivariation);
    };

    ~DbVar_t()
    {
        //FPS_ERROR_PRINT(" DbVar delete name [%s] uri [%s]\n", name.c_str(), uri);
        if(uri)free((void *)uri);
        if(format) free((void*)format);
        if(linkback) free((void *)linkback);
        if(linkuri) free((void *)linkuri);
    }

    void setEvar(const char* evar)
    {
        evariation = variation_decode(evar);
    }

    void setFloat(bool isFloat)
    {
        if(isFloat)
        {
            if (type == Type_Analog) 
            {
                variation = variation_decode("Group30Var5");
            }
            if (type == Type_Binary)
            {
                variation = variation_decode("Group1Var1");
            }
        }
    }

    void setClazz(int iclazz)
    {
        clazz = iclazz;
    }
    
    int addBit(const char*bit)
    {
        dbBits.push_back(std::make_pair(bit,0));
        return static_cast<int32_t>(dbBits.size());
    }

    int setBit(int idx, int val)
    {
        dbBits[idx].second = val;
        return val;
    }

    int setBit(const char* var, int val)
    {
        for (int i = 0; i < static_cast<int32_t>(dbBits.size()); i++)
        {
            if(strcmp(dbBits[i].first, var) == 0)
            {
                setBit(i,val);
                return 0;
            }
        }
        return -1;
    }

    int getBit(int idx)
    {
        int val = dbBits[idx].second;
        return val;
    }

    int getBit(const char* var)
    {
        for (int i = 0; i < static_cast<int32_t>(dbBits.size()); i++)
        {
            if(strcmp(dbBits[i].first, var) == 0)
            {
                return getBit(i);
            }
        }
        return -1;
    }

    std::string name;
    const char* site;      // future use for site.
    const char* uri;
    int type;
    int variation;         // space to flag different DNP3 variation like Group30var5
    int evariation;        // space to flag different DNP3 variation like Group30var5
    int offset;
    int bit;               // used to indiate which bit in parent
    DbVar_t* parent;       // I'm a bit and this is daddy 
    int site_offset;       // future use for site offset
    // values , most types use valuedouble
    double valuedouble;
    //we can set flags here
    int flags;
    std::string sflags;

    bool newFlags;
    bool newTime;
    long int time;
    

    uint8_t crob;
    int idx;      // type index
    int clazz;    // class

    // used for bit fields
    std::vector<std::pair<const char*,int>>/*bits_map*/dbBits;

    uint8_t init_set;
    DbVar_t* linkb;
    const char* linkback;
    const char* linkuri;
    bool sign;
    double scale;
    uint16_t pref;
    int vindex; // used when the compressed option is requested
    double deadband;
    double timeout; // used to detect COMM_LOSS
    double last_seen;  // time of last update
    double event_rate;  // min time betweem events
    int state;
    int vstate;     // records reset 0, online 1 , lost comms 2
    char* format;
    int fmt;
    bool events;
    std::string stime;
    std::string etime;
    std::string ltime;
    int crob_true;
    int crob_false;
    bool crob_string;
    bool crob_int;
    int crob_input;

    bool crob_bool;
    int who;
    double voffset;
    bool enable;
    int debug;
    bool not_batched;
    int value_set;
    int value_changed;


} DbVar;

// map names to vars ... deprecated becaus name + uri is used to identify a unique item
typedef std::map<std::string, DbVar_t*> dbvar_map;
// map of idx to vars
typedef std::map<int, DbVar_t*>dbix_map;
// map a vector list of dbvars against a vector of uris no longer used

//typedef std::map<const char*,std::vector<DbVar_t*>, char_dcmp>duri_map;
typedef std::vector<DbVar_t*> dbvec;

// used in parseBody the int is a print flag to include "value"
typedef std::vector<std::pair<DbVar*,int>>dbs_type; // collect all the parsed vars here

// this is for the bits
typedef std::map<const char*, std::pair<DbVar_t*,int>, char_dcmp>bits_map;

// do conversion for large values unsigned ints
double getF32Val(DbVar *db);
int32_t getInt32Val(DbVar *db);
int16_t getInt16Val(DbVar *db);

bool extractInt32Val(double &dval, DbVar *db);
bool extractInt16Val(double &dval, DbVar *db);


// the varlist is a map of vars associated with each uri
typedef struct varList_t {
    varList_t(const char* iuri)
    {
        uri = iuri;
    };
    ~varList_t(){
        for ( auto it : dbmap)
        {
            // dbvar_map
            auto dvar = it.second;
            delete dvar;

        }
    };

    void addDb(DbVar* db)
    {
        dbmap[db->name] = db;
    }
    int size()
    {
        return (int)dbmap.size();
    }

    std::string uri;
    dbvar_map dbmap;
} varList;

// this now replaces duri_map
typedef std::map<std::string, varList*> dburi_map;

enum {
    URI_FLAG_REPLY    = 1<<1,
    URI_FLAG_GET      = 1<<2,
    URI_FLAG_SET      = 1<<3,
    URI_FLAG_URIOK    = 1<<4,
    URI_FLAG_NAMEOK   = 1<<5,
    URI_FLAG_SINGLE   = 1<<6,
    URI_FLAG_SYSTEM   = 1<<7,
    URI_FLAG_DUMP     = 1<<8,
    URI_FLAG_VALUE    = 1<<9,
    URI_FLAG_FLAGS    = 1<<10,
    URI_FLAG_TIME     = 1<<11,
    URI_FLAG_SERVER   = 1<<12,
    URI_FLAG_CLIENT   = 1<<13,
    URI_FLAG_NAKED    = 1<<14,
    URI_FLAG_CLOTHED  = 1<<15,
    URI_FLAG_FULL     = 1<<16

};

typedef struct sysCfg_t {

    sysCfg_t() :name(NULL), protocol(NULL), id(NULL), ip_address(NULL), p_fims(NULL)
    {
        cj = NULL;
        cjPub = NULL;
        cjOut = NULL;
        cjloaded = 0;
        debug = 0;
        scanreq = 0;
        num_subs = 0;
        unsol = 0;
        cjclass = NULL;
        base_uri = NULL;
        local_uri = NULL;
        defUri = NULL;
        freq1 = 0;
        freq2 = 0;
        freq3 = 0;
        frequency = 1000;
        rangeAStart = 0;
        rangeAStop = 0;
        rangeBStart = 0;
        rangeBStop = 0;
        rangeFreq = 0;

        unsolUpdate = false;
        useOffset = false;
        pref = 1;
        useVindex = false;
        deadband =  0.01;
        timeout =  0.0;
        tNow = 0.0;
        tLastMsg = 0.0;

        base_time = -1.0;
        last_seen = -1.0;
        event_rate = 0;
        //state = 0;
        respTime = 2000;

        stackConfig = nullptr;
        OSconfig = nullptr;
        pubType = 0;  // 0: naked, 1: clothed, 2: add flags  4: add time ( TODO)  
        pubTypestr = nullptr;
        event_rate = 0.0;
        ts_base = std::chrono::system_clock::now();
        master = nullptr;
        format = nullptr;
        fmt = 0; // naked

        events = true;
        useGets = false;
        event_pub = false;
        pubOutputs = false;

        fname =  nullptr;
        pub_only =  true;
        batch_sets = 0.0;
        batch_sets_in = 0.0;
        batch_sets_max = 0.0;
        max_pub_droop = 0.9;
        next_batch_time = 0.0;
        event_buffer = 100;
        pub_timeout_detected = false;
        batch_pubs = 0.0;
        next_batch_pub_time = 0.0;
        batch_pub_debug = 0;
        enable_state_events = true;



        for (int i = 0; i <= static_cast<int32_t>(Type_of_Var::NumTypes) ; i++)
        {
            maxIdx[i] = 0;
        }
        conn_type = nullptr;
        // TLS stuff
        peerCertificate = nullptr;
        privateKey = nullptr;
        certificateChain = nullptr;
        caCertificate = nullptr;
        localCertificate = nullptr;
        tls_folder_path = nullptr;

        // RTU stuff:
        baud = 0;
        dataBits = 0;
        stopBits = 0.0;
        parity = nullptr;
        flowType = nullptr;
        asyncOpenDelay = 0;
        deviceName = nullptr; // this might just be name really?
        pubType = -1;  // init value 
        maxElapsed = 100;
        last_result = 0;
        last_res = -1;
        fname = NULL;
        pub_time = 0.0;
        max_pub_delay = 0.0;
        
    }
    ~sysCfg_t()
    {
        FPS_DEBUG_PRINT(" %s closing \n", __FUNCTION__);

        //if(name)free(name);
        if(protocol)free(protocol);
        if(id)free(id);
        if(ip_address)free(ip_address);
        //if (pub) free(pub);
        if (name) free(name);
        if(cjOut)cJSON_Delete(cjOut);

        cleanUpDbMaps();

        if (cj) cJSON_Delete(cj);
        if(base_uri) free(base_uri);
        if(local_uri) free(local_uri);
        if(defUri) free(defUri);

        //cleanUpDbMaps();
        if (conn_type) free(conn_type);
        // TLS stuff:
        if (privateKey) free(privateKey);
        if (certificateChain) free(certificateChain);
        if (caCertificate) free(caCertificate);
        if (peerCertificate) free(peerCertificate);
        if (localCertificate) free(localCertificate);
        if (tls_folder_path) free(tls_folder_path);

        // RTU stuff:
        if (parity) free(parity);
        if (flowType) free(flowType);
        if (deviceName) free(deviceName); // this might just be name really?
        // deprecated
        if (pubTypestr) free(pubTypestr); // this might just be name really?
        // "clothed" is the only key we need
        if (format) free(format); // this might just be name really?

    };

    public:
        // now always creates a var. it is added to the uri map
        DbVar* newDbVar(const char *iname, int type, int offset, char* uri, char* variation) 
        {
            DbVar* db = NULL;
            db = new DbVar(iname, type, offset, uri, variation);
            if(uri == NULL)
            {
                db->uri = strdup(defUri);
            }
            dbVec[type].push_back(db);
            db->vindex = dbVec[type].size();
            if (offset > maxIdx[type])
                maxIdx[type] = offset;
            db->format = nullptr; // naked
            db->fmt = 0;
     
            return db;
        };

        int32_t getTypeSize(int type) 
        {
            // we have an extra array do handle auto indexing
            if (type > NumTypes)
                return 0;
            if(useVindex)return dbMapIxs[type].size();
            return static_cast<int32_t>(maxIdx[type]+1);
        }

        // allow this to auto configure for any if db->idx == -1
        void setDbIdxMap(DbVar* db, int inParse)
        {
            int opType = db->type;
            // do these ones after parsing
            if (((db->type == Type_Crob) 
                || (db->type == AnF32) 
                || (db->type == AnIn16) 
                || (db->type == AnIn32)) 
                && (db->idx == -1)
                && inParse)
            {
                // set the db->ix later
                
                FPS_DEBUG_PRINT(" **** %s setting name [%s] typ [%d] idx [%d] Later \n", __FUNCTION__, db->name.c_str(), db->type, db->idx );
                return;
            }
            if(db->idx > -1)
            {
                if(debug)
                {
                    FPS_ERROR_PRINT(" **** %s setting name [%s] typ [%d] idx [%d] NOW \n", __FUNCTION__, db->name.c_str(), db->type, db->idx );
                }

                if((who == DNP3_MASTER) && useVindex && (db->type == Type_Analog || db->type== Type_Binary))
                    dbMapIxs[db->type][db->vindex] = db;
                else
                    dbMapIxs[db->type][db->idx] = db;

                return;
            }
            else
            {
                db->idx = 0;
            }
            opType = db->type;

            if ((db->type == AnF32) 
                || (db->type == AnIn16) 
                || (db->type == AnIn32)) 
            {
                opType = NumTypes;
            }
            while (dbMapIxs[opType].find(db->idx) != dbMapIxs[opType].end())
            {   
                db->idx++;
            }
            dbMapIxs[opType][db->idx] = db;
            FPS_ERROR_PRINT(" **** %s setting name [%s] typ [%d] idx [%d] AGAIN NOW \n", __FUNCTION__, db->name.c_str(), db->type, db->idx );
            dbMapIxs[db->type][db->idx] = db;
        } 
        // given a uri and a db name recover the dbvar if we have one
        DbVar* getDbVar(const char *uri, const char* name)
        {
            std::string sname = name;
            std::string suri = uri;
            dburi_map::iterator it = dburiMap.find(suri);
            if(it != dburiMap.end())
            {
                 // dbvar_map
                auto dvar = it->second;
                auto dbm = dvar->dbmap;
                dbvar_map::iterator itd = dbm.find(sname);
            
                if (itd != dbm.end() )
                {
                    return itd->second;
                }
            }
            return NULL;
        };
        // set a value  handles linkback as well
        int setDb(DbVar* db, double fval, cJSON* cjFlag = nullptr)
        {
            if(db != NULL)
            {
                if (db->linkback != NULL)
                {
                    if (db->linkb == NULL)
                    {
                        const char* luri = db->linkuri;
                        if (luri == NULL)
                            luri = db->uri;
                        db->linkb = getDbVar(luri, db->linkback);
                    }
                    if (db->linkb != NULL)
                    {
                        if (db->linkb->linkb == db)
                        {
                            FPS_ERROR_PRINT(" link loopback detected from [%s] to [%s] \n", db->name.c_str(), db->linkb->name.c_str());
                        }
                        else
                        {
                            setDb(db->linkb, fval, cjFlag);
                        }
                    }
                }
                db->init_set = 1;
                if (cjFlag)
                {
                    setDbFlags(db, cjFlag, nullptr);
                    // TODO decode string
                    // TODO decode time
                }
                switch (db->type) 
                {
                    case Type_Analog:
                    case Type_AnalogOS:
                    case AnF32:
                    case AnIn32:
                    case AnIn16:
                    {
                        // also copy valueint or the group30var5 stuff

                        if(db->valuedouble != fval)
                        {
                            db->value_changed++;
                        }
                        db->valuedouble = fval;
                        return 1;
                    }    
                    case Type_Binary:
                    case Type_BinaryOS:
                    {
                        if(db->valuedouble != fval)
                        {
                            db->value_changed++;
                        }
                        db->valuedouble = fval;
                        //db->valueint = static_cast<int32_t>(fval);
                        return  1;
                    } 
                    case Type_Crob:
                    {
                    //    db->crob = OperationTypeSpec::to_type(StringToOperationType(cj->valuestring));
                       return  1;
                    }
                    default:
                        return 0;
                }    
            }
            return 0;
        };

        int setDb(DbVar* db, int ival, cJSON *cjFlag = nullptr)
        {
            if(db != NULL)
            {
                if (db->linkback != NULL)
                {
                    if (db->linkb == NULL)
                    {
                        const char* luri = db->linkuri;
                        if (luri == NULL)
                            luri = db->uri;
                        db->linkb = getDbVar(luri, db->linkback);
                    }
                    if(db->linkb != NULL)
                    { 
                        if (db->linkb->linkb == db)
                        {
                            FPS_ERROR_PRINT(" link loopback detected from [%s] to [%s] \n", db->name.c_str(), db->linkb->name.c_str());
                        }
                        else
                        {
                            setDb(db->linkb, ival, cjFlag);
                        }
                    }
                }
                db->init_set = 1;
                if (cjFlag)
                {
                    setDbFlags(db, cjFlag, nullptr);

                }
                switch (db->type) 
                {
                    case Type_Analog:
                    case Type_AnalogOS:
                    case AnF32:
                    case AnIn16:
                    case AnIn32:
                   {
                        // also copy valueint or the group30var5 stuff
                        if(db->valuedouble != (double)ival)
                        {
                            db->value_changed++;
                        }
                        db->valuedouble = (double)ival;
                        //db->valueint =  ival;
                        return 1;
                    }    
                    case Type_Binary:
                    case Type_BinaryOS:
                    {
                        if(db->valuedouble != (double)ival)
                        {
                            db->value_changed++;
                        }
                        db->valuedouble = (double)ival;
                        //db->valueint = ival;
                        return  1;
                    } 
                    case Type_Crob:
                    {
                        if(db->crob != ival)
                        {
                            db->value_changed++;
                        }

                        db->crob = ival;// OperationTypeSpec::to_type(StringToOperationType(cj->valuestring));
                        return  1;
                    }
                    default:
                        return 0;
                }
            }
            return 0;
        };

        int setDbFlags(DbVar* db, cJSON* cjFlag, cJSON* cjTime)
        {
                cJSON* cj = cJSON_GetObjectItem(cjFlag,"flags");
                if (!cj)
                {
                    cj = cjFlag;
                }
                if (cj)
                {
                    if (db->flags != cj->valueint)
                    {
                        FPS_ERROR_PRINT(" db [%s] old Flag %04x new Flag %04x\n"
                            , db->name.c_str()
                            , db->flags
                            , cj->valueint
                            );
                        db->flags = cj->valueint;
                        db->newFlags = true;
                        if(cjTime)
                        {
                            db->newTime = true;
                            db->time = cjTime->valueint;
                        }
                    }
                }
                else
                {
                    FPS_ERROR_PRINT(" %s >>>  ERROR NO Flag Object db [%s] cjname [%s] curr Flag %04x "
                        , __func__
                        , db->name.c_str()
                        , cjFlag->string? cjFlag->string:" no cjFlag string"
                        , db->flags
                    );

                }
            return 0;

        }

        int setDbVar(DbVar* db, cJSON* cj, cJSON* cjFlag = nullptr, cJSON* cjTime=nullptr)
        {
            if(db != NULL)
            {
                switch (db->type) 
                {
                    case Type_AnalogOS:
                    case Type_Analog:
                    case AnF32:
                    case AnIn32:
                    case AnIn16:
                    {
                        return setDb(db, cj->valuedouble);
                    }    
                    case Type_BinaryOS:
                    case Type_Binary:
                    {
                        if(debug > 0)
                            FPS_ERROR_PRINT(" setting the binary value of [%s] %s to  int %d sign %d cjFlags %p\n"
                                , db->name.c_str(), iotypToStr(db->type), cj->valueint, db->sign, (void*)cjFlag );                

                        return setDb(db, cj->valueint);
                    } 
                    case Type_Crob:
                    {
                        // incoming CROB value  can be a string , int or bool
                        // if we are using a bool than translate the meaning of true/false into a crob value if crob_true or crob_false are defined
                        // the default is "LATCH_ON" (3) for true , "LATCH_OFF" (4) for false
                        if (cj->valuestring)
                        {
                            int cval =  OperationTypeSpec::to_type(StringToOperationType(cj->valuestring));
                            if(debug)
                                FPS_ERROR_PRINT(" setting the crob value of [%s] %s  from String [%s] cval [%d] cjFlags %p\n"
                                    , db->name.c_str(), iotypToStr(db->type), cj->valuestring, cval, (void*)cjFlag );                
                            return setDb(db, cval, cjFlag);
                        }
                        // crob can be  true/false
                        if ((cj->type == cJSON_True) || (cj->type == cJSON_False))
                        {
                            bool cval = (cj->type == cJSON_True);
                            int ival = 0;
                            if(cval)
                            {
                                if (db->crob_true >=0)
                                {
                                    ival = db->crob_true;
                                }
                                else
                                {
                                    // LATCH_ON
                                    ival = OperationTypeSpec::to_type(StringToOperationType("LATCH_ON"));
                                }
                            }
                            else
                            {
                                if (db->crob_false >=0)
                                {
                                    ival =  db->crob_false;
                                }
                                else
                                {
                                    //LATCH_OFF
                                    ival = OperationTypeSpec::to_type(StringToOperationType("LATCH_OFF"));
                                }
                            }
                            if(debug)
                                FPS_ERROR_PRINT(" setting the crob value of [%s] %s  from Bool, cval [%s] ival [%d] cjFlags %p\n"
                                    , db->name.c_str(), iotypToStr(db->type), cval?"true":"false",ival, (void*)cjFlag );                

                            return setDb(db, ival);

                        }

                        else 
                        {
                            int ival = cj->valueint;
                            if(debug)
                                FPS_ERROR_PRINT(" setting the crob value of [%s] %s  from Int, ival [%d] cjFlags %p\n"
                                    , db->name.c_str(), iotypToStr(db->type), ival, (void*)cjFlag );                
                            return setDb(db, ival);
                        }
                    }
                    default:
                        return 0;
                }
            }
            return 0;
        };

        int setDbVar(const char* uri, const char* name, cJSON* cj, cJSON* cjFlags, cJSON* cjTime)
        {
            DbVar* db = getDbVar(uri, name);
            if(db)
            {
                if(cjFlags)
                {
                    if(debug)
                        FPS_ERROR_PRINT( " *************** %s #2 Var [%s] processed db %p flags %p \n",__FUNCTION__, name, (void*)db, (void *) cjFlags);
                    setDbFlags(db, cjFlags, cjTime);
                }
                return setDbVar(db, cj);
            }
            else
            {
                FPS_ERROR_PRINT( " *************** %s #3 Var [%s] NOT FOUND  \n",__FUNCTION__, name);

            }
            return 0;
        };

        int setDbVar(const char* uri, const char *name, double dval)
        {
            DbVar* db = getDbVar(uri, name);
            if(db != NULL)
            {
                return setDb(db, dval);
            }
            return 0;
        };

        int setDbVar(const char* uri, const char *name, int ival)
        {
            DbVar* db = getDbVar(uri, name);
            if(db != NULL)
            {
                return setDb(db, ival);
            }
            return 0;
        };

        int setDbVarIx(int dbtype, int idx, double ival, CommandStatus  status, cJSON* cjFlag = nullptr)
        {
            DbVar* db = getDbVarId(dbtype, idx);
            if (db != NULL)
            {
                if((db->scale > 0.0) || (db->scale < 0.0))
                {
                    ival /= db->scale;
                }
                if(debug)
                    FPS_ERROR_PRINT(" setting the double value of [%s] %s to %f sign %d \n"
                            , db->name.c_str(), iotypToStr(dbtype), ival, db->sign );

                db->sflags =  CommandStatusSpec::to_human_string(status);               
                return setDb(db, ival);
            }
            else
            {
                FPS_ERROR_PRINT("  %s Error Set Double Variable %s idx %d value %f unknown Var\n"
                    , __FUNCTION__, iotypToStr(dbtype), idx, ival);                  
            }
            return 0;
        };

        int setDbVarIx(int dbtype, int idx, int ival, CommandStatus  status, cJSON *cjFlag = nullptr)
        {
            DbVar* db = getDbVarId(dbtype, idx);
            if (db != NULL)
            {
                if (dbtype == Type_Crob)
                {
                    //   NUL = 0x0,
                    //   PULSE_ON = 0x1,
                    //   PULSE_OFF = 0x2,
                    //   LATCH_ON = 0x3,
                    //   LATCH_OFF = 0x4,
                    //   Undefined = 0xFF
                    const char* cmd = OperationTypeSpec::to_string(OperationTypeSpec::from_type(ival));
                    if(debug)
                    {
                        FPS_ERROR_PRINT(" setting the crob value of [%s] %s to %d [%s] Flags %p\n"
                            , db->name.c_str(), iotypToStr(dbtype), ival, cmd, (void*)cjFlag );
                    }
                    db->sflags =  CommandStatusSpec::to_human_string(status);               
                    return setDb(db, ival);

                }
                else
                {
                    if((db->scale > 0.0) || (db->scale < 0.0))
                    {
                        ival /= db->scale;
                    }
                    if(debug)
                    {
                        FPS_ERROR_PRINT(" setting the int value of [%s] %s to %d sign %d Flags %p\n"
                            , db->name.c_str(), iotypToStr(dbtype), ival, db->sign, (void*)cjFlag );                
                    }
                    db->sflags =  CommandStatusSpec::to_human_string(status);               
                    return setDb(db, ival);
                }
            }
            else
            {
                FPS_ERROR_PRINT(" %s Set INT Variable %s index  %d unknown \n", __FUNCTION__, iotypToStr(dbtype), ival);                  
            }            
            return 0;
        };

        int setDbVarIx(int dbtype, int idx, bool ival, CommandStatus  status,cJSON * cjFlag = nullptr)
        {
            DbVar* db = getDbVarId(dbtype, idx);
            if (db != NULL)
            {
                if(debug)
                {
                    FPS_ERROR_PRINT(" setting the int value of [%s] %s to %s sign %d Flags %p \n"
                        , db->name.c_str(), iotypToStr(dbtype), ival?"true":"false", db->sign, (void*)cjFlag );
                }
                int mval = ival?1:0;
                if(db->scale < 0.0)
                {
                    mval = ival?0:1;
                }                   
                db->sflags =  CommandStatusSpec::to_human_string(status);               
                return setDb(db, mval);
            }
            else
            {
                FPS_ERROR_PRINT(" %s Set INT Variable %s index  %d unknown \n", __FUNCTION__, iotypToStr(dbtype), ival);                  
            }            
            return 0;
        };

        DbVar* getDbVarId(int dbtype, int idx)
        {
            if (dbtype > Type_of_Var::NumTypes) return NULL;
            if (dbMapIxs[dbtype].find(idx) != dbMapIxs[dbtype].end())
            {
                //use dbix_map
                return  dbMapIxs[dbtype][idx];
            }
            return NULL;
        };

        void cleanUpDbMaps()
        {
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                FPS_ERROR_PRINT(" cleanup dbvec [%d]\n", i);
                dbVec[i].clear();
            }
            int ix = 0;            
            dburi_map::iterator it;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                FPS_ERROR_PRINT(" cleanup dburiMap [%s] [%d]\n", it->first.c_str(), ix);

                delete it->second;
                ix++;

            }
            dburiMap.clear();            
        }

        void showDbMap()
        {
            FPS_ERROR_PRINT(" %s DbVars===> \n\n", __FUNCTION__);
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                if (dbVec[i].size() > 0)
                {
                    FPS_ERROR_PRINT("\n dnp3 type [%s]\n", iotypToStr(i)); 
                    for (int j = 0; j < static_cast<int32_t>(dbVec[i].size()); j++)
                    {
                        DbVar* db = dbVec[i][j];
                        if((db != NULL) &&(db->type == i))
                        {
                            FPS_ERROR_PRINT(" idx [%s.%d] ->name :[%s] offset : [%d]  vIdx [%d]===> "
                                , iotypToStr(db->type)
                                , db->idx
                                , db->name.c_str()
                                , db->offset
                                , j
                                );

                        }
                        if(db)
                            db = getDbVarId(i, db->idx);
                        if(db != NULL)// &&(db->type == i))
                        {
                            if(debug > 0)
                            {
                                FPS_ERROR_PRINT(" OK ***** idx [%s.%d] j = [%d] Item Already in dbMapIxs\n"
                                    , iotypToStr(i)
                                    , db->idx
                                    , j
                                    );
                            }
                            else
                            {
                                FPS_ERROR_PRINT("\n");
                            }
                        }
                        else 
                        {
                            FPS_ERROR_PRINT(" Error ***** idx [%s]  type = %d j = [%d] Item Not in dbMapIxs\n"
                                , iotypToStr(i)
                                , i 
                                , j
                                );
                        }
                    }
                }
            }           
        }

        void addPubOutputs(int fmt = 0)
        {
            if(debug)FPS_ERROR_PRINT(" %s   running\n", __FUNCTION__);
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {
                if (i == AnIn16 || i == AnIn32 || i == AnF32 || i == Type_Crob)
                {
                    if (dbVec[i].size() > 0)
                    {
                        if(debug > 0)
                            FPS_ERROR_PRINT(" Add pub dnp3 type [%s] size [%ld]\n", iotypToStr(i), dbVec[i].size());
                        for (int j = 0; j < static_cast<int32_t>(dbVec[i].size()); j++)
                        {
                            DbVar* db = dbVec[i][j];
                            if(debug > 0)
                                FPS_ERROR_PRINT("     name [%s] value [%2.3f]\n", db->name.c_str(), db->valuedouble);
                            if (i == Type_Crob)
                            {
                                if (db->crob_input == CROB_STRING)
                                {
                                    // for now we cannot pub strings
                                    //const char* cmd = OperationTypeSpec::to_string(OperationTypeSpec::from_type(db->crob));
                                    //addSysCjVal(sys, cj, db, flag, cmd);
                                    addPubVar(db, (double)db->crob, 0, fmt);
                                }
                                if (db->crob_input == CROB_INT)
                                {
                                    //addSysCjVal(sys, cj, db, flag, db->crob);
                                    //addPubVar(db, (db->valuedouble > 0), 0, fmt);
                                    addPubVar(db, (double)db->crob, 0, fmt);
                                }
                                if (db->crob_input == CROB_BOOL)
                                {
                                    if(db->crob_true == db->crob)
                                    {
                                        addPubVar(db, true, 0, fmt);
                                    }
                                    else if(db->crob_false == db->crob)
                                    {
                                        addPubVar(db, false, 0, fmt);
                                    }
                                    else if(0x03 == db->crob)
                                    {
                                        addPubVar(db, true, 0, fmt);
                                    }
                                    else 
                                    {
                                        addPubVar(db, false, 0, fmt);
                                    }
                                }
                            }
                            else
                            {
                                addPubVar(db, db->valuedouble, 0, fmt);
                            }
                        }
                    }
                }
            }
        }

        void addPubVar(DbVar* db, double val, unsigned int etype = 0, int fmt = 0)
        {
            int ptype = 0;
            if(etype == 0x2003 /*Group32Var3*/) ptype = 3;
            if(etype == 0x2001 /*Group32Var1*/) ptype = 2;  // now allow etime through
            if(etype == 0x0001) ptype = 1;
            std::string urist = db->uri;
            if(ptype > 1)
            {
                urist += "_event";
            }
            auto uri = urist.c_str();
            if (cjPub == NULL)
            {
                cjPub = cJSON_CreateObject();
                if(debug)FPS_ERROR_PRINT(" %s  created cjPub (double)  etype [%04x]\n", __FUNCTION__, etype);
            }
            cJSON* cjuri = cJSON_GetObjectItem(cjPub, uri);
            if(cjuri == NULL)
            {
                cjuri = cJSON_CreateObject();
                cJSON_AddItemToObject(cjPub, uri, cjuri);
                cjuri = cJSON_GetObjectItem(cjPub, uri);
            }
            if(cjuri != NULL)
            {
                if (db->pref == pref)
                {
                    cJSON_DeleteItemFromObjectCaseSensitive(cjuri, db->name.c_str());
                }

                if (fmt == 0)  // this is the naked form
                {
                    cJSON_AddNumberToObject(cjuri, db->name.c_str(), val);
                }
                else
                {
                    cJSON* cjv = cJSON_CreateObject();
                    cJSON_AddNumberToObject(cjv, "value", val);
                   
                    if(fmt >= 2)
                    {
                        cJSON_AddNumberToObject(cjv, "flags", db->flags);
                        if (db->sflags.length() > 0)
                        {
                            cJSON_AddStringToObject(cjv, "sflags", db->sflags.c_str());
                            if(ptype>=2) cJSON_AddStringToObject(cjv, "event", variation_encode(etype) /*db->event*/);
                        }
                        if (db->stime.length() > 0)
                        {
                           cJSON_AddStringToObject(cjv, "stime", db->stime.c_str());
                        }    
                        if (db->ltime.length() > 0)
                        {
                           cJSON_AddStringToObject(cjv, "ltime", db->ltime.c_str());
                        }
                        if (db->etime.length() > 0)
                        {
                           cJSON_AddStringToObject(cjv, "etime", db->etime.c_str());
                        }
                    }
                    cJSON_AddItemToObject(cjuri, db->name.c_str(), cjv);

                }
                db->pref = pref;
            }
            if(ptype > 1)
            {
                if(debug)FPS_ERROR_PRINT(" %s  pubbing (double)  ptype [%d]\n", __FUNCTION__, ptype);
                pubUris(true);
            }
        }

        void addPubVar(DbVar* db, bool val, unsigned int etype = 0, int fmt = 0)
        {
            int ptype = 0;
            if(etype == 0x0203 /*Group2Var3*/) ptype = 3;
            if(etype == 0x0201 /*Group2Var1*/) ptype = 2; // now allow ltime through
            if(etype == 0x0001) ptype = 1;
            std::string urist = db->uri;
            if(ptype > 1)
            {
                urist += "_event";
            }
            auto uri =  urist.c_str();
            if (cjPub == NULL)
            {
                cjPub = cJSON_CreateObject();
            }
            cJSON* cjuri = cJSON_GetObjectItem(cjPub, uri);
            if(cjuri == NULL)
            {
                cjuri = cJSON_CreateObject();
                cJSON_AddItemToObject(cjPub, uri, cjuri);
                cjuri = cJSON_GetObjectItem(cjPub, uri);
            }

            if(cjuri != NULL)
            {
                
                if (db->pref == pref)
                {
                    cJSON_DeleteItemFromObjectCaseSensitive(cjuri, db->name.c_str());
                }
                if(fmt == 0)
                {
                    cJSON_AddBoolToObject(cjuri, db->name.c_str(), val);
                }
                else
                {
                    cJSON* cjv = cJSON_CreateObject();
                    cJSON_AddBoolToObject(cjv, "value", val);
                    if (fmt >= 2) 
                    {
                        cJSON_AddNumberToObject(cjv, "flags", db->flags);
                        if (db->sflags.length() > 0)
                        {
                            cJSON_AddStringToObject(cjv, "sflags", db->sflags.c_str());
                        }
                        if(fmt > 1)
                        {
                            cJSON_AddStringToObject(cjv, "event", variation_encode(etype) /*db->event*/);
                        
                            if (db->stime.length() > 0)
                            {
                                cJSON_AddStringToObject(cjv, "stime", db->stime.c_str());
                            }
                            if (db->ltime.length() > 0)
                            {
                                cJSON_AddStringToObject(cjv, "ltime", db->ltime.c_str());
                            }
                            if (db->etime.length() > 0)
                            {
                                cJSON_AddStringToObject(cjv, "etime", db->etime.c_str());
                            }
                        }
                    }
                    cJSON_AddItemToObject(cjuri, db->name.c_str(), cjv);
                }
                db->pref = pref;
            }
            if(fmt > 1)
            {
                pubUris(true);
            }
        }

        void pubUris(bool pubFlag)
        {
            if (cjPub == NULL)
            {
                return;
            }
            if(pubFlag)
            {
                cJSON* obj;
                cJSON_ArrayForEach(obj, cjPub)
                {
                    addCjTimestamp(obj, "Timestamp");
                    char* out = cJSON_PrintUnformatted(obj);
                    if (out) 
                    {
                        char tmp[1024];
                        snprintf(tmp, 1024,"%s", obj->string);

                        if(p_fims)
                        {
                            p_fims->Send("pub", tmp, NULL, out);
                        }
                        free(out);
                    }
                }
            }
            cJSON_Delete(cjPub);
            cjPub =  NULL;
        }

        void assignIdx()
        {
            if(debug > 0)
                FPS_ERROR_PRINT(" %s Assign DbVar idx ===> SKIPPED \n\n", __FUNCTION__);
            return;
            for (int i = 0; i < static_cast<int32_t>(Type_of_Var::NumTypes); i++)
            {

                if (dbVec[i].size() > 0)
                {
                    if(debug > 0)
                        FPS_ERROR_PRINT(" Assign dnp3 type [%s]\n", iotypToStr(i)); 
                    for (int j = 0; j < static_cast<int32_t>(dbVec[i].size()); j++)
                    {
                        DbVar* db = dbVec[i][j];
                        if(db != NULL)
                        {
                            setDbIdxMap(db, 0);
                            if (dbMapIxs[db->type].find(db->idx) == dbMapIxs[db->type].end()) 
                            {
                                FPS_ERROR_PRINT("***********   name %s type %d idx %d NOT in dbMapIxs  \n",  db->name.c_str(), db->type, db->idx);                
                            }
                            else
                            {
                                if(debug > 0)
                                    FPS_ERROR_PRINT("***********  name %s type %d idx %d Already in dbMapIxs  \n", db->name.c_str(), db->type, db->idx);                            
                            }
                        }
                        if(db != NULL)
                        {
                            if(debug > 0)
                                FPS_ERROR_PRINT(" idx set [%s.%d] ->name :[%s] offset : [%d] ===> \n"
                                    , iotypToStr(db->type)
                                    , db->idx
                                    , db->name.c_str()
                                    , db->offset
                                    );
                        }
                        else
                        {
                            FPS_ERROR_PRINT(" ****** Error no db item idx [%s.%d]  \n"
                                , iotypToStr(db->type)
                                , db->idx
                                );
                        }
                    }
                }
            }
            if (debug > 0)
                FPS_ERROR_PRINT(" %s DbVars<=== \n\n", __FUNCTION__);
        }

        void addVarsToVec(std::vector<DbVar*>& dbs, const char* uri)
        {
           if(uri != NULL)
            {
                dburi_map::iterator it = dburiMap.find(uri);
                if (it != dburiMap.end())
                {
                    if(debug)
                        FPS_DEBUG_PRINT(" %s uris checking [%s] uri [%s] \n ", __FUNCTION__, it->first.c_str(), uri);
                    auto dvar = it->second;
                    auto dbm = dvar->dbmap;
                    dbvar_map::iterator itd;

                    for (itd = dbm.begin() ; itd != dbm.end(); ++itd)
                    {
                        DbVar* db = itd->second;
                        dbs.push_back(db);
                        if(debug)
                            FPS_ERROR_PRINT("added to Vector name: %s => Type: %d offset : %d\n", itd->first.c_str(), db->type, db->offset);
                    }
                }
            }
        }
        void addUrisToVec(dbs_type& dbs)
        {
            dburi_map::iterator itd;
            for (itd = dburiMap.begin();itd != dburiMap.end();++itd)
            {
                addVarsToVec(dbs, (const char*) itd->first.c_str(), PRINT_URI);
            }
        }
        
        void addVarsToVec(dbs_type& dbs, const char* uri, int flag=0)
        {
            if(uri != NULL)
            {
                dburi_map::iterator it = dburiMap.find(uri);
                //int flag = 0;

                if (it != dburiMap.end())
                {
                    // it.first is the uri
                    if(debug)
                        FPS_DEBUG_PRINT(" %s uris checking [%s] uri [%s] \n ", __FUNCTION__, it->first.c_str(), uri);
                    // dbvar_map
                    auto dvar = it->second;
                    auto dbm = dvar->dbmap;
                    dbvar_map::iterator itd;
                    //int flag = 0;
                    for (itd = dbm.begin() ; itd != dbm.end(); ++itd)
                    {
                        DbVar* db = itd->second;
                        dbs.push_back(std::make_pair(db, flag));
                    }
                }
            }
        }

        void showNewUris()
        {
            FPS_ERROR_PRINT(" %s uris===> \n\n", __FUNCTION__);

            dburi_map::iterator it;
            dbvar_map::iterator itd;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                FPS_ERROR_PRINT(" .. uri [%s] num vars %d\n", it->first.c_str(), static_cast<int32_t>(it->second->dbmap.size()));

                auto dvar = it->second;
                auto dbm = dvar->dbmap;
                for (itd = dbm.begin(); itd != dbm.end(); ++itd)
                {
                    FPS_ERROR_PRINT(" ..                            [%s] \n", itd->first.c_str());
                }
            }
            FPS_ERROR_PRINT(" %s<=== New uris \n\n", __FUNCTION__);
        }

        char* confirmUri(DbVar* &db, const char*uri, int who, char* &name, int& flags)
        {
            int old_debug = debug;
            //debug = 1;
            // first limit the uri 
            DbVar* dbf = NULL;
            if(debug>2)
                FPS_DEBUG_PRINT(" %s confirmUri===> \n", __FUNCTION__);
            char* turi = (char *)uri;
            char* nuri;
            char* tmp = NULL;
            if(strstr(uri, "/_system") != NULL) 
            {
                flags |= URI_FLAG_SYSTEM;
                flags |= URI_FLAG_URIOK;
                if(debug>2)
                    FPS_ERROR_PRINT(" %s confirmUri SYSTEM [_system] found in  [%s], flags set 0x%0x\n",__FUNCTION__, uri, flags);
               debug=old_debug;
               return (char*)uri;
            }

            asprintf(&tmp, "%s/reply",base_uri);
            if(debug>2)
                FPS_ERROR_PRINT(" %s confirmUri seeking REPLY [%s] in [%s]\n",__FUNCTION__, tmp, uri);

            if (strncmp(uri, tmp, strlen(tmp) )== 0)
            {

                flags |= URI_FLAG_REPLY;
                flags |= URI_FLAG_SET;
                turi = (char*)uri + strlen(tmp);
                if(debug>1)
                    FPS_ERROR_PRINT(" %s confirmUri REPLY found uri now [%s]\n",__FUNCTION__, turi);
            }
            else
            {
                turi = (char *)uri;
                if(local_uri != NULL)
                {
                    if(tmp) free((void *)tmp);
                    asprintf(&tmp, "%s/%s", local_uri, id);
                    if (strncmp(uri, tmp, strlen(tmp) )== 0)
                    {
                        flags |= URI_FLAG_DUMP;
                        flags |= URI_FLAG_URIOK;

                        turi = (char *)uri + strlen(tmp);
                        if(debug>2)
                            FPS_ERROR_PRINT(" %s confirmUri LOCAL [%s] DUMP [%s]\n",__FUNCTION__, tmp, turi);
                        if(tmp) free(tmp);
                        return turi;
                    }

                    if(tmp) free(tmp);
                    asprintf(&tmp, "%s", local_uri);
                    if (strncmp(uri, tmp, strlen(tmp) )== 0)
                    {
                        flags |= URI_FLAG_GET;
                        flags |= URI_FLAG_SET;
                        turi = (char *)uri + strlen(local_uri);
                        if(debug>2)
                            FPS_ERROR_PRINT(" %s confirmUri LOCAL [%s] found uri now [%s]\n",__FUNCTION__, tmp, turi);
                            
                        if(tmp) free(tmp);
                        asprintf(&tmp, "%s", "/flags");
                        if (strncmp(turi, tmp, strlen(tmp) )== 0)
                        {
                            flags |= URI_FLAG_VALUE;
                            flags |= URI_FLAG_FLAGS;
                            turi += strlen(tmp);
                        }
                        if(tmp) free(tmp);
                        asprintf(&tmp, "%s", "/time");
                        if (strncmp(turi, tmp, strlen(tmp) )== 0)
                        {
                            flags |= URI_FLAG_VALUE;
                            flags |= URI_FLAG_TIME;
                            turi += strlen(tmp);
                        }
                    }
                }
            }

            if(tmp) free(tmp);
            asprintf(&tmp, "%s", "/client");
            if (strncmp(turi, tmp, strlen(tmp) )== 0)
            {
                flags |= URI_FLAG_CLIENT;
                turi = turi + strlen(tmp);
                if(debug>2)
                    FPS_ERROR_PRINT(" %s client [%s] found uri now [%s]\n",__FUNCTION__, tmp, turi);
            }
            if(tmp) free(tmp);
            asprintf(&tmp, "%s", "/server");
            if (strncmp(turi, tmp, strlen(tmp) )== 0)
            {
                flags |= URI_FLAG_SERVER;
                turi = turi + strlen(tmp);
                if(debug>2)
                    FPS_ERROR_PRINT(" %s server [%s] found uri now [%s]\n",__FUNCTION__, tmp, turi);
            }
            // look for more prefixes like full/clothed/naked
            // these must happen after client/server
            if(tmp) free(tmp);
            asprintf(&tmp, "%s", "/naked");
            if (strncmp(turi, tmp, strlen(tmp) )== 0)
            {
                flags |= URI_FLAG_NAKED;
                turi = turi + strlen(tmp);
                if(debug>2)
                    FPS_ERROR_PRINT(" %s confirmUri naked [%s] found uri now [%s]\n",__FUNCTION__, tmp, turi);
            }        

            if(tmp) free(tmp);
            asprintf(&tmp, "%s", "/full");
            if (strncmp(turi, tmp, strlen(tmp) )== 0)
            {
                flags |= URI_FLAG_FULL;
                turi += strlen(tmp);
            }
            if(tmp) free(tmp);
            asprintf(&tmp, "%s", "/clothed");
            if (strncmp(turi, tmp, strlen(tmp) )== 0)
            {
                flags |= URI_FLAG_CLOTHED;
                turi += strlen(tmp);
            }
        
            if (tmp)
            {
                free((void *)tmp);
            }

            dburi_map::iterator it = dburiMap.find(turi);
            nuri = NULL;
            for  (it = dburiMap.begin() ; it != dburiMap.end(); it++)
            {
                if(debug>2)
                    FPS_ERROR_PRINT(" %s uris checking [%s] uri [%s] turi [%s]  value %d\n"
                            , __FUNCTION__, it->first.c_str(), uri, turi
                            , (int) strncmp(turi, it->first.c_str(), strlen(it->first.c_str()))
                                );

                if (strncmp(turi, it->first.c_str(), strlen(it->first.c_str())) == 0)
                {
                    nuri = turi + strlen(it->first.c_str());
                    if (nuri[0] == '/') nuri++;
                    flags |= URI_FLAG_URIOK;
                    if(debug>2)
                        FPS_ERROR_PRINT("    URI FOUND [%s] checking name nuri  [%s] (len) %ld \n", it->first.c_str(), nuri, (long int)strlen(nuri));
                    if(strlen(nuri)> 0)
                    {
                        auto dvar = it->second;
                        auto dbm = dvar->dbmap;
                        dbvar_map::iterator itd = dbm.find(nuri);

                        if (itd != dbm.end())
                        {
                            dbf = itd->second;
                            if(debug>2)
                                FPS_ERROR_PRINT(" URI Match                [%s] %d %d\n"
                                                    , dbf->name.c_str() 
                                                    , dbf->type
                                                    , dbf->idx
                                                    );
                            name = nuri;
                            flags |= URI_FLAG_NAMEOK;
                            flags |= URI_FLAG_SINGLE;
                            db = dbf;
                            debug = old_debug;
                            return turi;
                        }
                    }
                    else
                    {
                        db = NULL;
                        debug = old_debug;

                        return turi;
                    }                   
                }
            }
            if(debug>2)
                FPS_ERROR_PRINT(" %s<=== uris \n\n", __FUNCTION__);
            debug = old_debug;
            return NULL;
        }
    
        int addBits(DbVar *db, cJSON *bits)
        {
            int asiz = cJSON_GetArraySize(bits);
            for (int i = 0  ; i < asiz; i++ )
            {
                cJSON* cji = cJSON_GetArrayItem(bits, i);
                const char* bs = strdup(cji->valuestring);
                db->addBit(bs);
                bitsMap[bs] == std::make_pair(db, i);
            }
            return asiz;
        }

        bool checkUris(int who)
        {
            dburi_map::iterator it;
            dbvar_map::iterator itd;
            bool ret = true;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                auto dvar = it->second;
                auto dbm = dvar->dbmap;
                for (itd = dbm.begin(); itd != dbm.end(); ++itd)
                {
                    DbVar* db = itd->second;
                    if (db->init_set == 0)
                    {
                        if(who == DNP3_OUTSTATION)
                        {
                            if ((db->type == Type_Analog) || (db->type == Type_Binary))
                            {
                                if(debug > 1)
                                    FPS_ERROR_PRINT(" %s init missing for [%s]/[%s]\n", __FUNCTION__, db->uri, db->name.c_str());
                                ret = false;
                            }
                        }
                        else // all others are on master
                        {
                            if (
                                (db->type == AnIn16) 
                                || (db->type == AnIn32)
                                || (db->type == AnF32 )
                                || (db->type == Type_Crob)
                                )
                            { 
                                FPS_ERROR_PRINT(" %s init missing for %s/%s\n", __FUNCTION__, db->uri, db->name.c_str());
                                ret = false;
                            }
                        }
                    }
                }
            }
            return ret;
        };

        int getSubs(const char**subs, int num, int who)
        {
            if (num < static_cast<int32_t>(dburiMap.size()))
            {
                return dburiMap.size();
            }
            int idx = 0;
            if (subs)
            {
                dburi_map::iterator it;
                it = dburiMap.find(base_uri);
                if (it == dburiMap.end())
                {
                    subs[idx++] = base_uri;
                }
                if(local_uri)subs[idx++] = local_uri;

                for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
                {
                    subs[idx++]=it->first.c_str();
                }
                if(debug)
                {
                    for (int i = 0 ; i < idx; i++)
                    {
                        FPS_ERROR_PRINT("      ===>[%s]<=== \n", subs[i]);
                    }
                }
            }
            else
            {
                FPS_ERROR_PRINT(" %s No Subs \n", __FUNCTION__);
            }
            return idx;
        }
 
        int getUris(int who)
        {
            if(debug)
                FPS_ERROR_PRINT(" %s uris===>%d<=== \n\n", __FUNCTION__, who);

            dburi_map::iterator it;
            for (it = dburiMap.begin(); it != dburiMap.end(); ++it)
            {
                char replyto[1024];
                char getUri[1024];
                if (it->first[0] == '/') 
                {
                    snprintf(replyto, sizeof(replyto),"%s/reply%s",  base_uri, it->first.c_str());
                    snprintf(getUri,sizeof(getUri),"%s", it->first.c_str());
                } 
                else
                {
                    snprintf(replyto, sizeof(replyto),"%s/reply%s",  base_uri, it->first.c_str());
                    snprintf(getUri,sizeof(getUri),"%s", it->first.c_str());
                }
                if(debug)
                    FPS_ERROR_PRINT(" uri : [%s] replyto: [%s]\n"
                        , getUri
                        , replyto
                        );
        
                p_fims->Send("get", getUri, replyto, NULL);
            }
            if(debug)
                FPS_ERROR_PRINT(" %s<=== get uris DONE\n\n", __FUNCTION__);
            return 0;
        }

        void addDbUri(const char *uri, DbVar*db)
        {
            std::string mapUri = uri;
            if(dburiMap.find(uri) == dburiMap.end())
            {
                if(debug > 0)
                    FPS_ERROR_PRINT("     %s  ==> ADDED varlist [%s]  dburi size %d \n", __FUNCTION__, uri, (int) dburiMap.size());
                //dbvar_map dvar;
                dburiMap[mapUri] = new varList_t(uri); 
            }
            else
            {
                if(debug > 0)
                    FPS_ERROR_PRINT("     %s  ==> FOUND varlist [%s]  dburi size %d \n", __FUNCTION__, uri, (int) dburiMap.size());
            }
            varList_t *vl = dburiMap[mapUri];
            vl->addDb(db);
            if(debug > 0)
                FPS_ERROR_PRINT(" %s  ==> added var [%s]  dbm size %d \n", __FUNCTION__, db->name.c_str(), (int) vl->size());
        }

        char*getDefUri(int who)
        {
            if(defUri == NULL)
            {
                if(base_uri == NULL)
                {
                    if(who == DNP3_MASTER)
                    {
                        asprintf(&defUri,"/components/%s",id);
                    }
                    else
                    {
                        asprintf(&defUri,"/interfaces/%s",id);
                    }
                }
                else
                {
                    asprintf(&defUri,"%s", base_uri);
                }
                
            }
            return defUri;
        }   
        
        char* name;
        char* protocol;
        char* id;
        char* ip_address;
        char *defUri;
        int version;
        int port;
        int master_address;
        int station_address;
        int frequency;
        int freq1;
        int freq2;
        int freq3;
        bool unsolUpdate;
        bool useOffset;
        int rangeAStart;
        int rangeAStop;
        int rangeBStart;
        int rangeBStop;
        int rangeFreq;
        int idx;
        char* uri;
        bool useVindex;
        int who;   // master or outstation
        int state;
        double deadband;
        double timeout;  // if non zero use this for all vars
        double last_seen; // time of last update
        double tNow;
        double tLastMsg;

        double base_time;
        double respTime;

        dbvec dbVec[Type_of_Var::NumTypes];
        dbix_map dbMapIxs[Type_of_Var::NumTypes+1];
        int maxIdx[Type_of_Var::NumTypes+1];
        bits_map bitsMap;
        dburi_map dburiMap;

        int numObjs[Type_of_Var::NumTypes];

        fims* p_fims;
        cJSON* cj;  
        cJSON* cjOut;  
        cJSON* cjPub;
        int cjloaded;
        int debug;
        int scanreq;     //used to request a class 1, 2 or 3 scan 
        int unsol;       // set to 1 to allow unsol in oustation
        cJSON* cjclass;  // used to change class of a var in outstation
        char* base_uri;
        char* local_uri;
        uint16_t pref;
        int num_subs;
        MasterStackConfig *stackConfig;
        std::shared_ptr<IMaster> master;
        std::shared_ptr<IOutstation> outstation;
        OutstationStackConfig *OSconfig;

        char* conn_type; // TCP or TLS (error out on anything else)

        // TLS elements (might need another key):
        char* privateKey;
        char* peerCertificate;
        char* localCertificate;

        char* certificateChain;
        char* caCertificate;
        char* tls_folder_path;

        // RTU elements:
        int baud;
        int dataBits;
        double stopBits;
        char* parity;
        char* flowType;
        int asyncOpenDelay;
        char* deviceName; // this might just be name really?

        char* format;     // overrdden by db->format {"clothed"}
        bool events;      // overrdden by db->events  ( 0 or 1) 
        int fmt;          // decoded "clothed" 
        int pubType;
        char* pubTypestr;

        long unsigned int taskStart;
        int maxElapsed;
        double event_rate;
        std::chrono::_V2::system_clock::time_point ts_base;
        int last_result;
        int last_res;
        bool useGets;
        bool event_pub;
        bool pubOutputs;

        char* fname;
        bool pub_only;
        double batch_sets;
        double batch_sets_in;
        double batch_sets_max;
        double max_pub_droop;
        double next_batch_time;
        std::mutex setLock;
        int event_buffer;
        double pub_time;
        double max_pub_delay;
        bool pub_timeout_detected;
        double batch_pubs;
        double next_batch_pub_time;
        int batch_pub_debug;
        bool enable_state_events;

} sysCfg;


class SysCfg {

   int num_configs;
   int id;
   std::vector<sysCfg*> sys;
   // Private constructor so that no objects can be created.
   SysCfg() {
      num_configs = 0 ;
      id = 0;
   }
   ~SysCfg() {
   }

   public:

   static SysCfg *getInstance() {
        static SysCfg *instance;
        if (!instance)
                instance = new SysCfg;
        return instance;
   }

   int getNum() {
      return this->num_configs;
   }

   sysCfg* getSys(int num) {
       if (num < this->num_configs)
         return this->sys[num];
       return nullptr;
   }

   void setSys(sysCfg * _sys) {
      this-> sys.push_back(_sys);
      this-> num_configs++;
   }
   
   void setId(int _id) {
      this-> id =  _id;
   }
   
   int getId() {
      return this-> id;
   }

   void clear() {
        for ( auto xx: this->sys)
        {
            delete xx;
        }
        this->sys.clear();
        this->num_configs = 0;
        delete this;
   }
};

DbVar* getDbVar(sysCfg* sysdb, const char* name);
void emit_event(sysCfg *sys, const char* source, const char* message, int severity);
void emit_event_filt(sysCfg *sys, const char* source, const char* message, int severity);
cJSON* get_config_json(int argc, char* argv[], int num);

// new mapping
bool parse_system(cJSON* object, sysCfg* sys, int who);
bool parse_variables(cJSON* object, sysCfg* sys, int who);
cJSON* parseJSONConfig(char* file_path);

void pubWithTimeStamp(cJSON* cj, sysCfg* sys, const char* ev);

void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const AnalogOutputInt16& cmd, uint16_t index);
void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const AnalogOutputInt32& cmd, uint16_t index);
void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const AnalogOutputFloat32& cmd, uint16_t index);
void sysdbAddtoRecord(sysCfg* sysdb, const char* field, const char* cmd, uint16_t index);
const char* cfgGetSOEName(sysCfg* sysdb, const char* fname);

int addVarToCj(sysCfg*sys, cJSON* cj, DbVar*db);
int addVarToCj(sysCfg*sys, cJSON* cj, std::pair<DbVar*,int>dbp);
int addVarToCj(sysCfg*sys, cJSON* cj, const char *dname);
int addVarToCj(sysCfg*sys, cJSON* cj, DbVar* db, int flag);


cJSON* parseBody(dbs_type& dbs, sysCfg* sys, fims_message* msg, int who);
int addValueToVec(dbs_type& dbs, sysCfg* sys, fims_message* msg, const char* name, cJSON* cjvalue, int flag);
int addValueToVec(dbs_type& dbs, sysCfg* sys, char* curi, const char* name, cJSON* cjvalue, int flag);

bool checkWho(sysCfg* sys, const char* uri, const char* name, int who);
bool checkWho(sysCfg* sys, DbVar* db, int who);
int getSysUris(sysCfg **sys, int who, const char** &subs, bool* &bpubs, int n);

const char *getVersion();
int getConfigs(int argc, char *argv[], sysCfg**sys, int who, fims *p_fims);

std::string ToUTCString(const DNPTime& dnptime);
UTCTimestamp Now();
int HandleEvent(sysCfg *sys,  DbVar* db , int etype);
bool checkFileName(const char* fname);
bool checkDeviceName(const char* fname);

#endif
