/*
 * aset lists tests
 * this one uses the featdict and has scale, offset features* 
 * Basic use of different lists in the system.
 * The key goal is to get the system to load aan asset var(s) or some var params on demand
 * For example set  mode gridfollow then load a bunch of variables from a file (or a DB) 
 * First we review some of the list uses in the current development model of the ess_controller
 * We get a list of pubs etc.
 * Here is the code used.
 * 
 * typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
 * typedef std::map<std::string, std::vector<std::string>*>vecmap;
 *  varsmap vmap;
    varsmap pmap;
 *  vecmap vecs;
 *  int ccnt;
 * 
 *  vm->getVList(vecs, vmap, ess_man->amap, "ess", "Blocks", ccnt);
 *  vm->getVList(vecs, vmap, ess_man->amap, "ess", "Pubs", ccnt);
 * These populate vecs with lists of strings based on the contents of /config/ess/Pubs  , /config/ess/Subs
 * here it the contents of vec after these operations.
 * main >>  >>>>>>>>>>>>>start of vecs
 *  * showvecMap key [Pubs] > entry [0] is [/status/ess]
 * showvecMap key [Pubs] > entry [1] is [/variables/ess]
 * showvecMap key [Pubs] > entry [2] is [/config/ess]
 * showvecMap key [Pubs] > entry [3] is [/params/ess]
 * showvecMap key [Pubs] > entry [4] is [/status/ess]
 * showvecMap key [Pubs] > entry [5] is [/variables/ess]
 * showvecMap key [Pubs] > entry [6] is [/config/ess]
 * showvecMap key [Pubs] > entry [7] is [/params/ess]
 * showvecMap key [Subs] > entry [0] is [/ess]
 * showvecMap key [Subs] > entry [1] is [/components]
 * showvecMap key [Subs] > entry [2] is [/assets]
 * showvecMap key [Subs] > entry [3] is [/params]
 * showvecMap key [Subs] > entry [4] is [/status]
 * showvecMap key [Subs] > entry [5] is [/controls]
 * showvecMap key [Subs] > entry [6] is [/variables]
 * showvecMap key [Subs] > entry [7] is [/config]
 * showvecMap key [Subs] > entry [8] is [/tests]
 * showvecMap key [Subs] > entry [9] is [/default]
 * showvecMap key [Subs] > entry [10] is [/reload]
 * showvecMap key [Subs] > entry [11] is [/misc2]
 * showvecMap key [Subs] > entry [12] is [/ess]
 * showvecMap key [Subs] > entry [13] is [/components]
 * showvecMap key [Subs] > entry [14] is [/assets]
 * showvecMap key [Subs] > entry [15] is [/params]
 * showvecMap key [Subs] > entry [16] is [/status]
 * showvecMap key [Subs] > entry [17] is [/controls]
 * showvecMap key [Subs] > entry [18] is [/variables]
 * showvecMap key [Subs] > entry [19] is [/config]
 * showvecMap key [Subs] > entry [20] is [/tests]
 * showvecMap key [Subs] > entry [21] is [/default]
 * showvecMap key [Subs] > entry [22] is [/reload]
 * showvecMap key [Subs] > entry [23] is [/misc2]
 * main >> <<<<<<<<<<<<<<end of vecs
 * 

 *  char **subs2 = vm->getList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
 * 
 * The entry in the config file is like this:
 * 
 * "/config/ess": {
 *   "Subs": "/ess /components, /assets, /params, /status, /controls, /variables, /config, /tests, /default, /reload, /misc2 ",
 *   "Pubs": "/status/ess, /variables/ess, /config/ess, /params/ess ",
 *   "BlockSets": "/status/ess, /status/bms, /status/bms_1, /status/bms_2 "
 * },
 * The vecmap will contain a list of strings associated with the key word (s) Subs, Pubs, BlockSets etc 
 * The varsmap will contain a list of named assets associated with an asset name 
 * you can examine the list as follows.
 * 
 * vm->showList(subs2,"ess", ccnt);
 * 
 * The Subs list is used when setting up the FIMS connection.
 * 
 * here is it working
 * getListStr >> recovered [Subs] as [/ess /components, /assets, /params, /status, /controls, /variables, /config, /tests, /default, /reload, /misc2 ]
 * showList subs [ess] [0] = [/ess]
 * showList subs [ess] [1] = [/components]
 * showList subs [ess] [2] = [/assets]
 * showList subs [ess] [3] = [/params]
 * showList subs [ess] [4] = [/status]
 * showList subs [ess] [5] = [/controls]
 * showList subs [ess] [6] = [/variables]
 * showList subs [ess] [7] = [/config]
 * showList subs [ess] [8] = [/tests]
 * showList subs [ess] [9] = [/default]
 * showList subs [ess] [10] = [/reload]
 * showList subs [ess] [11] = [/misc2]
 * 
 * 
 *   ess_man->run_fims(1500, (char **)subs2, "essMan", ccnt);
 *
 * The Pubs list is used when  
 *  ess_man->setVmap(&vmap);
 *  ess_man->setPmap(&pmap); // pubs map 
 * 
 * The key to getting the pubs working id copying the list of tables to pmap.
 * There should be a VarMapUtils function for this.
 * 
 * And here is the way the pmap gets sent as a pub
 * * the pmap just contains a list of comps to send out in the pub.
 * 
 *   am->vm->vListPartialSendFims(*am->vmap, *am->pmap, "pub", am->p_fims);
 * A pub on /assets/ will also find /assets/bms/bms_1  etc.

 * This still needs to be done.
 * 
 *  vLists
 *   These are a cool way to sent a block of sets or pubs to Fims from inside  func.
 * The selected vars  can be sent to their default locations (defined by the comp element) or to an optional  different location(and :var)
 * They are mini varsmaps and can be created and deleted. 
 *   
 * 
 * varsmap *vlist = am->vm->createVlist();
 * am->vm->addVlist(vlist, amap["lastActiveCurrentSetpoint"],"/components/pcr/pcr_1:lastSetpoint");
 * am->vm->addVlist(vlist, amap["pcs_ActiveCurrent"]                                             );
 * am->vm->addVlist(vlist, amap["lastReactiveCurrentSetpoint"]                                   );
 * am->vm->addVlist(vlist, amap["pcs_ReactiveCurrent"]                                           );
 * am->vm->sendVlist(p_fims, "set", vlist);
 * am->vm->clearVlist(vlist);
 * 
 * The last (for now)  list to consider is the assetList.
 * This was created to maintain the order of a config file so that it can be used for a consistent UI presentation.
 * it is saved in varsmap as "_assetList/uri"
 *     contains the order of assetVars for publish.
 * its a "normal" assetVar with a value (bool at the moment) with a special component  an assetlist type
 * 
 * typedef std::vector<assetVar*>  assetlist;
 *  
 * When parsing the config file, the assetList iscreated as a named collection of assetVars.
 * When wanting to publish the list we just simply run through the list and produce the cjson objects.
 * 
 *  assetList* alist = getAlist(vmap, newuri);
 *               if(alist)
 *               {
 *                   unsigned int ix = 0;
 *                   assetVar* av;
 *                   do
 *                   {
 *                       av = alist->avAt(ix++);  // returns nullptr when the list is done.
 *                       if(av) av->showvarCJ(cji, opts);
 *                   } while(av);
 *               }
 * currently any "get" operation that just includes a uri will in fact first look for an asset list to define 
 * the list of assetVars to be presented.
 * This is, in effect, a viable interface. 
 * 
 * So how do we make an assetList
 * 
 * There is a VarMapUtils function for this. The uri can be any string. but they need to be unique.
 * 
 *   assetList* setAlist(varsmap& vmap, const char* uri)  (vm.setAlist....)
 * 
 * We can recover an asset list, if one exists, like this. 
 * 
 *  assetList* alist = getAlist(vmap, uri);    (vm.getAlsit ....)
 * 
 * How do we add assets to the assetList
 * 
 * The asset list class has an add ( there is no delete yet)
 * 
 * void add(assetVar* av)
 *    {
 *       unsigned int ix = aList.size();
 *       for (unsigned int i = 0; i < ix; i++)
 *       {
 *           if (aList[i] == av) 
 *               return;
 *       }
 *       aList.push_back(av);
 *   }
 * 
 * So once you have tour list simply add assets to it. They will be placed in the cJSON file in order.
 * 
 *   alist->add(av);
 * 
 * A getMapsCj with the created uri   is all you need
 * 
 * cJSON* getMapsCj(varsmap &vmap , const char* uri = nullptr, const char* var = nullptr, int opts = 0)
 * 
 * 
 * The other use for asset lists is to collect assetVars suitable to drop into or read from a file or DB.
 * Ideally this will done in a config file but it could be in a Func.
 * This config file option is the best but we have to make it so that the assets are mapped correctly.
 * 
 * /asset/bms/bms_1/MaxVoltageLimit
 * Asset lists are "hidden" in the varsmap with a '_' prefix to the name
 * 
 *  "_/assets/ess/ess_1":   {
 *               "assetList":    {
 *                       "value":        true
 *               }
 *       }
 *
 * so how about this
 * 
 * "/config/bms/bms_1": {
 *    "/asset/bms/bms_1/MaxLimits": "/asset/bms/bms_1:MaxVoltageLimit, /asset/bms/bms_1:MaxCurrentLimit ..." 
 *   "AssetLists": "/asset/bms/bms_1/MaxLimits, /asset/bms/bms_1/DischargeLimits",
 *   "BlockSets": "/status/ess, /status/bms, /status/bms_1, /status/bms_2 "
 * },

 * We use the "AssetLists" keyword to get a list of the lists. then process the lists specified from that string
 * Lets simply put the string into the assetList object at this time.
 * The first time the assetList is used it will translate  the string into the actual assetVars.
 * This will allow late runtime binding after the config is all set up.
 * Any vars in the list not in the config wil have to be created. But what is the type for those. 
 * We'll  have an undefined type perhaps and then set the type on the first value set. 
 *
 * VarMapUtils will have functions to Load / Unload assetlists using files.
 * Infact load is just a simple config .. its all the same
 * 
 * vm->configure_vmap(vmap, "saved_configs/some_file");//, nullptr, ess_man);
 *
 * The file write will be also quite simple.
 *
        cJSON *cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap, <asset name>);
        const char* fname =  "saved_configs/asset_name/asset_instance";
        vm->write_cjson(fname, cjbm);
        cJSON_Delete(cjbm);
 *
 * We'll create a simple varMapUtils opeation for this 
 * 
 * vm.writeAsset(asset_name, asset_instance)  
 * This will be performed by a set on a regular assetVar with the asset_instance as the string.
 * set /asset/bms/bms_1/assetSave "<asset_list>/during_precharge" 
 * set /asset/bms/bms_1/assetLoad "<asset_list>/during_precharge" 
 * 
 *  Now for the code , thankfully its not too much.   
 */

