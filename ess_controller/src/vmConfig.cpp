// vmConfig.cpp
// parts used for new config system
// P. Wilshire  09/12/2021
// P. Wilshire  09/19/2021 
//      still need to clean up piVec
//notes find /path/backups/ -type f -mtime +2 -exec rsync -vPhd -e "ssh -p 512" {} --delete --ignore-existing me@host:/remote/path/server-backups/ \;

#include "asset.h"
#include "assetVar.h"
#include "varMapUtils.h"
#include "formatters.hpp"
#include <openssl/md5.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "autoLoadHeaders.h"

const char* FimsDir;
#include <openssl/evp.h>
#include <string>
#include <vector>

#if (__GNUC__ > 11) || (__GNUC__ == 11 && __GNUC_MINOR__ >= 0)

std::string bodymd5(const char* body, int len) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;
    std::string result;

    md = EVP_md5();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, body, len);
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    // Convert the hash to a hexadecimal string
    for (i = 0; i < md_len; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", md_value[i]);
        result.append(hex);
    }

    return result;
}
#else

std::string bodymd5 (const char* body, int len)
{
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)body, len, result);
    //char result[MD5_DIGEST_LENGTH];
    //MD5((void*)body, len, result);

    std::ostringstream sout;
    sout<<std::hex<<std::setfill('0');
    for(long long c: result)
    {
        sout<<std::setw(2)<<(long long)c;
    }
    return sout.str();
}

#endif 
// stdcfg map for referencing JSON strings loaded in autoLoadHeaders.h
const std::map<std::string, const char*> autoCfgMap = 
{
    {"std_bms_manager_controls", std_bms_manager_controls_s},
    {"std_bms_manager_faults", std_bms_manager_faults_s},
    {"std_bms_manager", std_bms_manager_s},
    {"std_v_bms_manager", std_v_bms_manager_s},
    {"std_v_bms_manager_controls", std_v_bms_manager_controls_s},
    {"std_v_bms_manager_faults", std_v_bms_manager_faults_s},
    {"std_bms_rack", std_bms_rack_s},
    
    {"ess_controller", std_ess_controller_s},
    {"std_ess_controller_controls", std_ess_controller_controls_s},
    {"std_ess_controller_faults", std_ess_controller_faults_s},
    {"std_ess_controller_manager", std_ess_controller_manager_s},
    {"std_site_reporter", std_site_reporter_s},
    
    {"std_pcs_manager", std_pcs_manager_s},
    {"std_pcs_manager_controls", std_pcs_manager_controls_s},
    {"std_pcs_manager_faults", std_pcs_manager_faults_s},
    {"std_v_pcs_manager", std_v_pcs_manager_s},
    {"std_v_pcs_manager_faults", std_v_pcs_manager_faults_s},
    {"std_v_pcs_manager_controls", std_v_pcs_manager_controls_s},
    {"std_pcs_module", std_pcs_module_s},
    {"std_pcs_module_faults", std_pcs_module_faults_s}
};
//fims_send -m set -u /dbi/ess_controller/configs_ess_controller -f /usr/local/etc/config/site_controller/assets.json
//fims_send -m set -u /dbi/ess_controller/configs_risen_bms_manager -f /usr/local/etc/config/site_controller/sequences.json
//fims_send -m set -u /dbi/ess_controller/configs_risen_bms_template -f /usr/local/etc/config/site_controller/variables.json
std::string dbifs = "/dbi/ess_controller/configs_";
//std::string dbifs = fmt::format("/xxdbi/FlexEss/configs/");
std::string dbiess = "";// = fmt::format("{}", getSysName(vmap));

#define MAX_CONFIG_REQUESTS 5
#define MAX_PIVEC 10000

// final this is here 
//scripts/FlexPack/test_config_load.sh
//scripts/FlexPack/test_config_load_response.sh

// bms loader test is here
//sh scripts/FlexPack/test_config_files.sh
//sh scripts/FlexPack/test_config_file_responses.sh


// first put out request for /xxdbi/ config/FlexEss/<flex>_controller.json

// add this file to the /config/cfile list 
//  fims_send -m set -r /$$ -u /flex/full/config/cfile '{"ess_controller.json":
//                           '{"value":false,"aname":essName}}'
// the replyto will set this value true

// /config/cfile   we want these files read 
// /config/tmpl
extern "C++" {

    //int SimBmsOpenContactor(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int StopSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
    int loadSiteMapAv(varsmap &vmap, VarMapUtils *vm,  assetVar* aV);
    int runConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
};
// this stuff will auto initialize to handl this type of configuration.
// we'll detect the presense of the /config/cfile:ess_config var.
// move to lib
// creates a schedule var (if needed) and runs a mapVar from the amap every x seconds 


// move to lib
// creates a schedule var (if needed) and runs a mapVar from the amap every x seconds 
void RunAv(varsmap& vmap, VarMapUtils*vm, varmap& amap, const char* aname, asset_manager* am
                    , fims* p_fims
                    , const char* schedVar/*"/schedule/bms:bms_rack"*/
                    , const char* mapVar/*"runRackCloseContactor"*/
                    , double tNow, double every /*0.1*/, double offset)
{
    if(0)FPS_PRINT_INFO(" running schedvar [{}] mapVal [{}]->[{}]\n"
        , schedVar
        , mapVar
        , amap[mapVar]->getfName()
        );

    assetVar* avs = vm->getVar(vmap, schedVar, nullptr);
    if(!avs)
    {   
        double dval = 0.0;
        avs = vm->makeVar(vmap, schedVar, nullptr, dval);
    }
    avs->am = am;
    avs->setParam("uri",amap[mapVar]->getfName());
    avs->setParam("every",every);
    avs->setParam("offset",offset);  // priority  
    RunSched(vmap, amap, aname, p_fims, avs);
    if(0)FPS_PRINT_INFO(" after running schedvar [{}] mapVal [{}]->[{}]\n"
        , schedVar
        , mapVar
        , amap[mapVar]->getfName()
        );
}

// creates a schedule var (if needed) and stops a mapVar from the amap in x seconds 
void StopAv(varsmap& vmap, VarMapUtils*vm, varmap& amap, const char* aname, asset_manager *am, fims * p_fims
                    , const char* schedVar/*"/schedule/bms:bms_rack"*/
                    , const char* mapVar/*"runRackCloseContactor"*/
                    , double tNow, double in /*0.1*/)
{
    assetVar* avs = vm->getVar(vmap, schedVar, nullptr);
    if(!avs)
    {
        FPS_PRINT_INFO(" unable to get schedVar [{}] \n"
        , schedVar
        );
        return;
    }
    avs->setParam("uri", amap[mapVar]->getfName());
    avs->setParam("in", in);
    avs->setParam("every", 0);
    avs->am = am;
    if(0)FPS_PRINT_INFO(" >>>> stopping uri [{}] schedVar [{}] \n"
        , amap[mapVar]->getfName()
        , schedVar
        );
    StopSched(vmap, amap, aname, p_fims, avs);
    if(0)FPS_PRINT_INFO(" >>>> stopped uri [{}] schedVar [{}] \n"
        , amap[mapVar]->getfName()
        , schedVar
        );
}
void removeWhitespaceOutsideQuotes(std::string& str) {
    bool insideQuotes = false;
    std::string result;

    for (char c : str) {
        if (c == '"') {
            insideQuotes = !insideQuotes;
            result += c;
        } else if (!insideQuotes && std::isspace(c)) {
            continue; // Skip whitespace outside quotes
        } else {
            result += c;
        }
    }

    str = result; // Update the original string
}
// this looks for conifg files /config/cfiles and templates /config/tmpl  for items yet to be loaded and issues reload requests unti 
// they arrive
// it also checks the  /config/load system for complete collections of files to load.
// cfg/cfile/ess/ess_controller.json     will cause a config file to be loaded immediately
// this can cause other /config/cfiles and config/tmpl  requests to be registered.
// /config/load will also specify a  list of cfiles and templates and cause the template evalustion system to run when 
// all files are loaded.
// the tmpl files are loaded into a extras->tbody string  so they can be recovered and template substitution performed.
// one of the design goals was to make the substutution progressive to allow a bms to have racks , modules, and cells 
// the list of keys (replace/with) is progressive rack_01/module_01/cell_01   etc.
// an evaluated cell template will be repeated a number of times each with a different aname ("rack_xx_module_yy_cell_zz")
// using all the prior keys.
// fims_send -m set -r /$$ -u /flex/full/config/load '
// {
//      "name":"config for bms",
//      "bms_manager.json": {
//         "value":false,
//         "aname":"bms",
//         "options":[
//            { "tmpl":"bms_rack_tmpl.json", "from": 1, "to": 5, "type":"am", "aname":"rack_{:02d}",
//                               "reps":[
//                                {"replace":"##RACK_ID##" ,"with":"rack_{:02d}"},
//                                {"replace":"##RACK_NUM##" ,"with":"{:02d}"}
//                                ]
//           },
//           { "tmpl":"bms_module_tmpl.json","from": 1, "to": 38, "type":"am", "aname":"module_{:02d}",
//                               "reps":[
//                                {"replace":"##MODULE_ID##" ,"with":"module_{:02d}"},
//                                {"replace":"##MOSULE_NUM##","with":"{:02d}"}
//                                ]
//           },
//            { "tmpl":"bms_cell_tmpl.json","from": 1, "to": 10, "type":"ai""aname":"cell_{:02d}",
//                               "reps":[
//                                {"replace":"##CELL_ID##" ,"with":"cell_{:02d}"},
//                                {"replace":"##CELL_NUM##" ,"with":"{:02d}"}
//                                ]
//           }
//        ]
//      }
// }'  | jq
// fims_send -m set -r /$$ -u /flex/full/config/cfile '
// {
//      "name":"config for bms",
//      "bms_manager.json": {
//           "value":false
//           }
// }'
// fims_send -m set -r /$$ -u /flex/full/config/tmpl '
// {
//      "name":"config templates for bms",
//      "bms_rack_tmpl.json": {
//           "value":false
//           },
//      "bms_module_tmpl.json": {
//           "value":false
//           },
//      "bms_cell_tmpl.json": {
//           "value":false
//           },
// }'
//  auto fr = fmt::format("/config/cfile:{}",xfile);
//                         // first look for the av , it may have already been loaded.
//                         assetVar*avf = vm->getVar(vmap,fr.c_str(),nullptr);
//                         // if no av set up the default
//                         if (!avf)
//                         {
//                             bval =  false;
//                             avf = vm->setVal(vmap,fr.c_str(),nullptr,bval);
//                             if(aname)  avf->setcParam("aname",aname);
//                             if(pname)  avf->setcParam("pname",pname);
//                             if(ainame) avf->setcParam("ainame",ainame);
//
// use this for all files that are not templates
// if we need a file just check for it and the request will be made
// they get loaded  into /config/cfiles:<file name> , they need an asset (ainame) or an asset manager (amname) 
// who may need a parent (pname)                       }
bool checkcFile(varsmap &vmap, VarMapUtils* vm, cJSON* cjsi, const char* cfile)
{
    bool ret = false;
    auto lstr = fmt::format("/config/cfile:{}", cfile);
    assetVar* av = vm->getVar(vmap, lstr.c_str(), nullptr);
    if(av)
        ret = av->getbVal();
    else
    {
        ret =  false;
        av = vm->setVal(vmap,lstr.c_str(), nullptr, ret);
        const char* aname = nullptr;
        const char* amname = nullptr;
        const char* ainame = nullptr;
        const char* pname = nullptr;
        bool useConfigDoc = true;
        if (cjsi)
        {
            cJSON* cjaname   = cJSON_GetObjectItem(cjsi, "aname");
            cJSON* cjam      = cJSON_GetObjectItem(cjsi, "amname");
            cJSON* cjai      = cJSON_GetObjectItem(cjsi, "ainame");
            cJSON* cjap      = cJSON_GetObjectItem(cjsi, "pname");
            cJSON* cjbase    = cJSON_GetObjectItem(cjsi, "useConfigDoc");

            if(cjaname)aname = cjaname->valuestring;
            if(cjam)amname   = cjam->valuestring;  // this will be an encoded string 
            if(cjai)ainame   = cjai->valuestring;
            if(cjap)pname    = cjap->valuestring;
            if(cjbase)useConfigDoc = cJSON_IsTrue(cjbase);
        }
        if(av)
        {
            if(aname)  av->setcParam("aname", aname);
            if(amname) av->setcParam("amname", amname);
            if(pname)  av->setcParam("pname",  pname);
            if(ainame) av->setcParam("ainame", ainame);
            if(!useConfigDoc) av->setParam("useConfigDoc", useConfigDoc);
        }

    }

    if(!ret) if(0) FPS_PRINT_INFO(" seeking [{}]", cfile);
    return ret;
}
// use this for all files that are not templates
// they get loaded  into /config/tmpl:<file name> , then need an asset manager who may need a parent                       }

bool checkTmpl(varsmap &vmap,VarMapUtils* vm, cJSON* cjsi, const char* tmpl)
{
    bool ret = false;
    auto lstr = fmt::format("/config/ctmpl:{}", tmpl);
    assetVar* av = vm->getVar(vmap, lstr.c_str(), nullptr);
    if(av)
    {
        ret = av->getbVal();
    }
    else
    {
        ret =  false;
        av = vm->setVal(vmap, lstr.c_str(), nullptr, ret);
        const char* amname = nullptr;
        const char* ainame = nullptr;
        const char* pname = nullptr;
        if (cjsi)
        {
            cJSON* cjam      = cJSON_GetObjectItem(cjsi, "amname");
            cJSON* cjai      = cJSON_GetObjectItem(cjsi, "ainame");
            cJSON* cjap      = cJSON_GetObjectItem(cjsi, "pname");

            if(cjam)amname   = cjam->valuestring;  // this will be an encoded string 
            if(cjai)ainame   = cjai->valuestring;
            if(cjap)pname    = cjap->valuestring;
        }
        if(av)
        {
            if(amname) av->setcParam("amname", amname);
            if(pname)  av->setcParam("pname",  pname);
            if(ainame) av->setcParam("ainame", ainame);
        }
    }
    if(!ret) if(0)FPS_PRINT_INFO(" seeking [{}]", tmpl);
    return ret;
}


