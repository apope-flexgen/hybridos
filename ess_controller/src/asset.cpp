#ifndef ASSET_CPP
#define ASSET_CPP

#include "asset.h"
#include "assetVar.h"
#include "assetFunc.h"
#include "varMapUtils.h"
#include "formatters.hpp"
/******************************************************
 *              
 *                 asset.h
 *  p.wilshire
 *  01/10/2021
 *  added setParam("foo[1]", true);
 *      sets a bit in a param
 * 
 *    
 ******************************************************/

/***********************************************
 *                 assetUri
 ***********************************************/
assetUri::assetUri(const char* uri, const char* var)
{
    Uri = strdup(uri);
    sUri = nullptr;
    origuri = strdup(uri);
    origvar = nullptr;
    vecUri = nullptr;
    if(var)origvar = strdup(var);
    nfrags = getNfrags();
    Var = nullptr;
    Param = nullptr;
    index = -1;
    setValue = true;
    setup();
}

assetUri::~assetUri()
{
    free((void*)Uri);
    if(sUri)free((void*)sUri);
    free((void*)origuri);
    if(origvar)free(origvar);
    if(vecUri)free(vecUri);
    uriVec.clear();
}

void assetUri::single(void)
{
    if(origuri)
    {
        sUri = strdup(origuri);
        char*sp = sUri;
        sVar = nullptr;
        while(*sp++)
        {
            if (*sp == '/')
            {
                sVar=sp;
            }
        }
        if(sVar)
        {
            *sVar = 0;
            sVar++;
        }
    }
}

void assetUri::setup(void)
{
    char *sp = strstr(Uri,(char*)":");
    if(sp)
    {
        *sp++ = 0;
        Var = sp;
    }
    else
    {
        Var = origvar;
    }
    if(Var)
    {
        sp = strstr(Var,(char *)"@");
        if(sp)
        {
            *sp++ = 0;
            Param = sp;
        }
    }
    else
    {
        sp = strstr(Uri,(char *)"@");
        if(sp)
        {
            *sp++ = 0;
            Param = sp;
        //recover Var
            while ((*sp != '/') && (sp != Uri)) sp--;
            if(*sp == '/')
            {
                *sp++ = 0;
                Var = sp;
            }
        }

    }
    // now search for an index in var /a/b/c:d[23]  
    if(Var)
    {
        sp = strstr(Var,(char *)"[");
        if(sp)
        {
            *sp++ = 0;
            index = (int)strtol(sp, nullptr, 10);
        }

    }
    if(Param)
    {
        sp = strstr(Param,(char *)"[");
        if(sp)
        {
            *sp++ = 0;
            index = (int)strtol(sp, nullptr, 10);
            printf(" Param sp remaining >>%s<< index %d\n", sp, index);

        }
    }
    if(!Var&&!Param)
    {
        sp = strstr(Uri,(char *)"[");
        if(sp)
        {
            *sp++ = 0;
            index = (int)strtol(sp, nullptr, 10);
            printf(" Param sp remaining >>%s<< index %d\n", sp, index);
            //recover Var
            while ((*sp != '/') && (sp != Uri)) sp--;
            if(*sp == '/')
            {
                *sp++ = 0;
                Var = sp;
            }
        }

    }
    // now search for an index in param /a/b/c:d[23]  
    

}

int assetUri::setupUriVec()
{
    if(uriVec.size() > 0)
        return (int)uriVec.size();
    vecUri = strdup(origuri);
    char *sp = vecUri;
    char *sp1;
    sp++;
    do
    {
        sp1 = strstr(sp,(char*)"/");
        if(sp1)
        {
            *sp1++ = 0;
            uriVec.push_back(sp);
            if (0) FPS_PRINT_INFO("push back [{}]", cstr{sp});
            sp = sp1;
        }
    } while(sp1);
    if(sp!=vecUri)
    {
        uriVec.push_back(sp);
        if (0) FPS_PRINT_INFO("push back last [{}]", cstr{sp});

    }
    return (int)uriVec.size();
}
// was pull_one_uri
char* assetUri::pullOneUri(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    return uriVec[idx];
}

//char* pull_pfrag(const char* uri, int idx);
char* assetUri::pullPfrag(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    if (idx == 0) return strdup(Uri);
    std::string res= "";
    int i = 0;
    while(i<idx && i<ix)
    {
        res += "/";
        res +=uriVec[i++];
    }

    char* sp = (char*)strdup(res.c_str());
    return sp;
}

char* assetUri::pullPvar(int idx)
{
    int ix = setupUriVec();
    if (idx > ix) return nullptr;
    if (idx == 0) return nullptr;

    std::string res= "";
    while(idx<ix)
    {
        res += "/";
        res +=uriVec[idx];
        idx++;
    }
    char* spr = (char*)res.c_str();

    char* sp = (char*)strdup(spr+1);
    return sp;
}

//char* pull_first_uri(const char* uri, int n = 1)
char* assetUri::pullFirstUri(int n)
{
    return pullPfrag(n);
}

//char* pull_last_uri(const char* uri, int n = 1)
char* assetUri::pullLastUri(int n)
{
    int ix = setupUriVec();
    if((ix - n) >= 0)
        return (char *)strdup(uriVec[ix-n]);
    return nullptr;
}

//char* pull_uri(const char* uri, int idx)
char* assetUri::pullUri(int idx)
{
    // needs to be freed
    return pullPfrag(idx);
}

int assetUri::getNfrags()
{
    int nfrags = 0;
    char* sp = origuri;
    while (*sp)
    {
        if (*sp++ == '/') nfrags++;
    }
    return nfrags;
}

/***********************************************
 *                 asset
 ***********************************************/
asset::asset() :asset("asset") {
}

asset::asset(const char* _name) {
    name = _name;
    run_init = nullptr;
    run_wakeup = nullptr;
    vm = &defvm;
    p_fims = nullptr;
    am = nullptr;
    vecs = nullptr;
    sendOK=true;
}

asset::~asset()
{
    FPS_PRINT_INFO("{}", name);
}

void asset::setAm(asset_manager* _am) {
    am = _am;
}

void asset::setName(const char* _name) {
    name = _name;
}

void asset::cfgwrite(const char* fname, const char* aname)
{
    //VarMapUtils vm;

    cJSON* cj = getConfig();
    vm->write_cjson(fname, cj);
    cJSON_Delete(cj);
}

// to be developed .. the comman will be a fims message to be sent to all assets by the asset manager
const char* asset::get_command(const char* dest, const char* cmd)
{
    std::string rstr = "{\"asset_name\":";
    std::cout << "asset_dest :" << dest << " got  command:\n" << cmd << "\n";
    rstr.append(name);
    rstr.append(",\"cmd\":\"");
    rstr.append(cmd);
    rstr.append("\"}");
    return strdup(rstr.c_str());
}

