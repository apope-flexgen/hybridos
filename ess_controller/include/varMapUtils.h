#ifndef VARMAPUTILS_HPP
#define VARMAPUTILS_HPP
/*
 * this contains most of the data manipulation code for handling the internal
 * system data spaces.
 * This code is NOT ess_specific and can be used on other projects
 * double showing of options on a dump.
 * add dump option. /ess/dump  /ess/naked  etc
 *  
*/

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <malloc.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include <fims/libfims.h>

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif


#include "asset.h"
#include "assetVar.h"
#include "channel.h"
#include "assetFunc.h"
#include "dbi_funcs.hpp"

struct dbi_var_ref;

// this wont work #include "scheduler.h"

class scheduler;
// static int setvar_debug = 0;
// static int process_fims_debug = 0;

// pull in the process fims stuff
// accessor class for varmaps
// in asset.h
// typedef ammap std::map<std:string, asset_manager*>
// typedef aimap std::map<std:string, asset*>

class VarMapUtils
{
public:
    VarMapUtils();
    ~VarMapUtils();

public:
    double base_time;
    double ref_time;

    long int get_time_us();
    double get_time_dbl();
    double get_time_ref();
    double set_time_ref( int year, int month, int day, int hr = 0, int min = 0, int sec= 0);
    tm* get_local_time_now();
    void set_base_time(void);
    void setTime(void);
    double getTime(void);
    time_t getTNow(double tNow);

    varsmap* createVlist();
    cJSON* getVmapCJ(varmap& vm);
    bool notMissing(varmap& amap, const char* func, const char* vname);

    // uri is used for gets
    // pmap used as a selector
    //std::vector<std::string>* vx
    // int idx = 0;
            // if (vx)
            // {
            //     for (auto ix : *vx)
            //     {
            //         FPS_ERROR_PRINT("%s key [%s] > entry [%d] is [%s]\n", __func__, key, idx++, ix.c_str());
            //     }
            // }
    // uri is used for gets
    // INPROGRESS  not doing any gets yet
    bool processActOptions(varsmap& vmap, assetVar* av,  assetBitField* abf, const char *action, bool debug=false);
    bool processActRemapOptions(varsmap& vmap, assetVar* av,  assetVar** inAv, cJSON**cjov, assetBitField* abf, bool debug=false);
    bool processActEnumOptions(varsmap& vmap, assetVar* av, assetVar** inAv, 
           cJSON ** cjovp, assetBitField* abf, bool debug);
    int _vListSendFims(varsmap& vmap, const char* method, fims* p_fims, const char* dbi, const char* uri = nullptr, bool sim = false, assetVar* avd = nullptr);
    int vListSendFims(varsmap& vmap, const char* method, fims* p_fims, const char* uri = nullptr, bool sim = false, assetVar* avd = nullptr);
    int DbivListSendFims(varsmap& vmap, const char* method, fims* p_fims, const char* uri = nullptr, bool sim = false, assetVar* avd = nullptr);

    int vListAddVar(varsmap& vmap, assetVar* av, const char* comp = nullptr);
    int addVlist(varsmap* vl, assetVar* av, const char* comp = nullptr);
    int sendVlist(fims* p_fims, const char* method, varsmap* vl, bool sim = false, assetVar* avd=nullptr);
    int sendDbiVlist(fims* p_fims, const char* method, varsmap* vl, bool sim = false, assetVar* avd=nullptr);
    
    void clearVmapxx(varsmap& vmap, bool delAv = true);
    void clearVlist(varsmap* vl);

    void write_cjson(const char* fname, cJSON* cj);
    char* get_cjfile(const char* fname, std::vector<std::pair<std::string, std::string>>* reps = nullptr);
    cJSON* get_cjson(const char* fname, std::vector<std::pair<std::string, std::string>>* reps = nullptr);
  
    int fixupAMs(varsmap& vmap, fims * p_fims, cJSON* cjbase);
    asset_manager* fixupPname(varsmap& vmap, const char* pname);