// is a site a
bool checkSite(varsmap &vmap,VarMapUtils* vm, cJSON* cjsi, const char* tmpl)
{
    bool ret = false;
    auto lstr = fmt::format("/config/ctmpl:{}", tmpl);
    if(1)FPS_PRINT_INFO(" seeking [{}]", lstr);

    assetVar* av = vm->getVar(vmap, lstr.c_str(),nullptr);
    if(av)
    {
        ret = av->getbVal();
    }
    else
    {
        if(0)FPS_PRINT_INFO(" creating [{}]", lstr);
        // create the template record
        ret =  false;
        av = vm->setVal(vmap, lstr.c_str(), nullptr, ret);
        const char* amname = nullptr;
        const char* ainame = nullptr;
        const char* pname = nullptr;
        if (cjsi)
        {
            cJSON* cjam      = cJSON_GetObjectItem(cjsi, "amname");
            cJSON* cjai      = cJSON_GetObjectItem(cjsi, "ainame");
            cJSON* cjap      = cJSON_GetObjectItem(cjsi, "pname");

            if(cjam)amname   = cjam->valuestring;  // this will be an encoded string 
            if(cjai)ainame   = cjai->valuestring;
            if(cjap)pname    = cjap->valuestring;
        }
        if(av)
        {
            if(amname) av->setcParam("amname", amname);
            if(pname)  av->setcParam("pname",  pname);
            if(ainame) av->setcParam("ainame", ainame);
        }
    }
    if(!ret) if(0)FPS_PRINT_INFO(" seeking [{}]", tmpl);
    return ret;
}

bool checkLoad(varsmap &vmap, VarMapUtils* vm, cJSON* cjsi, const char* tmpl)
{
    bool ret = false;
    auto lstr = fmt::format("/config/load:{}", tmpl);
    assetVar* av = vm->getVar(vmap, lstr.c_str(), nullptr);
    if(av)
    {
         ret = av->getbVal();
    }
    else
    {

        ret =  false;
        av = vm->setVal(vmap, lstr.c_str(), nullptr, ret);
        if(0)FPS_PRINT_INFO(" creating [{}] av[{}] --> [{}]"
            , lstr
            , fmt::ptr(av)
            , av?av->getfName():" av create failed"
            );

        const char* aname = nullptr;
        const char* amname = nullptr;
        const char* ainame = nullptr;
        const char* pname = nullptr;
        av->setParam("file", tmpl);
        if (cjsi)
        {
            cJSON* cjaname   = cJSON_GetObjectItem(cjsi, "aname");
            cJSON* cjam      = cJSON_GetObjectItem(cjsi, "amname");
            cJSON* cjai      = cJSON_GetObjectItem(cjsi, "ainame");
            cJSON* cjap      = cJSON_GetObjectItem(cjsi, "pname");

            if(cjaname)aname = cjaname->valuestring;
            if(cjam)amname   = cjam->valuestring;  // this will be an encoded string 
            if(cjai)ainame   = cjai->valuestring;
            if(cjap)pname    = cjap->valuestring;
        }
        if(av)
        {
            if(aname)  av->setcParam("aname", aname);
            if(amname) av->setcParam("amname", amname);
            if(pname)  av->setcParam("pname",  pname);
            if(ainame) av->setcParam("ainame", ainame);
        }
    }

    if(!ret) if(0)FPS_PRINT_INFO(" seeking [{}]", tmpl);
    return ret;
}

// // check for the loader final file (scheduler)
// bool checkFinal(varsmap &vmap,VarMapUtils* vm, const char* tmpl)
// {
//     bool ret = false;
//     //assetVar* av = vm->getVar(vmap, "/config/final", tmpl);
//     auto lstr = fmt::format("/config/cfile:{}", tmpl);
//     assetVar* av = vm->getVar(vmap, lstr.c_str(), nullptr);
//     if(av) ret = av->getbVal();
//     return ret;
// }
int requestConfig(varsmap &vmap, VarMapUtils*vm, const char* aname, const char* fname)
{
    if(0)FPS_PRINT_INFO("   aname [{}] fname [{}]"
                , aname ? aname :" No aname"
                , fname ? fname :" No fname"
                );
    //auto dbifs = fmt::format("/xxdbi/ess_controller/configs/");
    //auto dbiess = fmt::format("{}", vm->getSysName(vmap));
    if(fname && aname)
    {
        bool bval =  false;
        assetVar *av = vm->setVal(vmap,"/config/cfile", fname, bval); 
        av->setcParam("file", fname);
        av->setcParam("aname", aname);
        //fname += strlen("cfile:");
        auto fs = fmt::format("{}{}/{}", dbifs, aname, fname);
        auto fr = fmt::format("/{}/cfg/cfile/{}/{}", vm->getSysName(vmap), aname, fname);
        if(0)FPS_PRINT_INFO(" fims send [{}] reply [{}] cfile [{}]"
                , fs
                , fr
                , fname
                );
        if(!vm->simdbi)
        {
            // if (vm->p_fims)
            //     vm->p_fims->Send("get", fs.c_str(), fr.c_str(), nullptr);
        }
        else
        {
            auto xfile = fmt::format("{}.json", fname);
            auto xname = vm->getFileName(xfile.c_str());
            if(!xname)
            {
                if(1)FPS_PRINT_INFO(" >>>>>> no xname for [{}]", fname);
            }
            auto cms = fmt::format(
                    "{}fims_send -f {}.json -m set  -u {} &"
                    , FimsDir ,xname? xname : fname, fr);
            //system(cms.c_str());
            if(0)FPS_PRINT_INFO(" >>>>>> simdbi {}", cms);

        }
    }
    return 0;
}

int requestCfile(varsmap &vmap, VarMapUtils*vm, assetVar* av)
{
    //auto dbifs = fmt::format("/xxdbi/ess_controller/configs/");
    //auto dbiess = fmt::format("{}", vm->getSysName(vmap));
    if(!av) return 0;
    char* fname = av->getcParam("file");

    // If the file parameter does not exist, we'll use the name of the assetVar as part of the
    // request for the cfile
    if (!fname)
        fname = (char*)av->name.c_str();

    char* aname = av->getcParam("aname");
    if (!aname)
    {
        FPS_PRINT_DEBUG("av [{}] does not have aname. Using ess as default aname", av->getfName());
        aname = vm->getSysName(vmap);
    }

    bool useConfigDoc = av->getbParam("useConfigDoc");

    if(0)FPS_PRINT_INFO(" fims send fname [{}] simdbi [{}] aname [{}] useConfigDoc [{}]", cstr{fname}, vm->simdbi, cstr{aname}, useConfigDoc);
    if(fname && aname)
    {
        //fname += strlen("cfile:");
        auto fs = fmt::format("{}{}", useConfigDoc ? dbifs : "/dbi/ess_controller/", fname);
        auto fr = fmt::format("/{}/cfg/cfile/{}/{}",vm->getSysName(vmap), aname, fname);
        if(0)FPS_PRINT_INFO(" fims send [{}] reply [{}] cfile [{}]"
                , fs.c_str()
                , fr.c_str()
                , fname
                );

        if (vm->autoLoad && strstr(fname, "std_")) {
            int single = 0;
            auto uri = fmt::format("/cfg/cfile/{}/{}", aname, fname);
            std::string body;

            auto it = autoCfgMap.find(fname);
            if (it != autoCfgMap.end())
            {
                body = it->second;
            }
            else
            {
                FPS_ERROR_PRINT("[%s] autoCfg file not found [%s]\n", __func__, fname);
            }
            FPS_PRINT_INFO("AutoLoading {}", fname);
            removeWhitespaceOutsideQuotes(body);
            vm->handleCfile(vmap, nullptr, "set", uri.c_str(), single, body.c_str(), nullptr, nullptr, nullptr);
        }
        else if(!vm->simdbi)
        {
            FPS_PRINT_INFO("Getting from DBI {}", fname);
            if (vm->p_fims)
            {
                vm->p_fims->Send("get",fs.c_str(),fr.c_str(),nullptr);
            }
        }
        else
        {
            auto xfile = fmt::format("{}.json", fname);
            auto xname = vm->getFileName(xfile.c_str());
            if(0)FPS_PRINT_INFO(" >>>>>> xname {}", fmt::ptr(xname));

            auto cms = fmt::format(
                "{}fims_send -f {}.json -m set  -u {} &"
                , FimsDir, xname?xname:fname, fr);
            //system(cms.c_str());
            if(0)FPS_PRINT_INFO(" >>>>>> simdbi {}", cms);

        } 

    }
    return 0;
}

// request final file 
// it will come back as a cfile

int requestFfile(varsmap &vmap, VarMapUtils*vm, const char*final)
{
    //auto dbifs = fmt::format("/xxdbi/ess_controller/configs/");
    //auto dbiess = fmt::format("{}", vm->getSysName(vmap));
    //bool bval = false;
    // set this as false 
    // it will come back as a cfile so /configs/cfile/<final> will be set true when it arrives 
    //assetVar* av = vm->setVal(vmap,"/configs/final",final, bval);
    //char* fname = av->getcParam("file");
    char* aname = vm->getSysName(vmap);
    if(final && aname)
    {
        auto fs = fmt::format("{}{}", dbifs, final);
        auto fr = fmt::format("/{}/cfg/cfile/{}/{}",aname, aname, final);
        if(1)FPS_PRINT_INFO(" fims send [{}] reply [{}] cfile [{}]"
                , fs.c_str()
                , fr.c_str()
                , final
                );
        if(!vm->simdbi)
        {
            if (vm->p_fims)
                vm->p_fims->Send("get",fs.c_str(),fr.c_str(),nullptr);
        }
        else
        {
            auto xfile = fmt::format("{}.json", final);
            auto xname = vm->getFileName(xfile.c_str());
            if(0)FPS_PRINT_INFO(" >>>>>> xname {}", fmt::ptr(xname));
            auto cms = fmt::format(
                "{}fims_send -f {}.json -m set  -u {} &"
                , FimsDir
                , xname?xname:final, fr);
            //system(cms.c_str());
            if(0)FPS_PRINT_INFO(" >>>>>> simdbi {}", cms);
        } 

    }
    return 0;
}

int requestTmpl(varsmap &vmap, VarMapUtils*vm, assetVar* av)
{
    //auto dbifs = fmt::format("/xxdbi/ess_controller/configs/");
    //auto dbiess = fmt::format("{}", vm->getSysName(vmap));
    char* fname = av->getcParam("file");
    if (!fname)
        fname = (char*)av->name.c_str();

    char* aname = av->getcParam("aname");
    if (!aname)
        aname = vm->getSysName(vmap);

    if(0)FPS_PRINT_INFO(" fims send fname [{}] simdbi [{}] aname [{}]", cstr{fname}, vm->simdbi, cstr{aname});
    if(fname && aname)
    {
        auto fr = fmt::format("/{}/cfg/ctmpl/{}/{}",aname, aname, fname);
        if(0)FPS_PRINT_INFO(" fims reply [{}] cfile [{}] dbifs [{}]"
                , fr.c_str()
                , fname
                , dbifs
                );
        auto fs = fmt::format("{}{}", dbifs, fname);
        if(0)FPS_PRINT_INFO(" fims send 1 [{}] cfile [{}]"
                , fs
                , fname
                );
        if (vm->autoLoad && strstr(fname, "std_")){
            int single = 0;
            auto uri = fmt::format("/cfg/ctmpl/{}/{}", aname, fname);
            std::string body;
            
            auto it = autoCfgMap.find(fname);
            if (it != autoCfgMap.end())
            {
                body = it->second;
            }
            else
            {
                FPS_ERROR_PRINT("[%s] autoCfg file not found [%s]\n", __func__, fname);
            }
            FPS_PRINT_INFO("AutoLoading {}", fname);
            removeWhitespaceOutsideQuotes(body);
            vm->handleCfile(vmap, nullptr, "set", uri.c_str(), single, body.c_str(), nullptr, nullptr, nullptr);
        }
        else if(!vm->simdbi)
        {
            if(0)FPS_PRINT_INFO(" fims send 1a [{}] cfile [{}]"
                        , fs
                        , fname
                    );
            if (vm->p_fims)
                vm->p_fims->Send("get",fs.c_str(),fr.c_str(),nullptr);
        }
        else
        {
            if(1)FPS_PRINT_INFO(" fims send 1b fname [{}]"
                , fname
                );
            auto xname = vm->getFileName(fname);
            if(0)FPS_PRINT_ERROR(" getFileName possibly  no xname [{}]"
                    , xname?xname:"no xname"
                );
            if(xname)
            {
                auto cms = fmt::format(
                    "{}fims_send 2 -f {}.json -m set  -u /{}{} &"
                    , FimsDir
                    , xname
                    , aname
                    , fr);
                    //system(cms.c_str());
                if(1)FPS_PRINT_INFO(" >>>>>> simdbi {}", cms);
            }
            else
            {
                if(1)FPS_PRINT_ERROR(" getFileName possibly  no xname [{}]"
                    , xname?xname:"no xname"
                );
            }
        } 
    }
    if(0)FPS_PRINT_INFO(" >>>>>> done");
    return 0;
}