// configure the asset
// use a list of reps to modify the file
// now returns a -1 for faulure
int asset::configure(const char* fname, std::vector<std::pair<std::string, std::string>>* reps, asset_manager* am, asset* ai)
{
    //VarMapUtils vm;
    int rc = -1;
    // use the proper ess_controller dir
    FPS_PRINT_INFO("seeking fname [{}]", fname);    
    char* fileName = vm->getFileName(fname);
    FPS_PRINT_INFO("got fileName [{}]", cstr{fileName});    

    cJSON* cjbase = nullptr;
    if(fileName)
    {
        cjbase = vm->get_cjson(fileName, reps);
        free(fileName);
    }
    cJSON* cj = nullptr;
    if (cjbase) cj = cjbase->child;
    //const char* vname;
    while (cj)
    {
        // uri - cj->string
        // uri->body - cj->child
        //FPS_PRINT_INFO(" cj->string [%s] child [%p]\n", cj->string, (void *) cj->child);
        char* body = cJSON_Print(cj);
        if (0) FPS_PRINT_INFO("cj->string [{}] child [{}] body [{}]"
                , cstr{cj->string}, fmt::ptr(cj->child), cstr{body});

        char* buf = vm->fimsToBuffer("set", cj->string, nullptr, body);
        free((void*)body);
        fims_message* msg = vm->bufferToFims(buf);
        free((void*)buf);
        cJSON* cjb = nullptr;
        vm->processFims(*vmap, msg, &cjb, am, ai);
        vm->free_fims_message(msg);

        buf = cJSON_Print(cjb);
        if (cjb) cJSON_Delete(cjb);

        if (0) FPS_PRINT_INFO("configured [{}]", cstr{buf});
        free((void*)buf);

        cj = cj->next;
    }
    if (cjbase)
    {
        cJSON_Delete(cjbase);
        rc = 0;
    }
    return rc;
}

cJSON* asset::getConfig(const char* uri, const char* var)
{
    //VarMapUtils vm;
    if (vmap)
        return  vm->getMapsCj(*vmap, uri, var);
    return nullptr;
}

bool asset::free_message(fims_message* message)
{
    if (message == nullptr)
        return false;
    delete(message);
    return true;
}

void asset::cleanup(void)
{
    for (auto& x : amap)
    {
        delete x.second;
    }
    amap.clear();
}

varmap* asset::getBmap()
{
    return &amap;
}

void asset::setVmap(varsmap* _vmap)
{
    vmap = _vmap;
}
void asset::setPmap(varsmap* _vmap)
{
    pmap = _vmap;
}

void asset::setVm(VarMapUtils* _vm)
{
    vm = _vm;
}

// //int asset::Send(const char* method, const char*uri, const char*rep, const char* body)
// {
//     //p_fims->Send("pub", comp, nullptr, dvar);
//     int ival = p_fims->Send(method, uri, rep, body);
//     return ival;
//}
int asset::Send(const char* method, const char*uri, const char*resp, const char* body)
{
    if(!sendOK) return 0;
    int ival = 0;
    if(method && uri && body)
    {
        ival = p_fims->Send(method, uri, resp, body);
    }
    return ival;
}

int asset::Send(const char* method, assetVar*av, const char*uri, const char*rep, const char* body)
{
    if(!sendOK) return 0;
    char *tmp=nullptr;
    if(!uri)
        uri = av->comp.c_str();
    if(!body)
    {
        cJSON*cja = av->getValCJ();
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddItemToObject(cji,"value",cja);
        tmp = cJSON_PrintUnformatted(cji);
        body =  tmp;
        cJSON_Delete(cji);
    }
     //p_fims->Send("pub", comp, nullptr, dvar);
    int ival = -1;
    if(method)
        ival = p_fims->Send(method, uri, rep, body);
    if(tmp)free(tmp);

    return ival;
}

//asset_manager

/***********************************************
 *                 asset_manager
 ***********************************************/
asset_manager::asset_manager() {
    run_init = nullptr;
    run_wakeup = nullptr;
    reload = 2; // force an init.
    vm = nullptr; //&defvm;
    vecs = nullptr;
    amap.clear();
    sendOK=true;

    // new sched stuff
    wakeChan = nullptr;  // anyone can wake us up
    reqChan  = nullptr; // anyone add to sched items
    fimsChan   = nullptr; // anyone deliver fims messages.
    // schreqs    = nullptr;
    setup = 0;
    run_secs = 0;
    syscVec = nullptr;

}

asset_manager::asset_manager(const char* _name) : asset_manager() {
    // run_init = nullptr;
    // run_wakeup = nullptr;
    setName(_name);

}

asset_manager::~asset_manager()
{
    if (1) FPS_PRINT_INFO("asset manager running cleanup");
    cleanup();
}

void asset_manager::setVmap(varsmap* _vmap)
{
    if (0) FPS_PRINT_INFO("asset manager setting vmap");
    vmap = _vmap;
}
void asset_manager::setPmap(varsmap* _vmap)
{
    pmap = _vmap;
}

varsmap* asset_manager::getVmap()
{
    if (0) FPS_PRINT_INFO("asset manager getting vmap");
    return vmap;
}

void asset_manager::setAm(asset_manager* _am) 
{
    am = _am;
}

void asset_manager::setName(const char* _name) 
{
    name = _name;
}
//  "/templates/bms":        {
//                 "bms_1":   { 
//                                    "template":"bms_catl_template.json".
//                                    "subs":[
//                                       {"replace":"@@BMS_ID@@","with":"bms_1"},
//                                       {"replace":"@@BMS_IP@@","with":"192.168.1.114"}
//                                       ]
//                                   }
//            }
void asset_manager::debugConfig(asset* pc, const char* dmsg)
{
    // this a test for our config.
    cJSON* cjbm = pc->getConfig();
    char* res = cJSON_Print(cjbm);
    FPS_PRINT_INFO("{}{} <<< done", dmsg, cstr{res});
    free((void*)res);
    cJSON_Delete(cjbm);
}