    assetVar* getaVParam(varsmap &vamp, assetVar* aV, const char* pname);
    char* xpull_pfrag(const char* uri, int idx);
    char* xpull_first_uri(const char* uri, int n = 1);
    char* xpull_last_uri(const char* uri, int n = 1);
    char* xpull_uri(const char* uri, int idx);
    char* xpull_one_uri(const char* uri, int idx);
    int get_nfrags(const char* uri);
  
    // make a var used for asset lists used to order the /asset/xxx  output
    assetVar* makeVar(varsmap& vmap, const char* comp, const char* var, assetList* alist);
    assetVar* makeAVar(varsmap& vmap, const char* comp, const char* var, assetVar* av);
    //assetVar* makeCJVar(varsmap& vmap, const char* comp, const char* var, cJSON* cj);
    assetVar* replaceAv(varsmap& vmap, const char* comp, const char* var, assetVar* av);
    void setFunc(varsmap& vmap, const char* aname, const char* fname, void* func);
    void setAmFunc(varsmap& vmap, const char* aname, const char *fname, const char* amname, asset_manager* am, void* func);
    void setAvFunc(varsmap& vmap, const char* aname, const char *fname, const char* amname, assetVar* am, void* func);

    int findSocket(varsmap& vmap,char* host,int port);
    int setSocket(varsmap& vmap,char* host,int port,int sock);

    void* getFunc(varsmap& vmap, const char* aname, const char* fname, assetVar*avi = nullptr);
    const char* getdefLink(const char* name, const char* base, const char* var, char* buf, int blen);
    template <class T>
    assetVar* linkVal(varsmap& vmap, const char* comp, const char* var, T& defvalue);
    // void setVLinks(varsmap& vmap, const char* aname=nullptr);
    //assetVar* setVLink(varsmap& vmap, const char* vname, const char* vlink);

    void setVLinks(varsmap& vmap, const char* aname=nullptr, bool mkfrom = false, bool mkto = false);
    assetVar* setVLink(varsmap& vmap, const char* vname, const char* vlink,
                               bool mkfrom = false, bool mkto = false , assetVar*aV=nullptr);

    std::string run_replace(const std::string& inStr, const std::string& replace, const std::string& with);
    int configure_vmapCJ(varsmap& vmap, cJSON*cjbase, asset_manager* am, asset* ai, bool delcj=true);

    cJSON* getMapsCjFixed(varsmap& vmap, cJSON* cji=nullptr);
    void setLinks(varsmap& vmap, const char* aname);


    template <class T>
    assetVar* setLinkVal(varsmap& vmap, const char* aname, const char* cname, const char* var, T& defvalue);
    template <class T>
    assetVar* makeVar(varsmap& vmap, const char* comp, const char* var, T& value);
  
    template <class T>
    assetVar* setVal(varsmap& vmap, const char* comp, const char* var, T& value);

    // this one allows an index through
    template <class T>
    assetVar* setVal(varsmap& vmap, assetUri &my, T& value);