class parseItem {

public:
    parseItem( cJSON* cj)
    {
        idx = 1;
        from = 1;
        to = 4;
        step = 1;
        mult = 1;
        add = 0;
        cji = cj;
        ai = nullptr;
        am = nullptr;
        pam = nullptr;

    };
    ~parseItem(){};
    int Init(varsmap* _vmap, VarMapUtils*_vm);
    int eValIdx(int idx);
    int setBody();
    int applyRepMap( std::map<std::string, std::string> *rmap);

    cJSON* cji;
    int idx;
    int nextIdx;
    int from;
    int to;
    int step;
    int mult;
    int add;
    std::map<std::string, std::string> repmap;

    assetVar* av;
    std::string bstr;
    std::string amstr;
    std::string aistr;
    std::string pstr;

    varsmap* vmap;
    VarMapUtils* vm;
    asset_manager* am;
    asset_manager* pam;
    asset* ai;
    char* amname;
    char* ainame;
    char* pname;

};

int parseItem::Init(varsmap * _vmap, VarMapUtils*_vm)
{
    from = 1;
    to = 4;
    step = 1;
    mult = 1;
    add = 0;
    idx  = 0;
    nextIdx = 0;
    vm = _vm;
    vmap = _vmap;
    av =  nullptr;
    cJSON* cj = cJSON_GetObjectItem(cji, "tmpl");
    if(cj && cj->valuestring)
    {
        av =  vm->getVar(*vmap, "/config/ctmpl", cj->valuestring);
    }
    //cJSON *cjfile = cJSON_GetObjectItem(cji, "file");
    cJSON *cjfrom    = cJSON_GetObjectItem(cji, "from");
    cJSON *cjto      = cJSON_GetObjectItem(cji, "to");
    cJSON *cjstep    = cJSON_GetObjectItem(cji, "step");
    cJSON *cjmult    = cJSON_GetObjectItem(cji, "mult");
    cJSON *cjadd     = cJSON_GetObjectItem(cji, "add");
    if(cjfrom) from  = cjfrom->valueint;
    if(cjto)   to    = cjto->valueint;
    if(cjstep) step  = cjstep->valueint;
    if(cjmult) mult  = cjmult->valueint;
    if(cjadd)  add   = cjadd->valueint;
    cJSON *cjrange   = cJSON_GetObjectItem(cji, "rangevar");
    if(cjrange) {
        auto rangeav =  vm->getVar(*vmap, "/config/range", cjrange->valuestring);
        if(1)FPS_PRINT_INFO(" range found [{}]", cjrange->valuestring);
        if (rangeav) {
            if(1)FPS_PRINT_INFO(" rangevar found [{}]", rangeav->getfName());
            bool bval = true;
            if (rangeav->gotParam("from")) from = rangeav->getiParam("from");
            if (rangeav->gotParam("to")) to = rangeav->getiParam("to");
            if (rangeav->gotParam("step")) step = rangeav->getiParam("step");
            if (rangeav->gotParam("mult")) mult = rangeav->getiParam("mult");
            if (rangeav->gotParam("add")) add = rangeav->getiParam("add");
            rangeav->setVal(bval);
        }
    }
    cJSON* cjam      = cJSON_GetObjectItem(cji, "amname");
    cJSON* cjai      = cJSON_GetObjectItem(cji, "ainame");
    cJSON* cjap      = cJSON_GetObjectItem(cji, "pname");
    amname = nullptr;
    ainame = nullptr;
    pname = nullptr;
    if(cjam)amname   = cjam->valuestring;  // this will be an encoded string 
    if(cjai)ainame   = cjai->valuestring;
    if(cjap)pname    = cjap->valuestring;
    if(cjap) if(0)FPS_PRINT_INFO(" pname found [{}]", pname);
    if(cjai) if(0)FPS_PRINT_INFO(" ainame found [{}]", ainame);
 
    return 0;
}

// for a given idx evaluate the reps
int parseItem::eValIdx(int _idx)
{

    cJSON *cjra = cJSON_GetObjectItem(cji, "reps");
    cJSON *cjri;
    cJSON_ArrayForEach(cjri,cjra)
    {
        cJSON *cjrep     = cJSON_GetObjectItem(cjri, "replace");
        cJSON *cjwith    = cJSON_GetObjectItem(cjri, "with");
        cJSON *cjwithids = cJSON_GetObjectItem(cjri, "withIds"); 
        cJSON *cjmult    = cJSON_GetObjectItem(cjri, "mult");
        cJSON *cjadd     = cJSON_GetObjectItem(cjri, "add");
        int rmult = mult;
        int radd = add;
        if(cjmult) rmult  = cjmult->valueint;
        if(cjadd)  radd   = cjadd->valueint;


        //if(0)FPS_PRINT_INFO(" loaded rep [{}] with [{}]",cjrep->valuestring,cjwith->valuestring);

        // If we are given cjwithids (withIds), which contains [bms_a, bms_b], for example, then we'll replace the placeholder
        // defined in cjrep (replace) with each entry in cjwithids
        if (cjwithids && cJSON_IsArray(cjwithids))
        {
            cJSON *entry = cJSON_GetArrayItem(cjwithids, _idx);
            if (entry)
            {
                const std::string vstr(entry->valuestring);
                if(0)FPS_PRINT_INFO(" loaded rep [{}] with [{}] => [{}]  idx [{}]"
                    , cjrep->valuestring
                    , entry->valuestring
                    , vstr
                    , _idx);

                repmap[cjrep->valuestring]=vstr;
            }
        }

        // Otherwise, replace the placeholder defined in cjrep (replace) with the value defined in cjwith (with)
        else
        {
            auto vstr = fmt::format(cjwith->valuestring,(_idx*rmult)+radd);
            if(0)FPS_PRINT_INFO(" loaded rep [{}] with [{}] => [{}]"
                , cjrep->valuestring
                , cjwith->valuestring
                , vstr);

            repmap[cjrep->valuestring]=vstr;
        }
    }
    return _idx;
}

// for a given idx evaluate the reps
int parseItem::setBody()
{
    cJSON* cjtmpl =  cJSON_GetObjectItem(cji, "tmpl");
    
    if(0)FPS_PRINT_INFO(" av  [{}] cjtmpl [{}]->[{}]  body param av [{}] param [{}]"
                , fmt::ptr(av) 
                , fmt::ptr(cjtmpl)
                , cjtmpl->valuestring
                , fmt::ptr(av)
                , av?av->gotParam("md5sum"):false
                );
                 
    if(av && av->gotParam("md5sum"))
    { 
        //bstr = av->getcParam("body");
        bstr = av->extras->tbody;
    }
    else
    {
        if(0)FPS_PRINT_INFO(" av  [{}]->[{}] has no body param"
            , fmt::ptr(av) 
            , av->getfName()
            );
    }
    // capture encoded am and ai vars
    amstr = "";
    aistr = "";
    pstr = "";

    if(amname) amstr = amname;
    if(ainame) aistr = ainame;
    if(pname)  pstr  = pname;
    return 0;
}

// for a given idx evaluate the reps
int parseItem::applyRepMap(std::map<std::string, std::string> *rmap)
{
    if(0)FPS_PRINT_INFO("ainame fixup ainame [{}] *rmap->size [{}] "
            , fmt::ptr(ainame)
            , rmap->size()
            );
    for (auto &xx : *rmap)
    {
        auto &fstr = xx.first;
        auto &tstr = xx.second;
        
        std::size_t start_pos = 0;
        //if(0)FPS_PRINT_INFO("ainame fixup ainame [{}]", fmt::ptr(ainame));

        while ((start_pos = bstr.find(fstr, start_pos)) != std::string::npos)
        {
            bstr.replace(start_pos, fstr.length(), tstr);
            start_pos += tstr.length(); // ...
        }
        if(amname)
        {
            start_pos = 0;
            while ((start_pos = amstr.find(fstr, start_pos)) != std::string::npos)
            {
                amstr.replace(start_pos, fstr.length(), tstr);
                start_pos += tstr.length(); // ...
            }
        }
        if(ainame)
        {
            start_pos = 0;
            if(0)FPS_PRINT_INFO("ainame [{}] fixup #1 [{}]", ainame, aistr);
            while ((start_pos = aistr.find(fstr, start_pos)) != std::string::npos)
            {
                aistr.replace(start_pos, fstr.length(), tstr);
                start_pos += tstr.length(); // ...
            }
            if(0)FPS_PRINT_INFO("ainame fixup #2 [{}]", aistr);
        }
        if(pname)
        {
            start_pos = 0;
            if(0)FPS_PRINT_INFO("pname fixup #1 [{}]", pstr);
            while ((start_pos = pstr.find(fstr, start_pos)) != std::string::npos)
            {
                pstr.replace(start_pos, fstr.length(), tstr);
                start_pos += tstr.length(); // ...
            }
            if(0)FPS_PRINT_INFO("pname fixup #2 [{}]", pstr);
        }
    }

    return 0;
}

bool initpiVec(std::vector<parseItem*>& piVec)
{
    for (auto xx :piVec)
    {
        xx->idx = xx->from-1;
        xx->nextIdx = xx->from;

    }
    return true;
}

bool cleanpiVec(std::vector<parseItem*>& piVec)
{
    for (auto xx :piVec)
    {
        delete xx;
    }
    piVec.clear();

    return true;
}