#include "asset.h"
#include "varMapUtils.h"
#include "alarm.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"



int TestEssStart(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    int reload;
    double dval = 3000.0;
    //double dvalHB = 1.0;
    int ival = 0;
    VarMapUtils* vmp = av->am->vm;
    bool bval = false;


    //double dvalHBnow = vmp->get_time_dbl();
    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* TestEssStart      = amap[__func__];  
    if(reload < 2)
    {
        //reload = 0;
        amap["TestEssStart"]             = vmp->setLinkVal(vmap, aname, "/reload",    "TestEssStart",      reload);
        amap["EssStartOK"]               = vmp->setLinkVal(vmap, aname, "/status",    "EssStartOK",        bval);
        amap["EssStartLimit"]            = vmp->setLinkVal(vmap, aname, "/config",    "EssStartLimit",     dval);
        amap["EssStartCmd"]              = vmp->setLinkVal(vmap, aname, "/assets",    "start_stop",       dval);
        //amap["]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            ival = 0; amap["EssStartOK"]->setVal(bval);
        }
    }
    ival = 2; TestEssStart->setVal(ival);


    // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
    double StartCmd = amap["EssStartCmd"]->getdVal();
    double StartLimit = amap["EssStartLimit"]->getdVal();
    dval = 1.0;
    // dont use valueChanged it resets the change currently
    if(StartCmd > StartLimit)
    {
        bval = true;
        amap["EssStartOK"]->setVal(bval);

        // this stuff collects a bunch of assetVars and send them out to their default locations.
        // the link will determine where that location is.
        // if the link is defined in the config file then that destination will be maintained.

        varsmap *vlist = vmp->createVlist();
        vmp->addVlist(vlist, amap["TestEssStart"]);
        vmp->addVlist(vlist, amap["EssStartOK"]);
        vmp->addVlist(vlist, amap["EssStartLimit"]);
        //vmp->addVlist(vlist, amap["EssStartCmd"]);  // this would cause a tight loop
        vmp->sendVlist(p_fims, "set", vlist);
        vmp->clearVlist(vlist);
        
    }

    return 0;
}
//"\"func\":["
//                        "{ \"enable\":\"/assets/ess:run_ok\", \"amap\":\"ess\",\"func\":\"readConfigFile\","
//                        "\"configType\":\"PowerConfig\",\"onOk\":\"/status/ess:PowerConfigOK\",\"onErr\": \"/status/ess:PowerConfigErr\"}"
int readConfigFile(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    int reload;
    //double dval = 3000.0;
    //double dvalHB = 1.0;
    int ival = 0;
    VarMapUtils* vmp = av->am->vm;
    bool bval = false;
    char *tval = (char *)" InitialStatus";
    //double dvalHBnow = vmp->get_time_dbl();
    //reload =0;
    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* readConfigFile      = amap[__func__];  
    if(reload < 2)
    {
        //reload = 0;
        amap["readConfigOk"]             = vmp->setLinkVal(vmap, aname, "/status",    "readConfigOk",        bval);
        amap["readConfig"]               = vmp->setLinkVal(vmap, aname, "/controls",  "readConfig",          tval);
        amap["readConfigErr"]            = vmp->setLinkVal(vmap, aname, "/status",    "readConfigErr",       bval);
        amap["readConfigProfile"]        = vmp->setLinkVal(vmap, aname, "/status",    "readConfigProfile",   tval);
        //amap["]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            ival = 0; amap["readConfigProfile"]->setVal(tval);
        }
        ival = 2; readConfigFile->setVal(ival);
    }
    int ret = 0;
    if(av->aVal->valuestring && av->abf && av->am && av->am->vm)
    {

        amap["readConfigProfile"]->setVal(av->aVal->valuestring);
        // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
        // need to get stuff from the av->func featduct
        // configType , enable, onOK, onErr 
        char* enable      = av->abf->getFeat("enable",    &enable);
        char* configType  = av->abf->getFeat("configType", &configType);
        char* onOk        = av->abf->getFeat("onOk",      &onOk);
        char* onErr       = av->abf->getFeat("onOerr",    &onErr);

        char *aload = nullptr;
        //int ret = 0;
        VarMapUtils *vm = av->am->vm;

        asprintf(&aload,"saved_configs%s:%s/%s", av->comp.c_str(), configType,av->aVal->valuestring);
        if(aload)
        {
            if(1)FPS_ERROR_PRINT("%s >> attempting to read config file [%s]\n", __func__, aload);
            // now load 
            ret = vm->configure_vmap(vmap, aload);
            free((void*)aload);
        }
    }
    if(ret == 0)
    {
        bval = true;
        amap["readConfigOk"]->setVal(bval);
        bval = false;
        amap["readConfigErr"]->setVal(bval);
    }
    else
    {
        bval = true;
        amap["readConfigErr"]->setVal(bval);
        bval = false;
        amap["readConfigOk"]->setVal(bval);
    }

    return 0;
}
//"\"func\":["
//                        "{ \"enable\":\"/assets/ess:run_ok\", \"amap\":\"ess\",\"func\":\"saveConfigFile\","
//                        "\"configType\":\"PowerConfig\",\"onOk\":\"/status/ess:PowerConfigOK\",\"onErr\": \"/status/ess:PowerConfigErr\"}"
// cJSON *cjbm = nullptr;
//         cjbm = vm->getMapsCj(vmap, <asset name>);
//         const char* fname =  "saved_configs/asset_name/asset_instance";
//         vm->write_cjson(fname, cjbm);
//         cJSON_Delete(cjbm);