// TODO after MVP remove the asset/asset manager split make them the same structure
void asset_manager::assconfigure(varsmap* vmap, const char* fname, const char* aname)
{
    //VarMapUtils vm; // use the asset_manager one

    //vm->getReps(const char *fname, const char *aname,)
    // use the proper ess_controller dir
    char* fileName = vm->getFileName(fname);
    cJSON* cjbase = nullptr;
    if(fileName)
    {
        cjbase = vm->get_cjson(fileName, nullptr);
        free(fileName);
    }
    //cJSON* cjbase = vm->get_cjson(fname, nullptr);
    if(!cjbase)
      return;

    char* assname = nullptr;
    asprintf(&assname, "/templates/%s", aname);

    //cJSON* cj = cjbase->child;
    cJSON* cja = cJSON_GetObjectItem(cjbase, assname);
    cja = cja->child;
    free((void*)assname);

    while (cja)
    {
        cJSON* cjsi;
        cJSON* cjt = cJSON_GetObjectItem(cja, "template");
        if (0) FPS_PRINT_INFO("found asset [{}]", cstr{cja->string});
        if (0) FPS_PRINT_INFO("found asset [{}] template [{}]", cstr{cja->string}, cjt->valuestring ? cjt->valuestring : "No template");

        cJSON* cjs = cJSON_GetObjectItem(cja, "subs");
        if (0) FPS_PRINT_INFO("found asset [{}] subs {} isArray {}"
            , cja->string, fmt::ptr(cjs)
            , cJSON_IsArray(cjs)
        );
        if (cJSON_IsArray(cjs))
        {
            std::vector<std::pair<std::string, std::string>> reps;

            cJSON_ArrayForEach(cjsi, cjs)
            {
                cJSON* cjsr = cJSON_GetObjectItem(cjsi, "replace");
                cJSON* cjsw = cJSON_GetObjectItem(cjsi, "with");

                if (cjsr && cjsw && cjsr->valuestring && cjsw->valuestring)
                {
                    if (1) FPS_PRINT_INFO("found asset [{}] sub from [{}] to [{}]"
                        , cstr{cja->string}
                        , cstr{cjsr->valuestring}
                        , cstr{cjsw->valuestring}
                        );
                    reps.push_back(std::make_pair(cjsr->valuestring, cjsw->valuestring));
                }
            }
            asset* ass = addInstance(cja->string);
            ass->setVmap(vmap);
            ass->setVm(vm);
            ass->p_fims = p_fims;

            ass->configure(cjt->valuestring, &reps);
        }
        cja = cja->next;
    }

    //assname = nullptr;
    asprintf(&assname, "/links/%s", aname);
    cja = cJSON_GetObjectItem(cjbase, assname);
    if (1) FPS_PRINT_INFO("seeking links [{}] in file [{}] cja {}", cstr{assname}, fname, fmt::ptr(cja));
    if(cja)
    {
        if (1) FPS_PRINT_INFO("found links [{}]", cstr{cja->string});
        amConfig(vmap, cja, this);
    }
    // configure_vmap

}


asset* asset_manager::addAsset(cJSON* cja, cJSON* cjt,
    std::vector<std::pair<std::string, std::string>>& reps, asset_manager* am)
{
    asset* ass = addInstance(cja->string);
    ass->setVmap(vmap);
    ass->vecs = vecs;
    ass->setVm(vm);
    ass->configure(cjt->valuestring, &reps, am, ass);
    //ass->configure(cjt->valuestring, reps, am, ass);
    return ass;
}