// set up the am or ai variables.
bool setVecAmAi(std::vector<parseItem*>& piVec, int lvl)
{
    VarMapUtils*vm = piVec[lvl]->vm;
    varsmap* vmap = piVec[lvl]->vmap;
    asset_manager*ambase = vm->getaM(*vmap, vm->getSysName(*vmap));
    // if we dont provide a parent we will use ambase on the top level
    // if we provide a pname we'll use that but link it to ambase 
    if(piVec[lvl]->pname)
    {
        piVec[lvl]->pam = vm->getaM(*vmap, piVec[lvl]->pstr.c_str());
        if (!piVec[lvl]->pam)
        {
            piVec[lvl]->pam = new asset_manager(piVec[lvl]->pstr.c_str()); 
            if(0)FPS_PRINT_INFO(" pname Setting piVec  PAM [{}]", piVec[lvl]->pstr.c_str() );
            vm->setaM(*vmap, piVec[lvl]->pstr.c_str(), piVec[lvl]->pam);
            piVec[lvl]->pam->vm = vm;
            if(lvl>0)
            {
                piVec[lvl]->pam->am = piVec[lvl-1]->am;
                piVec[lvl]->pam->setFrom(piVec[lvl-1]->am);
                piVec[lvl-1]->am->addManAsset(piVec[lvl]->pam, piVec[lvl]->pstr.c_str());
                if(1)FPS_PRINT_INFO(" pname Setting  pam [{}] parent am [{}] ", piVec[lvl]->pstr.c_str(), piVec[lvl-1]->am->name );
            }
            else
            {
                piVec[lvl]->pam->am = ambase;
                piVec[lvl]->pam->setFrom(ambase);
                ambase->addManAsset(piVec[lvl]->pam, piVec[lvl]->pstr.c_str());
                if(1)FPS_PRINT_INFO(" pname Setting  pam [{}] parent base [{}] ", piVec[lvl]->pstr.c_str(), ambase->name );
            }  

        }
        if (piVec[lvl]->pam)
        {
            ambase = piVec[lvl]->pam;
        }
         
    }
      
    if(piVec[lvl]->amname)
    {
        piVec[lvl]->am = vm->getaM(*vmap, piVec[lvl]->amstr.c_str());
        if (!piVec[lvl]->am)
        {
            piVec[lvl]->am = new asset_manager(piVec[lvl]->amstr.c_str()); 
            if(0)FPS_PRINT_INFO(" amname Setting piVec  AM [{}]", piVec[lvl]->amstr.c_str() );
            vm->setaM(*vmap, piVec[lvl]->amstr.c_str(), piVec[lvl]->am);
            piVec[lvl]->am->vm = vm;
            if(lvl>0)
            {
                piVec[lvl]->am->am = piVec[lvl-1]->am;
                piVec[lvl]->am->setFrom(piVec[lvl-1]->am);
                piVec[lvl-1]->am->addManAsset(piVec[lvl]->am, piVec[lvl]->amstr.c_str());
                if(1)FPS_PRINT_INFO(" amname Setting am  [{}] parent am [{}] ", piVec[lvl]->amstr.c_str(), piVec[lvl-1]->amstr.c_str() );
            }
            else
            {
                piVec[lvl]->am->am = ambase;
                piVec[lvl]->am->setFrom(ambase);
                ambase->addManAsset(piVec[lvl]->am, piVec[lvl]->amstr.c_str());
                if(1)FPS_PRINT_INFO(" amname Setting am  [{}] parent base [{}] ", piVec[lvl]->amstr.c_str(), ambase->name );
            }    
        } 
        return true;    // TODO possibly remove this return
    }
    if(piVec[lvl]->ainame)
    {
        piVec[lvl]->ai = vm->getaI(*vmap, piVec[lvl]->aistr.c_str());
        if (!piVec[lvl]->ai)
        {
            piVec[lvl]->ai = new asset(piVec[lvl]->aistr.c_str()); 
            if(1)FPS_PRINT_INFO(" ainame Setting piVec  AI [{}]", piVec[lvl]->aistr.c_str() );
            //void asset_manager::mapInstance(asset* item, const char* _name)
            vm->setaI(*vmap, piVec[lvl]->aistr.c_str(), piVec[lvl]->ai);
            piVec[lvl]->ai->vm = vm;
            if(lvl>0)
            {
                piVec[lvl]->ai->am = piVec[lvl-1]->am;
                piVec[lvl-1]->am->assetMap[piVec[lvl]->aistr.c_str()] = piVec[lvl]->ai;
                if(1)FPS_PRINT_INFO(" ainame Setting ai  [{}] parent am [{}] ", piVec[lvl]->aistr.c_str(), piVec[lvl-1]->amstr.c_str() );
            }
            else
            {
                piVec[lvl]->ai->am = ambase;
                //piVec[lvl]->am->setFrom(ambase);
                ambase->assetMap[piVec[lvl]->aistr.c_str()] = piVec[lvl]->ai;
                if(1)FPS_PRINT_INFO(" ainame Setting ai  [{}] parent ambase [{}] ", piVec[lvl]->aistr.c_str(), ambase->name );
           }
    
        } 
    }

    return true;
}
// start them all with idx = 0 and nextIdx = to 
// dont think we need the idx flag
bool runpiVec(std::vector<parseItem*>& piVec, int& lvl, int &idx)
{
    if(0)FPS_PRINT_INFO(" running  level is [{}] av [{}] av->name [{}]"
        , lvl
        , fmt::ptr(piVec[lvl]->av)
        , piVec[lvl]->av->name
        );

    if(!piVec[lvl]->av)
    {
        if(0)FPS_PRINT_INFO("   level [{}] has no av [{}] stop the process"
            , lvl
            , fmt::ptr(piVec[lvl]->av)
            );
        return false;
    }

    if(0)FPS_PRINT_INFO(" running  level is [{}] idx [{}] nextIdx [{}] step [{}]"
        , lvl
        , piVec[lvl]->idx
        , piVec[lvl]->nextIdx
        , piVec[lvl]->step
        );

    bool runit = false;
    // do we need to run this level 
    if (piVec[lvl]->idx!=piVec[lvl]->nextIdx)
    {
        //are we done with this level
        if (piVec[lvl]->nextIdx>piVec[lvl]->to)
        {
            piVec[lvl]->nextIdx=piVec[lvl]->from + piVec[lvl]->add;
            // inc the one prior
            if(lvl>0)
            {
                lvl = lvl -1;
                piVec[lvl]->nextIdx = piVec[lvl]->nextIdx + (piVec[lvl]->step * piVec[lvl]->mult);
                return true;
            }
            else 
            {
                return false;
            }
        }
        if(0)FPS_PRINT_INFO(" running  eValIdx level [{}] next [{}]"
                , lvl
                , piVec[lvl]->nextIdx
                );
        piVec[lvl]->eValIdx(piVec[lvl]->nextIdx);
        piVec[lvl]->idx = piVec[lvl]->nextIdx;
        if(0)FPS_PRINT_INFO(" running  setBody level [{}] next [{}] "
                , lvl
                , piVec[lvl]->nextIdx
                );
        piVec[lvl]->setBody();
        if(0)FPS_PRINT_INFO(" running  applyRemap level [{}] next "
                , lvl
                , piVec[lvl]->nextIdx
                );
        for ( int j = 0 ; j <= lvl; j++)
        {
            if(0)FPS_PRINT_INFO(" running  applyRemap level [{}] j [{}] "
                , lvl
                , j
                );
            piVec[lvl]->applyRepMap(&piVec[j]->repmap);
        }
        // set up assets and asset_managers
        setVecAmAi(piVec, lvl);
        // set config into vmap
        piVec[lvl]->vm->configure_vmapStr(*piVec[lvl]->vmap, piVec[lvl]->bstr.c_str(), piVec[lvl]->am, piVec[lvl]->ai, true );
        // move down one if we can 
        if(lvl < (int)piVec.size()-1)
        {
            lvl += 1;
            if(0)FPS_PRINT_INFO(" setting level to [{}]", lvl);
        }
        else 
        {
            piVec[lvl]->nextIdx = piVec[lvl]->nextIdx + (piVec[lvl]->step * piVec[lvl]->mult);
            if (piVec[lvl]->nextIdx > piVec[lvl]->to)
            {
                piVec[lvl]->nextIdx = piVec[lvl]->from + piVec[lvl]->add;
                if(lvl>0)
                { 
                    lvl = lvl - 1;
                    piVec[lvl]->nextIdx = piVec[lvl]->nextIdx + (piVec[lvl]->step * piVec[lvl]->mult);
                    return true;
                }
                else
                {
                    // scan complete
                    return false;
                }
            }
            
        }
        // configure_vmap
        runit = true;
    }
    return runit;
}
        