int saveConfigFile(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    int reload;
    //double dval = 3000.0;
    //double dvalHB = 1.0;
    int ival = 0;
    VarMapUtils* vmp = av->am->vm;
    bool bval = false;
    char *tval = (char *)" InitialStatus";
    //double dvalHBnow = vmp->get_time_dbl();
    //reload =0;
    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* saveConfigFile      = amap[__func__];  
    if(reload < 2)
    {
        //reload = 0;
        amap["saveConfigOk"]             = vmp->setLinkVal(vmap, aname, "/status",    "saveConfigOk",        bval);
        amap["saveConfig"]               = vmp->setLinkVal(vmap, aname, "/controls",  "saveConfig",          tval);
        amap["saveConfigErr"]            = vmp->setLinkVal(vmap, aname, "/status",    "saveConfigErr",       bval);
        amap["saveConfigProfile"]        = vmp->setLinkVal(vmap, aname, "/status",    "saveConfigProfile",   tval);
        //amap["]->setVal(2);  // revert reload
        if(reload == 0) // complete restart 
        {
            ival = 0; amap["saveConfigProfile"]->setVal(tval);
        }
        ival = 2; saveConfigFile->setVal(ival);
    }
    int ret = 0;
    if(av->aVal->valuestring && av->abf && av->am && av->am->vm)
    {

        amap["saveConfigProfile"]->setVal(av->aVal->valuestring);
        // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
        // need to get stuff from the av->func featduct
        // configType , enable, onOK, onErr 
        char* enable      = av->abf->getFeat("enable",    &enable);
        char* configType  = av->abf->getFeat("configType", &configType);
        char* onOk        = av->abf->getFeat("onOk",      &onOk);
        char* onErr       = av->abf->getFeat("onOerr",    &onErr);

        char *aload = nullptr;
        char *asname = nullptr;
        char *sname = nullptr;
        //int ret = 0;
        VarMapUtils *vm = av->am->vm;

// first need assetList

        asprintf(&sname,"mkdir -p saved_configs%s", configType);
        system(sname);
        asprintf(&asname,"saved_configs%s", configType);
        asprintf(&aload,"saved_configs%s/%s", configType, av->aVal->valuestring);
        if(aload)
        {
            cJSON *cjbm = nullptr;
            
            cjbm = vm->getMapsCj(vmap, asname);
            vm->write_cjson(aload, cjbm);
            cJSON_Delete(cjbm);
            if(1)FPS_ERROR_PRINT("%s >> attempting to save config file [%s]\n", __func__, aload);
            free((void*)aload);
            free((void*)asname);
            free((void*)sname);
        }
    }
    else
    {
        if(1)FPS_ERROR_PRINT("%s >> error with setup av->abf %p av->am %p av->am->vm %p\n"
            , __func__
            , av->abf
            , av->am
            , av->am?av->am->vm:0
            );
        ret = 1;   
    }
    
    if(ret == 0)
    {
        bval = true;
        amap["saveConfigOk"]->setVal(bval);
        bval = false;
        amap["saveConfigErr"]->setVal(bval);
    }
    else
    {
        bval = true;
        amap["saveConfigErr"]->setVal(bval);
        bval = false;
        amap["saveConfigOk"]->setVal(bval);
    }

    return 0;
}
     