    assetVar* runActBitFieldfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    assetVar* runActBitSetfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    assetVar* runActEnumfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);

    // set bits in output var based on mask
    assetVar* runActBitMapfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    assetVar* runActClonefromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    assetVar* setDefault(varsmap &vmap, const char* vcomp, const char*vname, const char * ltype);


    // new from spec // done
    // {
    // "/commands[/asset_id]":{"active_power_setpoint":
    //                        {"value":0,
    //                            "signed":true,
    //                            "actions":
    //                   {"onSet":{"remap":
    //                         [
    //                            { "enable":"/controls/ess_controls:active_power_enable","offset":0,  "scale":10, 
    //                                                       "uri":"/cmds/ess_controls:active_power_cmd"}
    //                         ]
    //                       }
    //                    }
    //                }
    //         }
    // }
    // /a/b/c:Av@Param

    // slight rework 12/8/2020....
    // enable .. well that's OK stays the same
    //  **inValue ... if set used to only remap if the value matches
    //  offset , scale  unchanged
    //  **"uri":"/cmds/ess_controls:active_power_cmd@Param "    extended to set PAram
    //  **outValue  if we are going to do the remap than set the outValue to this number 

    
    // gone to assetFunc.cpp
    assetVar* runActRemapfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    assetVar* runActLimitsfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);


    // Run a function following an onSet
    // this is moved to test/assetFunc.cpp
    assetVar* runActFuncfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    assetVar* runActValfromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug=false);
    // make this return a vector of actions
    // assetVar* xxsetActValfromCj(varsmap& vmap, assetVar* av)//,  cJSON *cj)
    // {
    //     //the value has already been set
    //     // now run the actions
    //     //auto aa = av->actMap["onSet"];
    //     auto aa = av->extras->actVec["onSet"];
    //     return runActValfromCj(vmap, av, aa);
    // }
    // this returns a vector of asset actions
    assetVar* setActVecfromCj(varsmap& vmap, assetVar* av);//,  cJSON *cj)
    assetVar* setActBitMapfromCj(assetVar* av, assetAction* aact, cJSON* cjbf, cJSON* cj);
    assetVar* setActVecfromCj(assetVar* av, const char* act, const char* opt, cJSON* cjbf, cJSON* cj);
    assetVar* setActOptsfromCj(assetVar* av, const char* act, const char* opt, cJSON* cj);
    assetVar* setActOptsfromCj(assetVar* av, const char* act, cJSON* cj);
    assetVar* setActfromCj(assetVar* av, cJSON* cj);
    assetVar* setActOptsfromCjxx(assetVar* av, const char* act, const char* opt, cJSON* cjopt);

    assetVar* setParamfromCj(varsmap& vmap, const char* comp, const char* var, const char *param, cJSON* cj, int uiObject = 0);
    assetVar* setParamfromCj(varsmap& vmap, assetUri &my, cJSON* cj, int uiObject = 0);

    // // How do we designate a alarm/fault object  We see ui_type as an alarm in loadmap 
    assetVar* setValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject = 0);
    //assetVar* orig_setValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject = 0);
    //assetVar* new_setValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject = 0);
    
    assetVar* getVar(varmap& amap, const char* var);
    bool linkVar(assetVar* av, assetVar* avl, bool force =  false);

    assetVar* getVar(varsmap& vmap, const char* comp, const char* var = nullptr);
    template <class T>
    T getVar(varsmap& vmap, const char* comp, const char* var, T& value);

    int baseVec(std::string& bs, std::vector<std::string>& buri, std::vector<std::string>& turi);
    int uriSplit(std::vector<std::string>& uriVec, const char* _uri);
    int uriSplit(std::vector<std::string>& uriVec, const char* _uri, const char* _key);

    cJSON* createUriListCj(varsmap& vmap, std::string& bs, const char* inuri, cJSON* incj, int options, std::vector<std::string>& uriVec);
    int createAssetListCj(varsmap& vmap, const char* uri, std::vector<char*>* syscVec, int opts, std::vector<std::string>& nVec);
    void addCjFrags(cJSON* cj, const char* uri, cJSON* junk);

    bool cJSON_Compare(cJSON *a, cJSON*b);
    bool assFeat_Compare(assFeat *a, assFeat *b, bool debug=false);

    cJSON* getMapsCj(varsmap& vmap, const char* inuri = nullptr, const char* var = nullptr, int opts = 0, const char* origuri = nullptr, cJSON* cji = nullptr);

    int getAssetListVersion(varsmap& vmap, const char* alistname, const char* alistVersion);
    void getMapsVm(varsmap& vmap, varsmap& vmr, const char* uri = nullptr, const char* var = nullptr);
    bool strMatch(const char* str, const char* key);
    cJSON* getCompsCj(varsmap& vmap, const char* key, const char* var = nullptr);
    void getCompsVm(varsmap& vmap, varsmap& rmap, const char* key, const char* var=nullptr);
    void processMsgGet(varsmap& vmap, const char* method, const char* uri, const char* body, cJSON** cjr, asset_manager* am = nullptr, asset* ai = nullptr);
    void processMsgSetPubUi(varsmap& vmap, const char* method, const char* uri, int& single, const char* body, cJSON** cjr, asset_manager* am = nullptr, asset* ai = nullptr);

    // make an assetList var only for uiObjects
    // This is a real hack we store the assetList away in the assetList area of varsmap
    assetList* setAlist(varsmap& vmap, const char* uri);
    assetList* getAlist(varsmap& vmap, const char* uri);
    void zapAlist(varsmap& vmap, const char* uri);

    // for the UI we need to keep a list of everything in order.
    // dont want to mess up this cde .. We could ussr the cjr to capture the exact order.
    // the vars map coild contain an assetList I guess.

    void processMsgSetPub(varsmap& vmap, const char* method, const char* uri, int& single, const char* body, cJSON** cjr, asset_manager* am = nullptr, asset* ai = nullptr);

    void processMsgSetReply(varsmap& vmap, const char* method, const char* inuri, const char* replyto, const char* body, cJSON** cjr, asset_manager* am = nullptr, asset* ai = nullptr);


    void processRawMsg(varsmap& vmap, const char* method, const char* uri, const char* replyto, const char* body,
        cJSON** cjr, asset_manager* am = nullptr, asset* ai = nullptr);
    void processFims(varsmap& vmap, fims_message* msg, cJSON** cjr, asset_manager* am = nullptr, asset* ai = nullptr);
    void free_fims_message(fims_message* msg);
    fims_message* bufferToFims(const char* buffer);
    char* fimsToBuffer(const char* method, const char* uri, const char* replyto, const char* body);
    void clearVm(varmap& vmap);
    void clearVmap(varsmap& vmap);
    int testRes(const char* tname, varsmap& vmap, const char* method, const char* uri, const char* body, const char* res, asset_manager* am = nullptr, asset* ai = nullptr);
    int configure_vmapStr(varsmap& vmap, const char* body ,asset_manager*am, asset*ai, bool delcj);
    int configure_vmap(varsmap& vmap, const char* fname, std::vector<std::pair<std::string, std::string>>* reps = nullptr, asset_manager* am = nullptr, asset* ai = nullptr);
    void CheckLinks(varsmap& vmap, varmap& amap, const char* aname);


    //template<class T>
    bool valueChanged(double one, double theother, double deadb);
    //template<class T>
    bool valueChanged(double one, double theother);
    //template<class T>
    bool valueChanged(assetVar* one, assetVar* theother, assetVar* deadb, double vtype, double timeout);
    //template<class T>
    bool valueChangednodb(assetVar* one, assetVar* theother, double vtype, double timeout);

    // send to fims or another assetVar
    int sendAssetVar(assetVar* av, fims* p_fims,  const char* meth=nullptr, const char* comp = nullptr, const char* var = nullptr, assetVar* avd=nullptr);

    int getSubCount(char* scopy);
    int showList(char** subs, const char* aname, int& ccnt);
    int addListToVec(vecmap& vecs, char** subs, const char* vname, int& ccnt);
    int clearList(char** subs, const char* aname, int& ccnt);
    void getVList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt);
    char** getVmapList(vecmap& vecs, varsmap& vmap, int& ccnt);
    int setupAssetList(varsmap& vmap, const char* aname, const char* alistname, const char* alistVersion, asset_manager* am);
    char** getListStr(vecmap& vecs, varsmap& vmap, const char* vname, int& ccnt, char* dbsubs);
    char** getList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt);
    char** getDefList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt);

    void handleCfile(varsmap& vmap, cJSON* cj, const char* method, const char* uri
            , int& single, const char* body, cJSON** cjr, asset_manager* am, asset* ai);


    template<class T>
    void vecMapAddEntry(vecmap& vecm, const char* name, T val);
    void vecMapAddChar(vecmap& vecm, const char* name, const char* val);
    template<class T>
    std::vector<T>* vecMapGetVec(vecmap& vecm, const char* name, T val);
    void testvecMap(void);
    void showvecMap(vecmap& vcmap, const char* key = nullptr);
    cJSON* loadVmap(varsmap& vmap, int single, const char* comp, const char* var, const char* body, asset_manager* am=nullptr, char * uri=nullptr);
    

    // hack do not allow pubs from /site anything
    bool checkedBlockedUri(varsmap& vmap, char** cName, int& opts, const char* method, const char* uri);
    // gets single and value response
    //const char* uri = "/variables/bms_1/ac_contactor";
    //    const char* body1 = "true";
    // if method == set we do it by detecting the body content to see if the last pfag was the variable
    //cJSON* cj = checkSingleUri(vmap,        single,        &vName,     &cName2,        msg->method, newUri,              msg->body);
    // will recover uri and var from a possible single uri
    cJSON* checkSingleUri(varsmap& vmap, int& single, char** vName, char** cName, const char* method, const char* uri = nullptr, const char* body = nullptr);


    cJSON* getVmap(varsmap& vmap, int& single, const char* key = nullptr, const char* var = nullptr, int opts = 0, const char *baseUri=nullptr);
    cJSON* loadUimap(varsmap& vmap, int single, const char* comp, const char* var, const char* body, asset_manager* am=nullptr);
    cJSON* getUimap(varsmap& vmap, int& single, const char* key = nullptr, const char* var = nullptr);

    //bool runFimsMsg(varsmap& vmap, fims_message* msg, fims* p_fims);
    bool runFimsMsg(varsmap& vmap, fims_message* msg, fims* p_fims = nullptr, cJSON**cjri= nullptr);
    bool runFimsMsgAm(varsmap& vmap, fims_message* msg, asset_manager* am, fims* p_fims = nullptr, cJSON**cjri= nullptr);

    int CheckReload(varsmap& vmap, varmap& amap, const char* aname, const char* fname, void* func = nullptr);

    int setAlarm(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* atext, int retval);
    asset_log* sendAlarm(varsmap& vmap, const char* srcUri, const char* destUri, const char* atype, const char* msg, int severity);
    asset_log* sendAlarm(varsmap& vmap, assetVar* srcAv, const char* destUri, const char* atype, const char* msg, int severity);
    int clearAlarm(varsmap& vmap, assetVar* srcAv, assetVar* destAv, const char* atype, const char* msg, int severity);
    int clearAlarm(varsmap& vmap, const char* srcUri, const char* destUri, const char* atype, const char* msg, int severity);
    int clearAlarm(varsmap& vmap, assetVar* srcAv, const char* destUri, const char* atype, const char* msg, int severity);
    int clearAlarms(varsmap& vmap, const char* destUri);
    //av.sendAlarm(vmap,  "srcUri", "destAv","atype","msg, severity)

    
    template <class T>
    void setParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname, T val);
    int getiParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname);
    double getdParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname);
    bool getbParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname);
    char* getcParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname);

    int setAmFunc(varsmap &vmap,varmap &amap, const char* aname, fims* p_fims
                , asset_manager*am,const char*vname
                    ,int (*func)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset_manager*am));

    int setAvFunc(varsmap &vmap,varmap &amap, const char* aname, fims* p_fims
                , assetVar*am,const char*vname
                    ,int (*func)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*am));
    //void setSequence(varsmap& vmap, const char* aname, const char* fname, void* func, const char* help);
    void setActions(varsmap& vmap, const char* fname);
    void setAction(varsmap& vmap, const char* aname, const char* fname, void* func, const char* help);


    bool loadSiteMap(varsmap& vmap, cJSON* cjss);
    void setMonitorList(varsmap& vmap, const char* comp);
    void setMonitorList2(varsmap& vmap, const char* comp, const char* wname);
    // //void runMonitorList(varsmap& vmap, const char* aname, double rate, double offset);
    // void runMonitorList(varsmap& vmap, varmap &amap, const char* aname, fims* p_fims, double rate, double offset);
    void runMonitorList2(varsmap& vmap, varmap &amap, const char* aname, fims* p_fims, const char* wname, bool debug = false);
    int setVecDepth(varsmap& vmap, varmap& amap, const char* vname, const char *vecName, int depth, double *lValp);
    int setVecHold(varsmap& vmap, varmap& amap, const char* vname, const char *vecName, bool hold);

    void setSysName(varsmap &vmap, const char* sname);
    char* getSysName(varsmap &vmap);

    //asset_manager* fixupPname(varsmap& vmap, const char* pname);

    //void setLinks(varsmap& vmap, const char* aname);

    char*getFileName(const char* fname);
    bool checkFileName(const char* fileName);
    void setFname(const char* fname);
    void setRunLog(const char* dirName);
    void setRunCfg(const char* dirName);
    std::string getDbiNameAv(std::string& comp);
    assetVar* setVal(varsmap& vmap, const char* comp, const char* var, assetVar* aV, const char* pname);
    void setVLinkFromCj(varsmap &vmap, cJSON* cji, assetVar*avl , assetVar*aV);

    std::vector<char *>* syscVec;
    std::vector<std::string>* sysVec;

    unsigned int alarmId; 

    char* configDir;
    char* configFile;
    varsmap funMap;
    varsmap* vmapp;
    varsmap* funMapp;
    char** getVecListbyName(vecmap& vecs, const char* vname, int& ccnt);    
    scheduler *sched;
    assetVar*schedaV;    // used to save the current sched control assetvar
    double schedTime;
    int schedActions;    // used to limit recursive sched actions 
    char* sysName;
    ammap amMap;
    aimap aiMap;
    int vmlen;
    int vmdebug;
    int useId;
    std::vector<std::string>*idVec;   
    std::vector<dbi_var_ref> dbi_update_list;

    using two_way_tbl = std::map<std::pair<double, double>, double>;
    std::map<std::string, std::shared_ptr<two_way_tbl>> tblMap;
    
    asset_manager* getaM(varsmap& vmap, const char*aname);
    asset* getaI(varsmap& vmap, const char*aname);
    void setaM(varsmap& vmap, const char*aname, asset_manager*am);
    void setaI(varsmap& vmap, const char*aname, asset* ai);
    int schedStop(varsmap& vmap, varmap& amap, const char* vname, double eTime);
