#ifndef ASSETVAR_HPP
#define ASSETVAR_HPP
/*
 * this contains most of the data manipulation code for handling the internal
 *  system data spaces.
 * This code is NOT ess_specific and can be used on other projects
 */

#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <malloc.h>
#include <map>
#include <mutex>
#include <pthread.h>
#include <stdarg.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <any>
#include <typeinfo>
#include <cxxabi.h>

#include <cjson/cJSON.h>
#include <fims/fps_utils.h>
#include <fims/libfims.h>

enum class Severity  // scoped enum, much better
{
    // Debug = 0,
    Info = 1,
    Status,
    Alarm,
    Fault
};

//#include "asset.h"
// DONE add concpt of linkVars

// DONE isDiff ValueChanged etc lVal pointer.
// DONE isDiff will always be true   remove references.

// DONE setVal(param,data)
//      getiVal(param)  etc..

// DONE SendVal follow link ( global lockout )
// Links   test
// Timer plus Sequence.

// DONE assetVar as Param.
// DEFERRED  pvec param version of avec
// DONE process action as a vector , allow multiple instances of an action.

// DONE add double settime to asset val  ... done now have to add setval/time
// done but needs to call nm.setTime() DONE  but just to bitmap do the same for
// the assetVar for scaling etc add the Feat Dict to keep all this crap DONE
// allow reconfig.. read the file and change key to new DONE use dict for
// assfeat .. DONE add depth vector to allow history of asset val DONE scaling
// and offset on remap  note we can have multiple remaps with different scaling
// DONE Fix assfeat for enum and bitmap
// DONE use dict for assfeat display
// DONE   more testing needed allow comp:var    for example
// /system/components:active_setpoint DONE assetVar needs to know its component
// DONE rework and test enum and bitmap to latest spec
// DONE load configs from a file with multiple subs
// DONE add linkVal  only sets the value if it is not in the config
// DONE access setTime;
// DONE put subs in the config file need to propagate
// DONE check config links override Function links
// DONE -Wall
// DONE Gcc 7.0
// DONE i think remove comp locks .. we'll do it at the asset level.
// DONE make run_asset / run_manager work
// DONE remove specific asset types its all in the amap and vmaps
// DONE valueChanged valueDiff .. perhaps we need valueIfDiff and ValueDiff
// value callback DONE set/get AssetVar  detects 1 layer component and adds
// assetID  everything must now b for an asset DONE publist a Vlist ot set of
// asset vars when required (tod example test_phil) DONE set both valueint and
// valuedouble  in the assetvals to help with the cJson representation DONE
// function refs in varsmap vm.add/getFunction ( name , void* ptr) not yet in
// use but works a treat DONE remove hard coded asset manager setup DONE
// register functions for init and wakeups DONE getLastSetDiff gets time after
// last set DONE setFunc DONE assetFeatDict. availalbe now for assetVar DONE
// TESTING add valueChanged valueDiff getStime DONE I think collect pubs blocks
// and subs using vecMap DONE create assetFunc.h .cpp

// INPROGRESS vlist to different components
// INPROGRESS fims accept, reject and fward methods.
// DONE fix PUBS
// INPROGRESS macros to stop crashes
// DEFERRED  Fix comp logic  only going with two levels for no but comp:var
// solves some problemse an asset example /status:soc becomes /status/bms_4:soc
// DONE add in params for Assetvars and spit then out when pubbing
// SKIP use vector for aVals
// DEFERRED  detect and use depth vector to allow history of asset val
// DONE create code review repo

// DONE response operation. We set a value on the peripheral and want to handle
// the reply verifing the set..... DONE faults etc. DONE add lastvar DONE rework
// amap links just do it for functions as needed DONE complete full start up.
// DONE clean up system cold and warm restarts
// DONE register functions for events and messages
// DONE only delete your own assetVars
// DEFERRED add locking if needed
// DEFERRED send messages back upstream
// DONE auto detect links test_bms.
// DONE test setTime
// INPROGRESS fix broken tests

