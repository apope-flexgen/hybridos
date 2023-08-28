#ifndef ASSET_HPP
#define ASSET_HPP
/*
* asset and asset manager
*/

// TODO combine the  asset and asset manager after MVP

#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <malloc.h>
#include <cjson/cJSON.h>
#include <poll.h>
#include <signal.h>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <fims/libfims.h>
#include <fims/fps_utils.h>

// logging and formatting utilites, DO NOT REMOVE
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/bundled/ranges.h"

#include "assetVar.h"
#include "channel.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"

//#include "varMapUtils.h"
//#include "scheduler.h"
//#include "assetFunc.h"

typedef void* (*vLoop)(void* args);

// an asset will have variables, states and parameters and possibly methods all in the amap
// an asset can also have alarms and warnings

// TODO remove old wake level stuff  after MVP
#define WAKE_LEVEL_DEMAND 0
#define WAKE_LEVEL1 1
#define WAKE_LEVEL2 2
#define WAKE_LEVEL3 3
#define WAKE_LEVEL4 4
#define WAKE_LEVEL_MANAGE 100
#define WAKE_LEVEL_PUB 256
#define WAKE_LEVEL_PUB_HS 257
#define WAKE_LEVEL5 5
#define WAKE_LEVEL10 10
#define WAKE_LEVEL20 20
#define WAKE_LEVEL50 50
#define WAKE_LEVEL100 100
#define WAKE_LEVEL200 200
#define WAKE_LEVEL500 500
#define WAKE_LEVEL1000 1000

// TODO remove default log dir
#define DEFAULT_LOG_DIR "run_logs"

typedef assetVar* av_ptr;

enum AssStates
{
    Asset_Init = 0,
    Asset_Ok,
    Asset_On,
    Asset_Off,
    Asset_Standby,
    Asset_Reset,
    Asset_Alarm,
    Asset_Fault,
    Asset_Restart
};
enum AssCmds
{
    AssetInit = 0,
    AssetOn,
    AssetOff,
    AssetStandby,
    AssetReset,
    AssetFault,
    AssetRestart
};

enum SysStates
{
    System_Init = 0,
    System_Alarm,
    System_Fault,
    System_Ready,
    System_Startup,
    System_RunMode1,
    System_RunMode2,
    System_Standby,
    System_Shutdown

};
enum SysCmds
{
    SystemInit = 0,
    SystemOn,
    SystemOff,
    SystemStandby,
    SystemReset,
    SystemFault,
    SystemRestart
};

// #define CommsOK     1<<0
// #define HBOK        1<<1
// #define SysOK       1<<2

class asset_manager;
class asset;

typedef void (*myAmInit_t)(asset_manager* data);
typedef bool (*myAmWake_t)(asset_manager* data, int wake);
typedef void (*myAssInit_t)(asset* data);
typedef bool (*myAssWake_t)(asset* data, int wake);


// split /a/b/c:var@param up into
// uri /a/b/c
// var var
// param param
// plus we'll add all the "get nparams" stuff to this as we need it.
// 
// allow var to be name@param as well.  yup already covered

class assetUri {

public:
    assetUri(const char* uri, const char* var= nullptr);
    ~assetUri();
    void setup();
    int setupUriVec();
    void single();
    char* pullOneUri(int idx);
    char* pullPfrag(int idx);
    char* pullPvar(int idx);
    char* pullFirstUri(int n = 1);
    char* pullLastUri(int n = 1);
    char* pullUri(int idx);
    int getNfrags();


    char* Uri;
    char* origuri;
    char *vecUri;
    char* origvar;
    int nfrags;
    char* Var;
    char* Param;
    char* sUri;   // single decode /a/b/c into /a/b  c
    char* sVar;

    std::vector<char *> uriVec;
    int index;
    bool setValue;

};

#include "varMapUtils.h"

class asset {
//asset::
public:

    asset();
    asset(const char* _name);
    ~asset();

    void setAm(asset_manager* _am);
    void setName(const char* _name);
    void cfgwrite(const char* fname, const char* aname = nullptr);
    const char* get_command(const char* dest, const char* cmd);
    int configure(const char* fname, std::vector<std::pair<std::string, std::string>>* reps = nullptr, asset_manager* am = nullptr, asset* ai = nullptr);
    cJSON* getConfig(const char* uri = nullptr, const char* var = nullptr);
    bool free_message(fims_message* message);
    void cleanup(void);
    varmap* getBmap();
    void setPmap(varsmap* _vmap);
    varmap* getVmap();
    void setVmap(varsmap* _vmap);
    void setVm(VarMapUtils* _vm);
    //int Send(const char* method, const char*uri, const char*rep, const char* body);
    int Send(const char* method, const char*uri=nullptr, const char*resp=nullptr, const char* body=nullptr);
    int Send(const char* method, assetVar *av, const char*uri=nullptr, const char*resp=nullptr, const char* body=nullptr);


    //WIP NOT TESTED YET sets up / configs amap and vars
// TODO  run_init / run_wakeup deprecated after MVP
    void (*run_init)(asset* ass);
    bool (*run_wakeup)(asset* ass, int wake);

    std::string id;
    std::string name;
    asset_manager* am;

    // context for asset
    varsmap* vmap;
    // pub list
    varsmap* pmap;

    // this is a list of the local asset variables
    // needs to be cleared
    varmap amap;

    // utility to access the vmap
    VarMapUtils* vm;
    VarMapUtils defvm;
    fims* p_fims;
    vecmap* vecs;
    bool sendOK;
    assetVar* av; // root asset (FlexPack)
};

