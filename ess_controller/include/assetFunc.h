#ifndef ASSETFUNC_HPP
#define ASSETFUNC_HPP
#include "asset.h"
#include "assetVar.h"
//#include "assetFunc.h"
#include "varMapUtils.h"
extern "C" typedef int (*myAifun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset* ai);
extern "C" typedef int (*myAmfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager* am);
extern "C" typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);

 class asset;
 class asset_manager;

class assetFunc : public assetVar {
    public:
    assetFunc()
    {
        raiFunc = nullptr;
        ramFunc = nullptr;
        ravFunc = nullptr;
        ai = nullptr;
        am = nullptr;
        av = nullptr;
        p_fims = nullptr;
        aname = nullptr;
    };
    
    assetFunc(const char *_name) : assetFunc()
    {
        name = _name;
    };

    ~assetFunc(){};
    // this is all related to the runFunc idea
    // run asset instance
    void xxsetupRaiFunc(int (*_runFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am),varsmap &_vmap, varmap &_amap,const char* _aname, fims* _p_fims, asset *_am)
    {
        raiFunc = _runFunc;
        vmap = &_vmap;
        amap = &_amap;
        aname = _aname;
        p_fims = _p_fims;
        ai = _am;
    };

    // this is all related to the runFunc idea
    // run assetManager
    void xxsetupRamFunc(int (*_runFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager *_am),
            varsmap &_vmap, varmap &_amap,const char* _aname, fims* _p_fims, asset_manager *_am)
    {
        ramFunc = _runFunc;
        vmap = &_vmap;
        amap = &_amap;
        aname = _aname;
        p_fims = _p_fims;
        am = _am;
    };
    // this is all related to the runFunc idea
    // run assetManager
    void setupRavFunc(int (*_runFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar *_av),
            varsmap &_vmap, varmap &_amap,const char* _aname, fims* _p_fims, assetVar *_av)
    {
        ravFunc = _runFunc;
        vmap = &_vmap;
        amap = &_amap;
        aname = _aname;
        p_fims = _p_fims;
        av = _av;
    };

    int (*raiFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *ai);
    int (*ramFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager *am);
    int (*ravFunc)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar *av);
    varsmap *vmap; 
    varmap *amap; 
    const char* aname;
    fims* p_fims;
    asset* ai;
    asset_manager* am;
    assetVar* av;
};

// hbTesFunc* hbTest;
// if ( !amap["CheckAssetHBFunc"] )
// {
//   hbTesFunc * hbTest = new hbTestFunc(aname);
//   amap["CheckAssetHBFunc"] = (assetVar *)hbTest;
//   hbTest->toErr = 5.0;
//   hbTest->toWarn = 3.5;
// }
// hbTest = (hbTestFunc*) amap["CheckAssetHBFunc"];

// hbTest->runFunc(vmap, amap, aname, p_fims, am);
class hbTestFunc : public assetFunc {
    public:
    hbTestFunc(){
        raiFunc = nullptr;
        ramFunc = nullptr;
    };

    hbTestFunc(const char *_name)
    {
        name = _name;
        bokFault = false;
        bokAlarm = false;
        bokOK = false;
        toFault  = 5;
        toAlarm = 3.5;
        hbInit = -21;
        totalHBFaults = 0;
        totalHBAlarms = 0;

    }
    ~hbTestFunc(){};
    

    int runFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am);


    int lastHeartBeat;
    int hbInit;
    bool bokFault; 
    bool bokAlarm;
    bool bokOK;
    double toFault;
    double toAlarm;
    int totalHBFaults;
    int totalHBAlarms;

};

// commsTestFunc* commsTest;
// if ( !amap["CheckAssetCommsFunc"] )
// {
//   commsTesFunc * commsTest = new commsTestFunc(aname);
//   amap["CheckAssetHBFunc"] = (assetVar *)hbTest;
//   commsTest->toErr = 5.0;
//   commsTest->toWarn = 3.5;
// }
// commsTest = (commsTestFunc*) amap["CheckAssetCommsFunc"];

// commsTest->runFunc(vmap, amap, aname, p_fims, am);

// TODO review after MVP this is so much like the HB test. We should subclass again.

class commsTestFunc : public assetFunc {
    public:
    commsTestFunc(){};
    commsTestFunc(const char *_name)
    {
        name = _name;
        bokFault = false;
        bokAlarm = false;
        bokOK = false;
        toFault  = 5;
        toAlarm = 3.5;
        tsInit = (char*)" No Timestmp dectected";
        lastTimestamp = tsInit;
        totalCommsFaults = 0;
        totalCommsAlarms = 0;

    }
    ~commsTestFunc(){};

    int runFunc(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am);

    char* lastTimestamp;
    char* tsInit;
    bool bokFault; 
    bool bokAlarm;
    bool bokOK;
    double toFault;
    double toAlarm;
    int totalCommsFaults;
    int totalCommsAlarms;
};



#endif