// DEFFERED << >> operators for channel.
// DONE   fixed in getMapsCj  fix asset config  . we can have three layers
// /asset/bms/bms_1 DEFERRED amap is amap leave it at that ..turn amap into a
// full varsmap test_bms DEFERRED for now ther are some common aspects but ..
// subclass asset_manager from asset...... DEFERRED  add hos asset

// DONE Deadband  "actions" , add a assDict into some new Params
// DEFFERED Assvar can hold sequences
// DONE Max min methods
// PENDING unOrdered map

// DEFFERED detect and use depth vector to allow history of asset val
// DEFFERED add auto cjson detect to find nfrags on a set or get ..... hmm how
// do we do get DEFFERED add fims set and get with notification DEFERRED file
// set and get this allows auto save and recall DEFERRED mongodb set and get ...
// same thing DONE limit checking on set against limits in variable names.  ( do
// it on remap) use max/min = /table:var  onMax onMin max/minerr/time =
// /table:var etc to map it...

// Response operation/
// DONE
// See doc/promise.txt
// We need a way to send a command to the asset and then get notified when that
// asset has confirmed the command through a status message. So set
// /controls/bms_1:OnCmd true will cause a response in /status/bms_1:status or
// some other response value.

#define DEF_DEADBAND 0.00000001;

class asset_log
{
public:
    asset_log();
    asset_log(void* src, void* dest, const char* atype, const char* msg, int sev, double ltime = 0.0);
    void setDestIdx(int idx);
    ~asset_log();

    void* srcAv;
    void* destAv;
    int destIdx;
    std::string altype;
    std::string almsg;
    int severity;
    std::string comp;
    std::string name;
    void* aVal;
    double base_time;  // set and used for stored logging
    double log_time;
};

class assetVar;

// something other than name, register_id, scale and unit
// TODO(root): AFTER MVP use featdict for options
class assFeat
{
public:
    assFeat(const char* _name, int val);
    assFeat(const char* _name, double val);
    assFeat(const char* _name, bool val);
    assFeat(const char* _name, const char* val);
    assFeat(const char* _name, assetVar* val);
    assFeat(const char* _name, void* val);
    ~assFeat();
    assFeat(const assFeat& other);
    assFeat& operator=(const assFeat& other);

    enum AFTypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AAVAR,
        AVOID,
        AEND
    };
    std::string name;
    // std::string fname;
    AFTypes type;
    double valuedouble;
    int valueint;
    char* valuestring;
    bool valuebool;
    assetVar* av;
    void* valuevoid;
    bool lock;
    bool unlock;
};

// one of these in the assetAction
// also one in the assetVar.

class assetFeatDict
{
public:
    assetFeatDict();
    ~assetFeatDict();
    assetFeatDict(const assetFeatDict& other);
    assetFeatDict& operator=(const assetFeatDict& other);
    int addCj(cJSON* cj, int uiObject = 0, bool skipName = true);
    int getFeat(const char* name, int* val);
    bool getFeat(const char* name, bool* val);
    double getFeat(const char* name, double* val);
    char* getFeat(const char* name, char** val);
    char* getcFeat(const char* name);
    assFeat* getFeat(const char* name);
    void* getFeat(const char* name, void** val);
    assetVar* getFeat(const char* name, assetVar** val);
    // moved to assetFunc.cpp
    cJSON* getFeat(const char* name, cJSON** cj);
    int getFeatType(const char* name);
    bool gotFeat(const char* name);