// config the loader entry on xfile
// we dont run this until we have all the required files in place.
int configLoad(varsmap &vmap, VarMapUtils* vm, const char* xfile)
{
    assetVar* av = vmap["/config/load"][xfile];
    if(av)
    {
        if(av->gotParam("configDone"))
        {
            if(0)FPS_PRINT_INFO("  FOUND  av [{}]  configDone", av->getfName());
            return 0;
        }
        if(0)FPS_PRINT_INFO("  SETTING av [{}]  configDone", av->getfName());

        bool cdone = true;
        av->setParam("configDone",cdone);


       // av looks like this
       //but it may be a simple non templated loader
       if(!av->extras || !av->extras->optVec || !av->extras->optVec->cjopts)
       {
          if(0)FPS_PRINT_INFO("  no cjopts options on av [{}] skipping ", av->getfName());
          return 0;   
       }

       cJSON *cj = av->extras->optVec->cjopts;
       if(0)FPS_PRINT_INFO("  config load [{}] found cj [{}]  av [{}]"
                , xfile
                , fmt::ptr(cj)
                , av->getfName()
                );
        auto tmp = cJSON_Print(cj);
        if(0)FPS_PRINT_INFO("  running cjopts  [{}]", tmp);  
        free(tmp);

        //build up an array of reps//
        std::vector<parseItem*> piVec;
        //reps      
        cJSON *cji;
        // only do this for templates
        cJSON_ArrayForEach(cji, cj) 
        {
            cJSON* cjsite  = cJSON_GetObjectItem(cji, "site");
            cJSON* cjtmpl  = cJSON_GetObjectItem(cji, "tmpl");
            cJSON* cjtmpls = cJSON_GetObjectItem(cji, "tmpls"); // If exists, use tmpls over tmpl
            if(!cjtmpl && !cjsite && !cjtmpls)
                continue;

            // If we have a ctmpls field, then we'll want to grab the template data (body) stored in each
            // template assetVar's tbody and add them in one parse item's assetVar's tbody
            if(cjtmpls)
            {
                FPS_PRINT_INFO("tmpls field is found in av [{}]. We'll perform template expansions with tmpls instead of tmpl", av->getfName());
                char* xtmp = cJSON_Print(cji);
                if(0)FPS_PRINT_INFO(" adding parseItem [{}] for tmpls", xtmp);  
                free(xtmp);
        
                parseItem* pi = new parseItem(cji);
                pi->Init(&vmap, vm);
                piVec.push_back(pi);

                // Add the first template assetVar that we get to the parse item's assetVar field
                // Exclude the first { and last } from the template data. This is because we
                // are combining other template data from other template assetVars to the parse item's
                // assetVar's tbody
                cJSON* item = cJSON_GetArrayItem(cjtmpls, 0);
                if (cJSON_IsString(item) && item->valuestring)
                {
                    pi->av = vm->getVar(vmap, "/config/ctmpl", item->valuestring);
                }

                size_t start = 1;
                size_t end = pi->av->extras->tbody.length() - 1;
                FPS_PRINT_INFO("tbody before: {}", pi->av->extras->tbody);
                pi->av->extras->tbody = pi->av->extras->tbody.substr(start, end - start);
                FPS_PRINT_INFO("tbody now: {}", pi->av->extras->tbody);

                // Iterate through tmpls and add the template data to the parseItem's assetVar's template body field (av->extras->tbody)
                int sz = cJSON_GetArraySize(cjtmpls);
                for (int i = 1; i < sz; i++)
                {
                    cJSON* currTmpl = cJSON_GetArrayItem(cjtmpls, i);
                    if (cJSON_IsString(currTmpl) && currTmpl->valuestring)
                    {
                        assetVar* tmplAv =  vm->getVar(vmap, "/config/ctmpl", currTmpl->valuestring);
                        size_t start = 1;
                        size_t end = tmplAv->extras->tbody.length() - 1;
                        if (end > start) {          // prevent empty files from adding commas
                            pi->av->extras->tbody += fmt::format(",{}", tmplAv->extras->tbody.substr(start, end - start));
                        }
                        FPS_PRINT_INFO("pi's av tbody now: {}", pi->av->extras->tbody);
                    }
                }

                // Finally, add the starting { and ending } so that the parse item's assetVar's tbody
                // is a valid JSON object
                pi->av->extras->tbody = fmt::format("{{{}}}", pi->av->extras->tbody);
                FPS_PRINT_INFO("Final tbody: {}", pi->av->extras->tbody);
            }
            else if(cjtmpl)
            {
                char* xtmp = cJSON_Print(cji);
                if(0)FPS_PRINT_INFO(" adding parseItem [{}] for tmpl", xtmp);  
                free(xtmp);
        
                parseItem* pi = new parseItem(cji);
                pi->Init(&vmap, vm);
                piVec.push_back(pi);
            }
            if(cjsite)
            {
                // get the uri, the av and run 
                cJSON* cja = cJSON_GetObjectItem(cji, "uri");
                if(cja && cja->valuestring)
                {
                    assetVar* aV = vm->getVar(vmap, cja->valuestring, nullptr);
                    if(aV) loadSiteMapAv(vmap, vm, aV);
                }
                //int loadSiteMapAv(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)

            }

            // DONE get body string
        }
        int lvl = 0;
        int idx = 0;
        int count = 0;
        if(0)FPS_PRINT_INFO(" Note start initpiVec piVec.size [{}]", piVec.size());
        if (piVec.size()> 0)
        {
            initpiVec(piVec);
            if(0)FPS_PRINT_INFO(" Note start runpiVec");
            bool ok = true;
            // TODO run unlimited perhaps
            while (ok && count< MAX_PIVEC)
            {
                if(0)FPS_PRINT_INFO(" runpiVec at lvl [{}]", lvl);
                count++;
                ok = runpiVec(piVec, lvl, idx);
            }
            if(0)FPS_PRINT_INFO(" Note end runpiVec ok [{}] count [{}]", ok, count);
            cleanpiVec(piVec);
        }
   } 
   else
   {
       if(0)FPS_PRINT_INFO(" ERROR config load [{}] not found", xfile);
   }
   return 0;
}
// runs every second 
// looks for config things to do
// request a reload of cfiles
// request a reload of templates
// check if loads are complete
// request final if load is complete.
int runConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av)
{
    asset_manager*am = av->am;
    VarMapUtils*vm = am->vm;
    double tNow = vm->get_time_dbl();
    if(0)FPS_PRINT_INFO(" running am [{}] vm [{}] val [{:2.3f}]"
        ,  am->name.c_str()
        , fmt::ptr(vm)
        , av->getdVal() 
        );
    bool nofiles = true;
    bool getsFailed = false;
    if(vmap.find("/config/cfile") != vmap.end())
    {
        nofiles = false;
        for ( auto xx : vmap["/config/cfile"])
        {
            if((xx.first == "name") || (xx.first == "notes"))
                continue;

            assetVar* avx = vmap["/config/cfile"][xx.first];
            
            if(0)FPS_PRINT_INFO(" found cfile [{}]  av [{}] value [{}]"
                    , xx.first
                    , avx->getfName()
                    , avx->getbVal()
                    );
            if(!avx->getbVal())
            {
                if(0)FPS_PRINT_INFO(" found pending cfile [{}]  av [{}] value [{}]"
                    , xx.first
                    , avx->getfName()
                    , avx->getbVal()
                    );

                if (tNow > avx->getdParam("reqTimeout"))
                {   
                    int ccount = avx->getiParam("reqCount");
                    if(ccount < MAX_CONFIG_REQUESTS)
                    {
                        if(1)FPS_PRINT_INFO(" requesting cfile [{}] "
                            , xx.first
                            );
                        requestCfile(vmap, vm, avx);
                        ccount++;
                        avx->setParam("reqCount", ccount);
                        double dval = tNow + 5.0;
                        avx->setParam("reqTimeout", dval);
                    }
                    else
                    {
                        FPS_PRINT_ERROR("Failed to receive cfile [{}] after {} tries"
                            , xx.first
                            , ccount);
                        getsFailed = true;
                        // Stop running runConfig()
                        am->amap["runConfig"]->setParam("endTime", 1);
                    }
                }
            }
            // if false and timed out resend the request
            // if false and count > 5 then read default
        }
    }
    if(0)FPS_PRINT_INFO("looking for tmpl files");
    // template files load the body into a "body" param.
    if(vmap.find("/config/ctmpl") != vmap.end())
    {
        nofiles = false;
        for (auto xx = vmap["/config/ctmpl"].begin(); xx != vmap["/config/ctmpl"].end(); ++xx)
        {
            if(0)FPS_PRINT_INFO(" found tmpl #0 [{}] "
                    , xx->first
                    );

            if((xx->first == "name") || (xx->first == "notes"))
                continue;

            assetVar* avx = vmap["/config/ctmpl"][xx->first];
            if(!avx)
            {
                if(0)FPS_PRINT_INFO("No avx  [{}] moving on..."
                    , xx->first
                    );
                continue;

            }
            if(0)FPS_PRINT_INFO(" found tmpl #1 [{}]  av [{}] value [{}]"
                    , xx->first
                    , avx->getfName()
                    , avx->getbVal()
                    );
            if(!avx->getbVal())
            {
                if(0)FPS_PRINT_INFO(" found tmpl #2 [{}]  av [{}] value [{}]"
                    , xx->first
                    , avx->getfName()
                    , avx->getbVal()
                    );
                if (tNow > avx->getdParam("reqTimeout"))
                {
                    int ccount = avx->getiParam("reqCount");
                    if(ccount < MAX_CONFIG_REQUESTS)
                    {
                        if(1)FPS_PRINT_INFO(" requesting ctmpl [{}] try number [{}] ", xx->first, ccount+1);
                        requestTmpl(vmap, vm, avx);
                        if(0)FPS_PRINT_INFO(" requested  ctmpl [{}] ", xx->first);
                        ccount++;
                        avx->setParam("reqCount", ccount);
                        double dval = tNow + 5.0;
                        avx->setParam("reqTimeout", dval);
                    }
                    else
                    {
                        FPS_PRINT_ERROR("Failed to receive tmpl file [{}] after {} tries"
                            , xx->first
                            , ccount);
                        getsFailed = true;
                    }
                }
            }
            if (std::next(xx) == vmap["/config/ctmpl"].end() && getsFailed)
            {
                FPS_PRINT_ERROR("Failed to receive 1 or more template files");
                // Stop running runConfig()
                am->amap["runConfig"]->setParam("endTime", 1);
            }
        }
    }

    if(0)FPS_PRINT_INFO("looking for load files");
    // this is a loader request .. we need the "file" parameter loaded.
    // if we dont have  file parameter then complain
    // if the "file" is not in /config/cfile then add it and request it.
    // the loader value is set to true when it is completed
    // a loader can call up other loaders all of which have to be "true"
    // before the final file is exercised.
    // If a loader has a file it has to be loaded before starting the loader/
    // if a loader has templates , they two have to be loaded before starting the loader.

// "/config/load": {       
//        "name":"sample config for ess",
//        "ess_controller.json": {  << load key 
//           "value":false,
//           "file":"ess_controller.json",
//           "aname":essName,
//           "final":"ess_final.json",

    if(vmap.find("/config/load") != vmap.end())
    {
        const char* final = nullptr;
        nofiles = false;
        bool loadComplete = true;

        // each xx.first is a single load key for a single load request
        // we' create file requests and load request entries
        // skip name and notes
        for ( auto xx : vmap["/config/load"])
        {
            const char* aname = nullptr;
            const char* ainame = nullptr;
            const char* pname = nullptr;
            const char* xfile = nullptr;

            bool bval =  false;
            // skip notes etc
            if((xx.first == "name") || (strncmp(xx.first.c_str(), "notes", strlen("notes"))==0))
            {
                continue;
            }
            // xx.first is the load name not the file name
            assetVar* avx = vmap["/config/load"][xx.first];
            // runload 
            bool runLoad = false;

            // request the file if it is not loaded
            if(0)FPS_PRINT_INFO(" request load #1  [{}]  av [{}] value [{}]"
                , xx.first
                , avx->getfName()
                , avx->getbVal()
                );
            // if the value is false we have not started
            if(avx->gotParam("aname"))
            {
                aname = avx->getcParam("aname");
            }
            if(avx->gotParam("pname"))
            {
                pname = avx->getcParam("pname");
            }
            if(avx->gotParam("ainame"))
            {
                ainame = avx->getcParam("ainame");
            }
            if(!avx->getbVal())
            {
                if(0)FPS_PRINT_INFO(" request load #2 [{}]  av [{}] value [{}]"
                    , xx.first
                    , avx->getfName()
                    , avx->getbVal()
                    );
                // no file param so this one can be ignored
                if(!avx->gotParam("file"))
                {
                    if(0)FPS_PRINT_INFO(
                        " load <<file>> param missing;  bypass loader xx.first [{}]  av [{}] value [{}]"
                        , xx.first
                        , avx->getfName()
                        , avx->getbVal()
                        );
                    xfile = xx.first.c_str();
                    // skip this loader nothing mre we can do with it 
                    //bool bval = true;
                    avx->setParam("file",xfile); 
                    //continue;
                }
                {
                // currently every loader must have a file.. if not specified the file is the loader name
                // may fix that later this week. 09/21/2021
                    xfile = avx->getcParam("file");
                    if(!xfile)
                    {
                        if(0)FPS_PRINT_INFO(" load file param NULL  xx.first [{}]  av [{}] value [{}]"
                            , xx.first
                            , avx->getfName()
                            , avx->getbVal()
                            );
                            xfile = xx.first.c_str();
                        // skip this loader nothing mre we can do with it 
                        //bool bval = true;
                        avx->setParam("file",xfile); 
                        // skip this loader
                        // bool bval = true;
                        // avx->setVal(bval);
                        //continue;
                    }
                    //else
                    {
                        //we have a file , need to run requestCfile 
                        // also need to set up the aname for this file.
                        auto fr = fmt::format("/config/cfile:{}" , xfile);
                        // first look for the av , it may have already been loaded.
                        assetVar*avf = vm->getVar(vmap, fr.c_str(), nullptr);
                        // if no av set up the default
                        if (!avf)
                        {
                            bval =  false;
                            avf = vm->setVal(vmap, fr.c_str(), nullptr, bval);
                            if(aname)  avf->setcParam("aname",  aname);
                            if(pname)  avf->setcParam("pname",  pname);
                            if(ainame) avf->setcParam("ainame", ainame);
                        }
                        // see if this file has been loaded
                        //bval =  avf->getbVal();

                    }
                }
                    // we have a file name, we need cfile and load entries for this 
                if(0)FPS_PRINT_INFO(" found load  [{}] xfile [{}]  av [{}] value [{}]"
                    , xx.first
                    , xfile
                    , avx->getfName()
                    , avx->getbVal()

                    );
                
                runLoad  = checkcFile(vmap, vm, nullptr, xfile);
                if(!runLoad)
                {
                    loadComplete = false;
                    // allow us to scan the rest of the system 
                    if(0)FPS_PRINT_INFO(" aborting load  FILE MISSING  [{}] xfile [{}]  av [{}] value [{}]"
                    , xx.first
                    , xfile
                    , avx->getfName()
                    , avx->getbVal()

                    );
                    continue;
                }
                // the file option, if present, has been loaded.
                // we can continue to look for the other features of this load request
                // we'll deal with the cjopts and then the final (if we have one)
                // does this loader have cjopts
                // if not it has nothing to do after the file is loaded
                if(!avx->extras || !avx->extras->optVec|| !avx->extras->optVec->cjopts)
                {
                    if(0)FPS_PRINT_INFO(" load options missing;  bypass loader xx.first [{}]  av [{}] value [{}]"
                        , xx.first
                        , avx->getfName()
                        , avx->getbVal()
                        );
                    bool bval = true;
                    avx->setVal(bval);
                    continue;
                }
                if(0)FPS_PRINT_INFO(" xfile [{}] completed", xfile);
            }
            //avf is the file 
            // avx is the loader. if  avx is done then no need to look at it.....
            // move this up above some of the preceeding code
            if(avx->getbVal())
            {
                continue;
            }        
            // we have a loader request ( we are still false) 
            // we also have a loaded loader config file
            // we also have loader options
            // process them
            // we must have all these loaded before we can run the loader  
            bool optionsComplete = true;

            //if(avx->extras && avx->extras->optVec)
            {
                // make sure we have all the parts set to pending.
                //cjopts is an array object 
                // we can have 
                //        load
                //        tmpl
                //        file
                //          (load and tml will both need a file)
                cJSON* cjsi;
                cJSON_ArrayForEach(cjsi, avx->extras->optVec->cjopts)
                {
                    cJSON* cjfile   = cJSON_GetObjectItem(cjsi, "file");
                    if(cjfile)
                    {
                        if(optionsComplete)
                        {
                            optionsComplete  &= checkcFile(vmap, vm, cjsi, cjfile->valuestring);
                        }
                    }

                    // If we have a tmpls field, then we'll request for all template files defined in this field
                    // Otherwise, request for template file defined in tmpl field, if it exists
                    cJSON* cjtmpl   = cJSON_GetObjectItem(cjsi, "tmpl");
                    cJSON* cjtmpls  = cJSON_GetObjectItem(cjsi, "tmpls");
                    if (cjtmpls)
                    {
                        if(0)FPS_PRINT_INFO("tmpls field is found in avx [{}]. Requests for template files will be made for tmpls instead of tmpl", avx->getfName());
                        if(optionsComplete)
                        {
                            cJSON* currTmpl = NULL;
                            cJSON_ArrayForEach(currTmpl, cjtmpls)
                            {
                                if (cJSON_IsString(currTmpl) && currTmpl->valuestring)
                                {
                                    optionsComplete &= checkTmpl(vmap, vm, cjsi, currTmpl->valuestring);
                                }
                            }
                        }
                    }
                    else if(cjtmpl)
                    {
                        FPS_PRINT_INFO("tmpl field is found in avx [{}], which is {}", avx->getfName(), cstr{cjtmpl->valuestring});
                        if(optionsComplete)
                        {
                            optionsComplete  &= checkTmpl(vmap, vm, cjsi, cjtmpl->valuestring);
                        }
                    }

                    // note that in setup we'll need to 
                    // trigger the tmpl and file loaders
                    // we'll also have to set up the submodule loaders
                    // we wont set these up automatically now.... for this version
                    cJSON* cjload = cJSON_GetObjectItem(cjsi, "load");
                    if(cjload)
                    {
                        if(optionsComplete)
                        {
                            optionsComplete  &= checkLoad(vmap, vm, cjsi, cjload->valuestring);
                        }
                    }
                    // site will need a tmpl file. 
                    // the site interface will be in the tmpl body Param
                    // once we have that we can  
                    //int loadSiteMapAv(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
                    cJSON* cjsite = cJSON_GetObjectItem(cjsi, "site");
                    if(cjsite)
                    {
                        if(optionsComplete)
                        {
                            optionsComplete  &= checkSite(vmap, vm, cjsi, cjsite->valuestring);
                        }
                    }
                }
            }


            if(0)FPS_PRINT_INFO(" avx [{}] optionsComplete [{}] loadComplete [{}]"
                , avx->getfName()
                , optionsComplete
                , loadComplete
                );
                // free(stmp);
        
            // optionsComplete will be true when all except final have loaded
            // but will be set false after the load while waitingfor the final file.
            if(!optionsComplete)
            {
                if(0)FPS_PRINT_INFO(" LOAD >>> Still Waiting for files for  [{}] in /config/load", xx.first);
                continue;
            }

            // loadComplete remains set after we have all the files 
            // avx will be false until the load is complete
            bool runitall = false;
            if (avx->gotParam("loadComplete"))
            {
                runitall = avx->getbParam("loadComplete");
            }
            if(!runitall)
            {
                // runLoad true means we've got all the files ( including other loaders)
                // we can run this load
                if(optionsComplete)
                {
                    if(1)FPS_PRINT_INFO(" All files loaded for loader [{}] ready to run av [{}] load"
                            , xx.first
                            , avx->getfName()
                            );
                    // we got all the options run the loader
                    // once processed 
                    // av->gotParam("configDone"))
                    //         is set true 
                    configLoad(vmap, vm, xx.first.c_str());

                    bool bval =  true;
                    avx->setVal(bval);
                    avx->setParam("loadComplete",bval);
                    //bval =  false;
                    //vm->setVal(vmap,"/config/cfile:bms_manager_test.json", nullptr, bval);
                    char *fname = avx->getfName();
                    if(0)FPS_PRINT_INFO(" After load, set avx [{}] true", fname );

                    // now process the "final" param
                    if (avx->gotParam("final"))
                    {
                        final = avx->getcParam("final");

                        if(final)
                        {
                            if(0)FPS_PRINT_INFO(" After load, requesting final [{}]", final);
                            bval =  false;
                            assetVar* avff = vm->getVar(vmap,"/config/cfile", final);
                            if(!avff)
                            {
                                avff = vm->setVal(vmap,"/config/cfile", final, bval);
                                avff->setcParam("file", final);
                            }
                            bval = avff->getbVal();
                            // on the final we keep doing this
                            if(!bval)
                            {
                                requestFfile(vmap, vm, final);
                                if(1)FPS_PRINT_INFO(" After requestFfile, requesting final [{}]", final);
                                // Stop running runConfig()
                                am->amap["runConfig"]->setParam("endTime", 1);
                            } 
                        }
                    }

                    // load saved registers from dbi
                    auto replyto = fmt::format("/{}/cfg/cjson/saved_registers", aname);
                    if (vm->p_fims)
                        vm->p_fims->Send("get", "/dbi/ess_controller/saved_registers", replyto.c_str(),nullptr);
                    
                }
                if(0)FPS_PRINT_INFO(" After load, requested final");
            }
            // this is the end of this load xx.first
        }
    }
    if(0)FPS_PRINT_INFO("done looking for files");

    if(nofiles)
    {
        if(0)FPS_PRINT_INFO("no file requests found");
    } 
  return 0;
}