// TODO after MVP tidy up amConfig
int asset_manager::amConfig(varsmap* vmap, cJSON* cj, asset_manager* am)
{
    char* body = cJSON_Print(cj);
    if (1) FPS_PRINT_INFO("cj->string [{}] child [{}] body [{}]"
        , cstr{cj->string}
        , fmt::ptr(cj->child)
        , cstr{body}
        );

    char* buf = vm->fimsToBuffer("set", cj->string, nullptr, body);
    free((void*)body);
    fims_message* msg = vm->bufferToFims(buf);
    free((void*)buf);
    cJSON* cjb = nullptr;
    vm->processFims(*vmap, msg, &cjb, am, nullptr);
    vm->free_fims_message(msg);

    buf = cJSON_Print(cjb);
    if (cjb) cJSON_Delete(cjb);

    if (1) FPS_PRINT_INFO("configured [{}] [{}]", cstr{cj->string}, cstr{buf});
    if(buf)free(buf);
    return 0;
}
//Note syscVec is for Uri ordering
//This is the asset_manager config we need the wake up function and the asset manager pointer
// Used here if(ass_man->configure(&vmap, fname, aname, &syscVec, myAwake, ass_man) < 0)
int asset_manager::configure(varsmap* vmap, const char* fname, const char* aname, std::vector<char *>* syscVec, bool(*assWake)(asset*, int), asset_manager* am)
{
    //VarMapUtils vm; // use the asset_manager one
    int rc = -1;
    //vm->getReps(const char *fname, const char *aname,)
    //seeking
    FPS_PRINT_INFO("AM seeking fname [{}]", fname);    
    char* fileName = vm->getFileName(fname);
    FPS_PRINT_INFO("AM got fileName [{}]", cstr{fileName});    
    cJSON* cjbase = nullptr;
    if(fileName)
    {
        cjbase = vm->get_cjson(fileName, nullptr);
        free(fileName);
    }
    //cJSON* cjbase = vm->get_cjson(fname, nullptr);
    if (cjbase == nullptr)
    {
        if (1) FPS_PRINT_ERROR("error in fileName [{}] fname [{}]", cstr{fileName}, fname);
        return rc;
    }
    rc = 0;
    if (1) FPS_PRINT_INFO("incoming vmap [{}]", fmt::ptr(vmap));

    // More UI hacks
    if (syscVec)
    {
        //syscVec->push_back(aname);
        // HACK for UI
        char* tmp;
        asprintf(&tmp, "%s/summary", aname);
        syscVec->push_back(tmp); // we'll delete it later
        FPS_PRINT_INFO("addded [{}] to syscVec size {}", cstr{tmp}, syscVec->size());
    }

    cJSON* cji = cjbase->child;
    while (cji)
    {
        FPS_PRINT_INFO("Base child [{}] running amConfig", cstr{cji->string});
        amConfig(vmap, cji, am);
        cji = cji->next;
    }

    //FPS_PRINT_INFO("%s >>  configured [%s]\n",__func__, buf);

    char* sumname = nullptr;
    asprintf(&sumname, "/assets/%s/summary", aname);
    //cJSON* cj = cjbase->child;

    cJSON* cjs = cJSON_GetObjectItem(cjbase, sumname);
    if (cjs)
    {
        // amConfig(vmap, cjs, am);

        // char* cjtmp = cJSON_Print(cjs);
        // if (cjtmp)
        // {
        //     if (1)FPS_PRINT_INFO(" %s >> found [%s] cjc >>%s<< \n", __func__, sumname, cjtmp);
        //     free((void*)cjtmp);
        // }
        //cJSON_Delete(cjs);
    }
    if (sumname) free((void*)sumname);

    char* cfgname = nullptr;
    asprintf(&cfgname, "/config/%s", aname);
    //cJSON* cj = cjbase->child;

    cJSON* cjc = cJSON_GetObjectItem(cjbase, cfgname);
    if (cjc)
    {
        FPS_PRINT_INFO("config [{}] child [{}] running amConfig", cstr{cfgname}, cstr{cjc->string});

        amConfig(vmap, cjc, am);
        char* cjtmp = cJSON_Print(cjc);
        if (cjtmp)
        {
            if (1) FPS_PRINT_INFO("found /config/[{}] [{}] \n cjc >>{}<<", aname, cstr{cfgname}, cstr{cjtmp});
            free((void*)cjtmp);
        }
        //cJSON_Delete(cjc);
    }
    if (cfgname) free((void*)cfgname);

    char* assname = nullptr;
    asprintf(&assname, "/templates/%s", aname);


    //cJSON* cj = cjbase->child;
    cJSON* cja = cJSON_GetObjectItem(cjbase, assname);

    if (cja && cja->child)
        cja = cja->child;
    if (assname) free((void*)assname);

    while (cja)
    {
        cJSON* cjsi;
        cJSON* cjt = cJSON_GetObjectItem(cja, "template");
        if (cjt && cjt->valuestring)
        {
            char *tmp;
            asprintf(&tmp, "%s/%s", aname, cja->string);
            // std::string sname = aname;
            // sname += "/";
            // sname += cja->string;

            // save the list of assts in syscVec for ui pubs
            if (syscVec)syscVec->push_back(tmp);
            if (0) FPS_PRINT_INFO("found asset [{}]",cstr{cja->string});
            if (0) FPS_PRINT_INFO("found asset [{}] template [{}]", cstr{cja->string}, cjt->valuestring ? cjt->valuestring : "No template");

            cJSON* cjs = cJSON_GetObjectItem(cja, "subs");
            if (0) FPS_PRINT_INFO("found asset [{}] subs {} isArray {}"
                , cja->string, fmt::ptr(cjs)
                , cJSON_IsArray(cjs)
            );
            if (cJSON_IsArray(cjs))
            {
                std::vector<std::pair<std::string, std::string>> reps;
                reps.clear();


                cJSON_ArrayForEach(cjsi, cjs)
                {
                    cJSON* cjsr = cJSON_GetObjectItem(cjsi, "replace");
                    cJSON* cjsw = cJSON_GetObjectItem(cjsi, "with");

                    if (cjsr && cjsw && cjsr->valuestring && cjsw->valuestring)
                    {
                        if (1) FPS_PRINT_INFO("found asset [{}] sub from [{}] to [{}]"
                            , cstr{cja->string}
                            , cstr{cjsr->valuestring}
                            , cstr{cjsw->valuestring}
                            );
                        reps.push_back(std::make_pair(cjsr->valuestring, cjsw->valuestring));
                    }
                }
                // Here we create the asset 
                asset* ass = addAsset(cja, cjt, reps, am);
                ass->run_wakeup = assWake;
                ass->setVmap(vmap);
                ass->setVm(vm);
                ass->p_fims = p_fims;

                // watch this we transfer the vecs from  am->vecs
                if(am) ass->vecs = am->vecs;

                // for review
// TODO after MVP make full use of the Vlists for pubs subs and blocks
                int ccntai = 0;
                vm->getVList(*ass->vecs, *ass->vmap, ass->amap, ass->name.c_str(), "Pubs", ccntai);
                FPS_PRINT_INFO("{} found {} [Pubs]", ass->name, ccntai);
                vm->getVList(*ass->vecs, *ass->vmap, ass->amap, ass->name.c_str(), "Subs", ccntai);
                FPS_PRINT_INFO("{} found {} [Subs]", ass->name, ccntai);
                vm->getVList(*ass->vecs, *ass->vmap, ass->amap, ass->name.c_str(), "Blocks", ccntai);
                FPS_PRINT_INFO("{} found {} [Blocks]", ass->name, ccntai);
            }
        }
        cja = cja->next;
    }
    assname = nullptr;
    asprintf(&assname, "/links/%s", aname);
    cja = cJSON_GetObjectItem(cjbase, assname);
    if (0) FPS_PRINT_INFO("seeking links [{}] in file [{}] cja {}", assname, fname, fmt::ptr(cja));
    if(cja)
    {
        if (1) FPS_PRINT_INFO("found links [{}]", cstr{cja->string});
        //amConfig(vmap, cja, am);
    }
    free((void*)assname);
    // This wont work .. put them inito the template file . They will be inserted multiple tines but that's OK
    // asprintf(&assname, "/components/%s", aname);
    // asprintf(&assname, "/components");
    // cja = cJSON_GetObjectItem(cjbase, assname);
    // if (0)FPS_PRINT_INFO(" %s >> seeking components  [%s] in file [%s] cja %p\n", __func__, assname, fname, (void*) cja);
    // if(cja)
    // {
    //     if (1)FPS_PRINT_INFO(" %s >> found components  [%s]\n", __func__, cja->string);
    //     amConfig(vmap, cja, am);
    // }
    // free((void*)assname);
    if(cjbase)cJSON_Delete(cjbase);
    return rc;
}

// send a command to one or all the assets.
const char* asset_manager::send_command(const char* dest, const char* cmd)
{
    const char* res;
    std::string rstr = "";
    if (strcmp(dest, "all") == 0)
    {
        rstr.append("["); // its a list
        for (auto it = assetMap.begin(); it != assetMap.end(); ++it)
        {
            if (it != assetMap.begin())
                rstr.append(",");
            res = (it->second)->get_command(it->first.c_str(), cmd);
            rstr.append(res);

            std::cout << " Manager sent command  " << cmd << " to : " << it->first.c_str() << " res: [" << res << "] rstr: [" << rstr << "]\n";
            free((void*)res);
        }
        rstr.append("]");

    }
    else
    {
        auto it = assetMap.find(dest);
        if (it != assetMap.end()) {
            res = assetMap[dest]->get_command(dest, cmd);
            rstr.append(res);
            std::cout << " Manager sent command  " << cmd << " to : " << dest << " res: [" << res << "] rstr: [" << rstr.c_str() << "]\n";
            free((void*)res);
        }
        else
        {
            std::cout << " Manager dest  [" << dest << "] not in asset map\n";
            for (auto const& x : assetMap) {
                std::cout << "Item Name :[" << x.first << "]\n";
            }
        }
    }
    return rstr.c_str();
}

// a pure virtual function
// this has to be done in the target class to get the full asset type
// but do we need it
//virtual asset* addInstance(const char * name) = 0;
void asset_manager::mapInstance(asset* item, const char* _name)
{
    if (_name)
        item->setName(_name);
    auto it = assetMap.find(item->name);
    if (it == assetMap.end()) {
        FPS_PRINT_INFO("mapped instance {} OK", item->name);
        assetMap[item->name] = item;
        vm->setaI(*vmap, item->name.c_str(), item);
    }
    else
    {
        FPS_PRINT_ERROR("mapped instance %s FAILED", item->name);
    }
}