    void setFeat(const char* name, cJSON* cj, assetVar* av = nullptr);
    void setFeat(const char* name, assetVar* val, assetVar* av = nullptr);
    void setFeatfromAv(const char* name, assetVar* val, const char* param = nullptr);
    void setFeat(const char* name, double val, assetVar* av = nullptr);
    void setFeat(const char* name, int val, assetVar* av = nullptr);
    void setFeat(const char* name, int idx, bool val, assetVar* av = nullptr);
    void setFeat(const char* name, bool val, assetVar* av = nullptr);
    void setFeat(const char* name, const char* val, assetVar* av = nullptr);
    void setFeat(const char* name, void* val, assetVar* av = nullptr);
    void setFeat(const char* name, int idx, cJSON* val, assetVar* av = nullptr);
    template <class T>
    void addFeat(const char* name, T val);
    void addFeat(cJSON* cj);
    void showFeat();
    // moved to assetFunc.cpp
    void showCj(cJSON* cjix);

    char* tmpval;
    std::map<std::string, assFeat*> featMap;
};

// used for the bit mapping a bit  added a featDict for vars
class assetBitField
{
public:
    assetBitField(int _mask, int _bit, const char* _uri, const char* _var, char* tmp);
    assetBitField(cJSON* cj);
    ~assetBitField();
    // template<class T>
    int getFeat(const char* name, int* val);
    bool getFeat(const char* name, bool* val);
    double getFeat(const char* name, double* val);
    char* getFeat(const char* name, char** val);
    char* getcFeat(const char* name);
    // T getFeat(const char* name, T* val);
    cJSON* getFeat(const char* name, cJSON** cj);
    void* getFeat(const char* name, void** cj);
    bool gotFeat(const char* name);
    assFeat* getFeat(const char* name);
    int getFeatType(const char* name);
    template <class T>
    void addFeat(const char* name, T val);
    void showFeat();
    char* getTmpval();

    // new stuff v1.1.0
    void setFeatfromAv(const char* name, assetVar* Av, const char* param = nullptr);
    void setFeat(const char* name, double val);
    void setFeat(const char* name, bool val);
    void setFeat(const char* name, int val);
    void setFeat(const char* name, const char* val);
    assFeat* outaf;
    assetVar* inAv;
    char* inParam;
    bool inAvOK;
    assFeat* avaf;
    assetVar* invAv;
    char* invParam;
    char* outParam;
    assetVar* varAv;
    assetVar* outAv;
    assetVar* enAv;
    assFeat* inaf;
    assetVar* ignAv;
    assFeat* ignaf;
    int mask;
    int bit;
    int shift;
    char* uri;
    char* var;
    double adval;
    double scale;
    double offset;
    int aval;
    bool trigger;
    bool useSet;
    bool useRange;
    bool setup;
    bool useAv;

    // Done store all these in a Feat Dict
    int atype;
    // char* tmpval;
    // std::map<std::string, assFeat*>featMap;
    assetFeatDict* featDict;
    // function/amap  pointer
    void* fptr;
    void* amapptr;
    bool isAi;
    bool isAm;
};

class assetAction
{
public:
    assetAction(const char* aname);
    ~assetAction();
    assetBitField* addBitField(cJSON* cjbf);
    void showBitField(int show = 0);
    assetBitField* getBitField(int num);
    assFeat* getFeat(int num, const char* aname);

    int idx;
    // if onSet has  runFunc then run it;

    //    // this is all related to the runFunc idea
    //     void setupRunFunc(int (*_runFunc)(varsmap &vmap, varmap &amap, const
    //     char* aname, fims* p_fims, asset *am),varsmap *_vmap, varmap
    //     *_amap,const char* _aname, fims* _p_fims, asset *_am)
    //     {
    //         runFunc = _runFunc;
    //         vmap = _vmap;
    //         amap = _amap;
    //         aname = _ _aname;
    //         p_fims = _p_fims;
    //         am = _am;
    //     };

    //     int (*runFunc)(varsmap &vmap, varmap &amap, const char* aname, fims*
    //     p_fims, asset *am); varsmap *vmap; varmap *amap; const char* aname;
    //     fims* p_fims;
    //     asset *am;

    std::map<int, assetBitField*> Abitmap;
    std::string name;
};