assetVar* setupCfile(varsmap& vmap, VarMapUtils* vm)
{
    char* aname = vm->getSysName(vmap);
    
    auto cfgstr = fmt::format("/config/control:{}_config", aname);
    auto cfgsp = cfgstr.c_str();
    double tNow = vm->get_time_dbl();
    //if(0)FPS_PRINT_INFO(" >>>>>> running  getSysName \n");
    assetVar* av = vm->getVar(vmap, cfgsp, nullptr);
    asset_manager* am = vm->getaM(vmap, aname);
    if(!am)
    {
        am = new asset_manager (aname);
        if(0)FPS_PRINT_INFO(" Setting base AM [{}]", aname );
        vm->setaM(vmap, aname, am);
        am->vm = vm;
    }
    if(!av)
    {
        double dval = 0.0;
        if(0)FPS_PRINT_INFO(" Setting up config system  1 ");
        av = vm->setVal(vmap, cfgsp, nullptr, dval);
        av->am = am;

        am->amap["runConfig"] = av;
        
        vm->setAvFunc(vmap
            , am->amap
            , (const char*)aname
            , vm->p_fims, av
            , "runConfig"
            , &runConfig);
            // void RunAv(varsmap& vmap, VarMapUtils*vm, varmap& amap, const char* aname
            //          , asset_manager*am
            //         , fims * p_fims
            //         , const char* schedVar/*"/schedule/bms:bms_rack"*/
            //         , const char* mapVar/*"runRackCloseContactor"*/
            //         , double tNow, double every /*0.1*/, double offset);
        if(0)FPS_PRINT_INFO(" Setting up config system  2 ");
        RunAv(vmap, vm, am->amap, aname, am, vm->p_fims
           , "/schedule/ess:run_config"
           , "runConfig", tNow, 0.1, 0.0);
        if(0)FPS_PRINT_INFO(" Setting up config system  3 ");

    }

    return av;
}

// get the management layout for a config file 
void getManagement(varsmap&vmap, VarMapUtils* vm, assetVar*av, asset_manager**pamp, asset_manager**amp, asset**aip)
{
    asset_manager*ambase = vm->getaM(vmap,vm->getSysName(vmap));
    asset_manager*am = nullptr;
    asset_manager*pam = ambase;
    asset*ai = nullptr;
    
    // cfiles will have asset_managers {am} defined in aname  also pname and asname for simple assets  
    // if this screws up for templates we may have to preload them in the manager files. no biggie
    // an asset will need a pname
    char *aname   =  av->getcParam("aname");
    char *pname   =  av->getcParam("pname");
    char *ainame  =  av->getcParam("ainame");
    FPS_PRINT_DEBUG("av: [{}] aname: [{}] pname: [{}] ainame: [{}]", av->getfName(), cstr{aname}, cstr{pname}, cstr{ainame});
    if(pname)
    {
        pam = vm->getaM(vmap, pname);
        if(!pam)
        {
            pam = new asset_manager (pname);
            vm->setaM(vmap, pname, pam);

            pam->am = nullptr;
            if (ambase)
            {
                pam->setFrom(ambase);
                ambase->addManAsset(pam, pname);
            }
            pam->vm = vm;
            //am->vecs = &vecs;         // used to store pubs , subs and blocks
            pam->syscVec = vm->syscVec;   // used to store UI publist   

        }
        ambase=pam;
    }

    if(aname)
    {
        am = vm->getaM(vmap, aname);
        if(0)FPS_PRINT_INFO(" >>>>>   seeking aname [{}]  found am [{}]", aname, fmt::ptr(am)); 
        if(!am)
        {
            am = new asset_manager (aname);
            vm->setaM(vmap, aname, am);
            am->am = nullptr;  
            if (ambase)
            {
                am->setFrom(ambase);
                ambase->addManAsset(am, aname);
            }
            am->vm = vm;
            //am->vecs = &vecs;         // used to store pubs , subs and blocks
            am->syscVec = vm->syscVec;   // used to store UI publist   
            if(0)FPS_PRINT_INFO(" >>>>>   created am  aname [{}]  found am [{}]", aname, fmt::ptr(am)); 

        }
    }
    if(ainame)
    {
        ai = vm->getaI(vmap, ainame);
        if(!ai)
        {
            FPS_PRINT_INFO("Asset instance [{}] is null. Creating new one", ainame);
            ai = new asset(ainame);
            vm->setaI(vmap, ainame, ai);
            ai->am = nullptr;   
            if (ambase)
            {
                FPS_PRINT_INFO("Setting asset manager [{}] to asset instance [{}]", ambase->name, ai->name);
                //ai->setFrom(ambase);
                ai->am = ambase;
                // TODO
                //ambase->addAsset(ai, sp);
            }
            ai->vm = vm;
            //am->vecs = &vecs;         // used to store pubs , subs and blocks
            //am->syscVec = syscVec;   // used to store UI publist   
        }
    }
    if(pam)*pamp = pam;
    if(am)*amp = am;
    if(ai)*aip = ai;

}

/**
 * @brief Removes selected fields inside the cJSON message body
 * 
 * @param cj the cJSON message body
 * @param filterItems the collection of fields to remove from the cJSON message body
 */
void filtercJSONMsg(cJSON* cj, const std::initializer_list<std::string> filterItems)
{
    for (const std::string& item : filterItems)
    {
        FPS_PRINT_DEBUG("Got item {}", item);
        cJSON* cjitem = cJSON_GetObjectItem(cj, item.c_str());
        while (cjitem)
        {
            cJSON* itemToDetach = cJSON_DetachItemFromObject(cj, item.c_str());
            FPS_PRINT_INFO("Deleting {} field from cJSON object", item);
            cJSON_Delete(itemToDetach);
            cjitem = cJSON_GetObjectItem(cj, item.c_str());
        }
    }
}

int VarMapUtils::configure_vmapStr(varsmap& vmap, const char* body, asset_manager*am, asset*ai, bool delcj)
{
    int ret = -1;
    cJSON*cj = cJSON_Parse(body);

    if(cj)
    {
        const char* amname = nullptr;
        const char* ainame = nullptr;
        const char* pname = nullptr;
        FPS_PRINT_INFO("process file seeking header stuff am [{}] ai [{}]", am?am->name:"no am" , ai?ai->name:"no ai");
        cJSON* cjam = cJSON_DetachItemFromObject(cj, "amname");
        cJSON* cjai = cJSON_DetachItemFromObject(cj, "ainame");
        cJSON* cjap = cJSON_DetachItemFromObject(cj, "pname");
        if(0)FPS_PRINT_INFO("process file seeking header stuff am [{}] ai [{}] cjap [{}] cjam [{}] cjai [{}]"
                , am?am->name:"no am" 
                , ai?ai->name:"no ai"
                , cjap?cjap->valuestring:".."
                , cjam?cjam->valuestring:".."
                , cjai?cjai->valuestring:".."
                );

        // Delete fields in the cJSON object that should not be included before processing
        filtercJSONMsg(cj, {"_doc", "_id", "_version"});

        // Iterate through each object in the cJSON body and replace keys, which are uris, containing _ with /
        FPS_PRINT_INFO("body: {}", cstr{body});
        cJSON* currentElem = NULL;
        cJSON_ArrayForEach(currentElem, cj)
        {
            if (currentElem->string)
            {
                std::string newKey = run_replace(std::string(currentElem->string), "#", "/");
                free((void*)currentElem->string);
                currentElem->string = strdup((char*)newKey.c_str());
            }
        }

        if(cjam)amname   = cjam->valuestring;  // this will be an encoded string 
        if(cjai)ainame   = cjai->valuestring;
        if(cjap)pname    = cjap->valuestring;

        FPS_PRINT_INFO("amname: [{}] ainame: [{}] pname: [{}]", cstr{amname}, cstr{ainame}, cstr{pname});

        if(!pname)pname=getSysName(vmap);
        asset_manager* pam = nullptr;
        // get the parent, make it if we have to 
        pam = getaM(vmap, pname);

        if(!pam)
        {
            // this will be an unrooted am , we'll root it later perhaps
            
            FPS_PRINT_INFO("creating asset_manager pname: [{}]", cstr{pname});
            pam = new asset_manager (pname);
            setaM(vmap, pname, pam);
            //am->setFrom(pname);
            //ambase->addManAsset(pam, pname);
            pam->vm = this;
            //am->vecs = &vecs;         // used to store pubs , subs and blocks
            pam->syscVec = syscVec;   // used to store UI publist   
        }

        if(amname)
        {
            am = getaM(vmap, amname);
        }

        if(!am && amname && pam)
        {
            FPS_PRINT_INFO("process file creating asset_manager amname: [{}]", cstr{amname});
            am = new asset_manager (amname);
            setaM(vmap, amname, am);
            am->setFrom(pam);
            pam->addManAsset(am, amname);
            am->vm = this;
            //am->vecs = &vecs;         // used to store pubs , subs and blocks
            am->syscVec = syscVec;   // used to store UI publist   
        }

        if(!am && ainame)
        {
            ai = getaI(vmap, ainame);
            if(!ai && pam)
            {
                FPS_PRINT_INFO("process file creating asset  ainame: [{}]", cstr{ainame});
                ai = new asset(ainame);
                setaI(vmap, ainame, ai);
                FPS_PRINT_INFO("Setting asset manager [{}] to asset instance [{}]", pam->name, ai->name);
                ai->am = pam;
                ai->vm = this;
                pam->assetMap[ainame] = ai;
                //pam->addAsset(ai, ainame);
                //ai->syscVec = syscVec;   // used to store UI publist   
            }
        }

        if(cjam)
        {
            FPS_PRINT_ERROR("process file , amname found [{}] ", amname);
            cJSON_Delete(cjam);
        }
        if(cjai) 
        {
            FPS_PRINT_ERROR("process file , ainame found [{}] ", ainame);
            cJSON_Delete(cjai);
        }
        if(cjap)
        {
            FPS_PRINT_ERROR("process file , pname found [{}] ", pname);
            cJSON_Delete(cjap);
        }
        if(!am && !ai)
        {
            FPS_PRINT_ERROR("Unable to process file , please add amname or ainame, and possible pname fields ");
            cJSON_Delete(cj);
            return -1;
        }
        // TODO do the same for the ai 
        ret = configure_vmapCJ(vmap, cj , am, ai, true);
    }

    return ret;
}

/**
 * This function parses the saved registers coming from DBI in the form of:
 * {
 *  "controls":
 *      {
 *          "pcs":
 *              {
 *                  "active_power_gradient": {"value": 5}
 *              }
 *      }
 * }
 * 
 * This function iterates through the outer two objects to construct uris in the form of
 * /controls/pcs, and adds the value of the third level to the map as /controls/pcs/active_power_gradient.
*/
void VarMapUtils::readMapfromDbi(varsmap& vmap, const char* fims_message_body) {
    cJSON* cj_msg_body = cJSON_Parse(fims_message_body);
    cJSON* cj_level_1;
    cJSON* cj_level_2;

    // iterate through the message body sent from dbi
    // pull out the first two levels of keys
    // use that to reconstruct the first part of the uri
    if(cj_msg_body && cj_msg_body->type == cJSON_Object) {
        cj_level_1 = cj_msg_body->child;
        while(cj_level_1) {
            if(cj_level_1->type == cJSON_Object) {
                cj_level_2 = cj_level_1->child;
                while(cj_level_2) {
                    if(cj_level_2->type == cJSON_Object) {
                        cJSON* cj_assetVar = cj_level_2->child;
                        while (cj_assetVar) {
                            // Construct the uri by joining the first two levels of keys
                            std::string asset_uri = "/" + std::string(cj_level_1->string) + "/" + std::string(cj_level_2->string);
                            // Use the third level as the var
                            // insert its value into the vmap
                            setValfromCj(vmap, asset_uri.c_str(), cj_assetVar->string, cj_assetVar, 0);
                            // move onto the next var
                            cj_assetVar = cj_assetVar->next;
                        }
                    }
                    cj_level_2 = cj_level_2->next;
                }
            }
            cj_level_1 = cj_level_1->next;
        }
    }

    // Don't forget to free the cJSON object after you're done with it to avoid memory leaks
    cJSON_Delete(cj_msg_body);
}