asset* asset_manager::addInstance(const char* _name)
{
    asset* item = new asset(_name);
    // if (item) {
    //     item->setAm(this);
    // }
    FPS_PRINT_INFO("added asset instance [{}] OK", _name);
    mapInstance(item, _name);
    item->am = this;

    return item;
}

asset* asset_manager::getInstance(const char* _name)
{
    auto it = assetMap.find(_name);
    if (it != assetMap.end()) {
        return assetMap[_name];
    }
    return nullptr;
}

int asset_manager::getNumAssets()
{
    return assetMap.size();
}
// set up the thread data systems.
void asset_manager::setupChannels()
{
    t_data.message_chan = &msgchan;
    m_data.message_chan = &msgchan;
    f_data.fims_chan = &fimschan;
}

cJSON* asset_manager::getConfig(const char* uri, const char* var)
{
    //VarMapUtils vm;
    if (vmap)
        return  vm->getMapsCj(*vmap, uri, var);
    return nullptr;
}
// sets up / configs amap and vars

bool asset_manager::runChildren(int wakeup)
{
    //return true;
    if (0) FPS_PRINT_INFO("%s Manager Loop >>>>>>>>>>> looking for child Asset Manager", name);

    if (1)
    {
        for (auto ix : assetManMap)
        {
            if (0) FPS_PRINT_INFO("{} Manager Loop >>>>>>>>>>> running for Asset Manager [{}]", name, ix.first);

            asset_manager* am2 = ix.second;  //am->getManAsset("bms");
            if (am2->run_wakeup)
            {
                if (0) FPS_PRINT_INFO("{} Manager Loop Wakeup >>>>>>>>>>> running for Asset Manager [{}]", name, ix.first);
                // first trigger the wakeup there is no thread in the lower leel managers
                    //am2->wakechan.put(wakeup);
                am2->run_wakeup(am2, wakeup);
            }
            else
            {
                FPS_PRINT_INFO("%s Manager Loop NO Wakeup >>>>>>>>>>> running for Asset Manager [{}]", name, ix.first);
            }

            //am2->wakechan.put(wakeup);
        }
    }
    if (1)
    {
        // now do the assets
        for (auto ix : assetMap)
        {
            asset* ass = ix.second; //am->getManAsset("bms");
            if (0) FPS_PRINT_INFO("{} ASSETS >>>>>>>>>>> running for Asset [{}]", name, ix.first);
// TODO after MVP remove the asset / asset manager wakeup structure is is deprecated 
            if (ass->run_wakeup)
                ass->run_wakeup(ass, wakeup);
        }
    }
    return true;
}
// this is the main wake up loop 
// we are cutting down the complexity of this since we have developed the "cascade " concept.
void asset_manager::manager_loop()
{
    int wakeup;
    // char *item3;
    // fims_message * msg;
    // int tnum = 0;
    char* pname;
    asprintf(&pname, "/asset/%s", name.c_str());
    if (0) FPS_PRINT_INFO("MANAGER LOOP Starting for [{}]", name);
    // you could use setvar to make sure its there
    // OOPS this look common again
    // any wake up comes here
    while (wakechan.get(wakeup, true)) {
        // then service the other channels 
        //
        if (0) FPS_PRINT_INFO("MANAGER LOOP Wakeup for [{}] wval {}", name, wakeup);

        // a wakeup of 0 means service the others
        // a wakeup if 1 means process the asset
        // a wakeup of 2 means pub the asset
        if (run_wakeup)
        {
            if (0) FPS_PRINT_INFO("MANAGER LOOP Run Wakeup for [{}] wval {}", name, wakeup);

            // SURELY this should all be in the wakeup code....
            // now do the under managers
            // cut out the auto stuff for now. It was too complex
            if (wakeup != 0)  // dont pass on controller level wakeups 
            {
                run_wakeup(this, wakeup);

                if (0)
                {
                    for (auto ix : assetManMap)
                    {
                        if (0) FPS_PRINT_INFO("{} Manager Loop >>>>>>>>>>> running for Asset Manager [{}]", name, ix.first);

                        asset_manager* am2 = ix.second;  //am->getManAsset("bms");
                        if (am2->run_wakeup)
                        {
                            if (0) FPS_PRINT_INFO("{} Manager Loop Wakeup >>>>>>>>>>> running for Asset Manager [{}]", name, ix.first);
                            // first trigger the wakeup there is no thread in the lower leel managers
                            //am2->wakechan.put(wakeup);
                            am2->run_wakeup(am2, wakeup);
                            am2->runChildren(wakeup);
                        }
                        else
                        {
                            FPS_PRINT_INFO("{} Manager Loop NO Wakeup >>>>>>>>>>> running for Asset Manager [{}]", name, ix.first);
                        }

                        //am2->wakechan.put(wakeup);
                    }
                }
                if (0)
                {

                    // now do the assets
                    for (auto ix : assetMap)
                    {
                        asset* ass = ix.second; //am->getManAsset("bms");
                        if (1) FPS_PRINT_INFO("{} ASSETS >>>>>>>>>>> running for Asset [{}]", name, ix.first);
                        if (ass->run_wakeup)
                            ass->run_wakeup(ass, wakeup);
                    }
                }
                if (wakeup == -1)
                {
                    // quit
                    FPS_PRINT_INFO("MANAGER LOOP {} QUITTING", name);
                    running = 0;
                    break;//return false;
                }
            }
        }
    }
    if (pname)free((void*)pname);
}

// manager time loop
void asset_manager::man_timer_loop()
{
    chan_data* td = &t_data;
    FPS_PRINT_INFO("MANAGER LOOP {} POLL WAKEUPS DELAY {}", name, td->delay);
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);

        //wakeval=1;  // process scan
        if (td->count++ > 10)
        {
            //wakeval = WAKE_LEVEL_PUB;  // run pubs
            td->wake_up_chan->put(WAKE_LEVEL_PUB);
            td->count = 0;
        }
        if (td->count >= 0)
        {
            if (0) FPS_PRINT_INFO("MANAGER LOOP {} POLL WAKEUPS", name);
            td->wake_up_chan->put(WAKE_LEVEL1);
            td->wake_up_chan->put(WAKE_LEVEL2);
            td->wake_up_chan->put(WAKE_LEVEL3);
            td->wake_up_chan->put(WAKE_LEVEL4);
        }
    }

    //pthread_exit(nullptr);
}

// asset timer loop
// Take the timed out asset from the top of the timer queue and throw it at the input channel.
// Queue upthe timer with the next entry
// 
// class Compare
// {
// public:
//     bool operator()(timeVar* t1, timeVar * t2)
//     {
//         return (t1->tt > t2->tt);
//     }
// }