class assetVar;
class asset;
class asset_manager;

// typedef std::map<std::string, pthread_mutex_t*> locksmap;
typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
typedef std::map<std::string, assetVar*> varmap;
typedef std::map<std::string, varmap*> nvarsmap;

typedef std::map<std::string, std::vector<std::vector<std::pair<std::string, std::string>>>*> optvec;
typedef std::vector<assetVar*> assetlist;
typedef std::vector<assetVar*> schAvlist;

typedef std::map<std::string, std::map<std::string, std::vector<assetVar*>>> avarmap;

typedef std::map<std::string, asset_manager*> ammap;
typedef std::map<std::string, asset*> aimap;

// saved in varsmap as _assetList/comp
// contains the order of assetVars for publish.
// UI Hack to maintain order for assetVars
class assetList
{
public:
    assetList();
    assetList(const char* _name);
    ~assetList();
    void add(assetVar* av);
    assetVar* avAt(unsigned int ix);
    const char* getName();

    int size();

    std::string name;
    assetlist aList;
};

// picks up the "options":[<av>,<av>] in the Ui thing
class assetOptVec
{
public:
    assetOptVec();
    ~assetOptVec();
    void showCj(cJSON* cj);
    void addCj(cJSON* cj);
    optvec OptVec;
    cJSON* cjopts;
    std::string name;
};

// the assetVal allows us to keep the lastval and a mini history if needed.
class assetVal
{
public:
    enum ATypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AAVAR,
        AEND
    };
    assetVal();
    assetVal(int val);          //:assetVal(){}
    assetVal(double val);       //:assetVal(){}
    assetVal(bool val);         //:assetVal(){}
    assetVal(const char* val);  //:assetVal(){}

    assetVal(assetVar* val);  //:assetVal(){}

    ~assetVal();

    bool getVal(bool val);
    int getVal(int val);
    double getVal(double val);
    char* getVal(char* val);
    assetVar* getVal(assetVar** val);
    bool IsNumeric();
    bool IsString();
    bool IsBool();
    bool IsAv();
    void setVal(bool val);
    void setVal(int val);
    void setVal(double val);
    void setVal(const char* val);
    void setVal(char* val);
    // void setVal(char const* val);
    bool setVal(cJSON* cj);
    void setVal(assetVar* val);
    void setType(ATypes t);
    double getsTime();
    double getcTime();
    bool setVal(int index, bool val);
    bool getbVal(int index);

    assetVal(const assetVal& other);
    assetVal& operator=(const assetVal& other);

    cJSON* getValCJ();
    cJSON* getValCJ(double scale, double offset);

    ATypes type;
    double valuedouble;
    int valueint;
    char* valuestring;
    bool valuebool;
    void* valuevoid;
    double setTime;
    double chgTime;
    assetVar* av;
    bool lock;
};

class asset_manager;
class asset;

class assetExtras;
// this is the assetvar we love
class assetVar
{
public:
    assetVar();
    assetVar(const char* _name, int val);
    assetVar(const char* _name, double val);
    assetVar(const char* _name, const char* val);
    assetVar(const char* _name, bool val);
    assetVar(const char* _name, cJSON* cjval);
    assetVar(const char* _name, assetList* val);

    enum ATypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AVAR,
        AVOID,
        AEND
    };

    void SetFunc(assetVar* av);
    void SetPubFunc(assetVar* av);
    void PubFunc(assetVar* av);
    const char* getName();

    template <class T>
    assetVar(const char* _name, const char* _comp, T val) : assetVar(_name, val)
    {
        comp = _comp;
    }

    virtual ~assetVar();