//
// the cfiles are just read straight in, they may call up tmpl files.
// dummy call to kick off loader 
//vm->handleCfile(vmap, nullptr, "set", "/cfg/crun", single, nullptr, nullptr, nullptr, nullptr);
void VarMapUtils::handleCfile(varsmap& vmap
   , cJSON* cj, const char* method, const char* uri
   , int& single, const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    
    if(0)FPS_PRINT_INFO(" >>>>>> uri [{}]", uri);

    if (dbiess == "")
    {
        //dbifs = fmt::format("/xxdbi/ess_controller/configs/");
        dbiess = fmt::format("{}", getSysName(vmap));
    }
    //assetVar* avcfg = 
    setupCfile(vmap, this);
    VarMapUtils* vm = this; 

    double tNow = get_time_dbl();

    if (strncmp(uri,"/cfg/setup", strlen("/cfg/setup"))==0)
    {
        char* aname = getSysName(vmap);

        auto cjb = cJSON_Parse(body);
        if (! cjb)
        {
            FPS_PRINT_INFO("  no body I'm outta here");
            if(cj)cJSON_Delete(cj);
            return;
        }
        FPS_PRINT_INFO("what a body - [{}]", body);
        //RunVmapSetup(vmap, this, cj, cjb);
        auto cjf = cJSON_GetObjectItem(cjb, "func");
        if (! cjf || cjf->type != cJSON_String)
        {
            FPS_PRINT_INFO("  no function in body I'm outta here");
            if(cj)cJSON_Delete(cj);
            if(cjb)cJSON_Delete(cjb);

            return;
        }
        auto cjn = cJSON_GetObjectItem(cjb, "name");
        if (! cjn || cjn->type != cJSON_String)
        {
            FPS_PRINT_INFO("  no name in body I'm outta here");
            if(cj)cJSON_Delete(cj);
            if(cjb)cJSON_Delete(cjb);

            return;
        }
        char *fun = cjf->valuestring;
        char *sname = cjn->valuestring;

        auto res1 = getFunc(vmap, aname, fun);
        // get runSetup function
        if (res1 == nullptr) {
            // FPS_PRINT_ERROR(" Running Setup  setting up function [{}] uri [{}] aname [{}] body [{}]"
            //             fun, uri, aname?aname:"I aint got no name",body?body:"I aint got no body"); 
            FPS_PRINT_INFO( "Missing function called [{}]", fun);
            //setFunc(vmap, am->name.c_str(), fun, (void*)&setupBMS);
            //res1 = getFunc(vmap, aname, fun);
        }
        if (res1 != nullptr) {
            FPS_PRINT_INFO(" found func [{}]", fun);
            auto res1Func = reinterpret_cast<myAvfun_t> (res1);
            // have to create an av from the body (cjb) 
            auto av = setValfromCj(vmap,"/system/setup",sname, cjb);
            // bool update = true;
            // av->setParam("update", update);
            av->am = am;
            am->vm = this;

            res1Func(vmap, am->amap, am->name.c_str(), p_fims, av);
        }

        if(cj)cJSON_Delete(cj);
        if(cjb)cJSON_Delete(cjb);
        return;
    }

    if (strncmp(uri,"/cfg/cfile/", strlen("/cfg/cfile/"))==0)
    {
        if(0)FPS_PRINT_INFO(" >>>>>> handling uri [{}] ", uri);

        // extract /cfile/bms  it has to be rooted at sysname for now
        char*spuri = strdup(uri);
        char*spav = nullptr;
        char* sp = spuri + strlen("/cfg/cfile/");
        while(*sp)
        {
            if (*sp == '/' )
            {
                *sp = 0;
            }
            sp++;
        }
        // TODO BUG if spav is missing 
        sp = spuri + strlen("/cfg/cfile/");
        spav = sp + strlen(sp)+1;
        if(0)FPS_PRINT_INFO(" >>>>>> uri [{}] am [{}] spav [{}]", spuri, sp, spav);
        //load it for an am or an ai we got from the loader spec 
        auto cfile = fmt::format("/config/cfile:{}", spav);
        asset_manager* pam = nullptr;
        asset_manager* am  = nullptr;
        asset* ai          = nullptr;
        am = getaM(vmap,sp);  // get loader
        // looks for the config cfile entry 
        assetVar*avt = vm->getVar(vmap, cfile.c_str(),nullptr);
        if(avt)
        {
            if(1)FPS_PRINT_INFO(" >>>>>> before getting management #1 info from  [{}]  am at start [{}]"
                , avt->getfName()
                , am?am->name:"no Am"
                );
            getManagement(vmap, this, avt, &pam, &am, &ai);
            if(0)FPS_PRINT_INFO(" >>>>>> after getting management info from  [{}]  am after getManagement [{}]"
                , avt->getfName()
                , am?am->name:"no Am"
                );
        }
        else
        {
            //no cfile entry so set one up
            // we may have to extract amname and pname from the incoming body
            if(0)FPS_PRINT_INFO(" NOTE setting up  uri [{}]", cfile); 
            bool bval = false;
            avt = vm->setVal(vmap, cfile.c_str(),nullptr, bval);
            avt->am = am;
        }
        // TODO need to sort this out...
        if(!avt->am)
        {
            avt->am = am;
        }
        bool bval =  true;
        avt->setVal(bval);

        avt->setParam("reqResp",tNow);
        std::string md5 = bodymd5 (body, (int)strlen(body));
        avt->setcParam("md5sum", md5.c_str());

        // cfiles load right away
        if(ai)
        {
            configure_vmapStr(vmap, body, nullptr, ai, true);
        }
        if(am)
        {
            configure_vmapStr(vmap, body, am, nullptr, true);
        }
        else
        {

            if(1)FPS_PRINT_INFO(" ERROR >>>>>> am not found using data from file"); 
            configure_vmapStr(vmap, body, nullptr, nullptr, true);

        }
        if(spuri) free(spuri);
        spuri = nullptr;

    }
//        fims_send -f bms_manager_tmpl.json -m set -r /$$ -u /flex/cfg/ctmpl/bms/bms_catl_new_tmpl.json

    if (strncmp(uri,"/cfg/ctmpl/", strlen("/cfg/ctmpl/"))==0)
    {
        // extract /ctmpl/bms  it has to be rooted at sysname for now
        char*spuri = strdup(uri);
        if(0)FPS_PRINT_INFO(" >>>>>> uri [{}] \n", spuri);

        char* sp = spuri + strlen("/cfg/ctmpl/");
        char* spav;
        while(*sp)
        {
            if (*sp == '/' )
            {
                *sp = 0;
            }
            sp++;
        }

        sp = spuri + strlen("/cfg/ctmpl/");
        spav = sp + strlen(sp)+1;
        if(0)FPS_PRINT_INFO(" >>>>>> #1 uri [{}]", spuri);
        if(0)FPS_PRINT_INFO(" >>>>>> #2 uri [{}] sp [{}] ", spuri, sp);
        if(0)FPS_PRINT_INFO(" >>>>>> #3 uri [{}] sp [{}] spav [{}]", spuri, sp, spav);
        auto cfile = fmt::format("/config/ctmpl:{}", spav);
        //asset_manager* pam = nullptr;
        //asset_manager* am  = nullptr;
        //asset* ai          = nullptr;

        assetVar*avt = vm->getVar(vmap, cfile.c_str(), nullptr);
        if(!avt)
        {
            bool bval = false;
            if(1)FPS_PRINT_INFO(" Created Template file  [{}]", cfile);
            avt = vm->setVal(vmap, cfile.c_str(), nullptr, bval);
        }
        if(avt)
        {
            // if(1)FPS_PRINT_INFO(" >>>>>> before getting management #2 info from  [{}]  am at start [{}]"
            //         , avt->getfName()
            //         , am?am->name:"no Am"
            //         );
            // no management yet on templates that's done later
            // getManagement(vmap, vm, avt, &pam, &am, &ai);
         
            bool bval =  true;
            avt->setVal(bval);

            avt->setParam("reqResp",tNow);
            // templates are savd in the tbody string in the extras 
            if(!avt->extras)
            {
                avt->extras=new assetExtras;                
            }
            avt->extras->tbody = body;

            std::string md5 = bodymd5 (body, (int)strlen(body));
            avt->setcParam("md5sum", md5.c_str());
       
        }
        else
        {
            // our manager should have loaded one 
            if(1)FPS_PRINT_INFO(" NO /config/ctmpl def found "); 
        }
        if(cj)cJSON_Delete(cj);
        if(spuri) free(spuri);
        spuri = nullptr;
        return;
    }
    if (strncmp(uri,"/cfg/cstop", strlen("/cfg/cstop"))==0)
    {
        char* aname = getSysName(vmap);
        asset_manager* am = getaM(vmap,aname);

        // void StopAv(varsmap& vmap, VarMapUtils*vm, varmap& amap, const char* aname, asset_manager *am, fims * p_fims
        //             , const char* schedVar/*"/schedule/bms:bms_rack"*/
        //             , const char* mapVar/*"runRackCloseContactor"*/
        //             , double tNow, double in /*0.1*/)
        StopAv(vmap, vm, am->amap, aname, am, vm->p_fims
                , "/schedule/ess:run_config"
                , "runConfig", tNow, 0.0);
        FPS_PRINT_INFO(" Stopped Loader "); 
    }

    if (strncmp(uri,"/cfg/crun", strlen("/cfg/crun"))==0)
    {
        char* aname = getSysName(vmap);
        asset_manager* am = getaM(vmap,aname);

        RunAv(vmap, vm, am->amap, aname, am, vm->p_fims
           , "/schedule/ess:run_config"
           , "runConfig", tNow, 0.1, 0.0);
        FPS_PRINT_INFO(" Running Loader "); 
    }

    if (strncmp(uri,"/cfg/cjson/", strlen("/cfg/cjson/"))==0)
    {
        readMapfromDbi(vmap, body);
    }

    if(cj)cJSON_Delete(cj);
}


// std::vector<std::string>amVec;
//            std::string manName ="";
//            char *aman = av->getcParam("aman");
//            char *aitem = av->getcParam("aitem");
//            int nmaps = 0;
//            if (aman != nullptr)
//            {
//                nmaps = uriSplit(amVec, aman);
//            }
// only does 1 layer at the moment but it could be recursive
int deepDive (cJSON* cjii, VarMapUtils* vm, asset_manager* am, std::string&item, int opts)
{
    int ret = 0;
    {
        if (am->amap.find(item) != am->amap.end())
        {
            auto x = am->amap[item];
            if(x) 
            {
                ret++;

                cJSON* cji = cJSON_CreateObject();
                x->showvarCJ(cji, opts);
                cJSON_AddItemToObject(cjii,x->getfName(), cji);
            }
        }
    }
    for (auto xx : am->assetManMap)
    {
        if(0)FPS_PRINT_INFO("am   [{}]  child am [{}]  ", am->name, xx.first); 
        if (xx.second->amap.find(item) != xx.second->amap.end())
        {
            auto x = xx.second->amap[item];
            if(x) 
            {
                ret++;
                deepDive (cjii, vm, xx.second, item, opts);
                // cJSON* cji = cJSON_CreateObject();
                // x->showvarCJ(cji, opts);
                // cJSON_AddItemToObject(cjii,x->getfName(), cji);

            }
        }

    }
    for (auto xx : am->assetMap)
    {
        if(0)FPS_PRINT_INFO("am   [{}]  child ai [{}]  ", am->name, xx.first);
        if (xx.second->amap.find(item) != xx.second->amap.end())
        {
            auto x = xx.second->amap[item];
            if(x) 
            {
                ret++;

                cJSON* cji = cJSON_CreateObject();
                x->showvarCJ(cji, opts);
                cJSON_AddItemToObject(cjii,x->getfName(), cji);

            }
        }
    }
    return ret;
}