// std::priority_queue<assetVar*, std::vector<assetVar*>, Compare > tqueue;

// // here is a simple test 
// void test_tqueue ()
// {
//     double c_time = get_time_dbl();
//     tqueue.push(new timeVar(c_time+ 0.5,5));
//     tqueue.push(new timeVar(c_time+ 0.1,1));
//     tqueue.push(new timeVar(c_time+ 0.8,8));
//     tqueue.push(new timeVar(c_time+ 1.5,15));

//     while (!tqueue.empty())
//     {
//         printf("xtt %f somestuff %d\n",tqueue.top()->tt,tqueue.top()->somestuff);
//         delete tqueue.top();
//         tqueue.pop();
//     }
// }
// manager time loop
// TODO after MVP remove ass_timer_loop it is deprecated
void asset_manager::ass_timer_loop()
{
    chan_data* td = &t_data;
    FPS_PRINT_INFO("ASSET LOOP {} POLL WAKEUPS DELAY {}", name, td->delay);
    while (*td->run)
    {
        // Note we could do that anyway with the main loop.
        //
        // We'll have a wake up FD as well (socketpair perhaps) to allow a retest of the queue after a new top guy appears 
        poll(nullptr, 0, td->delay);

        //wakeval=1;  // process scan
        if (td->count++ > 10)
        {
            //wakeval = WAKE_LEVEL_PUB;  // run pubs
            td->wake_up_chan->put(WAKE_LEVEL_PUB);
            td->count = 0;
        }
        if (td->count >= 0)
        {
            if (0) FPS_PRINT_INFO("MANAGER LOOP {} POLL WAKEUPS", name);
            td->wake_up_chan->put(WAKE_LEVEL1);
            td->wake_up_chan->put(WAKE_LEVEL2);
            td->wake_up_chan->put(WAKE_LEVEL3);
            td->wake_up_chan->put(WAKE_LEVEL4);
            //td->wake_up_chan->put(WAKE_TIMER);
        }
    }

    //pthread_exit(nullptr);
}

// virtual void man_timer_loop()
// {
//     chan_data* td = &t_data;
//     int wakeval;
//     while (*td->run)
//     {
//         poll(nullptr,0,td->delay);

//         wakeval=1;  // process scan
//         if (td->count++ > 10)
//         {
//             wakeval = 2;  // run pubs
//             td->count = 0;
//         }
//         td->wake_up_chan->put(wakeval);
//     }

//     //pthread_exit(nullptr);
// }
// we are going to wake up every 5 ms  td->delay may vary 
// only gpio_controller runs at 5 right now
// we'll have 5, 10,20 50, 100, 200, 500 and 1000 ms Wakeups 
// 
void asset_manager::timer_loop()
{
    chan_data* td = &t_data;
    vm->setTime();
// TODO after MVP remove asset_manager::timer_loop stuff
    int pubTime = 500/td->delay;
    int hspubTime = 50/td->delay;
    int wake1Time = 100/td->delay;
    int wake2Time = 200/td->delay;
    int wake3Time = 500/td->delay;
    int wake4Time = 1000/td->delay;
    int wake10Time = 10/td->delay;
    int wake20Time = 20/td->delay;
    int wake50Time = 50/td->delay;
    int wake100Time = 100/td->delay;
    int wake200Time = 200/td->delay;
    int wake500Time = 500/td->delay;
    int wake1000Time = 1000/td->delay;
    int nTime = 0;
    int tchange = 1;
    // base this off the delay
    //FPS_PRINT_INFO("%s %2.3f TIMER LOOP %s POLL WAKEUPS DELAY %d \n", __func__, vm->get_time_dbl(), name.c_str(), td->delay);
    while (*td->run)
        //int wakeval;
        while (*td->run)
        {
            poll(nullptr, 0, td->delay);
            td->count++;

            if (0) FPS_PRINT_INFO("count {} mod {} count500 {}"
                    , td->count
                    , (500/td->delay)
                    , td->count %(500/td->delay)
                    );

            //wakeval=1;  // process scan
            // if (td->count++ == 200)
            // {
            //     //wakeval = WAKE_LEVEL_PUB;  // run pubs
            //     td->wake_up_chan->put(WAKE_LEVEL_PUB);
            //     td->count = 0;
            // }
            if(td->delay == 5)
                td->wake_up_chan->put(WAKE_LEVEL5); // 5Ms
            //10/td>delay
            if ((td->count %(wake10Time))==0)
                td->wake_up_chan->put(WAKE_LEVEL10); // 10Ms
            if ((td->count %(wake20Time))==0)
                td->wake_up_chan->put(WAKE_LEVEL20); // 20Ms
            if ((td->count %(wake50Time))==0)
                td->wake_up_chan->put(WAKE_LEVEL50); // 50Ms
            if ((td->count %(wake100Time))==0)
                td->wake_up_chan->put(WAKE_LEVEL100); // 200Ms
            if ((td->count %(wake200Time))==0)
                td->wake_up_chan->put(WAKE_LEVEL200); // 200Ms
            if ((td->count %(wake500Time))==0)
            {
                td->wake_up_chan->put(WAKE_LEVEL500); // 500Ms
            }
            if ((td->count %(pubTime))==0)
            {
                td->wake_up_chan->put(WAKE_LEVEL_PUB);
            }
            if ((td->count %(hspubTime))==0)
            {
                td->wake_up_chan->put(WAKE_LEVEL_PUB_HS);
            }

            if ((td->count %(wake1000Time))==0)
            {
                if(tchange >0) tchange = 0;
                if(amap["PubTime"]) {
                    nTime = amap["PubTime"]->getiVal()/td->delay;
                    if(nTime != pubTime)
                    {
                        pubTime = nTime;
                        tchange++;
                    }
                }
                if(amap["hsPubTime"]) {
                    nTime = amap["hsPubTime"]->getiVal()/td->delay;
                    if(nTime != hspubTime)
                    {
                        hspubTime = nTime;
                        tchange++;
                    }
                }
                if(amap["Wake1Time"]) {
                    nTime = amap["Wake1Time"]->getiVal()/td->delay;
                    if(nTime != wake1Time)
                    {
                        wake1Time = nTime;
                        tchange++;
                    }
                }
                if(amap["Wake2Time"]) {
                    nTime = amap["Wake2Time"]->getiVal()/td->delay;
                    if(nTime != wake2Time)
                    {
                        wake2Time = nTime;
                        tchange++;
                    }
                }
                if(amap["Wake3Time"]) {
                    nTime = amap["Wake3Time"]->getiVal()/td->delay;
                    if(nTime != wake3Time)
                    {
                        wake3Time = nTime;
                        tchange++;
                    }
                }
                if(amap["Wake4Time"]) {
                    nTime = amap["Wake4Time"]->getiVal()/td->delay;
                    if(nTime != wake4Time)
                    {
                        wake4Time = nTime;
                        tchange++;
                    }
                }
                td->wake_up_chan->put(WAKE_LEVEL1000); // 1000Ms
            }
            if ((td->count %(wake1Time))==0)
            {
                if (tchange) FPS_PRINT_INFO("TIMER LOOP {} Wake1 Time", name);

                td->wake_up_chan->put(WAKE_LEVEL1); // 100Ms
            }
            if ((td->count %(wake1Time))==0)
            {
                if (tchange) FPS_PRINT_INFO("TIMER LOOP {} Wake1 Time", name);

                td->wake_up_chan->put(WAKE_LEVEL1); // 100Ms
            }
            if ((td->count %(wake2Time))==0)
            {
                if (tchange) FPS_PRINT_INFO("TIMER LOOP {} Wake2 Time", name);
                td->wake_up_chan->put(WAKE_LEVEL2); // 100Ms
            }
            if ((td->count %(wake3Time))==0)
            {
                if (tchange) FPS_PRINT_INFO("TIMER LOOP {} Wake3 Time", name);

                td->wake_up_chan->put(WAKE_LEVEL3); // 100Ms
            }
            if ((td->count %(wake4Time))==0)
            {
                if (tchange) FPS_PRINT_INFO("TIMER LOOP {} Wake4 Time", name);

                td->wake_up_chan->put(WAKE_LEVEL4);
            }

        }

    //pthread_exit(nullptr);
}