//    asset* getaI(varsmap& vmap, const char*aname);
    char *runLogDir;
    char *runCfgDir;

    std::map<assetVar *,void *>delavMap;
    int argc;
    fims* p_fims; 
    bool simdbi;
    bool noLog;


};


// needs to be in vmActions.h

int remapSendFims(varsmap& vmap, const char *fims, assetUri &auri, assetVar * av
     , assetVar* aV2 , assetBitField* abf, cJSON* cjov, cJSON* cjav, bool debug);

int remapSendAv(varsmap& vmap, VarMapUtils* vm, assetUri &auri, assetVar * av, assetBitField* abf, cJSON* cjov, cJSON* cjav, bool debug);

int remapSendAvParam(varsmap& vmap, assetUri &auri, assetVar * av, assetBitField* abf, cJSON* cjov, cJSON* cjav, bool debug);

// char *aprune_pfrag(const char *pfrag)
// {
//    char* comp_id = strdup(pfrag);
//    char* comp_end = strstr(comp_id,"/");
//    if(comp_end)
//        *comp_end = 0;
//    return comp_id;
// }

// Variadic template and base case for setting links, very useful/powerful stuff (CAUTION: arg/args can be anything, so check for compiler error):
template <typename dataType>
inline void linkVals(VarMapUtils& vm, varsmap& vmap, varmap& amap, const char* aname, const char* URI, dataType& type)
{
    return;
}
template <typename dataType, typename Arg, typename... Args>
inline void linkVals(VarMapUtils& vm, varsmap& vmap, varmap& amap, const char* aname, const char* URI, dataType& type, Arg&& arg, Args&&... args)
{
    // ((amap[std::forward<Args>(args)] = vm->setLinkVal(vmap, aname, URI, std::forward<Args>(args), type)), ...); // C++17 and up way to do it!
    amap[std::forward<Arg>(arg)] = vm.setLinkVal(vmap, aname, URI, std::forward<Arg>(arg), type);
    linkVals(vm, vmap, amap, aname, URI, type, args...); // With C++17 and up this will become obsolete along with the base case function.
}