// the asset manager can set up one or more assets from an asset config file.
// the asset manager can also distribute command, status and control messages to and from assets.
class asset_manager {
//asset_manager::
public:
    asset_manager();
    asset_manager(const char* _name);
    ~asset_manager();
    void setVmap(varsmap* _vmap);
    void setPmap(varsmap* _vmap);
    varsmap* getVmap();
    varsmap* getPmap();
    void setAm(asset_manager* _am);
    void setName(const char* _name);

// TODO after MVP find out which (debugConfig assConfigure) is now used
    void debugConfig(asset* pc, const char* dmsg);
    void assconfigure(varsmap* vmap, const char* fname, const char* aname);
    asset* addAsset(cJSON* cja, cJSON* cjt,
        std::vector<std::pair<std::string, std::string>>& reps, asset_manager* am = nullptr);
    int amConfig(varsmap* vmap, cJSON* cj, asset_manager* am = nullptr);
    int configure(varsmap* vmap, const char* fname, const char* aname, std::vector<char *>* syscVec = nullptr, bool(*assWake)(asset*, int) = nullptr, asset_manager* am = nullptr);

// TODO  send_command ... old code remove after MVP
    const char* send_command(const char* dest, const char* cmd);


    void mapInstance(asset* item, const char* _name = nullptr);
    asset* addInstance(const char* _name);
    asset* getInstance(const char* _name);
    void configInstance(asset* item);
    int getNumAssets();
    void setupChannels();
    cJSON* getConfig(const char* uri = nullptr, const char* var = nullptr);

    void (*run_init)(asset_manager* am);
    bool (*run_wakeup)(asset_manager* am, int wake);

// TODO runchildren . manager loop . man_timer_loop all old code remove after MVP
    bool runChildren(int wakeup);
    void manager_loop();
    void man_timer_loop();
    void ass_timer_loop();
    void timer_loop();
    void  message_loop();
    void fims_loop();


    void cleanup(void);
    void run_manager(fims* _p_fims);
    void run_timer(int period);
    void run_message(int period);
    void run_fims(int period, char** subs, const char* name, int numSubs = 1);
    varmap* getAmap();
    bool addManAsset(asset_manager* am, const char* name);
    asset_manager* getManAsset(const char* name);
    void cfgwrite(const char* fname, const char* aname = nullptr);
    void setVm(VarMapUtils* _vm);

// TODO cascadeAI cascade AM, old Code remove after MVP
    int cascadeAI(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am
        , int(*runAI)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am));
    int cascadeAM(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am
        , int(*runAM)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
        , int(*runAI)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
    );
    //int Send(const char* method, const char*uri, const char*rep, const char* body);
    int Send(const char* method, const char*uri=nullptr, const char*resp=nullptr, const char* body=nullptr);
    int Send(const char* method, assetVar *av, const char*uri=nullptr, const char*resp=nullptr, const char* body=nullptr);

    fims* p_fims;
    // this is a list of the local asset variables
    varmap amap;
    // this is a list of the local asset perf variables
    varmap lmap;

    std::string name;
    //varmap assetMap;

    // this is a list of the managed assets
    std::map<std::string, asset*>assetMap;

    std::map<std::string, asset_manager*>assetManMap;
    // global context for asset manager
    varsmap* vmap;

    varsmap* pmap;
    VarMapUtils* vm;
  
    //typedef std::map<std::string, std::vector<std::string>*>vecmap;
    vecmap* vecs;

// TODO remove chan_data stuff after MVP
    chan_data t_data;  // time channel
    chan_data m_data;  // message channel
    chan_data f_data;  // fims channel 
    int tnum;

    channel <int> man_wakechan;         // this is for manager wakeups
    channel <int> wakechan;         // this is for wakeups
    channel <char*>msgchan;       // this is for messages ( will probably be fims sort of messages)
    channel <fims_message*>fimschan;  // this is for real (external) fims messages
    volatile int running;
    std::thread manager_thread;
    int reload;
    asset_manager* am;
    bool sendOK;
    // new sched uses these

    channel <int>* wakeChan;
    void* reqChan; // anyone can post run_reqs

    channel <fims_message*>*fimsChan; // anyone can post run_reqs

    // deprecated
    channel <int> *wakeUpChan;
    int setup;
    int run_secs;
    std::vector<char *>* syscVec;
    
    void setFrom(asset_manager* base);

    //int addSchedReq(schAvlist& rreq, double tshot, double trep);

};
extern "C++" {
    
    
    int  SetupGit(varsmap& vmap, VarMapUtils* vm
                 , const char*gbranch
                 , const char*gcommit
                 , const char*gtag
                 , const char*gversion
                 );

}
// the asset manager can set up one or more assets from an asset config file.
// the asset manager can also distribute command, status and control messages to and from assets.


//common stuff in assets
//     fims* p_fims;
//     std::string name;
//     asset_manager* am;
//     // context for asset
//     varsmap *vmap;
//     // pub list
//     varsmap *pmap;
//     varmap amap;
//     VarMapUtils *vm;
//     vecmap  *vecs;
//
// unique to asset_maager
//     void (*run_init)(asset* ass);
//     bool (*run_wakeup)(asset* ass, int wake);
//     std::string id;  // is this used ??
//     VarMapUtils defvm;  // is this used 

// // the types of the manage class factories
// typedef asset_manager* createm_t(const char *name);
// typedef void destroym_t(asset_manager*);

// // the types of the class factories
// typedef asset* create_t();
// typedef void destroy_t(asset*);
// // This function will create a  FIMS message buffer
// char* fimsToBuffer(const char* method, const char* uri, const char* replyto, const char* body);
// fims_message* bufferToFims(const char *buffer);

#endif