// this is the same as all others 
// put in lib

void  asset_manager::message_loop()
{
    char* mdata;
    chan_data* td = &m_data;
    while (*td->run)
    {
        poll(nullptr, 0, td->delay);

        asprintf(&mdata, " this is message number %d", td->count++);
        td->message_chan->put(mdata);
        td->wake_up_chan->put(0);
    }
    //pthread_exit(nullptr);
}

// needs cname , subs
void asset_manager::fims_loop()
{
    chan_data* td = &f_data;
// TODO after MVP remove asset_manager fims_loop 
    while (*td->run)
    {
        fims_message* msg = p_fims->Receive_Timeout(1000);
        if (msg)
        {
            if (strcmp(msg->method, "get") == 0)
            {
                if (0) FPS_PRINT_INFO("name {} GET uri [{}]"
                    , name, cstr{msg->uri});
            }
            if(*td->run) 
            {
                td->fims_chan->put(msg);
                td->wake_up_chan->put(0);   // but this did not get serviced immediatey
                if (run_wakeup)
                {
                    run_wakeup(this, 0);
                }
            }
            else
            {
                if (p_fims)p_fims->free_message(msg);
                 //p_fims->delete
            }
        }
    }
    FPS_PRINT_INFO("name {} fims shutting down"
        , name);
    // if(p_fims)delete p_fims;
    // p_fims = nullptr;
    //pthread_exit(nullptr);
}


void asset_manager::setFrom(asset_manager* base)
{
    am = base;
    vmap = base->vmap;
    vm = base->vm; 
    vecs = base->vecs;
    syscVec = base->syscVec;
    wakeChan = base->wakeChan;
    reqChan = base->reqChan;  
    vm->setaM(*vmap, name.c_str(), this);

}

void asset_manager::cleanup()
{
    if (1) FPS_PRINT_INFO("STARTING for {}", name);
    if (1) FPS_PRINT_INFO("STARTING for {} amap.size {}", name, amap.size());
    for (auto& x : amap)
    {
        assetVar*av = x.second;
        if (1) FPS_PRINT_INFO("Running (no delete) for {} item [{}] {}"
            , name
            , x.first
            , fmt::ptr(x.second)
            );

        if(0 && av) FPS_PRINT_INFO("Running (no delete) for {} item [{}] {} [{}:{}]"
            , name
            , x.first
            , fmt::ptr(x.second)
            , av ? av->comp.c_str() : "NoAvComp"
            , av ? av->name.c_str() : "NoAvName"
            );
        //if(x.second) delete x.second;
    }
    amap.clear();
    if (1) FPS_PRINT_INFO("{} assetMap.size {}", name, assetMap.size());
    for ( auto aa: assetManMap)
    {
        asset_manager* am = aa.second;
        if (1) FPS_PRINT_INFO("asset_manager [{}] ->> [{}]", aa.first, am->name);
        delete am;
    }
    if (1) FPS_PRINT_INFO("{} assetMap.size {}", name, assetMap.size());
    for ( auto aa: assetMap)
    {
        asset* ai = aa.second;
        if (1) FPS_PRINT_INFO("asset [{}] ->> [{}]", aa.first, ai->name);
        delete ai;
    }

    // note typedef std::map<std::string, std::vector<std::string>*>vecmap;
    // vecmap vecs .. somewhere in main so the cleanup is outside of asset_manager
    if(vecs)
    {
        auto vsize = vecs->size();
        if (1) FPS_PRINT_INFO("vecs->size {}", vsize);
        
        if (vsize > 0)
        {
            for (auto aa : *vecs)
            {
                {
                    std::vector<std::string>* vp = aa.second;
                    if (0) FPS_PRINT_INFO("vecs [{}] vp {} size {}", aa.first, fmt::ptr(vp), vp->size());
                    for (auto ap : *vp)
                    {
                        if (0) FPS_PRINT_INFO("vec [{}]", ap);

                    }
                }
            }
            //vecs->clear();
        }
        //delete vecs;
    }
    if (1) FPS_PRINT_INFO("DONE for {}", name);

}

//run the main_loop every 100 mS 
void asset_manager::run_manager(fims* _p_fims)
{
    p_fims = _p_fims;
    // t_data.run = &running;
    // t_data.count = 0;
    // t_data.delay = period;
    // t_data.wake_up_chan = &wakechan;
    FPS_PRINT_INFO("STARTING MANAGER LOOP for {}", name);
    manager_thread = std::thread(&asset_manager::manager_loop, this);
    FPS_PRINT_INFO(">MANAGER LOOP STARTED for {}", name);
    return;// t_data.cthread;
    //return run(&t_data, (void *(*)(void*))timer_loop,  running, period,  &wakechan);
}

//run the timer_loop every 100 mS now reduced to 10Ms
void asset_manager::run_timer(int period)
{
    t_data.run = &running;
    t_data.count = 0;
    t_data.delay = period;
    t_data.wake_up_chan = &wakechan;
    t_data.cthread = std::thread(&asset_manager::timer_loop, this);
    return;// t_data.cthread;
    //return run(&t_data, (void *(*)(void*))timer_loop,  running, period,  &wakechan);
}