public:
    int depth;
    int cval;
    std::string name;  // this is really id

    std::string comp;  // new we  set comp
    ATypes type;
    assetVal* aVal;  // current assetval
    assetVal* lVal;  // last assetval

    double dbV;
    assetExtras* extras;
    assetVar* linkVar;
    bool setNaked;
    assetVar* aVar;
    asset_manager* am;
    asset* ai;
    assetAction* aa;
    assetBitField* abf;
    int abNum;  // bit map sequence number  still needed ??
    bool IsDiff;
    // TODO(root): after MVP loose IsDiff
    bool valChanged;
    int ui_type;  // moved to extras
    // int users;
    char* fname;
    bool lock;

    char* getfName(void);
    double getLastSetDiff(double tnow);
    double getLastChgDiff(double tnow);
    double getSetTime(void);
    double getChgTime(void);
    double getdVal();
    int getiVal();
    bool getbVal();
    char* getcVal();
    void* getvVal();
    double getdLVal();
    int getiLVal();
    bool getbLVal();
    char* getcLVal();

    bool getbVal(int index);

    bool setVal(int index, bool val);

    cJSON* getValCJ();

    template <class T>
    T getVal(T val);
    // template <class T>
    int addVal(int val);
    double addVal(double val);
    template <class T>
    T subVal(T val);
    template <class T>
    T getLVal(T val);
    // template <class T>
    bool setVal(int val);
    bool setVal(double val);
    bool setVal(bool val);
    bool setVal(const char*);
    void setVal(assFeat* af, bool force = false);

    void setParam(const char* pname, bool val);
    void setParam(const char* pname, double val);
    void setParam(const char* pname, int val);
    void setParam(const char* pname, char* val);
    void setParam(const char* pname, void* val);
    void setParamfromCj(const char* pname, cJSON* val);
    void setParamfromAv(const char* pname, assetVar* val);

    void setParamIdxfromCj(const char* pname, int idx, cJSON* val);
    void setcParam(const char* pname, const char* val);

    int getiParam(const char* pname) const;
    double getdParam(const char* pname) const;
    bool getbParam(const char* pname) const;
    char* getcParam(const char* pname) const;
    void* getvParam(const char* pname);
    char* getcAParam(const char* pname);
    assetVar* getaParam(const char* pname);
    cJSON* getCjParam(const char* pname, int options = 0);

    bool gotParam(const char* pname);

    template <class T>
    void setFimsVal(T val, fims* p_fims, const char* acomp = nullptr);

    // template < class T>
    bool valueChanged(int reset = 0);
    // bool valueChanged();
    bool valueChangedReset();
    template <class T>
    void resetChanged(T val);
    // template <class T>
    bool valueIsDiff(bool val);
    bool valueIsDiff(int val);
    bool valueIsDiff(double val);
    bool valueIsDiff(const char* val);

    bool valueIsDiff(double db, const char* val);
    bool valueIsDiff(double db, bool val);
    bool valueIsDiff(double db, int val);
    bool valueIsDiff(double db, double val);
    template <class T>
    T valueDiff(T val);

    void setDbVal(double val);
    double getDbVal();

    assetVal* makeCopy(assetVal* av);
    template <class T>
    void setLVal(T val);
    bool setCjVal(cJSON* cj, bool force = false);
    void getCjVal(cJSON** cj);

    double setVecVal(double dval, int depth = 0);
    double getVecdVal(int vdepth);
    int getVecDepth();
    int setVecDepth(int vdepth);

    int getVecVals(int depth, double& avval, double& minval, double& maxval, double& spval, bool debug = false);
    void showvarExtrasCJ(cJSON* cjv, int opts);
    void showvarAlarmsCJ(cJSON* cjv, int opts);
    // // opts 0 :normal 1: naked 2:value only
    void showvarValueCJ(cJSON* cj, int opts = 0);
    void showvarValueOnlyCJ(cJSON* cj, int opts = 0);
    void showvarCJ(cJSON* cj, int opts = 0, const char* showas = nullptr);
    cJSON* getAction(assetAction* aact);

    asset_log* sendAlarm(assetVar* destAv, const char* atype, const char* msg, int severity);
    void setAlarm(asset_log* avAlarm);

    asset_log* getAlarm(assetVar* srcAv, const char* atype, int num = 1);

    int getNumAlarm(assetVar* srcAv, const char* atype);

    int clearAlarm(asset_log* avAlarm);

    int clearAlarm(assetVar* destAv, const char* atype);

    int clearAlarms();
    int sendEvent(const char* source, fims* p_fims, Severity severity, const char* msg, ...);

    // these are deprecated and are being removed
    // int sendLog(assetVar*av, const char *msg,...);
    // int flushLog(void);
    // int openLog(char* logDir, const char* fname, int depth=1);
    // int closePerf(const char* fname=nullptr);
    // int logAlways(bool flag=true);
    int addSchedReq(schAvlist& rreq);
    int addSchedReq(schAvlist& rreq, double tshot, double trep);

    // adds a remap action to this av using this uri. allows for any type of remap configuration
    // the only required parameter is "uri", all other params are optional
    void addRemap(std::string uri, int offset = 0, const std::any& inValue = std::any{},
                  const std::any& outValue = std::any{}, const std::any& ifChanged = std::any{}, std::string fims = "",
                  std::string replyto = "");

};  // end if assetVar::