// Variadic template and base case for linking Vector Vals, as seen in test_release_funcs.cpp under function : SetupLimitsVec
template <typename dataType>
inline void linkAvarmapVals(VarMapUtils & vm, varsmap & vmap, varmap & amap, avarmap & avmap, const char* aname, const char* vname, std::string & mvar, std::string & mname, const char* URI, dataType & type)
{
    return;
}
template <typename dataType, typename Arg, typename... Args>
inline void linkAvarmapVals(VarMapUtils & vm, varsmap & vmap, varmap & amap, avarmap & avmap, const char* aname, const char* vname, std::string & mvar, std::string & mname, const char* URI, dataType & type, Arg && arg, Args&&... args)
{
    mvar = mname + std::forward<Arg>(arg);
    amap[mvar.c_str()] = vm.setLinkVal(vmap, aname, URI, mvar.c_str(), type);
    avmap[aname][vname].push_back(amap[mvar.c_str()]);

    linkAvarmapVals(vm, vmap, amap, avmap, aname, vname, mvar, mname, URI, type, args...);
}

void runLogAv2(asset_manager* am, assetVar* logav, const char *lname, double tVal);
void runLogAv(asset_manager* am, assetVar* logav, double tNow);
assetVar* getLogAv(asset_manager* am,  const char* aname, const char* lname);