//run the timer_loop every 100 mS
void asset_manager::run_message(int period)
{
    m_data.run = &running;
    m_data.count = 0;
    m_data.delay = period;
    m_data.wake_up_chan = &wakechan;
    m_data.cthread = std::thread(&asset_manager::message_loop, this);
    //return run(&m_data, (void *(*)(void*))message_loop,  running, period,  &wakechan);
    return;//m_data.cthread;
}


// run the message loop eery 1.5 seconds
//pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running, 1500,  &pcs_man->wakechan);

// the fims system will get pubs and create a varsMap for  the items.
//pcs_man->run_fims(&pcs_man->f_data, fims_loop,    &running, 1500,  &pcs_man->wakechan);
void asset_manager::run_fims(int period, char** subs, const char* name, int numSubs)
{
    FPS_PRINT_INFO("{} numSubs {}", name, numSubs);
    f_data.subs = subs;
    f_data.numSubs = numSubs;
    f_data.name = name;
    f_data.run = &running;
    f_data.count = 0;
    f_data.delay = period;
    f_data.wake_up_chan = &wakechan;
    f_data.cthread = std::thread(&asset_manager::fims_loop, this);
    return;// f_data.cthread;
    //return run(&f_data, (void *(*)(void*))fims_loop,  running, period,  &wakechan);
}

// this will 


varmap* asset_manager::getAmap()
{
    return &amap;
}

bool asset_manager::addManAsset(asset_manager* am, const char* name)
{
    assetManMap[name] = am;
    return true;
}

asset_manager* asset_manager::getManAsset(const char* name)
{
    auto ix = assetManMap.find(name);
    if (ix != assetManMap.end())
    {
        return assetManMap[name];
    }
    return nullptr;
}

void asset_manager::cfgwrite(const char* fname, const char* aname)
{

    cJSON* cj = nullptr;
    if (aname != nullptr)
    {
        //vm->getReps(const char *fname, const char *aname,)
    //cJSON* cjbase = vm->get_cjson(fname, nullptr);
    //    cj = vm->getConfig(aname);
    }
    else
    {
        //  cj = vm->getConfig();
    }
    if (cj)
    {
        vm->write_cjson(fname, cj);
        cJSON_Delete(cj);
    }
}

void asset_manager::setVm(VarMapUtils* _vm)
{
    vm = _vm;
}

// thse guys are the wrappers to enable runing sub managers mmanaers and / or children.
int asset_manager::cascadeAI(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am
    , int(*runAI)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am))
{
    return runAI(vmap, amap, aname, p_fims, am);
}
// call this is a method to pass control to sub managers 
// specify the call backs for submanagers  (runAM) or children (runAI) 
// either can be nullptr allowing a cascade to children only or to sub managers only.
int asset_manager::cascadeAM(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am
    , int(*runAM)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
    , int(*runAI)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
)
{
    if (am)
    {
        // assets are in assetMap sub managers are in assetManMap
        for (auto ix : am->assetManMap)
        {
            asset_manager* amc = ix.second;

            if (0) FPS_PRINT_INFO("cascading function to >>>>> Manager [{}]", amc->name);
            if (runAM)runAM(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            cascadeAM(vmap, amc->amap, amc->name.c_str(), p_fims, amc, runAM, runAI);

        }
        // run each of the asst INstances
        for (auto ix : am->assetMap)
        {
            asset* amc = ix.second;

            if (0) FPS_PRINT_INFO("cascading function to >>>> Asset [{}]", amc->name);

            //runAI(vmap, amc->amap,amc->name.c_str(), p_fims, amc);
            if (runAI)cascadeAI(vmap, amc->amap, amc->name.c_str(), p_fims, amc, runAI);

        }
    }
    return 0;
}

// int asset_manager::Send(const char* method, const char*uri, const char*rep, const char* body)
// {
//     //p_fims->Send("pub", comp, nullptr, dvar);
//     int ival = p_fims->Send(method, uri, rep, body);
//     return ival;
//}
int asset_manager::Send(const char* method, assetVar*av, const char*uri, const char*rep, const char* body)
{
    if(!sendOK) return 0;
    char *tmp=nullptr;
    if(!uri)
        uri = av->comp.c_str();
    if(!body && av)
    {
        cJSON*cja = av->getValCJ();
        cJSON* cji = cJSON_CreateObject();
        cJSON_AddItemToObject(cji,av->name.c_str(),cja);
        tmp = cJSON_PrintUnformatted(cji);
        body =  tmp;
        cJSON_Delete(cji);
    }
     //p_fims->Send("pub", comp, nullptr, dvar);
     int ival = -1;
     if (method && uri)
        ival = p_fims->Send(method, uri, rep, body);
    if(tmp)free(tmp);

    return ival;
}

int asset_manager::Send(const char* method, const char*uri, const char*resp, const char* body)
{
    if(!sendOK) return 0;
    int ival = 0;
    if(method && uri && body)
    {
        ival = p_fims->Send(method, uri, resp, body);
    }
    return ival;
}


#include <chrono>
#include <ctime>
int forceasTemplates()
{
    using namespace std::chrono;

   //ess_man = new asset_manager("ess_controller");
    varsmap vmap;
    VarMapUtils vm;
    asset_manager *am = new asset_manager("test");
    assetVar* av;
    am->am = nullptr;
    am->running = 1;
    char *cval=(char*)"1234";

    am->vmap = &vmap;
    am->vm = &vm;

    bool bval = false;
    double dval = 0.0;
    int ival = 0;

    am->amap["bval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "bval", bval);
    am->amap["ival"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "ival", ival);
    am->amap["dval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "dval", dval);
    am->amap["cval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "cval", cval);
    av = am->amap["dval"];
    am->amap["Av"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "Av", av);


    av->addVal(dval);
    av->addVal(ival);
    av->addVal(dval);

    av->subVal(dval);
    av->subVal(ival);
    av->subVal(dval);
    
    av->setVal(dval);
    av->setVal(bval);
    av->setVal(ival);
    av->setVal((const char*)cval);
    av->setVal((char*)cval);

    av->setLVal(dval);
    av->setLVal(bval);
    av->setLVal(ival);
    av->setLVal((const char*)cval);
    //assetBitField bf(1,2,nullptr,nullptr,nullptr);
    assetBitField bf2(nullptr);
    ival = bf2.getFeat("d",&ival);
    cval = bf2.getFeat("d",&cval);
     av->sendAlarm(av, nullptr, nullptr, 1);
      bval = av->valueChanged();
    bval = am->vm->valueChanged(dval,dval);
    //bval = am->vm->valueChanged(dval,ival);
    bval = am->vm->valueChanged(av, av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, bval, dval);
    return 0;
}

/**************************************************************************************************************************************************************/
#endif