// //  vm->setFunc(vmap, "dcr", "run_asset_wakeup" , (void*) &run_bms_asset_wakeup);


//     //void (*tf)(void *) = (void (*tf)(void *))
//     void *res1 = vm->getFunc(vmap, "ess","run_init" );
//     void *res2 = vm->getFunc(vmap, "ess","run_wakeup" );
   
//typedef void (*myAmInit_t)(asset_manager * data);

// here are the superfunctions to get / set assetlists
// int getAssetListVersion(varsmap &vmap, const char* alistname, const char* alistVersion)
// {
//     char* aload;
//     asprintf(&aload,"saved_configs/%s/%s", alistname,alistVersion);
//     // now load 
//     configure_vmap(vmap, aload );

// }

// sets up the internal structure for an assetList either from a config string or 
// a template file
// aname = "ess" 
// int setUpAssetList(varsmap &vmap, const char* aname, const char* alistname, const char* alistVersion, asset_manager* am)
// {
//     int ccnt = 0;
//     vecmap vecs;  // may not be used
//     char** assets  = vm.getList(vecs, vmap, am->amap, am->name.c_str(), alistname, ccnt);
//     // if ccnt == 0 then look for a template file

//     if(1)FPS_ERROR_PRINT(" %s >> seeking [%s] Found ccnt as %d\n", __func__, alistname, ccnt);
//     if(ccnt > 0)
//     {
//         vm.showList(assets,am->name.c_str(), ccnt);
//         assetList* alist = vm.setAlist(vmap, alistname);
//         //now we need to add the assets mentioned.  we make have to make the assets.
//         //
//         //one for each ccnt in assets
//         //
//         for (int iy = 0; iy < ccnt; iy++)
//         {
//             double dval = 0.0;  // this will set up the av a a double 
//             assetVar* av = vm.setVal(vmap, assets[iy], nullptr, dval);
//             alist->add(av);
//         }
//     }
//     return ccnt;
// }

typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
//     myAmInit_t myessMinit = myAmInit_t(res1);
//     myAmWake_t myessMwake = myAmWake_t(res2);
    
int main(int argc, char *argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;
    vm.setFunc(vmap, "ess", "TestEssStart" , (void*) &TestEssStart);
    vm.setFunc(vmap, "ess", "readConfigFile" , (void*) &readConfigFile);
    vm.setFunc(vmap, "ess", "saveConfigFile" , (void*) &saveConfigFile);


    asset_manager *ess_man = new asset_manager("ess");
    ess_man->am = nullptr;
    ess_man->vm = &vm;


    fims* p_fims = new fims;

    //p_fims->Connect((char *)"fimsEssTest");
    bool connectOK = p_fims->Connect((char *)"FimsEssTest");
    if (!connectOK)
    {
       FPS_ERROR_PRINT("%s >> unable to connect to fims server. All is lost.\n",__func__ ); 
       //running = 0;
    }
    ess_man->p_fims = p_fims;
    ess_man->running = 1;


    // Hack to set up the am in the av object not normally needed in a "normal" system config
    // cJSON* cj2 = cJSON_Parse("{\"value\":\"InitValcJ\"}");
    // assetVar* av = vm.setValfromCj(vmap,"/assets/ess","readConfig", cj2);
    // av->am = ess_man;
    // cJSON_Delete(cj2);

    int rc;
    const char *var1 = "{\"start_stop\":{\"value\":0,"
            "\"actions\":{"
                 "\"onSet\":{"
                    "\"func\":["
                        "{ \"enable\":\"/assets/ess:start_enable\", \"amap\":\"ess\",\"func\":\"TestEssStart\","
                        "\"onOK\":\"/asset/ess/controls/startOK\",\"onErr\": \"/asset/ess/controls/startErr\"}"
                    "]"
                "}"
            "}"
        "}"
    "}";

    const char* var2 = "{"
        "\"/config/ess:ChargeLimits\": \"/config/ess:MaxVoltageLimit, /config/ess:MaxCurrentLimit, /config/ess:MinCurrentLimit, /config/ess:MinVoltageLimit\"," 
        "\"/config/ess:DischargeLimits\": \"/config/ess:MaxVoltageLimit, /config/ess:MaxCurrentLimit, /config/ess:MinCurrentLimit, /config/ess:MinVoltageLimit\"," 
        "\"/config/ess:StartConfig\": \"/config/ess:MaxVoltageLimit, /config/ess:MaxCurrentLimit, /config/ess:MinCurrentLimit, /config/ess:MinVoltageLimit\"," 
        
        "\"AssetLists\": \"/config/ess:ChargeLimits, /config/ess:DischargeLimits, /config/ess:StartConfig\""
        "},";
        

    const char *var4 = "{\"readConfig\":{\"value\":\"GridFollow\","
            "\"actions\":{"
                 "\"onSet\":{"
                    "\"func\":["
                        "{ \"enable\":\"/assets/ess:run_ok\", \"amap\":\"ess\",\"func\":\"readConfigFile\","
                        "\"configType\":\"PowerConfig\",\"onOK\":\"/status/ess:PowerConfigOK\",\"onErr\": \"/status/ess:PowerConfigErr\"}"
                    "]"
                "}"
            "}"
        "}"
    "}";

    const char *var5 = "{\"saveConfig\":{\"value\":\"GridFollow\","
            "\"actions\":{"
                 "\"onSet\":{"
                    "\"func\":["
                        "{ \"enable\":\"/assets/ess:run_ok\", \"amap\":\"ess\",\"func\":\"saveConfigFile\","
                        "\"configType\":\"/config/ess:StartConfig\",\"onOK\":\"/status/ess:StartConfigOK\",\"onErr\": \"/status/ess:StartConfigErr\"}"
                    "]"
                "}"
            "}"
        "}"
    "}";

    const char* rep1 = "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"func\":"
                        "[{\"enable\":\"/assets/ess:start_enable\",\"amap\":\"ess\",\"func\":\"TestEssStart\","
                        "\"onOK\":\"/asset/ess/controls/startOK\",\"onErr\":\"/asset/ess/controls/startErr\"}]}}}}";
 

    const char* rep3 ="{\"/assets/ess\":{"
                        "\"readConfig\":{\"value\":\"GridFollow\",\"actions\":{\"onSet\":{\"func\":"
                        "[{"
                        "\"amap\":\"ess\","
                        "\"configType\":\"PowerConfig\","
                        "\"enable\":\"/assets/ess:run_ok\","
                        "\"func\":\"readConfigFile\","
                        "\"onErr\":\"/status/ess:PowerConfigErr\","
                        "\"onOK\":\"/status/ess:PowerConfigOK\""
                        "}]}}},"
                        "\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"func\":"
                        "[{\"amap\":\"ess\",\"enable\":\"/assets/ess:start_enable\",\"func\":\"TestEssStart\","
                        "\"onErr\":\"/asset/ess/controls/startErr\",\"onOK\":\"/asset/ess/controls/startOK\"}]}}}}}";

    const char* rep4 = "{\"readConfig\":{\"value\":\"GridFollow\",\"actions\":{\"onSet\":{\"func\":"
                        "[{\"enable\":\"/assets/ess:run_ok\",\"amap\":\"ess\",\"func\":\"readConfigFile\","
                        "\"configType\":\"PowerConfig\","
                        "\"onOK\":\"/status/ess:PowerConfigOK\",\"onErr\":\"/status/ess:PowerConfigErr\"}]}}}}";

    const char* rep5 = "{\"saveConfig\":{\"value\":\"GridFollow\",\"actions\":{\"onSet\":{\"func\":"
                        "[{\"enable\":\"/assets/ess:run_ok\",\"amap\":\"ess\",\"func\":\"saveConfigFile\","
                        "\"configType\":\"StartConfig\","
                        "\"onOK\":\"/status/ess:StartConfigOK\",\"onErr\":\"/status/ess:StartConfigErr\"}]}}}}";

    rc = vm.testRes(" Test 1", vmap
            ,"set"
            ,"/assets/ess"
            ,var1
            ,rep1
           , ess_man
           , nullptr
           );

    rc = vm.testRes(" Test 1.1", vmap
            ,"set"
            ,"/assets/ess"
            , var4
            , rep4
            , ess_man
            , nullptr
           );

    rc = vm.testRes(" Test 2", vmap
            , "get"
            , "/assets/ess"
            , nullptr
            , rep3
        );

    rc = vm.testRes(" Test 2.1", vmap
            ,"set"
            ,"/assets/ess"
            , var5
            , rep5
            , ess_man
            , nullptr
           );

// this is broken we think
    rc = vm.testRes(" Test 3.0", vmap
             ,"set"
             ,"/assets/ess:start_stop"
             ,"0"
             ,"{\"start_stop\":0}"
         );
    rc = vm.testRes(" Test 3.1", vmap
             ,"set"
             ,"/assets/ess"
             ,"{\"start_stop\":1000}"
             ,"{\"start_stop\":1000}"
         );
    rc = vm.testRes(" Test 3.2", vmap
             ,"set"
             ,"/assets/ess"
             ,"{\"start_stop\":2000}"
             ,"{\"start_stop\":2000}"
         );

    rc = vm.testRes(" Test 4", vmap
             ,"set"
             ,"/assets/ess"
             ,"{\"start_stop\":13000}"
             ,"{\"start_stop\":13000}"
         );

    rc = vm.testRes(" Test 5", vmap
             ,"set"
             ,"/assets/ess"
             ,"{\"readConfig\":\"gridFollow\"}"
             ,"{\"readConfig\":\"gridFollow\"}"
         );

    rc = vm.testRes(" Test 6", vmap
             ,"set"
             ,"/assets/ess"
             ,"{\"saveConfig\":\"StartUp\"}"
             ,"{\"saveConfig\":\"StartUp\"}"
         );

    // rc = testRes(" Test 5", vmap
    //          ,"get"
    //          ,"/system/remap_controls/rm_start_stop"
    //          , nullptr
    //          ,"{\"value\":129800}"
    //      );

    // rc = vm.testRes(" Test 6", vmap
    //          ,"get"
    //          ,"/system/enum_controls/onval31"
    //          , nullptr
    //          ,"{\"value\":3100}"
    //      );


    // rc = vm.testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );

    cJSON *cj = vm.getMapsCj(vmap);
    char* res = cJSON_Print(cj);
    rc = 0; // -Wall
    if(rc==0)    printf("\n\n#########vmap after tests \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);
    rc = vm.testRes(" Test 10", vmap
            ,"set"
            ,"/config/ess"
            ,var2
            ,nullptr
           , ess_man
           , nullptr
           );
    cj = vm.getMapsCj(vmap);
    res = cJSON_Print(cj);
    rc = 0; // -Wall
    if(rc==0)    printf("\n\n#########vmap after asset Load \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);
    int ccnt=0;
    vecmap vecs;
    char **assets2 = vm.getList(vecs, vmap, ess_man->amap, "ess", "AssetLists", ccnt);
    printf(" Found ccnt as %d\n", ccnt);
    vm.showList(assets2,"ess", ccnt);
    for (int ix = 0 ; ix< ccnt; ix++)
    {
        vm.setupAssetList(vmap, ess_man->name.c_str(), assets2[ix], nullptr, ess_man);

        cJSON *cjbm = nullptr;
        printf(" seeking assetList [%s]\n",assets2[ix]);

        // Done remove the asset name past the ":" in cjbm
        // get assetMaps perhaps
        cjbm = vm.getMapsCj(vmap, assets2[ix]);
        char* atmp;
        asprintf(&atmp,"mkdir -p saved_configs/%s", assets2[ix]);
        system( atmp );
        free((void*)atmp);
        // DONE take the ":" out of the asset name
        asprintf(&atmp,"saved_configs/%s/asset_test", assets2[ix]);
        vm.write_cjson(atmp, cjbm);
        free((void*)atmp);
 
        // asprintf(&atmp,"saved_configs/%s/asset_load", assets2[ix]);
        // vm.write_cjson(atmp, cjbm);
        // free((void*)atmp);
 
        cJSON_Delete(cjbm);
        printf(" Done seeking assetList [%s]\n",assets2[ix]);
         
        //vm.clearList(assets3,nullptr, ccnt3);

    }

    
    cj = vm.getMapsCj(vmap);
    res = cJSON_Print(cj);
    rc = 0; // -Wall
    if(rc==0)    printf("\n\n#########vmap after asset Lists \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);   



    // now load 
    // note you have to copy asset_test to sset_load or whatever the name is. 
    //  /assets/ess/ChargeLimits asset_load
    const char* aload = "saved_configs/config/ess:ChargeLimits/asset_load";
    vm.configure_vmap(vmap, aload );

    // delete bms_man;
    cj = vm.getMapsCj(vmap);
    res = cJSON_Print(cj);
    rc = 0; // -Wall
    if(rc==0)    printf("\n\n#########vmap after asset load \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);   


    vm.clearVmap(vmap);

    return 0;

}