cJSON* loadAmap(VarMapUtils * vm, varsmap& vmap, int single, const char* comp, const char* var, const char* body, asset_manager*am, char *uri)
{
    std::vector<std::string>amVec;
    cJSON* cjbody = nullptr;
    int nmaps = 0;
    vm->uriSplit(amVec, uri);
    nmaps = (int)amVec.size();
    if(1)FPS_PRINT_INFO("uri  [{}] nmaps [{}]  ", uri, nmaps);
    for (int i = 0 ; i < nmaps; i++)
    {
        if(1)FPS_PRINT_INFO("uri  [{}] idx [{}] item [{}] ", uri, i, amVec[i]);
    }
    if(nmaps == 0)
    {
        return nullptr;
    }
    if(amVec[0] == "amap")   // strncmp(uri,"/amap/", strlen("/amap/"))==0)
    {
        const char* aname = nullptr;
        if (nmaps > 1)     //strdup(&uri[strlen("/amap/")]);
        {
            aname = amVec[1].c_str(); 
        }
        // now /amap/aname/item   we need to split the uri

        asset_manager* am = nullptr;
        if(aname) am = vm->getaM(vmap, (const char*)aname);
        asset* ai = nullptr;
        if(aname) ai =  vm->getaI(vmap, (const char*)aname);
        if(1)FPS_PRINT_INFO("looking for amap [{}] [{}] am {} ai {}", uri, aname?aname:"no aname", fmt::ptr(am), fmt::ptr(ai));
        if(am || ai)
        {
            cjbody = cJSON_Parse(body);
        }
        if(nmaps>2)
        {
            for (auto x : am->amap)
            { 
                bool match = (bool)(x.first == amVec[2]);
                if(nmaps>3)
                {
                    if(1)FPS_PRINT_INFO("nmaps [{}] item [{}] filt [{}] match [{}]"
                        , nmaps
                        , x.first
                        , (nmaps>2)?amVec[2]:"none"
                        , match //(bool)(x.first == amVec[2]) 
                        );
                }

                if(0)FPS_PRINT_INFO("nmaps [{}] item [{}] filt [{}] match [{}]"
                            , nmaps
                            , x.first
                            , (nmaps>2)?amVec[2]:"none"
                            , match //(bool)(x.first == amVec[2]) 
                            );
                

                if (match) 
                {
                    //if(nmaps <=3)
                    {
                        if(1)FPS_PRINT_INFO("     nmaps [{}] item [{}] {} filt [{}] match [{}] body [{}] [{}]"
                            , nmaps
                            , x.first
                            , fmt::ptr(x.second)
                            , (nmaps>2)?amVec[2]:"none"
                            , (bool)(x.first == amVec[2]) 
                            , body
                            , fmt::ptr(cjbody)
                            );
                    }

                    //if(x.second) x.second->showvarCJ(cjii, opts);
                    if(x.second && cjbody) vm->setValfromCj (vmap, x.second->getfName(),nullptr, cjbody);
                    //deepDive (cjii, vm, am, amVec[2], opts);
                }
            }
        }
    }
    if(cjbody) cJSON_Delete(cjbody);
    return nullptr;

}
// tag this on here
cJSON*getAmap(VarMapUtils*vm, varsmap &vmap, char*uri, int opts)
{
    std::vector<std::string>amVec;
    int nmaps = 0;
    vm->uriSplit(amVec, uri);
    nmaps = (int)amVec.size();
    if(0)FPS_PRINT_INFO("uri  [{}] nmaps [{}]  ", uri, nmaps);
    for (int i = 0 ; i < nmaps; i++)
    {
        if(0)FPS_PRINT_INFO("uri  [{}] idx [{}] item [{}] ", uri, i, amVec[i]);
    }
    if(nmaps == 0)
    {
        return nullptr;
    }
    //int opts = 0;
    if(amVec[0] == "amap")   // strncmp(uri,"/amap/", strlen("/amap/"))==0)
    {
        const char* aname = nullptr;
        if (nmaps > 1)     //strdup(&uri[strlen("/amap/")]);
        {
            aname = amVec[1].c_str(); 
        }
        // now /amap/aname/item   we need to split the uri

        asset_manager* am = nullptr;
        if(aname) am = vm->getaM(vmap, (const char*)aname);
        asset* ai = nullptr;
        if(aname) ai =  vm->getaI(vmap, (const char*)aname);
        if(0)FPS_PRINT_INFO("looking for amap [{}] [{}] am {} ai {}", uri, aname?aname:"no aname", fmt::ptr(am), fmt::ptr(ai));
        if(am)
        {
            cJSON* cj = cJSON_CreateObject();
            cJSON* cjii = cJSON_CreateObject();
            opts |= 0x0010;

            for (auto x : am->amap)
            { 
                if(nmaps>2)
                {
                    bool match = false;
                    match = (bool)(x.first == amVec[2]);
                    if(nmaps>3)
                    {
                        if(1)FPS_PRINT_INFO("nmaps [{}] item [{}] filt [{}] match [{}]"
                            , nmaps
                            , x.first
                            , (nmaps>2)?amVec[2]:"none"
                            , match //(bool)(x.first == amVec[2]) 
                            );
                    }
                    if (match) 
                    {
                        if(nmaps <=3)
                        {
                            if(1)FPS_PRINT_INFO("     nmaps [{}] item [{}] {} filt [{}] match [{}]"
                                , nmaps
                                , x.first
                                , fmt::ptr(x.second)
                                , (nmaps>2)?amVec[2]:"none"
                                , (bool)(x.first == amVec[2]) 
                                );
                        }

                        if(x.second) x.second->showvarCJ(cjii, opts);
                        deepDive (cjii, vm, am, amVec[2], opts);
                    }
                }
                else
                {
                    if(x.second) x.second->showvarCJ(cjii, opts);
                }
            }
            if (nmaps>2)
            {
                auto xs = fmt::format("/amap/{}/{}", amVec[1], amVec[2]);
                cJSON_AddItemToObject(cj, xs.c_str(), cjii);
            }
            else
            {
                cJSON_AddItemToObject(cj,"amap", cjii);
            }
            if(nmaps < 3) 
            {
                char* tmp;
                vm->vmlen = asprintf(&tmp, "/links/%s", aname);
                if(vmap.find(tmp)!= vmap.end())
                {
                    cJSON* cjil = cJSON_CreateObject();

                    auto y = vmap[tmp];
                    for ( auto x : y)
                    {
                        if(0)FPS_PRINT_INFO("link [{}] {}", x.first, fmt::ptr(x.second));
                        if(x.second) x.second->showvarCJ(cjil, opts);
                    }
                    cJSON_AddItemToObject(cj,"links", cjil);
                }
                free(tmp);
                vm->vmlen = asprintf(&tmp, "/vlinks/%s", aname);
                if(vmap.find(tmp)!= vmap.end())
                {
                    cJSON* cjil = cJSON_CreateObject();

                    auto y = vmap[tmp];
                    for ( auto x : y)
                    {
                        if(0)FPS_PRINT_INFO("vlink [{}] {}", x.first, fmt::ptr(x.second));
                        if(x.second) x.second->showvarCJ(cjil, opts);
                    }
                    cJSON_AddItemToObject(cj,"vlinks", cjil);
                }
                free(tmp);        
            }
            //free(aname);
            //free(tmp);
            return cj;
        }
        if(ai)
        {
            cJSON* cj = cJSON_CreateObject();
            cJSON* cjii = cJSON_CreateObject();
            opts |= 0x0010;
            for ( auto x : ai->amap)
            {

                if(nmaps>2)
                {
                    bool match = false;
                    match = (bool)(x.first == amVec[2]);
                    if(nmaps>3)
                    {
                        if(1)FPS_PRINT_INFO("nmaps [{}] item [{}] filt [{}] match [{}]"
                                , nmaps
                                , x.first
                                , (nmaps>2)?amVec[2]:"none"
                                , match //(bool)(x.first == amVec[2]) 
                                );
                    }
                    if (match) 
                    {
                        if(1)FPS_PRINT_INFO("nmaps [{}] item [{}] {} filt [{}] match [{}]"
                            , nmaps
                            , x.first
                            , fmt::ptr(x.second)
                            , (nmaps>2)?amVec[2]:"none"
                            , match 
                            );

                        if(x.second) x.second->showvarCJ(cjii, opts);
                        //deepDive (cjii, vm, am, amVec[2], opts);
                    }
                }
                else
                {
                    if(x.second) x.second->showvarCJ(cjii, opts);
                }
            }
            cJSON_AddItemToObject(cj,aname, cjii);
            //free(aname);
            return cj;
        }
        //free(aname);
    }
    //else
    {
        cJSON* cj = cJSON_CreateObject();
        cJSON* cji = cJSON_CreateArray();
        int found=0;
        for ( auto x : vm->amMap)
        {
            found++;
            if(1)FPS_PRINT_INFO("aM [{}]", x.first);
            asset_manager* am = vm->getaM(vmap, x.first.c_str());
            if (am)
            {
                cJSON*cjam = cJSON_CreateObject();
                cJSON*cjaa = cJSON_CreateObject();
                cJSON_AddStringToObject(cjam,"name",x.first.c_str());
                if (am->am)
                    cJSON_AddStringToObject(cjam,"p-name", am->am->name.c_str());
                cJSON_AddNumberToObject(cjam,"aMs",am->assetManMap.size());
                cJSON_AddNumberToObject(cjam,"aIs",am->assetMap.size());
                if(nmaps == 2)
                {
                    cJSON* cjai = cJSON_CreateArray();
                    for (auto xx : am->amap)
                    {
                        cJSON_AddItemToArray(cjai,cJSON_CreateString(xx.first.c_str()));
                    }
                    cJSON_AddItemToObject(cjaa,"objects", cjai);
                }
                cJSON_AddItemToObject(cjaa,x.first.c_str(), cjam);
                cJSON_AddItemToArray(cji,cjaa);
            }
            else
            {
                cJSON_AddItemToArray(cji,cJSON_CreateString(x.first.c_str()));
            }
        }
        if(found)
        {
            cJSON_AddItemToObject(cj, "aMs", cji);
            cji = cJSON_CreateArray();
        }
        found=0;
        for ( auto x : vm->aiMap)
        {
            found++;
            if(1)FPS_PRINT_INFO("aI [{}]", x.first);
            asset* ai = vm->getaI(vmap, x.first.c_str());
            if (ai)
            {

                cJSON*cjam = cJSON_CreateObject();
                cJSON*cjaa = cJSON_CreateObject();
                cJSON_AddStringToObject(cjam,"name",x.first.c_str());
                if (ai->am)
                    cJSON_AddStringToObject(cjam,"p-name", ai->am->name.c_str());
                cJSON_AddItemToObject(cjaa,x.first.c_str(), cjam);
                if(nmaps == 2)
                {
                    cJSON* cjai = cJSON_CreateArray();
                    for (auto xx : ai->amap)
                    {
                        cJSON_AddItemToArray(cjai,cJSON_CreateString(xx.first.c_str()));
                    }
                    cJSON_AddItemToObject(cjaa,"objects", cjai);
                }
                cJSON_AddItemToArray(cji,cjaa);
            }
            else
            {
                cJSON_AddItemToArray(cji,cJSON_CreateString(x.first.c_str()));
            }

            //cJSON_AddItemToArray(cji,cJSON_CreateString(x.first.c_str()));
        }
        if(found)
        {
            cJSON_AddItemToObject(cj, "aIs", cji);
        }
        else
        {
            cJSON_Delete(cji);
        }
        return cj;
    }
    return nullptr;
}

//if(my.Var == nullptr) this is a reference to a full table (e.g. my.Uri is a full table)
int runLocks(varsmap &vmap, assetUri &my, const char* pname, bool lockState, assetVar* aV) // maybe void or bool? - return true if locks changed, false if locks not changed?
{

    bool debug = 0;
    if(aV->gotParam("debug"))
        {
            debug = aV->getbParam("debug");
        }
    if(debug)FPS_ERROR_PRINT("[%s] function called\n", __func__);
    if(debug)FPS_ERROR_PRINT("[%s] Changing lock status of [%s:%s@%s] to [%s]\n", 
        __func__,
        my.Uri?my.Uri:"no Uri",
        my.Var?my.Var:"no Var",
        pname?pname:"no parameter",
        lockState?"true":"false");
    if(aV->am)
    {
        essPerf ePerf(aV->am, (char*)aV->am->name.c_str(), "locks", nullptr);
    }
    else
    {
        if(debug)FPS_ERROR_PRINT("[%s] aV->am not found\n", __func__);
    }
	assetVar* localav = nullptr;

    if (my.Var == nullptr) //object to be locked is a whole table
    {
        for(auto& xx : vmap[my.Uri])
        {
            if(xx.second == nullptr)
            {
                continue;
            }
            xx.second->lock = lockState;
        }
    }
    else //object to be locked is a single assetVar or parameter
    {
        if(!aV->am)
        {
            if(debug)FPS_ERROR_PRINT("[%s] NOTE assetManager cannot be found for while locking elements in [%s:%s]\n", __func__, my.Uri, my.Var);
        }
        else if(!aV->am->vm)
        {
            if(debug)FPS_ERROR_PRINT("[%s] NOTE varMapUtils cannot be found for while locking elements in [%s:%s]\n", __func__, my.Uri, my.Var);
        }
        else
        {
            localav = aV->am->vm->getVar(vmap, my.Uri, my.Var);
        }
        if(localav != nullptr)
        {
            if(pname!=nullptr) //Object to be locked is a parameter
            {
                if(localav->extras)
                {
                    assetFeatDict* afdict = localav->extras->baseDict;  
                    auto& fmap = afdict->featMap;
                    if(fmap.find(pname) != fmap.end())
                    {
                        if(lockState)
                        {
                            fmap[pname]->lock = true;
                            fmap[pname]->unlock = false;
                        }
                        else
                        {
                            fmap[pname]->lock = false;
                            fmap[pname]->unlock = true;
                        }
                        if(debug)FPS_PRINT_INFO("[%s] updated locking for parameter [%s:%s@%s] to lock state [%s]\n", 
                            __func__, my.Uri, my.Var, pname,
                            lockState?"true":"false");
                        return 0;
                    }
                    else
                    {
                        if(debug)FPS_ERROR_PRINT("[%s] parameter [%s] not found in assetVar [%s:%s]\n", __func__, pname, my.Uri, my.Var);
                    }
                }
                else
                {
                    if(debug)FPS_ERROR_PRINT("[%s] extras not found in assetVar [%s:%s]\n", __func__, my.Uri, my.Var);
                }
            }
            else //object to be locked is an entire assetVar
            {
            localav->lock = lockState;
            return 0;
            }
        
        }
    }
    return 1;
}


int runAllLocks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    bool debug = false;
    const char* pname = nullptr;
    char* tblptr = nullptr;
    bool lockState = false;
    if(av->gotParam("debug"))
    {
        debug = av->getbParam("debug");
    }
    if(1)FPS_PRINT_INFO("function called for {}", cstr{av->getfName()});

    if (av-> gotParam("uri")) 
    {
        tblptr = av-> getcParam("uri");
        if(debug)FPS_PRINT_INFO("URI?? {}", tblptr);
    }
    else 
    {
        FPS_PRINT_ERROR("No Lock table found");
        return 0;
    }
    auto x = vmap.find(tblptr);
    {
        if (x == vmap.end())
        {
            if(debug)FPS_PRINT_ERROR("NOTE configuration locking table not found. No locks will be added at this time\n");
        }
        else
        {
            if(debug)FPS_PRINT_INFO("[{}] Configuration locking table found: [{}]\n", __func__, tblptr);
            for(auto& xx : vmap[tblptr]) //xx.first is object to be locked. xx.second is lockState. 
            {
                assetUri my(xx.first.c_str()); //my contains assetVar that is to be locked or contains a parameter to be locked. 
                if(debug)FPS_PRINT_INFO("[line 2747] assetUri my.Uri: [{}]. my.Var: [{}].\n", my.Uri?my.Uri:"no URI", my.Var?my.Var:"no Var");
                if(xx.second->extras)
                    {
                    assetFeatDict* lockDict = xx.second->extras->baseDict; //TODO gate this with xx.second->extras check. This does not get actions (vector in extras?) or options (optDict?). 
                    auto& lockParam = lockDict->featMap;
                    if(lockParam.empty())
                        {
                            if(debug)FPS_PRINT_INFO("NOTE no parameters found in {}. Locking assetVar or table\n",
                                xx.second);
                        }
                    else
                        {
                        for(auto& yy : lockParam)
                            {   
                                pname = yy.first.c_str();
                                if(xx.second->gotParam(pname))
                                {
                                    lockState = xx.second->getbParam(pname);
                                    runLocks(vmap, my, pname, lockState, av);
                                    lockState = false; //default behavior should be to unlock, so re-set lockState here. 
                                }
                                //lockParam(vmap, my, yy.first.c_str(), /*yy.second*/ true, *av); // yy.first is param name, yy.second is lock state
                            }
                        }
                    }
                lockState = xx.second->getbVal();
                if(debug)FPS_PRINT_INFO("NOTE configuration locking item, {}",xx.first.c_str());
                if(debug)FPS_PRINT_INFO("NOTE configuration locking uri, {}", my.Uri?my.Uri:"no URI");
                if(debug)FPS_PRINT_INFO("NOTE configuration locking var, {}", my.Var?my.Var:"no Var");
                if(debug)FPS_PRINT_INFO("NOTE configuration locking state, {}", lockState?"true":"false");
                runLocks(vmap, my, nullptr, lockState, av); //passing nullptr to pname indicates to runLocks() that this is an assetVar not a param
                lockState = false; 
            }
        }
    }
    return 0;
}