// //typedef std::map<std::string, pthread_mutex_t*> locksmap;
// typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
// typedef std::map<std::string, assetVar*> varmap;
// typedef std::map<std::string, varmap*>nvarsmap;
typedef std::map<std::string, std::vector<std::string>*> vecmap;
// typedef std::map<std::string, std::vector<assetVar>*>optvec;

class VarsMap
{
    // VarsMap::
public:
    VarsMap();
    ~VarsMap();
    // TODO(root): map_lock  is not used remove after <MVP
    pthread_mutex_t map_lock;
    // locksmap amap;

    nvarsmap vmap;
};

// holds all the extra things that make the assetVar so big
typedef int (*myAvfun_t)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

class assetExtras
{
    // assetExtras::
public:
    enum ATypes
    {
        AINT,
        AFLOAT,
        ASTRING,
        ABOOL,
        AVAR,
        AEND
    };
    assetExtras();
    ~assetExtras();

public:
    int cval;
    ATypes type;

    ////std::vector<assetVal*>aVals;
    // std::map<std::string,assFeat*>featMap;
    // std::map<std::string, assetAction*>actMap;   //list of actions we can have
    // a single action or a vector ( but probably not both)
    std::map<std::string, std::vector<assetAction*>> actVec;  // list of actions for each category
    // adds 48 bytes (264 -> 312)
    // std::map<std::string, std::vector<assetAction*>>actVec2;   //list of
    // actions for each category
    assetFeatDict* featDict;
    assetFeatDict* optDict;
    // these two are used in the ui thing
    assetFeatDict* baseDict;
    assetOptVec* optVec;
    assetVar* SetPubFunc;
    assetVar* SetFunc;
    assetVar* GetFunc;
    assetVar* PubFunc;
    assetVar* RunFunc;
    void* monFunc;

    void* compFunc;  // used to trigger on component ident

    assetVar* aVar;
    asset_manager* am;
    asset* ai;
    assetAction* aa;
    assetBitField* abf;
    int abNum;  // bit map sequence number
                // TODO(root): loose IsDiff after MVP
    bool IsDiff;
    bool valChanged;
    int ui_type;

    // costs 100 bytes
    // std::map<std::string, asset_log*> alarmMaps;
    std::vector<asset_log*> alarmVec;
    bool useAlarms;
    // std::vector<asset_log*> logVec;
    int logfd;
    char* logFile;
    int logDepth;
    bool logAlways;
    std::string optName;
    std::vector<double> valVec;
    int valDepth;

    // notused yet but we'll use it for value string stuff
    std::vector<char*> charVec;
    int charDepth;
    std::string tbody;  // place to store the template body if we have one
};

#endif