// todo: remove this
class essPerf {
    public:
    essPerf(asset_manager *_am, const char* aname, const char* lname, double *invalue=nullptr);
    ~essPerf();
    assetVar* getePerfAv(const char* aname, const char* lname);

    void runePerfAv2(asset_manager* am, assetVar* logav, const char *lname, double tVal);
    void runePerfAv();
    double tNow;// =  am->vm->get_time_dbl();
    assetVar* logav;// = getLogAv(am, "ess", "pubLog");
    bool bval;// = logav->getbVal();
    asset_manager* am;
    double *inval;
    char *aname;
    char *lname;

};
double getTempFile(const char* fname);
void getTemps(double* package, double* core0, double* core1, double* core2, double* core3);
void getMemory( int* currRealMem, int* peakRealMem, int* currVirtMem, int* peakVirtMem);
int debugSystemLoad(VarMapUtils*vm, varsmap& vmap, vecmap &vecs, std::vector<char*>* syscpVec, const char *aname, const char* logdir);
//int debugSystemLoad(VarMapUtils*vm, vecmap &vecs, std::vector<char*>* syscpVec, const char *aname);
int loadSiteManager(VarMapUtils*vm, varsmap&vmap, const char* sname, const char*aname);
int  loadEssConfig(VarMapUtils*vm, varsmap& vmap, const char *cname, asset_manager* am, scheduler* sc);
asset_manager* setupEssManager(VarMapUtils*vm, varsmap& vmap, vecmap &vecs, std::vector<char*>* syscpVec, const char*aname, scheduler*sc);
bool cJSON_StringsToId(cJSON* item, std::vector<std::string>* idVec);

bool checkEnable(VarMapUtils*vm, varsmap& vmap, assetVar* av, bool debug);
bool checkabfEnable(VarMapUtils*vm, varsmap& vmap, assetBitField* abf, assetVar* av, bool debug);
bool checkabfChanged(VarMapUtils*vm,  varsmap& vmap, assetBitField* abf, assetVar* av, bool debug);
bool checkChanged(VarMapUtils*vm, varsmap& vmap,  assetVar* av, bool debug);

int checkAv(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
bool getLoggingEnabled(varsmap &vmap, VarMapUtils& vm);
bool getLoggingTimestamp(varsmap &vmap, VarMapUtils& vm);
char* getLogDir(varsmap &vmap, VarMapUtils& vm);
void setLoggingSize(varsmap &vmap, VarMapUtils& vm);

#endif
