/*
 * p.wilshire
 * utility functions for making assetVars work within the vmap data area.
 *
 */
/*
TODO add rdtsc
*/
#ifndef VARMAPUTILS_CPP
#define VARMAPUTILS_CPP

#include <iomanip>
#include <stdint.h>
#include <sys/types.h>

#include "tscns.h"
#include <sys/stat.h>
#include <unistd.h>

#include "asset.h"
#include "assetVar.h"
#include "ess_utils.h"
#include "formatters.hpp"
#include "varMapUtils.h"

#define MAX_FIMS_MSG_SIZE 924200

int setvar_debug = 0;
int process_fims_debug = 0;

double g_setTime = 0.0;
int debug_action = 0;
int setvar_debug_asset = 0;

/******************************************************
 *
 *                 VarMapUtils.h
 *
 ******************************************************/

/***********************************************
 *                 VarMapUtils
 ***********************************************/

VarMapUtils::VarMapUtils()
{
    syscVec = nullptr;
    sysName = nullptr;  // do not free
    alarmId = 0;
    // set_base_time();
    base_time = 0.0;
    ref_time = set_time_ref(2021, 1, 28, 6, 11, 0);
    configDir = nullptr;
    runLogDir = nullptr;
    runCfgDir = nullptr;

    configFile = nullptr;
    vmapp = nullptr;
    funMapp = nullptr;
    argc = 0;
    vmdebug = 0;
    p_fims = nullptr;
    simdbi = false;
    noLog = false;
    idVec = nullptr;  // new std::vector<std::string>;
    autoLoad = false;
}

VarMapUtils::~VarMapUtils()
{
    // FPS_PRINT_INFO
    auto xx = fmt::format("{} >>>>>>>>>>>>>>>>>>>>>>> Shutting down VarMapUtils delmap size = {}", __func__,
                          delavMap.size());
    if (!noLog)
        printf("%s\n", xx.c_str());
    if (delavMap.size() > 0)
    {
        for (auto a : delavMap)
        {
            assetVar* av = a.first;
            if (av)
            {
                if (0)
                    FPS_PRINT_INFO("deleting av [{}:{}]", av->comp, av->name);
                delete av;
                if (0)
                    FPS_PRINT_INFO("deleted av", NULL);
            }
        }
        delavMap.clear();
        if (vmdebug)
            FPS_PRINT_INFO("cleared avMap", NULL);
        if (configFile)
            free(configFile);
        if (configDir)
            free(configDir);
        if (runLogDir)
            free(runLogDir);
        if (runCfgDir)
            free(runCfgDir);
        configFile = nullptr;
        configDir = nullptr;
        runLogDir = nullptr;
        runCfgDir = nullptr;
        if (vmdebug)
            FPS_PRINT_INFO("all done", NULL);
        if (sysName)
        {
            // FPS_PRINT_INFO(" clearing my name\n");
            free(sysName);
            sysName = nullptr;
        }
    }

    if (idVec)
    {
        // printf(" delete idVec... \n");
        // idVec->clear();
        delete idVec;
        // printf(" delete idVec .. done\n");
        idVec = nullptr;
    }
}

void VarMapUtils::setSysName(varsmap& vmap, const char* sname)
{
    asset_manager* am = getaM(vmap, sname);
    if (!am)
    {
        am = new asset_manager(sname);
        setaM(vmap, sname, am);
    }

    if (sysName == nullptr)
    {
        sysName = strdup(sname);
    }
    setVal(vmap, "/system/name", "ident", sysName);
}

// TODO use FlexName as default
char* VarMapUtils::getSysName(varsmap& vmap)
{
    if (!sysName)
    {
        setSysName(vmap, "ess");
    }
    return (char*)sysName;
}
// if the assetVar has "actions" then decode and run them
// this is the "func" action

// modified to allow an inVar to be used as athe assetVar

//
// useful test function .... keep it
int VarMapUtils::testRes(const char* tname, varsmap& vmap, const char* method, const char* uri, const char* body,
                         const char* res, asset_manager* am, asset* ai)
{
    UNUSED(am);
    UNUSED(ai);
    int rc = 1;
    char* tmp;
    // this is our map utils factory
    VarMapUtils vm;
    cJSON* cjr = nullptr;  // cJSON_CreateObject();
    const char* replyto = "/mee";
    char* buf = vm.fimsToBuffer(method, uri, replyto, body);
    // free((void *)body);
    fims_message* msg = vm.bufferToFims(buf);
    free((void*)buf);

    runFimsMsg(vmap, msg, nullptr, &cjr);
    // processFims(vmap, msg, &cjr, am, ai);
    free_fims_message(msg);
    tmp = cJSON_PrintUnformatted(cjr);
    if (tmp)
    {
        if (!res)
        {
            FPS_PRINT_INFO("{} Ok reply >>{}<<", tname, cstr{ tmp });
        }
        else if (strcmp(res, tmp) == 0)
        {
            // FPS_PRINT_INFO(" PASSED \n");
            FPS_PRINT_INFO("{} Ok PASSED reply >>{}<<", tname, cstr{ tmp });
        }
        else
        {
            unsigned int ires = strlen(res);
            unsigned int itmp = strlen(tmp);
            unsigned int iuse = itmp;

            if (iuse > ires)
            {
                iuse = ires;
            }

            for (unsigned int i = 1; i < iuse; i++)
            {
                if (strncmp(res, tmp, i) != 0)
                {
                    FPS_PRINT_INFO("test failed at loc {} tmp[{}] res[{}]", i, &tmp[i - 1], &res[i - 1]);
                    break;
                }
            }

            FPS_PRINT_INFO("{} FAILED reply >>{}<<", tname, cstr{ tmp });
            rc = 0;
        }
        free((void*)tmp);
    }
    cJSON_Delete(cjr);
    return rc;
}
// assets/pcs/summary"maint_mode": {
// actions":{"onSet":[{"remap":{"uri":"/assets/pcs/summary:start@enabled"},
//                            {"uri":"/assets/pcs/summary:stop@enabled"}
//                            }]}

// processes the following options
// "debug"    <true:false>          : turn off:on debug
// "enable"    <assetVar>          : an asset var used to enable the function if
// its true "ifChanged " ><true:false>      : if present and set to true
// valueChanged is used to trigger  the function "resetChange " ><true:false>
// : if present and set to true the ValueChange detection is reset after running
// actions " inValue "  :<somevalue>       : used to select a value match
// "ignoreValue" :<somevalue>      : used to select a value to ignore (This can
// be applied to the av as well) useRange :   <true:false>       : the value
// will be tested as a float  against a range " inValue+ "  :<somevalue>       :
// add to inValue for range test " inValue "  :<somevalue>       : subtract from
// invalue for range test "mask"  : <some value >          : shifted mask for
// value detect "shift"  : <some value >         : shift value detect "scale"  :
// <some value >          : used to scale value "offset"  : <some value >
// : used to offset value
//
//

// void VarMapUtils::setActions(varsmap& vmap, const char* fname)
// {
//     setAction(vmap, fname, "bitfield", (void*)nullptr,"decode value bits
//     into a number  of different values"); setAction(vmap, fname, "bitset",
//     (void*)nullptr,"set or clear a bit in an output var"); setAction(vmap,
//     fname, "enum", (void*)nullptr,"decode a value into a number of different
//     values"); setAction(vmap, fname, "remap", (void*)nullptr,"forward a value
//     to a different uri"); setAction(vmap, fname, "limit", (void*)nullptr,"set
//     limits on a value"); setAction(vmap, fname, "func", (void*)nullptr,"run a
//     func using an assetVar as an argument"); setAction(vmap, fname, "bitmap",
//     (void*)nullptr,"use a bitmap to set the output variable");
// //    setAction(vmap, fname, "clone", (void*)nullptr,"expand a template into
// a mapped system");
// }

// asset_manager* VarMapUtils::fixupPname(varsmap& vmap, const char* pname)
// {

//    std::vector<std::string>amVec;
//    std::string manName ="";
//    int nmaps = 0;
//    nmaps = uriSplit(amVec, pname);

//    FPS_PRINT_INFO(" %s >> >>>> pname [{}] nmps [%d]\n" ,
//                        pname, nmaps);

//    asset_manager* amx =  nullptr;//vm->getaM(vmap, spi);
//    asset_manager* amy =  getaM(vmap, amVec[0].c_str());
//    if(!amy)
//    {
//        amy = new asset_manager(amVec[0].c_str());
//        amy->am = nullptr;
//        amy->vmap = &vmap;
//        amy->vm = this;
//        amy->vecs = nullptr;
//        amy->syscVec = syscVec;
//        amy->wakeChan = nullptr; //base->wakeChan;
//        amy->reqChan = nullptr; //base->reqChan;
//        //amy->setFrom(amy);
//    }
//    if ((amy->am == nullptr) && (strcmp(amy->name, getSysName(vmap)) != 0))
//    {
//        amy->am = getaM(vmap, getSysName(vmap));
//        amy->setFrom(amy->am);
//        amy->addManAsset(amy, getSysName(vmap));
//    }
//    FPS_PRINT_INFO("%s >> >>>>>> uriSplit  nmaps [%d] pname [{}] \n",
//        nmaps, pname);
//    if(nmaps > 0)
//    {
//        for (int i = 1; i < nmaps; i++)
//        {
//            //manName += amVec[i];
//            manName = amVec[i];
//            const char* spi = manName.c_str();
//            amx =  getaM(vmap, spi);
//            if(!amx)
//            {
//                FPS_PRINT_INFO("%s >>  adding amap for [{}] amy->name [{}]\n"
//
//                    , spi
//                    , amy->name.c_str()
//                    );
//                amx = new asset_manager(spi);
//                amx->setFrom(amy);
//                amy->addManAsset(amx, spi);
//            }
//            else
//            {
//                FPS_PRINT_INFO("%s >>  found amap for [{}] ManMap size [%d]
//                \n"
//
//                , spi
//                , (int) amx->assetManMap.size()
//                );
//            }
//            amy = amx;
//            manName += "/";
//        }
//    }

//    return amy;
// }

// test code used in Clone
int VarMapUtils::fixupAMs(varsmap& vmap, fims* p_fims, cJSON* cjbase)
{
    FPS_PRINT_INFO("cjbase->string [{}] type {}", cstr{ cjbase->string }, cjbase->type);
    cJSON* cji = cjbase->child;
    while (cji)
    {
        FPS_PRINT_INFO("cji->string [{}], child->string []", cstr{ cji->string } /*,cji->child->string*/);
        cJSON* cjj = cji->child;
        while (cjj)
        {
            assetVar* av = getVar(vmap, cji->string, cjj->string);
            FPS_PRINT_INFO("cji->string [{}], cjj->string [{}] av {}", cstr{ cji->string }, cstr{ cjj->string },
                           fmt::ptr(av));
            std::vector<std::string> amVec;
            std::string manName = "";
            char* aman = av->getcParam("aman");
            char* aitem = av->getcParam("aitem");
            int nmaps = 0;
            if (aman != nullptr)
            {
                nmaps = uriSplit(amVec, aman);
            }
            else
            {
                FPS_PRINT_INFO("uriSplit av [{}] nmaps [{}] aitem [{}]", cstr{ av->getfName() }, nmaps, cstr{ aitem });
            }
            if (nmaps > 0)
            {
                FPS_PRINT_INFO("uriSplit nmaps [{}] aman [{}] map[0][{}] aitem [{}]", nmaps, cstr{ aman }, amVec[0],
                               cstr{ aitem });

                asset_manager* amx = nullptr;  // vm->getaM(vmap, spi);
                asset_manager* amy = getaM(vmap, amVec[0].c_str());
                if (!amy)
                {
                    amy = new asset_manager(amVec[0].c_str());
                    amy->am = nullptr;
                    amy->vmap = &vmap;
                    amy->vm = this;
                    amy->vecs = nullptr;
                    amy->syscVec = syscVec;
                    amy->wakeChan = nullptr;  // base->wakeChan;
                    amy->reqChan = nullptr;   // base->reqChan;
                                              // amy->setFrom(amy);
                }
                for (int i = 1; i < nmaps; i++)
                {
                    // manName += amVec[i];
                    manName = amVec[i];
                    const char* spi = manName.c_str();
                    amx = getaM(vmap, spi);
                    if (!amx)
                    {
                        FPS_PRINT_INFO("adding amap for [{}] amy->name [{}]", spi, amy->name);
                        amx = new asset_manager(spi);
                        amx->setFrom(amy);
                        amy->addManAsset(amx, spi);
                    }
                    else
                    {
                        FPS_PRINT_INFO("found amap for [{}] ManMap size [{}]", spi, amx->assetManMap.size());
                    }

                    amy = amx;
                    manName += "/";
                }
                av->am = amx;
                asset* ass = amx->addInstance(av->name.c_str());
                ass->av = av;
                FPS_PRINT_INFO("added instance [{}] assetMap size {}", av->name, amx->assetMap.size());
                ass->setVmap(&vmap);
                ass->setVm(this);
                ass->p_fims = p_fims;
            }

            cjj = cjj->next;
        }

        cji = cji->next;
    }

    // VarMapUtils *vm = baseAm->vm;
    // double r = rand()%10-5;
    cji = cjbase->child;
    while (cji)
    {
        FPS_PRINT_INFO("cji->string [{}], child->string [{}]", cstr{ cji->string }, cstr{ cji->child->string });
        if (0)
        {
            std::vector<std::string> amVec;
            std::string manName = "";
            assetVar* av = getVar(vmap, cji->string, cji->child->string);
            char* aman = av->getcParam("aman");
            char* aitem = av->getcParam("aitem");
            int nmaps = uriSplit(amVec, aman);
            FPS_PRINT_INFO("uriSplit nmaps [{}] aman [{}] map[0][{}] aitem [{}]", nmaps, cstr{ aman }, amVec[0],
                           cstr{ aitem });
            if (nmaps > 0)
            {
                asset_manager* amx = nullptr;  // vm->getaM(vmap, spi);
                asset_manager* amy = getaM(vmap, amVec[0].c_str());

                for (int i = 1; i < nmaps; i++)
                {
                    manName += amVec[i];
                    const char* spi = manName.c_str();
                    amx = getaM(vmap, spi);
                    if (!amx)
                    {
                        FPS_PRINT_INFO("adding amap for [{}] amy->name [{}]", spi, amy->name);
                        amx = new asset_manager(spi);
                        amx->setFrom(amy);
                        amy->addManAsset(amx, spi);
                    }
                    else
                    {
                        FPS_PRINT_INFO("found amap for [{}]", spi);
                    }
                    amy = amx;
                    manName += "/";
                }
                av->am = amx;
                asset* ass = amx->addInstance(av->name.c_str());
                FPS_PRINT_INFO("added instance [{}] assetMap size {}", av->name, amx->assetMap.size());
                ass->setVmap(&vmap);
                ass->setVm(this);
                ass->p_fims = p_fims;
            }
        }
        cji = cji->next;
    }
    return 0;
}
// new clone function
// source param is either another uri or a file
assetVar* VarMapUtils::runActClonefromCj(varsmap& vmap, assetVar* av, assetAction* aa, bool debug)
{
    // for now run the sets directly
    // std::map<int,assetBitField *> bitmap;
    // assetVal* aVal = av->linkVar?av->linkVar->aVal:av->aVal;
    // int aval = (int)aVal->valuedouble;
    asset_manager* am = av->am;
    bool srcfile = false;
    bool destfile = false;
    bool enabled = true;
    char* src = nullptr;
    if (av->gotParam("enabled"))
    {
        enabled = av->getbParam("enabled");
    }

    if (av->gotParam("src"))
    {
        src = av->getcParam("src");
    }
    char* dest = nullptr;
    if (av->gotParam("dest"))
    {
        dest = av->getcParam("dest");
    }
    if (src)
    {
        if (strncmp(src, "file:", strlen("file:")) == 0)
        {
            srcfile = true;
            src += strlen("file:");
        }
        if (strncmp(src, "uri:", strlen("uri:")) == 0)
        {
            srcfile = false;
            src += strlen("uri:");
        }
    }
    if (dest)
    {
        if (strncmp(dest, "file:", strlen("file:")) == 0)
        {
            destfile = true;
            dest += strlen("file:");
        }
        if (strncmp(dest, "uri:", strlen("uri:")) == 0)
        {
            destfile = false;
            dest += strlen("uri:");
        }
    }

    FPS_PRINT_INFO(
        "called av [{}] enabled [{}] src [{}] dest [{}] srcfile [{}] "
        "destfile [{}]",
        cstr{ av->getfName() }, enabled, src ? src : "Error no src", dest ? dest : "Use vmap dest", srcfile, destfile);

    if (!enabled)
    {
        FPS_PRINT_INFO("called av [{}] action not enabled", cstr{ av->getfName() });
    }
    char* srcdata = nullptr;
    cJSON* cjbase = nullptr;
    // now read the input file
    if (srcfile)
    {
        char* sfile = getFileName(src);
        if (sfile)
        {
            // read_file
            cjbase = get_cjson(sfile, nullptr);
            FPS_PRINT_INFO("called read sfile [{}] result [{}]", cstr{ sfile }, fmt::ptr(cjbase));
            free(sfile);
        }
        if (cjbase)
        {
            srcdata = cJSON_Print(cjbase);
            cJSON_Delete(cjbase);
        }
    }
    if (!srcdata)
    {
        FPS_PRINT_INFO("unable to find or parse file [{}]", cstr{ src });
        return av;
    }
    else
    {
        FPS_PRINT_INFO("read file [{}] srcdata [{}]", cstr{ src }, cstr{ srcdata });
    }
    // this decodes all the featDict [{...}] but we are not using any of that
    // each one of these is a possible multiple clone operation

    // so now we have the source
    // we modify the source and insert it into our temvm.
    // then extract the tmpvm as the new source and repeat.
    // we should probably start with a clean tmpvm for each round.

    for (auto& x : aa->Abitmap)
    {
        varsmap tmpvmap;

        assetBitField* abf = x.second;
        bool triggered = false;
        // processActOptions(vmap, av,  abf, __func__, debug);

        av->abf = abf;
        int from = 0;
        int step = 1;
        int to = 1;
        char* replace = nullptr;
        char* with = nullptr;

        bool gotfrom = abf->gotFeat("from");
        bool gotto = abf->gotFeat("to");
        bool gotstep = abf->gotFeat("step");
        bool gotreplace = abf->gotFeat("replace");
        bool gotwith = abf->gotFeat("with");
        if (gotfrom)
        {
            from = abf->getFeat("from", &from);
        }
        if (gotto)
        {
            to = abf->getFeat("to", &to);
        }
        if (gotstep)
        {
            step = abf->getFeat("step", &step);
        }
        if (gotreplace)
        {
            replace = abf->getFeat("replace", &replace);
        }
        if (gotwith)
        {
            with = abf->getFeat("with", &with);
        }
        if (triggered && gotwith && gotreplace)
        {
            // do this for each replace / with line
            for (int i = from; i <= to; i += step)
            {
                char* sp;
                vmlen = asprintf(&sp, with, i);
                if (debug)
                    FPS_PRINT_INFO("replacing [{}] with [{}]", cstr{ replace }, cstr{ sp });
                // modify srcdata
                const std::string inStr(srcdata);
                const std::string replaceStr(replace);
                const std::string withStr(sp);
                std::string repStr = run_replace(inStr, replaceStr, withStr);
                free(sp);

                if (debug)
                    FPS_PRINT_INFO("result [{}]", repStr);
                // TODO insert the modified string into tmpvmap
                // use loadvmap perhaps
                cJSON* cjbase = cJSON_Parse(repStr.c_str());
                configure_vmapCJ(tmpvmap, cjbase, am, nullptr);
                {
                    cJSON* cjx = getMapsCjFixed(tmpvmap);
                    FPS_PRINT_INFO("End of round result cjx->string [{}] cjx->child->string [{}]", cstr{ cjx->string },
                                   cstr{ cjx->child->string });
                    char* xsrcdata = cJSON_Print(cjx);
                    cJSON_Delete(cjx);

                    // reset tmpvmap
                    // clearVmap(tmpvmap);
                    FPS_PRINT_INFO("End of round result repStr [{}] xsrcdata [{}]", repStr, cstr{ xsrcdata });
                    free(xsrcdata);
                }
                // free(repStr);
                // always start with the input ( or extracted)  srcdata for these rounds
            }
            // extract new srcdata string from the stuffed tmpvmap
            // TODO because of the bug we have to "nurse" all the objects into the
            // vmap
            free(srcdata);
            cJSON* cj = getMapsCjFixed(tmpvmap);
            FPS_PRINT_INFO("End of round result cj->string [{}] cj->child->string [{}]", cstr{ cj->string },
                           cstr{ cj->child->string });
            srcdata = cJSON_Print(cj);
            cJSON_Delete(cj);

            // reset tmpvmap
            clearVmap(tmpvmap);
            FPS_PRINT_INFO("End of round result [{}]", cstr{ srcdata });
        }
    }
    if (srcdata)
    {
        cJSON* cjbase = cJSON_Parse(srcdata);
        configure_vmapCJ(vmap, cjbase, /*am*/ nullptr, nullptr, false);
        fixupAMs(vmap, nullptr, cjbase);
        cJSON_Delete(cjbase);
        free(srcdata);
    }
    return av;
}

// used in clone
std::string VarMapUtils::run_replace(const std::string& inStr, const std::string& replace, const std::string& with)
{
    std::string outStr = inStr;
    if (0)
        FPS_PRINT_INFO("Replacing [{}] with [{}] in file [{}]", replace, with, inStr);

    size_t start_pos = 0;
    while ((start_pos = outStr.find(replace, start_pos)) != std::string::npos)
    {
        outStr.replace(start_pos, replace.length(), with);
        start_pos += with.length();
    }
    return outStr;
}

// std::map<std::string,void *>
// typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
// typedef std::map<std::string, assetVar*> varmap;
void VarMapUtils::setAction(varsmap& vmap, const char* aname, const char* fname, void* func, const char* help)
{
    // return;
    // assetVar* av = (assetVar*)func;
    assetVar* av = (assetVar*)func;

    if (aname && fname)
    {
        if (!funMapp)
        {
            FPS_PRINT_INFO("Error no funMapp not adding aname [{}] fname [{}] to funMap av {}", aname, fname,
                           fmt::ptr(av));
            funMapp = &funMap;
            // return;
        }
        if (funMapp->find(aname) == funMapp->end())
        {
            FPS_PRINT_INFO("Note: adding aname [{}] to funMap av {}", aname, fmt::ptr(av));
        }
        const char* uri = "/system/actions";
        assetUri my(uri, fname);
        // add it in here for reference
        setVal(vmap, (const char*)my.Uri, (const char*)my.Var, help);
        //(*actMapp)[aname][fname] = av;// (assetVar*)func;
    }
}

// write a cJSON thing to a file
void VarMapUtils::write_cjson(const char* fname, cJSON* cj)
{
    if (0)
        FPS_PRINT_INFO("cj {} file {}", fmt::ptr(cj), fname);

    FILE* fp = nullptr;
    fp = fopen(fname, "w");
    if (fp == nullptr)
    {
        FPS_PRINT_INFO("Failed to open file {}", fname);
        return;
    }

    char* res = cJSON_Print(cj);
    if (res)
    {
        size_t bytes_written = fwrite(res, 1, strlen(res), fp);
        FPS_PRINT_INFO("Wrote {} bytes to file {}", bytes_written, fname);
        free(res);
    }
    fclose(fp);
}

// get a file and replace string patterns found
// replace xxx with yyyy
char* VarMapUtils::get_cjfile(const char* fname, std::vector<std::pair<std::string, std::string>>* reps)
{
    FILE* fp = nullptr;

    if (fname == nullptr)
    {
        FPS_PRINT_INFO("Failed to get the path of the config file", NULL);
        return nullptr;
    }
    fp = fopen(fname, "r");
    if (fp == nullptr)
    {
        FPS_PRINT_INFO("Failed to open file {}", fname);
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    long unsigned file_size = ftell(fp);
    rewind(fp);

    // create Configuration_file and read file in Configuration_file
    char* config_file = (char*)calloc(1, file_size + 1);
    if (config_file == nullptr)
    {
        FPS_PRINT_INFO("Memory allocation error for {} bytes", file_size);
        fclose(fp);
        return nullptr;
    }

    size_t bytes_read = fread(config_file, 1, file_size, fp);
    fclose(fp);
    if (bytes_read != file_size)
    {
        FPS_PRINT_INFO("Read size error bytes requested {} bytes got {}", file_size, bytes_read);
        free((void*)config_file);
        return nullptr;
    }
    // terminate
    config_file[bytes_read] = 0;

    if (reps)
    {
        std::string cstr = config_file;
        for (int i = 0; i < (int)reps->size(); i++)
        {
            auto x = reps->at(i);
            std::string fstr = x.first;
            std::string tstr = x.second;
            std::size_t start_pos = 0;
            FPS_PRINT_INFO("Replacing [{}] with [{}] in file [{}]", fstr, tstr, fname);

            while ((start_pos = cstr.find(fstr, start_pos)) != std::string::npos)
            {
                cstr.replace(start_pos, fstr.length(), tstr);
                start_pos += tstr.length();  // ...
            }
        }

        free((void*)config_file);
        config_file = strdup(cstr.c_str());
    }
    return config_file;
}

cJSON* VarMapUtils::get_cjson(const char* fname, std::vector<std::pair<std::string, std::string>>* reps)
{
    char* config_file = get_cjfile(fname, reps);
    cJSON* cj = nullptr;
    if (config_file)
    {
        cj = cJSON_Parse(config_file);
        free((void*)config_file);
    }
    if (cj == nullptr)
        FPS_PRINT_INFO("Invalid JSON object in file", NULL);
    return cj;
}

assetVar* VarMapUtils::makeVar(varsmap& vmap, const char* comp, const char* var, assetList* alist)
{
    UNUSED(alist);
    assetUri my(comp, var);
    if (0)
        FPS_PRINT_INFO("makeVar alist comp [{}] var [{}] my.Uri [{}] my.Var[{}] my.Param [{}]", comp, var, my.Uri,
                       my.Var, my.Param);

    assetVar* av = nullptr;
    bool tval = true;
    av = new assetVar(my.Var, my.Uri, tval);
    vmap[my.Uri][my.Var] = av;
    return av;
}

// create a duplicate link to the same av
assetVar* VarMapUtils::makeAVar(varsmap& vmap, const char* comp, const char* var, assetVar* av)
{
    assetUri my(comp, var);
    if (0)
        FPS_PRINT_INFO("makeVar for Av input av {}", fmt::ptr(av));
    if (0)
        FPS_PRINT_INFO(
            "makeAVar comp [{}] var [{}] my.Uri [{}] my.Var[{}] "
            "my.Param [{}] av {}",
            comp, var, my.Uri, my.Var, my.Param, fmt::ptr(av));

    vmap[my.Uri][my.Var] = av;
    return av;
}

// new code we want the linked var exposed
assetVar* VarMapUtils::replaceAv(varsmap& vmap, const char* comp, const char* var, assetVar* av)
{
    assetUri my(comp, var);

    if (0)
        FPS_PRINT_INFO(
            "replaceAv comp [{}] var [{}] my.Uri [{}] my.Var[{}] "
            "my.Param [{}] av {}",
            comp, var, my.Uri, my.Var, my.Param, fmt::ptr(av));

    assetVar* oldAv = getVar(vmap, comp, var);
    if (oldAv)
    {
        if (0)
            FPS_PRINT_INFO(
                "NOT removing comp [{}] var [{}] old AV comp[{}] name[{}] "
                "old  {} new {}",
                comp, var, oldAv->comp, oldAv->name, fmt::ptr(oldAv), fmt::ptr(av));

        // DONE make sure this is deleted
        delavMap.insert(std::pair<assetVar*, void*>(oldAv, nullptr));
    }
    vmap[my.Uri][my.Var] = av;
    return av;
}

// void setAmFunc(vmap, "comp", "/alarms", ass_man, void*func);
// vm->setAmFunc(vmap, "comp", "/alarms", aname, ass_man,
// (void*)&dummy_bms_alarm);

void VarMapUtils::setAmFunc(varsmap& vmap, const char* aname, const char* fname, const char* amname, asset_manager* am,
                            void* func)
{
    UNUSED(vmap);
    UNUSED(amname);
    UNUSED(am);
    if (aname && fname)
        funMap[aname][fname] = (assetVar*)func;
}
void VarMapUtils::setAvFunc(varsmap& vmap, const char* aname, const char* fname, const char* amname, assetVar* am,
                            void* func)
{
    UNUSED(vmap);
    UNUSED(amname);
    UNUSED(am);
    if (aname && fname)
        funMap[aname][fname] = (assetVar*)func;
}

// lets have a *funMap and see if that works better
// std::map<std::string,void *>
// typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
// typedef std::map<std::string, assetVar*> varmap;
void VarMapUtils::setFunc(varsmap& vmap, const char* aname, const char* fname, void* func)
{
    // return;
    assetVar* av = (assetVar*)func;

    if (aname && fname)
    {
        if (funMapp->find(aname) == funMapp->end())
        {
            FPS_PRINT_INFO("Note: adding aname [{}] to funMap av {}", aname, fmt::ptr(av));
            // varmap tmap;
            // tmap.insert(std::pair<std::string, assetVar*>(fname,av));
            // funMap.insert(std::pair<std::string,std::map<std::string,
            // assetVar*>>(aname,std::move(tmap)));
            // //tmap[fname] = av;
            // //funMap[aname] = tmap;
            // return;
        }
        //             std::map<std::string,void *> tmap;
        //             std::pair<std::string,void*> tpair =
        //             std::pair<std::string,void*>(fname,func);
        //  //mymap.insert ( std::pair<char,int>('a',100) );
        // //            tmap.insert(std::pair<std::string,
        // std::pair<std::string,void*>>(aname,) void*>(fname,func));
        //             tmap.insert(tpair);
        //             std::pair<std::string,std::pair<std::string,void*>> fpair =
        //             std::pair<std::string,std::pair<std::string,void*>>(aname,tmap);

        //             funMap[aname] = tmap;
        //             //funMap.insert(std::pair<std::string, std::map<std::string,
        //             void*>(aname,tmap));
        //         }
        const char* uri = "/system/functions";
        assetUri my(uri, fname);
        // add it in here for reference
        setVal(vmap, (const char*)my.Uri, (const char*)my.Var, func);

        (*funMapp)[aname][fname] = av;  // (assetVar*)func;
    }
}
//    ammap amMap;
//    aimap aiMap;
asset_manager* VarMapUtils::getaM(varsmap& vmap, const char* aname)
{
    UNUSED(vmap);
    if (0)
        FPS_PRINT_INFO("looking for asset manager  aname [{}]", aname);
    // If we use a module or shared library  then do that instead
    if (aname)
    {
        auto x = amMap.find(aname);
        if (x != amMap.end())
        {
            return amMap[aname];
        }
    }
    return nullptr;
}
asset* VarMapUtils::getaI(varsmap& vmap, const char* aname)
{
    UNUSED(vmap);
    if (0)
        FPS_PRINT_INFO("looking for asset instance aname [{}]", aname);
    // If we use a module or shared library  then do that instead
    if (aname)
    {
        auto x = aiMap.find(aname);
        if (x != aiMap.end())
        {
            return aiMap[aname];
        }
    }
    return nullptr;
}

void VarMapUtils::setaM(varsmap& vmap, const char* aname, asset_manager* am)
{
    UNUSED(vmap);
    amMap[aname] = am;
}

void VarMapUtils::setaI(varsmap& vmap, const char* aname, asset* ai)
{
    UNUSED(vmap);
    aiMap[aname] = ai;
}

void* VarMapUtils::getFunc(varsmap& vmap, const char* aname, const char* fname, assetVar* avi)
{
    UNUSED(avi);
    // varsmap &fm = *funMap;

    void* res = nullptr;
    if (0)
        FPS_PRINT_INFO("looking for fname [{}] in aname [{}] vmap {} this {} funMap {}", fname, aname, fmt::ptr(&vmap),
                       fmt::ptr(this), fmt::ptr(funMapp));
    // If we use a module or shared library  then do that instead
    if (!funMapp)
    {
        FPS_PRINT_ERROR("aname [{}] fname [{}] no funmap", aname, fname);
        funMapp = &funMap;
    }
    if (aname)
    {
        auto xx = funMapp->find(aname);
        if (xx != funMapp->end())
        {
            res = (*funMapp)[aname][fname];  // = (assetVar*)func;
        }
        else
        {
            if (0)
                FPS_PRINT_ERROR("aname [{}] not found in funmap {}", aname, fmt::ptr(funMapp));
        }
    }
    else
    {
        if (0)
            FPS_PRINT_ERROR("aname nullptr  fname [{}] funmap {}", fname, fmt::ptr(funMapp));
    }
    if (!res)
    {
        if (0)
            FPS_PRINT_ERROR("aname [{}] , fun [{}] not found in funmap {}", aname, fname, fmt::ptr(funMapp));
    }
    return res;
    // return (void*)res;
}

// template <class T>
// assetVar*setVal2(varsmap &vmap, const char* comp, const char* var, T &value)
// {

//     // only set it if we can find it
//     assetVar* av = setVal(vmap, comp, var, value);
//     if(av)
//     {
//         av->setVal(value);
//         return av;
//     }

//     return av;

// }

const char* VarMapUtils::getdefLink(const char* name, const char* base, const char* var, char* buf, int blen)
{
    snprintf(buf, blen, "%s/%s:%s", base, name, var);
    return (const char*)buf;
}

// new from FlexPack
asset_manager* VarMapUtils::fixupPname(varsmap& vmap, const char* pname)
{
    std::vector<std::string> amVec;
    std::string manName = "";
    std::string mpname = "";  // pname;
    int nmaps = 0;
    if (pname[0] != '/')
    {
        mpname = "/";
        mpname += pname;
    }
    else
    {
        mpname = pname;
    }
    // urisplit needs a leading '/'
    nmaps = uriSplit(amVec, mpname.c_str());

    FPS_PRINT_INFO(">>>> pname [{}] nmps [{}]", pname, nmaps);

    asset_manager* amx = nullptr;  // vm->getaM(vmap, spi);
    asset_manager* amy = getaM(vmap, amVec[0].c_str());
    if (!amy)
    {
        amy = new asset_manager(amVec[0].c_str());
        amy->am = nullptr;
        amy->vmap = &vmap;
        amy->vm = this;
        amy->vecs = nullptr;
        amy->syscVec = syscVec;
        amy->wakeChan = nullptr;  // base->wakeChan;
        amy->reqChan = nullptr;   // base->reqChan;
                                  // amy->setFrom(amy);
    }
    if ((amy->am == nullptr) && (strcmp(amy->name.c_str(), getSysName(vmap)) != 0))
    {
        amy->am = getaM(vmap, getSysName(vmap));
        amy->setFrom(amy->am);
        amy->addManAsset(amy, getSysName(vmap));
    }
    FPS_PRINT_INFO(">>>>>> uriSplit nmaps [{}] pname [{}]", nmaps, pname);
    if (nmaps > 0)
    {
        for (int i = 1; i < nmaps; i++)
        {
            // manName += amVec[i];
            manName = amVec[i];
            const char* spi = manName.c_str();
            amx = getaM(vmap, spi);
            if (!amx)
            {
                FPS_PRINT_INFO("adding amap for [{}] amy->name [{}]", spi, amy->name);
                amx = new asset_manager(spi);
                amx->setFrom(amy);
                amy->addManAsset(amx, spi);
            }
            else
            {
                FPS_PRINT_INFO("found amap for [{}] ManMap size [{}]", spi, amx->assetManMap.size()

                );
            }

            amy = amx;
            manName += "/";
        }
    }

    return amy;
}

void VarMapUtils::setLinks(varsmap& vmap, const char* aname)
{
    char* sp = nullptr;
    char* splink = nullptr;
    // asset_manager* am = nullptr;
    if (aname)
    {
        vmlen = asprintf(&splink, "/links/%s", aname);
    }
    else
    {
        vmlen = asprintf(&splink, "/%s", "links");
    }

    if (0)
        FPS_PRINT_INFO("splink [{}]", splink);

    for (auto& x : vmap)
    {
        if (0)
            FPS_PRINT_INFO("testing [{}] for splink [{}]", x.first, splink);

        if (strncmp(x.first.c_str(), splink, strlen(splink)) == 0)
        {
            FPS_PRINT_INFO("found [{}] splink [{}]", x.first, splink);
            sp = (char*)x.first.c_str();
            sp += strlen("/links/");
            asset_manager* am = getaM(vmap, (const char*)sp);
            FPS_PRINT_INFO("looking for amap [{}] got {}", sp, fmt::ptr(am));
            if (!am)
            {
                am = new asset_manager((const char*)sp);
                setaM(vmap, (const char*)sp, am);
            }
            FPS_PRINT_INFO("confirming amap [{}] got {} am->am {}", sp, fmt::ptr(am), fmt::ptr(am->am));
            // look for amap defined by sp or aname
            // if we dont have an amap then make one and look for a parent  pname
            // this can be a composite structure based in anywhere !!!
            // lav is the amap ref amap[vname]
            // if its in amap it has been made already
            // if not this may be an "outside" job.
            // Ie creating a link outside of a function "setup" or "reload"
            // if this is the case we need to collect all the link information inside
            // this av.
            //
            // we need name (valuestring)
            // amap  /links/<amap> or aname
            // we can also use pname to position th amap in the
            // system hierarchy
            // [/flex/]bms/rack_1/rack_1_module_1/rack_1_module_1_cell_1
            //
            // find/make the aav
            // if x.second is not in amap then make the linkvar
            for (auto& y : x.second)
            {
                assetVar* av = y.second;
                char* vname = av->getcVal();

                char* lname = nullptr;
                char* pname = nullptr;
                av->gotParam("linkvar");
                {
                    lname = av->getcParam("linkvar");
                }

                if (av->gotParam("pname"))
                {
                    pname = (char*)av->getcParam("pname");
                    asset_manager* amy = fixupPname(vmap, pname);
                    // am = new asset_manager(spi);
                    am->setFrom(amy);
                    // amy->addManAsset(am, spi);
                    amy->addManAsset(am, sp);
                    FPS_PRINT_INFO("amy->name [{}] pname [{}] sp [{}]", amy->name, pname, sp);
                }
                FPS_PRINT_INFO("links [{}] linkvar [{}] amap [{}] pname [{}]", x.first, lname ? lname : "no linkvar",
                               am->name, pname ? pname : "no pname");
                const char* tsp = y.first.c_str();
                assetVar* tav = nullptr;  // this is the target  in components
                assetVar* lav = nullptr;  // this is the (optional) linked  in linkvar
                // do we already have an amap entry ??
                // if not make one and point to target
                double dval = 0.0;
                tav = getVar(vmap, vname, nullptr);
                if (!tav)
                {
                    FPS_PRINT_INFO("have to create target var [{}]", vname);
                    if (av->gotParam("default"))
                    {
                        // new function to set a value from a param
                        tav = setVal(vmap, vname, nullptr, av, "default");
                    }
                    else
                    {
                        tav = makeVar(vmap, vname, nullptr, dval);
                    }
                }
                am->amap[tsp] = tav;
                FPS_PRINT_INFO(" linked [{}] as tsp [{}] ", tav->getfName(), tsp);

                // problem no one created amap["BMSFaultSeen"]
                // we have to use linkvar which would normally be ignored
                if (lname)
                {
                    double dval = 0.0;
                    lav = getVar(vmap, lname, nullptr);
                    if (!lav)
                    {
                        FPS_PRINT_INFO("have to create linkvar [{}]", lname);
                        // lav = makeVar(vmap, lname, nullptr, dval);
                        if (av->gotParam("default"))
                        {
                            // new function to set a value from a param
                            lav = setVal(vmap, lname, nullptr, av, "default");
                        }
                        else
                        {
                            lav = makeVar(vmap, lname, nullptr, dval);
                        }
                    }
                    FPS_PRINT_INFO(" setup linkvar [{}] for tsp [{}] ", lav->getfName(), tsp);
                    // where ever lav was pointing to its going to be lost
                    vmap[lav->comp][lav->name] = tav;
                    //                    am->amap[tsp] = tav;
                }

                // if(am->amap.find(tsp) == am->amap.end())
                // {
                //     // problem no one created amap["BMSFaultSeen"]
                //     // we have to use linkvar which would normally be ignored
                //     if(lname)
                //     {
                //         double dval = 0.0;
                //         tav = getVar(vmap, lname, nullptr);
                //         if(!tav)
                //         {
                //             FPS_PRINT_INFO("have to create linkvar [{}]" ,lname );
                //             tav = makeVar(vmap, lname, nullptr, dval);
                //         }
                //         FPS_PRINT_INFO(" found  linkvar [{}] for tsp [{}] ",
                //         tav->getfName(), tsp ); am->amap[tsp] = tav;
                //     }
                // }
                // tav's av will be replaced by lav
                // if(am->amap.find(tsp) != am->amap.end())
                // {
                //     tav = am->amap[tsp];
                // }

                // if(tav && vname)
                // {
                //     if (1)FPS_PRINT_INFO("on [{}] attempting to link value [{}] to
                //     link [{}]"
                //             , y.first
                //             , vname
                //             , lname
                //             );
                //     // now look for "/components/sbmu_1:bms_fault"
                //     // we must make one of the same type
                //     assetVar* lav = getVar(vmap, vname, nullptr);
                //     if (!lav)
                //     {
                //         cJSON* cjval = nullptr;
                //         // /status/bms:BMSFaultSeen
                //         // no target to var so make one  TODO get type
                //         av->gotParam("defval");
                //         {
                //             cjval = av->getCjParam("defval");
                //         }

                //         double dval = 0.0;

                //         lav = makeVar(vmap, vname, nullptr, dval);
                //         if (0)FPS_PRINT_INFO(
                //             "created target comp [{}] av now {} av [{}] cjval {}"
                //                         , vname, fmt::ptr(lav), av->getfName(),
                //                         fmt::ptr(cjval));
                //         if(cjval)lav->setCjVal(cjval, true);

                //     }
                //     //
                //     // create a duplicate link to the same av from comp var
                //     // this
                //     vmap[tav->comp][tav->name] = lav;

                //     if (1)FPS_PRINT_INFO(
                //         "now creating var for amap [{}] tav [{}] lav [{}]"
                //                 , am->name, tav->getfName(), lav->getfName());
                // }
            }
        }
    }
    if (splink)
        free(splink);
}

// vm.setVal2(vmap, link,"AcContactor",               sAcContactor);
// vm.setVal3(vmap, link,"AcContactor",               "/status", name.c_str());
// vm.setVal3(vmap, "/link/bms_1","AcContactor",               "/status",
// "AcContactor";
// assetVar*setVal3(varsmap &vmap, const char* comp, const char*var,  const
// char* base, const char * name)
// {
//     char buf[1024];
//     const char* value = getdefLink(name, base, var, buf, sizeof(buf));
//     return setVal2(vmap, comp, var, value);
// }
// links to the value from the config
// does not set a value
// HeartBeat                 = linkVal(vmap, link, "HeartBeat", ival);

template <class T>
assetVar* VarMapUtils::linkVal(varsmap& vmap, const char* comp, const char* var, T& defvalue)
{
    // FPS_PRINT_INFO(" %s looking for comp [{}] var [%]\n",comp,var);
    // this gets the link
    // comp may have a : so remove it
    char* comp1 = strdup(comp);
    char* sp = strstr(comp1, ":");
    if (sp)
        *sp = 0;

    assetVar* av = getVar(vmap, comp1, var);
    assetVal* aVal = av->linkVar ? av->linkVar->aVal : av->aVal;

    if (0)
        FPS_PRINT_INFO("looking for comp [{}] comp1 [{}] var [{}] got {} {}", comp, comp1, var, fmt::ptr(av),
                       av ? aVal->valuestring : "noval");
    free((void*)comp1);
    if (av)
    {
        if (aVal->valuestring)
        {
            char* comp2 = aVal->valuestring;
            char* comp3 = strdup(comp2);
            char* sp = strstr(comp3, ":");
            if (sp)
                *sp = 0;

            av = getVar(vmap, comp3, var);
            if (0)
                FPS_PRINT_INFO("now looking for linked comp3 [{}] var [{}] got {}", comp3, var, fmt::ptr(av));
            if (!av)
            {
                av = makeVar(vmap, comp3, var, defvalue);
                if (0)
                    FPS_PRINT_INFO("now created linked comp [{}] var [{}] av now {}", comp3, var, fmt::ptr(av));
            }
            free((void*)comp3);
            return av;
        }
        else
        {
            if (1)
                FPS_PRINT_INFO("rogue link comp [{}] var [{}] skipped", comp, var);
        }
    }
    // we cant find it so make it and set a default value
    av = makeVar(vmap, comp, var, defvalue);
    return av;
}
// creates a vlink from vname to lname
// this means that vname has its assetVal linked to lname using the linkVar
// thing
// "/vlinks/ess": {
//     "MinCellVolt": {
//     "value": "/site/ess:MinCellVolt",
//     "vlink": "/components/catl_mbmu_control_r:mbmu_min_cell_voltage"
// }

int VarMapUtils::findSocket(varsmap& vmap, char* host, int port)
{
    int sock = -1;
    char* vname;
    vmlen = asprintf(&vname, "/sockets/tcp:%s_%04d", host, port);
    assetVar* av = getVar(vmap, vname, nullptr);
    if (av)
    {
        sock = av->getiVal();
    }
    free(vname);
    return sock;
}

int VarMapUtils::setSocket(varsmap& vmap, char* host, int port, int sock)
{
    char* vname;
    vmlen = asprintf(&vname, "/sockets/tcp:%s_%04d", host, port);
    setVal(vmap, vname, nullptr, sock);
    free(vname);
    return 0;
}
// this is probably deprecated look for the flexpack solution
void VarMapUtils::setVLinks(varsmap& vmap, const char* aname, bool mkfrom, bool mkto)
{
    UNUSED(aname);
    for (auto& x : vmap)
    {
        if (strncmp(x.first.c_str(), "/vlinks", strlen("/vlinks")) == 0)
        {
            for (auto& y : x.second)
            {
                char* vname = y.second->getcVal();
                char* vlink = nullptr;
                if (y.second->gotParam("vlink"))
                {
                    vlink = y.second->getcParam("vlink");
                }
                if (0)
                    FPS_PRINT_INFO("on [{}] attempting to link value [{}] to vlink [{}] default {}", y.first,
                                   vname ? vname : "no Vname", vlink ? vlink : "no Vlink",
                                   y.second->gotParam("default"));

                if (vname)
                {
                    if (vlink)
                    {
                        // if (1)FPS_PRINT_INFO("%s >> on [{}] attempting  to link value
                        // [{}] \n"
                        //
                        //     , y.first.c_str()
                        //     , vname
                        //     );
                        // if (1)FPS_PRINT_INFO("%s >> on [{}] attempting  to   vlink
                        // [{}]\n"
                        //
                        //     , y.first.c_str()
                        //     , vlink
                        //     );
                        mkfrom = true;
                        mkto = true;
                        setVLink(vmap, vname, vlink, mkfrom, mkto, y.second);
                    }
                }
            }
        }
    }
}

// look for a file
// if it starts with "configs" and we have a configdir then replace configs with
// configdir

bool VarMapUtils::checkFileName(const char* fname)
{
    struct stat sb;
    memset(&sb, 0, sizeof(struct stat));

    stat(fname, &sb);
    if ((sb.st_mode & S_IFMT) == S_IFREG)
    {
        return true;  // = strdup(fstr.c_str());
    }
    return false;
}
char* VarMapUtils::getFileName(const char* fileName)
{
    struct stat sb;
    memset(&sb, 0, sizeof(struct stat));

    char* fname = nullptr;

    if (!configDir)
    {
        configDir = (char*)strdup("configs");
    }
    vmlen = asprintf(&fname, "%s/%s", configDir, fileName);

    if (0)
        FPS_PRINT_INFO("seeking fname [{}]", fname);
    stat(fname, &sb);
    if ((sb.st_mode & S_IFMT) == S_IFREG)
    {
        return fname;  // = strdup(fstr.c_str());
    }

    free(fname);
    return nullptr;
}

#include <libgen.h>

//    char *dirname(char *path);
//    char *basename(char *path);
//  path       dirname   basename
//        /usr/lib   /usr      lib
//        /usr/      /         usr
//        usr        .         usr
//        /          /         /
//        .          .         .
//        ..         .         ..

// used to extract the configDir and configFile from arg[1];
void VarMapUtils::setFname(const char* dirName)
{
    if (!dirName)
        return;
    if (configDir)
    {
        free(configDir);
    }
    configDir = strdup(dirName);
}

void VarMapUtils::setRunLog(const char* dirName)
{
    if (!dirName)
        return;
    runLogDir = strdup(dirName);
    char* tmp;
    vmlen = asprintf(&tmp, "sudo mkdir -p %s", runLogDir);
    if (argc != 2)
        system(tmp);
    free(tmp);
    vmlen = asprintf(&tmp, "sudo chmod a+rw %s", runLogDir);
    if (argc != 2)
        system(tmp);
    free(tmp);
}
void VarMapUtils::setRunCfg(const char* dirName)
{
    if (!dirName)
        return;
    runCfgDir = strdup(dirName);
    char* tmp;
    vmlen = asprintf(&tmp, "sudo mkdir -p %s", runCfgDir);
    if (argc != 2)
        system(tmp);
    free(tmp);
    vmlen = asprintf(&tmp, "sudo chmod a+rw %s", runCfgDir);
    if (argc != 2)
        system(tmp);
    free(tmp);
}

// probably deprecated
// amap["HandleLoadRequest"]      = vm.setVL(vmap, aname, "/controls",
// "HandleLoadRequest",         reload);
assetVar* VarMapUtils::setVLink(varsmap& vmap, const char* vname, const char* vlink, bool mkfrom, bool mkto,
                                assetVar* aV)
{
    // char* avar = var;
    // bool var = false;
    assetVar* link = getVar(vmap, vlink, nullptr);
    assetVar* av = getVar(vmap, vname, nullptr);
    // target assetvar  /components/pcs_pe:pmode_control
    double dval = 0.0;

    if (0)
        FPS_PRINT_INFO("link vname->linkVar [{}] {} to [{}] {} mkfrom [{}] mkto [{}]", vname, fmt::ptr(av), vlink,
                       fmt::ptr(link), mkfrom, mkto);
    if (!av)
    {
        if (1 || mkto)
        {
            if (aV && aV->gotParam("default"))
            {
                // new function to set a value from a param
                av = setVal(vmap, vname, nullptr, aV, "default");
            }
            else
            {
                av = makeVar(vmap, vname, nullptr, dval);
            }
        }
        if (!av)
        {
            if (1)
                FPS_PRINT_INFO(
                    ">>> xxx  Unable to link vname [{}] {} to  [{}] {}"
                    " please add incoming config var [{}]",
                    vname, fmt::ptr(av), vlink, fmt::ptr(link), vlink);
        }
    }
    if (!link)
    {
        if (1 || mkfrom)
        {
            if (aV && aV->gotParam("default"))
            {
                // new function to set a value from a param
                link = setVal(vmap, vlink, nullptr, aV, "default");
            }
            else
            {
                link = makeVar(vmap, vlink, nullptr, dval);
            }
        }
        if (!link)
        {
            if (1)
                FPS_PRINT_INFO(
                    ">>> yyy Unable to link vname [{}] {} to  [{}] {}"
                    " please add incoming config var [{}]",
                    vname, fmt::ptr(av), vlink, fmt::ptr(link), vlink);
        }
    }

    if ((av != nullptr) && (link != nullptr))
    {
        if (1)
            av->linkVar = link;
    }
    else
    {
        if (1)
            FPS_PRINT_INFO(
                ">>> Unable to link vname [{}] {} to  [{}] {} please add "
                "incoming config var [{}]",
                vname, fmt::ptr(av), vlink, fmt::ptr(link), vlink);
    }
    return av;
}

// links to the value from the config
// does not set a value
// HeartBeat                 = linkVal(vmap, link, "HeartBeat", ival);
// amap["HandleLoadRequest"]      = vm.setLinkVal(vmap, aname, "/controls",
// "HandleLoadRequest",         reload);
template <class T>
assetVar* VarMapUtils::setLinkVal(varsmap& vmap, const char* aname, const char* cname, const char* var, T& defvalue)
{
    // FPS_PRINT_INFO(" %s looking for comp [{}] var [%]\n",comp,var);
    // this gets the link
    // comp may have a : so remove it
    // char *comp1 = strdup(comp);
    // char *sp = strstr(comp1,":");
    // if(sp)
    //  *sp = 0;

    char* linkcomp = nullptr;
    char* linkvar = nullptr;
    char* comp3 = nullptr;
    char* acomp = nullptr;
    // char* avar = var;

    vmlen = asprintf(&linkcomp, "/links/%s", aname);
    // linked assetvar  /links/pcs
    vmlen = asprintf(&acomp, "%s/%s", cname, aname);
    // linked assetvar  /links/pcs
    assetVar* lav = getVar(vmap, linkcomp, var);
    // target assetvar  /components/pcs_pe:pmode_control
    assetVar* tav = nullptr;
    assetVar* aav = nullptr;
    // = getVar(vmap, acomp, var);

    // assetVal* laVal = lav->linkVar?lav->linkVar->aVal:lav->aVal;

    if (0)
        FPS_PRINT_INFO("looking for linkcomp [{}] cname [{}] var [{}] got {} {}", linkcomp, cname, var, fmt::ptr(lav),
                       lav ? lav->aVal->valuestring : "noval");
    if (!lav)
    {
        // aname = bms_1
        // cname = /params
        // var = LoadSetpoint
        // create default /links/bms_1/LoadSetpoint with a string  value of
        // /params/bms_1:LoadSetpoint
        vmlen = asprintf(&linkvar, "%s/%s:%s", cname, aname, var);
        lav = makeVar(vmap, linkcomp, var, linkvar);
        lav->setVal(linkvar);
        // laVal = lav->linkVar?lav->linkVar->aVal:lav->aVal;
        // if (0)FPS_PRINT_INFO(" %s >>created linkcomp [{}]  var [{}] linkvar [{}]
        // got {} %s\n"
        //
        //     , linkcomp
        //     , var
        //     , linkvar
        //     , (void*)lav
        //     , lav ? laVal->valuestring : "noval"
        // );
    }

    if (lav)
    {
        if (0)
            FPS_PRINT_INFO("found linkcomp [{}] var [{}] linkvar [{}] got {} {}", linkcomp, var, linkvar, fmt::ptr(lav),
                           lav ? lav->aVal->valuestring : "noval");

        // now see if the target variable exists
        // /components/pcs_pe:pmode_control
        if (lav->aVal->valuestring)
        {
            char* comp2 = lav->aVal->valuestring;
            comp3 = strdup(comp2);
            char* sp = strstr(comp3, ":");
            if (sp)
            {
                *sp = 0;
                sp++;
            }
            else
            {
                sp = (char*)var;
            }
            tav = getVar(vmap, comp3, sp);
            if (0)
                FPS_PRINT_INFO("now looking for link target comp3 [{}] var [{}] got {}", comp3, sp, fmt::ptr(tav));

            // no target to var so make one
            if (!tav)
            {
                tav = makeVar(vmap, comp3, sp, defvalue);
                if (0)
                    FPS_PRINT_INFO("created target comp [{}] var [{}] av now {}", comp3, sp, fmt::ptr(tav));
            }
        }
        // now look for /controls/pcs:pmode
        aav = getVar(vmap, acomp, var);
        if (0)
            FPS_PRINT_INFO("now looking for amap [{}] var [{}] got {}", acomp, var, fmt::ptr(aav));
        if (!aav)
        {
            aav = makeAVar(vmap, acomp, var, tav);
            if (0)
                FPS_PRINT_INFO("now creating var for amap [{}] var [{}] got {}", acomp, var, fmt::ptr(aav));
        }
        else
        {
            if (aav != tav)
            {
                if (0)
                    FPS_PRINT_INFO("now replacing aVar for amap [{}] var [{}] {} with target av {}", acomp, var,
                                   fmt::ptr(aav), fmt::ptr(tav));
                // amap assetvar /controls/pcs/pmode  ( will have the same av as the
                // taget av)
                aav = replaceAv(vmap, acomp, var, tav);
            }
            else
            {
                if (0)
                    FPS_PRINT_INFO(
                        "same var >>>> NOT replacing aVvar for amap [{}] var "
                        "[{}] with target av",
                        acomp, var);
            }
        }
    }
    else
    {
        FPS_PRINT_INFO("rogue link comp [{}] var [{}] skipped", linkcomp, var);
    }

    if (0)
        FPS_PRINT_INFO("link comp [{}] var [{}] completed", linkcomp, var);
    if (linkcomp)
        free((void*)linkcomp);
    if (linkvar)
        free((void*)linkvar);
    if (comp3)
        free((void*)comp3);
    if (acomp)
        free((void*)acomp);

    return tav;
}

// assets/pcs/summary"maint_mode": {
// actions":{"onSet":[{"remap":{"uri":"/assets/pcs/summary:start@enabled"},
//                            {"uri":"/assets/pcs/summary:stop@enabled"}
//                            }]}

// this most likely needs a rework, I don't think stringifying and comparing
// will help this is more of a design issue, when we upgrade this can be
// simplified: depending on when we move to C++20, we can rewrite a lot of logic
// in this controller in general using the new jsonVal important changes 070721
// we can , as an option, use inVar to specify an input av for the inValue test
// we can use aV instead of uri to use av->setCjVal that bypasses the actions
// opertions.
// bool assetVar::setCjVal(cJSON* cj, bool forceType)
// when remapping you can either use the inVar value (default) or, if
// "useAv":true then the old av invalue can be used. this is a massive "case
// switch" feature
// fims_send -m set -r /$$ -u /flex/full/system/commands/run '
//                    {"value":22,"uri":"/control/pubs:SendDb","after":2.0,"every":1.0,"offset":0,"debug":0}'
// setValfromCj(vmap,uri,nullptr, cJSON_Parse("{value":"{}"","uri":"{}" after /
// every))

// // needs uri, offset, after , every , for,  debug all packed into a dummy aV
// extern "C++" {
// int RunSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV);
// };

// handles the complexity of value = as well as naked ? sets
// assetVar* setActMapfromCj(assetVar* av, const char* act, const char* opt,
// cJSON* cjbf, cJSON* cj)
// {
//     if (0)FPS_PRINT_INFO(" %s >>  act [{}]  btype [{}]\n", act, opt);

//     assetAction* aact;
//     //Sets up the actMap dict for on set actions
//     // we need a vector under this
//     // new assetVec
//     // assetVec push_back assetAction
//     if (!av->actMap[act])
//     {
//         if (0)FPS_PRINT_INFO("%s >>  setting new actMap entry for act [{}]
//         opt [{}]  \n"
//
//             , act
//             , opt
//         );

//     }
//     aact = av->actMap[act] = new assetAction(opt);
//     //cJSON *cji;
//     //setActMapfromCj >>  setting act [onSet] opt [limits]   av->actMap size
//     1 if (0)FPS_PRINT_INFO("%s >>  setting act [{}] opt [{}]   av->actMap
//     size %d\n"
//
//         , act
//         , opt
//         , (int)av->actMap.size()
//     );
//     av = setActBitMapfromCj(av, aact, cjbf, cj);
//     // av->actMap[act]>push_back(aact);

//     return av;
// }
// // handles the complexity of value = as well as naked ? sets
assetVar* VarMapUtils::setActVecfromCj(assetVar* av, const char* act, const char* opt, cJSON* cjbf, cJSON* cj)
{
    if (0)
        FPS_PRINT_INFO("act [{}]  btype [{}]", act, opt);

    assetAction* aact;
    // Sets up the actMap dict for on set actions
    // we need a vector under this
    // new assetVec
    // assetVec push_back assetAction
    if (!av->extras)
    {
        av->extras = new assetExtras;
    }
    if (av->extras->actVec.find(act) == av->extras->actVec.end())
    {
        if (0)
            FPS_PRINT_INFO("setting new actVec entry for act [{}] opt [{}]"

                           ,
                           act, opt);
        // av->actVec[act] = std::vector<assetAction *>;
    }
    aact = new assetAction(opt);
    av->extras->actVec[act].push_back(aact);
    // cJSON *cji;
    // setActMapfromCj >>  setting act [onSet] opt [limits]   av->actMap size 1
    if (0)
        FPS_PRINT_INFO("setting act [{}] opt [{}] av->extras->actVec[act] size {}"

                       ,
                       act, opt, (int)av->extras->actVec[act].size());
    av = setActBitMapfromCj(av, aact, cjbf, cj);
    // av->actMap[act]>push_back(aact);

    return av;
}

assetVar* VarMapUtils::setActOptsfromCj(assetVar* av, const char* act, const char* opt, cJSON* cj)
{
    cJSON* cjact = cJSON_GetObjectItem(cj, act);
    if (cjact)
    {
        if (0)
            FPS_PRINT_INFO("act [{}] opt [{}] type {}", act, opt, cjact->type);
        if (cjact->type != cJSON_Array)
        {
            cJSON* cjopt = cJSON_GetObjectItem(cjact, opt);
            if (cjopt)
            {
                // av = setActMapfromCj(av, act, opt, cjopt, cj);
                av = setActVecfromCj(av, act, opt, cjopt, cj);
            }
        }
        else
        {
            cJSON* cji;
            cJSON_ArrayForEach(cji, cjact)
            {
                cJSON* cjopt = cJSON_GetObjectItem(cji, opt);
                if (cjopt)
                {
                    av = setActVecfromCj(av, act, opt, cjopt, cj);
                }
            }
        }
    }
    return av;
}

assetVar* VarMapUtils::setActOptsfromCjxx(assetVar* av, const char* act, const char* opt, cJSON* cjopt)
{
    // cJSON* cjact = cJSON_GetObjectItem(cj, act);
    // cJSON* cjopt = cJSON_GetObjectItem(cji, opt);  // this is incorrect we dont
    // need to search  INIT 4 had cjopt
    if (cjopt)
    {
        if (0)
            FPS_PRINT_INFO("2.1 {} act [{}] opt [{}] type {} next {}", fmt::ptr(cjopt), act, opt, cjopt->type,
                           fmt::ptr(cjopt->next));
        // THIS is the one but we should not repeat it.
        av = setActVecfromCj(av, act, opt, cjopt, nullptr);
        // cjopt = cjopt->next;
    }
    return av;
}
// sets up the options field for the action
// note this currently deletes any existing actions ans overwrites with new
// ones.

assetVar* VarMapUtils::setActOptsfromCj(assetVar* av, const char* act, cJSON* cj)
{
    if (0)
        FPS_PRINT_INFO("starting for av->name [{}], act [{}]"

                       ,
                       av->name.c_str(), act
                       //, opt
        );
    if (!av->extras)
    {
        av->extras = new assetExtras;
    }
    // if we already have actions then delete the stack
    if (av->extras->actVec.find(act) != av->extras->actVec.end())
    {
        if (0)
            FPS_PRINT_INFO("clearing actVec entry for act [{}]"

                           ,
                           act
                           //, opt
            );
        // delete av->actVec[act] =
        // std::vector<assetAction *>;
        // this deletes ALL old actions
        while (av->extras->actVec[act].size() > 0)
        {
            assetAction* aa = av->extras->actVec[act].back();
            if (aa)
                delete aa;  //
            av->extras->actVec[act].pop_back();
        }
    }

    if (0)
        FPS_PRINT_INFO("setting actions for name [{}] act [{}]", av->name, act);
    cJSON* cji = cj;
    while (cji)
    {
        if (0)
            FPS_PRINT_INFO("string [{}] child [{}]"  // child->string[{}]
                                                     // child->child->string[{}]
                                                     // child->child->next {}"

                           ,
                           cji->string, fmt::ptr(cji->child)
                           // , cji->child->string
                           // , cji->child->child->string
                           // , fmt::ptr(cji->child->child->next)
            );

        // dont walk past the actions
        if (strcmp(cji->string, "actions") != 0)
        {
            if (0)
                FPS_PRINT_INFO("break setting actions for name [{}] act [{}]", av->name, act);
            break;
        }

        // NOT Array
        // "maint_mode": {
        // "name": "Maintenance Mode",
        // "value": false,
        // "unit": "",
        // "scaler": 0,
        // "enabled": true,
        // "ui_type": "control",
        // "type": "enum_slider",
        // "actions":{
        //     "onSet":[ <<<< cji->child->string
        //         {"remap":   <<<cji->child->child
        //             [
        //                 {"uri":"/assets/bms/summary:start@enabled"},
        //                 {"uri":"/assets/bms/summary:stop@enabled"},
        //                 {"uri":"/assets/bms/summary:enter_standby@enabled"},
        //                 {"uri":"/assets/bms/summary:exit_standby@enabled"}
        //             ]
        //         }
        //     ]
        // },
        // ARRAY
        // "@@BMS_ID@@_warning_23": {
        //     "value": 0,
        //     "note":"Sbmu Warning table without degrees reg 3 Appendix 4",
        //     "actions": {
        //         "onSet": {
        //             "enum": [
        //                 { "shift": 0,"mask": 1,"inValue": 0,"uri":
        //                 "/fault/@@BMS_ID@@:TMS_communications", "outValue":
        //                 "Normal"}, { "shift": 0,"mask": 1,"inValue": 1,"uri":
        //                 "/fault/@@BMS_ID@@:TMS_communications", "outValue":
        //                 "Fault"}, { "shift": 2,"mask": 1,"inValue": 0,"uri":
        //                 "/fault/@@BMS_ID@@:TMS_mode_conflict", "outValue":
        //                 "Normal"}, { "shift": 2,"mask": 1,"inValue": 1,"uri":
        //                 "/fault/@@BMS_ID@@:TMS_mode_conflict", "outValue":
        //                 "Fault"}, { "shift": 8,"mask": 1,"inValue": 0,"uri":
        //                 "/fault/@@BMS_ID@@:rack_door", "outValue": "Normal"}, {
        //                 "shift": 8,"mask": 1,"inValue": 1,"uri":
        //                 "/fault/@@BMS_ID@@:rack_door", "outValue": "Fault"}
        //             ]
        //         }
        //     }
        // }
        // "start_grad_p": {
        //     "value": 10.0,
        //     "actions": {
        //         "onSet": {
        //             "limits": [{"low": 0.1,"high": 3000.0}],
        //             "remap": [{"bit": 0, "uri": "/components/pe_pcs","var":
        //             "pcs_start_grad_p"}]
        //         }
        //     }
        // }

        if (strcmp(cji->child->string, act) == 0)
        {
            if (cji->child)
            {
                if (cji->child->child)
                {
                    cJSON* cjy = cji->child->child;
                    while (cjy)
                    {
                        cJSON* cjx = cjy->child;
                        while (cjx)
                        {
                            if (0)
                                FPS_PRINT_INFO("INIT 4 >> {} string [{}] type [{}] next [{}] child {}"

                                               ,
                                               fmt::ptr(cjx), cjx->string, cjx->type, fmt::ptr(cjx->next),
                                               fmt::ptr(cjx->child));
                            setActOptsfromCjxx(av, act, cjx->string, cjx);

                            cjx = cjx->next;
                        }
                        cjy = cjy->next;
                    }
                }
            }
            //     cJSON* cji2 = cji->child->child;
            //     if(cji2->type == cJSON_Array)
            //     {
            //         if(0)FPS_PRINT_INFO("%s >>     >> >> act [{}] cji2 is an Array
            //         \n"
            //
            //                 , act
            //         );
            //         cJSON* cjii;
            //         cJSON_ArrayForEach(cjii, cji2)
            //         {
            //             char* tmp = cJSON_PrintUnformatted(cjii);
            //             if (0) FPS_PRINT_INFO("%s >> ..%s<<\n", tmp);
            //             free((void *)tmp);

            //         }
            //         cji2=nullptr;  // We dont handle this yet

            //     }
            //     else if(cji2->type == cJSON_Object)
            //     {
            //         if(0)FPS_PRINT_INFO("%s >>     >> >> act [{}] cji2 is an Object
            //         child func [{}]\n"
            //
            //                 , act
            //                 , cji2->child->string
            //         );
            //         cji2 = cji2->child;

            //     }

            //     while(cji2)
            //     {
            //         if(0)FPS_PRINT_INFO("%s >>     >> >> act [{}] cji2 string [{}]
            //         child [{}]\n"
            //
            //         , act
            //         , cji2->string
            //         , cji2->child
            //         );
            //         setActOptsfromCj(av, act, cji2->string, cj);

            //         cji2 = cji2->next;
            //     }
        }

        cji = cji->next;
    }

    if (0)
        FPS_PRINT_INFO("done setting actions for name [{}] act [{}]", av->name, act);

    // setActOptsfromCj(av, act, "bitfield", cj);
    // setActOptsfromCj(av, act, "enum", cj);
    // setActOptsfromCj(av, act, "func", cj);
    // setActOptsfromCj(av, act, "bitset", cj);
    // setActOptsfromCj(av, act, "limits", cj);
    // setActOptsfromCj(av, act, "remap", cj);
    return av;
}

// handles the complexity of value = as well as naked ? sets
// looks for onSet, onGet, onPub
// DONE preserve action order using a vec

assetVar* VarMapUtils::setActfromCj(assetVar* av, cJSON* cj)
{
    setActOptsfromCj(av, "onSet", cj);
    setActOptsfromCj(av, "onGet", cj);
    setActOptsfromCj(av, "onPub", cj);

    return av;
}

// handles the complexity of value = as well as naked ? sets
// int uiObject = 0 ; asset type
// uIobject = 1 means uiType
// uiObject = 2 means uIalarm / fault
//

// assetVar* xxsetOldValfromCj(varsmap& vmap, const char* comp, const char* var,
// cJSON* cj, int uiObject = 0);
// How do we designate a alarm/fault object  We see ui_type as an alarm in
// loadmap

assetVar* VarMapUtils::setParamfromCj(varsmap& vmap, assetUri& my, cJSON* cj, int uiObject)
{
    UNUSED(uiObject);
    assetVar* av = getVar(vmap, my.Uri, my.Var);
    if (av)
    {
        // cJSON*cjv = cJSON_GetObjectItem(cj,"value");
        // if (cjv) cj = cjv;
        // oh boy we have to run setParam lets hope we have a cj options
        if (my.index >= 0)
        {
            av->setParamIdxfromCj(my.Param, my.index, cj);
        }
        else
        {
            av->setParamfromCj(my.Param, cj);
        }
    }
    return av;
}

assetVar* VarMapUtils::setParamfromCj(varsmap& vmap, const char* comp, const char* var, const char* param, cJSON* cj,
                                      int uiObject)
{
    UNUSED(uiObject);
    assetVar* av = getVar(vmap, comp, var);
    if (av)
    {
        av->setParamfromCj(param, cj);
    }
    return av;
}

bool VarMapUtils::linkVar(assetVar* av, assetVar* avl, bool force)
{
    bool ok = false;
    if ((av->linkVar == nullptr) || force)
    {
        ok = true;
        av->linkVar = avl;
    }
    return ok;
}

assetVar* VarMapUtils::getVar(varmap& amap, const char* var)
{
    assetVar* av = nullptr;
    if (amap.find(var) != amap.end())
    {
        av = amap[var];
    }
    return av;
}

assetVar* VarMapUtils::getVar(varsmap& vmap, const char* comp, const char* var)
{
    if (!comp)
    {
        FPS_PRINT_ERROR(" must have a comp argument please fix", NULL);
        return nullptr;
    }
    if ((comp[0] != '/') && (comp[0] != '_'))
    {
        FPS_PRINT_ERROR(" comp [{}] must start with '/' or '_'  please fix", comp);
        // return nullptr;
    }

    assetUri my(comp, var);
    if (0)
        FPS_PRINT_INFO("getVar comp [{}] var [{}] my.Uri [{}] my.Var[{}] my.Param [{}]", comp ? comp : "no comp",
                       var ? var : "no var", my.Uri, my.Var ? my.Var : " no my.Var",
                       my.Param ? my.Param : "no my.Param");

    assetVar* av = nullptr;

    if (0)
        FPS_PRINT_INFO("looking for comp [{}] var [{}]", my.Uri, my.Var);

    if (vmap.size() > 0)
    {
        auto ic = vmap.find(my.Uri);
        if (ic == vmap.end())
        {
            if (0)
                FPS_PRINT_INFO("NOTE created comp [{}] var [{}]", my.Uri, my.Var);
        }
        if (ic != vmap.end())
        {
            if (0)
                FPS_PRINT_INFO("found comp [{}] looking for var [{}] size {}", my.Uri, my.Var, vmap[my.Uri].size());

            if (my.Var)
            {
                auto iv = vmap[my.Uri].find(my.Var);
                if (iv != vmap[my.Uri].end())
                {
                    av = vmap[my.Uri][my.Var];
                }
            }
        }
    }
    return av;
}

template <class T>
T VarMapUtils::getVar(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetVar* av = getVar(vmap, comp, var);
    if (av)
    {
        return av->getVal(value);
    }
    return value;
}

// rework
// given a list or URI's populate an incoming cj with the data  adding
// individual uris if needed options 0x0100 will produce a segmented list
//   for example inuri /assets
//               uri   /assets/bms/summary
// will produce {"bms":{"summary":{<data>}}}
//
// TODO baseVec review/rework after MVP
int VarMapUtils::baseVec(std::string& bs, std::vector<std::string>& buri, std::vector<std::string>& turi)
{
    int rc = 0;
    if (0)
        FPS_PRINT_INFO("sizes buri [{}] turi [{}]"

                       ,
                       buri.size(), turi.size());
    while ((rc < (int)buri.size()) && (rc < (int)turi.size()))
    {
        if (buri[rc] != turi[rc])
        {
            if (0)
                FPS_PRINT_INFO("uri [{}] furi [{}] break rc {}"

                               ,
                               buri[rc], turi[rc], rc);
            // rc = 0;  // flag difference
            break;
        }
        bs += "/";
        bs += buri[rc];
        rc++;
    }
    if ((rc == (int)buri.size()) && (rc == (int)turi.size()) && (rc > 0))
    {
        if (0)
            FPS_PRINT_INFO("uri [{}] furi [{}] identical  rc {}"

                           ,
                           buri[rc - 1], turi[rc - 1], rc);
        rc = 0;  // flag identical
    }

    return rc;
}

// TODO rework uriSplit  , move to assetUri after MVP
int VarMapUtils::uriSplit(std::vector<std::string>& uriVec, const char* _uri)
{
    int nfrags = 0;
    if (0)
        FPS_PRINT_INFO("uri [{}]  nfrags {}", _uri, nfrags);
    if (!_uri)
    {
        FPS_PRINT_ERROR("bad uri", NULL);
        return 0;
    }
    if (strlen(_uri) == 0)
    {
        FPS_PRINT_ERROR("bad uri length"

                        ,
                        NULL);
        return 0;
    }

    std::string uri = _uri;
    std::string key = "/";

    std::size_t startf;
    std::size_t endf = 0;
    do
    {
        if (endf == 0)
        {
            startf = 1;
        }
        else
        {
            std::string furi = uri.substr(startf, (endf - startf));
            if (0)
                FPS_PRINT_INFO("uri [{}] furi [{}]", _uri, furi);
            startf = endf + 1;
            uriVec.push_back(furi);
            nfrags++;
        }

    } while ((endf = uri.find(key, startf)) != std::string::npos);
    std::string furi = uri.substr(startf, (endf - startf));
    uriVec.push_back(furi);

    if (0)
        FPS_PRINT_INFO("last >> uri [{}] furi [{}]", _uri, furi);
    return nfrags;
}

int VarMapUtils::uriSplit(std::vector<std::string>& uriVec, const char* _uri, const char* _key)
{
    int nfrags = 0;
    if (0)
        FPS_PRINT_INFO("uri [{}]  nfrags {}", _uri, nfrags);
    if (!_uri)
    {
        FPS_PRINT_ERROR("bad uri", NULL);
        return 0;
    }
    if (strlen(_uri) == 0)
    {
        FPS_PRINT_ERROR("bad uri length"

                        ,
                        NULL);
        return 0;
    }

    std::string uri = _uri;
    // std::string key = "/";
    std::string key = _key;

    std::size_t startf;
    std::size_t endf = 0;
    do
    {
        if (endf == 0)
        {
            startf = 1;
        }
        else
        {
            std::string furi = uri.substr(startf, (endf - startf));
            if (0)
                FPS_PRINT_INFO("uri [{}] furi [{}]", _uri, furi);
            startf = endf + 1;
            uriVec.push_back(furi);
            nfrags++;
        }

    } while ((endf = uri.find(key, startf)) != std::string::npos);
    std::string furi = uri.substr(startf, (endf - startf));
    uriVec.push_back(furi);

    if (0)
        FPS_PRINT_INFO("last >> uri [{}] furi [{}]", _uri, furi);
    return nfrags;
}

// TODO Urgh createUriListCj is  UGLY needs review/rework adfter MVP
cJSON* VarMapUtils::createUriListCj(varsmap& vmap, std::string& bs, const char* inuri, cJSON* incj, int options,
                                    std::vector<std::string>& uriVec)
{
    // split uri up into strings
    std::vector<std::string> inVec;
    if (incj == nullptr)
    {
        incj = cJSON_CreateObject();
    }
    // int infrags =
    uriSplit(inVec, inuri);  //  inVec /status/bms   uriVec /status/bms_1,2,3,4 etc
    for (auto& x : uriVec)
    {
        const char* myuri = x.c_str();
        if (0)
            FPS_PRINT_INFO("uri [{}] uriVec [{}] opts {:#04x}", inuri, myuri, options);
        assetList* alist = getAlist(vmap, myuri);

        if (0)
            FPS_PRINT_INFO(
                ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj "
                "map for [{}] assetList {} cj {}"

                ,
                myuri, fmt::ptr(alist), fmt::ptr(incj));
        // we only need do this for options 0x0100
        if (!(options & 0x0100))
        {
            cJSON* cjii = cJSON_CreateObject();

            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    // TODO v1.1.0 check this put with linked vars
                    av = alist->avAt(ix++);
                    if (av)
                        av->showvarCJ(cjii, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)
                        FPS_PRINT_INFO("getting cj for [{}] av [{}] ", y.first, fmt::ptr(y.second));
                    // bug fix 03-10-2022
                    if (y.second)
                    {
                        y.second->showvarCJ(cjii, options, y.first.c_str());
                    }
                    else
                    {
                        FPS_PRINT_ERROR("MISSING AV for [{}:{}] av[{}]", x.c_str(), y.first, fmt::ptr(y.second));
                    }
                    if (0)
                        FPS_PRINT_INFO("got cj for [{}]", y.first);
                    // cJSON_AddItemToObject(cj,y.first,cji);
                }
            }
            cJSON_AddItemToObject(incj, myuri, cjii);
            continue;
        }
        std::vector<std::string> uVec;
        // int infrags =
        uriSplit(uVec, x.c_str());
        std::string bsx;
        int bvec = baseVec(bsx, inVec, uVec);
        bs = bsx;
        if (0)
            FPS_PRINT_INFO("inuri [{}] uriVec [{}] opts {:#04x} bvec {} bs[{}]", inuri, x, options, bvec, bs);
        cJSON* cji = incj;
        cJSON* cjii = nullptr;
        // we had a match but got more than one answer
        // need to step back and show the kids
        if (bvec == 0)
        {
            if (uVec.size() == 1)
            {
                cjii = cji;
            }
            else
            {
                cjii = cJSON_CreateObject();
            }

            // now run getMapsCj on x.c_str() into cjii
            if (0)
                FPS_PRINT_INFO("running getMapsCj [{}] into base node alist {} cji->string [{}]", x, fmt::ptr(alist),
                               cji->string ? cjii->string : "noname");
            // cjii = cJSON_CreateObject();

            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    // TODO v1.1.0 this may give the wrong names
                    av = alist->avAt(ix++);
                    if (av)
                        av->showvarCJ(cjii, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)
                        FPS_PRINT_INFO("getting cj for [{}]", y.first);
                    y.second->showvarCJ(cjii, options, y.first.c_str());
                    if (0)
                        FPS_PRINT_INFO("got cj for [{}]", y.first);
                    // cJSON_AddItemToObject(cj,y.first,cji);
                }
            }

            if (uVec.size() > 1)
            {
                cJSON_AddItemToObject(cji, uVec[uVec.size() - 1].c_str(), cjii);
            }
        }
        else if (bvec < (int)uVec.size())
        {
            if (0)
                FPS_PRINT_INFO("bvec small {} need to find / create trees", bvec);
            while (bvec < (int)uVec.size())
            {
                if (0)
                    FPS_PRINT_INFO("bvec small {}  find / create tree [{}] inuri [{}]", bvec, uVec[bvec], inuri);
                cjii = cJSON_GetObjectItem(cji, uVec[bvec].c_str());
                if (!cjii)
                {
                    if (0)
                        FPS_PRINT_INFO("bvec small {} create tree [{}] inuri [{}]", bvec, uVec[bvec], inuri);

                    cjii = cJSON_CreateObject();
                    cJSON_AddItemToObject(cji, uVec[bvec].c_str(), cjii);
                }
                // build tree
                cji = cjii;
                bvec++;
            }
            // now run getMapsCj on x.c_str() into cjii
            if (0)
                FPS_PRINT_INFO("running getMapsCj [{}] into end node", x);
            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    av = alist->avAt(ix++);
                    if (av)
                        av->showvarCJ(cjii, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)
                        FPS_PRINT_INFO("getting cj for [{}]", y.first);
                    y.second->showvarCJ(cjii, options, y.first.c_str());
                    if (0)
                        FPS_PRINT_INFO("got cj for [{}]", y.first);
                    // cJSON_AddItemToObject(cj,y.first,cji);
                }
            }
        }
        else
        {
            // now run getMapsCj on x.c_str() into cjii
            if (0)
                FPS_PRINT_INFO("running getMapsCj [{}] into end node alist {}", x, fmt::ptr(alist));
            if (alist)
            {
                unsigned int ix = 0;
                assetVar* av;
                do
                {
                    av = alist->avAt(ix++);
                    if (av)
                        av->showvarCJ(cji, options);
                } while (av);
            }
            else
            {
                for (auto& y : vmap[x.c_str()])
                {
                    if (0)
                        FPS_PRINT_INFO("getting cj for [{}]", y.first);
                    y.second->showvarCJ(cji, options, y.first.c_str());
                    if (0)
                        FPS_PRINT_INFO("got cj for [{}]", y.first);
                    // cJSON_AddItemToObject(cj,y.first,cji);
                }
            }
        }
    }
    if (0)
    {
        char* tmp = cJSON_Print(incj);
        if (tmp)
        {
            FPS_PRINT_INFO("incj at end >>{}<<", tmp);
            free((void*)tmp);
        }
    }
    return incj;
}

// 1/ create a list of stuff in varsmap
// 2/ put items in syscvec order ( if you have a syscVec)
// 3/ find  matches to the uri
// std::vector<std::string> nVec;
int VarMapUtils::createAssetListCj(varsmap& vmap, const char* uri, std::vector<char*>* syscVec, int opts,
                                   std::vector<std::string>& nVec)
{
    std::vector<std::string> xVec;
    std::vector<std::string> yVec;
    std::vector<std::string>* yVecp;
    std::vector<std::string> zVec;

    std::string yuri = uri;
    std::size_t found = yuri.find("/", 1);
    std::string ystart = uri;  //"/"+ yuri.substr(0,found+1);

    for (auto& x : vmap)
    {
        if (0 || process_fims_debug)
            FPS_PRINT_INFO("xVec push_back [{}]", x.first);
        xVec.push_back(x.first);
    }
    // if (1) FPS_PRINT_INFO("xVec size [{}] uri [{}]", xVec.size(), uri);
    yVecp = &xVec;
    if (0)
        FPS_PRINT_INFO("*yVecp #1 size [{}] syscVec->size [{}]", yVecp->size(), syscVec ? syscVec->size() : -1);

    int idx = 0;
    // now extract it from syscVec if we have one and we are running a uri quesy
    if (syscVec && syscVec->size() > 0 && (opts & 0x0100))
    {
        for (auto y : *syscVec)
        {
            if (0)
                FPS_PRINT_INFO("syscVec idx [{}] entry [{}]", idx++, y);

            std::string sy = y;
            // y == whole string
            // yend == last /thing
            found = sy.find_last_of("/");
            // std::cout << " path: " << str.substr(0,found) << '\n';
            std::string yend = sy.substr(found + 1);

            // awful UI hack for summary
            if (yend == "summary")
                yend = sy;

            if (0)
                FPS_PRINT_INFO("syscvec found {} item sy [{}] ystart [{}] yend [{}]", found, sy, ystart, yend);
            for (auto z : xVec)
            {
                if (0)
                    FPS_PRINT_INFO("syscvec y [{}] z [{}]", y, z);

                if ((y == z) || (z.find(yend) != std::string::npos))
                {
                    // if(z[0]!='_')
                    {
                        if (0)
                            FPS_PRINT_INFO(">> >> yVec push_back [{}]", z);
                        yVec.push_back(z);
                    }
                }
            }
            // nVec.push_back(x.first);
        }
        yVecp = &yVec;

        if (0)
            FPS_PRINT_INFO("yVec size [{}]", yVec.size());
    }
    // if(!syscVec)
    // {

    // }
    // if (1) FPS_PRINT_INFO("*yVecp #2 size [{}]", yVecp->size());

    for (auto& y : *yVecp)
    {
        std::string suri = uri;
        auto x = y.find(suri);
        // if (0) FPS_PRINT_INFO(">> >> suri [{}] yVec option [{}] x {}", suri, y,
        // x);
        if (x == 0)
        {
            // if (0) FPS_PRINT_INFO(">> >> >>nVec push_back [{}] suri [{}]", y,
            // suri);
            nVec.push_back(y);
        }
    }
    int rc = (int)nVec.size();
    // if (1) FPS_PRINT_INFO("nVec size [{}]", rc);
    return rc;
}

// TODO v1.2.0 use formatter
int VarMapUtils::_vListSendFims(varsmap& vmap, const char* method, fims* px_fims, const char* dbi, const char* uri,
                                bool sim, assetVar* avd)
{
    UNUSED(px_fims);
    if (!p_fims && !avd)
    {
        if (1)
            FPS_PRINT_ERROR("No Fims Connection", NULL);
        return -1;
    }
    char* msg;
    for (auto ix : vmap)
    {
        cJSON* cj = nullptr;
        if (dbi)
        {
            cj = cJSON_CreateObject();
            varmap& vm = ix.second;
            for (auto& avMap : vm)
            {
                assetVar* Av = avMap.second;
                auto tmp = fmt::format("{{{:c}}}", *Av);
                FPS_PRINT_INFO(" :c option [>>{}<<] ", tmp);
                // avMap.second
                Av->showvarCJ(cj, 0x0001);
                // avMap.second->showvarCJ(cj, 0x1110);
            }
        }
        else
        {
            cj = getVmapCJ(ix.second);  // Add flag for naked ,value or full dict  var:
                                        // val var:value:val full = var:{value:val
                                        // ,scale: scale } html ???
        }
        msg = sim ? cJSON_PrintUnformatted(cj->child ? cj->child : cj) : cJSON_PrintUnformatted(cj);
        cJSON_Delete(cj);
        if (msg)
        {
            // TODO resolve after MVP "get" needs captive replyto address, at the
            // moment its set to nullptr
            if (strcmp(method, "get") == 0)  // need a uri for a get
            {
                if (uri && strlen(uri) > 0)
                {
                    char* atmp;
                    vmlen = asprintf(&atmp, "%s/%s", uri, ix.first.c_str());
                    if (atmp)
                    {
                        p_fims->Send(method, atmp, nullptr, nullptr);
                        free(atmp);
                    }
                }
            }
            else
            {
                // const char* dest = ix.first.c_str();
                // char* dest = nullptr;
                // if(dbi)  vmlen = asprintf(&dest, ("/dbi/ess_controller" +
                // ix.first).c_str());  else  vmlen = asprintf(&dest, ix.first.c_str());
                auto dest = fmt::format("{}", ix.first);
                if (!dest.empty())
                {
                    if (0)
                        FPS_PRINT_INFO("method [{}] dest [{}] msg [{}]", method, dest, msg);
                    if (p_fims)
                        p_fims->Send(method, (char*)dest.c_str(), nullptr, msg);
                }

                if (avd)
                    avd->setVal(msg);
                // if (dest) free(dest);
            }
            free(msg);
        }
    }
    return 0;
}

int VarMapUtils::vListSendFims(varsmap& vmap, const char* method, fims* p_fims, const char* uri, bool sim,
                               assetVar* avd)
{
    return _vListSendFims(vmap, method, p_fims, nullptr, uri, sim, avd);
}
/**
 * @brief uses run_replace to get the dbiName for an assetVar
 *
 * @param vm the assetVar referencing the dbi
 * @param  comp the dbi assetVar
 */
std::string VarMapUtils::getDbiNameAv(std::string& comp)
{
    std::string sp1 = run_replace(comp.c_str(), "_", "__");
    std::string sp2 = run_replace(sp1, "/", "_");
    return sp2;
}

// this is special we are using naked values here
int VarMapUtils::DbivListSendFims(varsmap& vmap, const char* method, fims* p_fims, const char* uri, bool sim,
                                  assetVar* avd)
{
    return _vListSendFims(vmap, method, p_fims, "dbi", uri, sim, avd);
}
//
// new wrinkle send to another assetvar
int VarMapUtils::sendAssetVar(assetVar* av, fims* p_fims, const char* meth, const char* comp, const char* var,
                              assetVar* avd)
{
    cJSON* cj = nullptr;

    const char* svar = var ? var : av->name.c_str();
    const char* scomp = comp ? comp : av->comp.c_str();
    const char* smeth = meth ? meth : "pub";

    // get the value from the assetVar
    av->getCjVal(&cj);
    if (cj && cj->child)
    {
        // we may want to send the value out under a different name
        cJSON* cjval = cJSON_Duplicate(cj->child, true);
        cJSON* cjsend = cJSON_CreateObject();
        cJSON_AddItemToObject(cjsend, svar, cjval);
        char* spsend = cJSON_PrintUnformatted(cjsend);
        cJSON_Delete(cjsend);
        if (spsend)
        {
            if (1)
                FPS_PRINT_INFO("meth [{}] comp [{}] msg [{}]"

                               ,
                               smeth ? smeth : "no meth", scomp ? scomp : " no cpmp", spsend ? spsend : " no body");
            if (p_fims)
                p_fims->Send(smeth, scomp, nullptr, spsend);
            if (avd)
                avd->setVal(spsend);
            free(spsend);
        }
    }
    if (cj)
        cJSON_Delete(cj);
    return 0;
}

// TODO checkSingleUri review after MVP
// uri could be /a/b/c@p?a=3,b=45   ..
// uri could be /a/b:c@p?a=3,b=45   ..

cJSON* VarMapUtils::checkSingleUri(varsmap& vmap, int& single, char** vName, char** cName, const char* method,
                                   const char* uri, const char* body)
{
    assetUri my(uri);
    my.single();
    if (0)
        FPS_PRINT_INFO(
            "we're working on this one uri [{}] nfrags {} my.Uri [{}] "
            "my.Var [{}] my.Param [{}]",
            uri ? uri : "no uri", my.nfrags, my.Uri ? uri : "no Uri", my.Var ? my.Var : "no Var",
            my.Param ? my.Param : "no Param");
    if (0)
        FPS_PRINT_INFO("     my.sUri [{}] my.sVar [{}]", my.sUri ? my.sUri : "no uri", my.sVar ? my.sVar : "no var");
    // single = 0;
    cJSON* cj = nullptr;
    char* vv1 = nullptr;  // my.pullPfrag(my.nfrags-1); // /a/b/c  -> /a/b
    char* vv2 = nullptr;  // my.pullPvar(my.nfrags-1);  // /a/b/c  ->c
    if (my.nfrags > 0)
    {
        vv1 = my.pullPfrag(my.nfrags - 1);  // /a/b/c  -> /a/b
        vv2 = my.pullPvar(my.nfrags - 1);   // /a/b/c  ->c
        if (0 || process_fims_debug)
            FPS_PRINT_INFO("method [{}] uri  [{}] acomp [{}] avar [{}]", method, uri, vv1 ? vv1 : "no comp",
                           vv2 ? vv2 : "no var");
    }

    if (strcmp(method, "get") == 0)
    {
        // Why ?? single |= 0x0100;   // lets try anyway.
        auto ix = vmap.find(my.Uri);
        if (ix != vmap.end())
        {
            if (process_fims_debug)
                FPS_PRINT_INFO("Table found for get uri [{}]", uri);
        }
        else
        {
            if ((strlen(uri) == 0) || strcmp(uri, "/") == 0)
            {
                if (0 || process_fims_debug)
                    FPS_PRINT_INFO("Fetch full data [{}]", uri);
                cj = getMapsCj(vmap, nullptr, nullptr);
                return cj;
            }

            if (process_fims_debug)
                FPS_PRINT_INFO("Table NOT found for  get uri  [{}]", uri);
        }

        if (ix == vmap.end())
        {
            if (0)
                FPS_PRINT_INFO("possible get single uri [{}] vv1 [{}] vv2 [{}]", uri, vv1, vv2);

            // single |= 1;
            // rework uri to be one less
            // /x/y/z  => /x/y  with a ar of Z
            // int parts = get_nfrags(uri);
            char* varName = vv2;  // pull_pfrag(uri, parts);
            char* uriName = vv1;  // pull_uri(uri, parts);
            if (0)
                FPS_PRINT_INFO("possible get single ({:#04x}) uri  [{}] comp [{}] len {} var [{}]", single, uri,
                               uriName, strlen(uriName), varName);

            if (strlen(uriName) == 0)
            {
                *cName = strdup(uri);
            }
            else
            {
                *cName = strdup(uriName);
            }

            *vName = varName ? strdup(varName) : nullptr;
            auto ix2 = vmap.find(*cName);
            if (ix2 == vmap.end())
            {
                // No luck finding table
                if (0)
                    FPS_PRINT_INFO("Did not find  comp single uri  [{}] comp [{}] var [{}]", uri, *cName,
                                   varName ? varName : "no Var");
                free((void*)*cName);
                if (*vName)
                    free((void*)*vName);
                *vName = nullptr;
                *cName = strdup(uri);

                if (vv1)
                    free(vv1);
                if (vv2)
                    free(vv2);
                return nullptr;
            }
            auto ix3 = vmap[*cName].find(varName);
            if (ix3 == vmap[*cName].end())
            {
                // No luck finding var in table
                if (0)
                    FPS_PRINT_INFO("Did not find  var single uri  [{}] comp [{}] var [{}]", uri, uriName, varName);
                if (vv1)
                    free(vv1);
                if (vv2)
                    free(vv2);
                return nullptr;
            }
            single |= 0x0001;
            // TODO after MVP proper mask design for opts and single when creating
            // reply to get
            if (vv1)
                free(vv1);
            if (vv2)
                free(vv2);
            return nullptr;
        }
        else
        {
            *vName = nullptr;
            *cName = strdup(uri);
            // Why
            // single |= 0x0100;
            if (vv1)
                free(vv1);
            if (vv2)
                free(vv2);

            return nullptr;
        }
    }
    else  // Set or Pub
    {
        auto ix = vmap.find(uri);
        if (ix == vmap.end())
        {
            if (0)
                FPS_PRINT_INFO("possible single uri [{}]", uri);
            // single |= 1;
        }
        else
        {
            if (0)
                FPS_PRINT_INFO("possible single uri  [{}]", uri);
        }

        if (body)
            cj = cJSON_Parse(body);
        if (!cj)
        {
            if (0)
                FPS_PRINT_INFO("cj failed def single", NULL);
            single |= 0x0001;
            single |= 0x0100;
        }
        else
        {
            // look for name and ! value
            if (0)
                FPS_PRINT_INFO("cj OK, may not be single body [{}] type {}", body, cj->type);
            if (cj->type == cJSON_Object)
            {
                cJSON* cji = cJSON_GetObjectItem(cj, "value");
                cJSON* cjn = cJSON_GetObjectItem(cj, "name");

                if (cji)
                {
                    if (0)
                        FPS_PRINT_INFO(
                            "cj OK  found value, confirmed SINGLE  uri [{}] "
                            "body [{}] type {}",
                            uri, body, cj->type);
                    single |= 0x0001;
                    single |= 0x0010;
                }
                else if (cjn)
                {
                    if (0)
                        FPS_PRINT_INFO(
                            "cj no value but found Name , possible uiasset  uri "
                            "[{}] body [{}] type {}",
                            uri, body, cj->type);
                    // this means that it will have naked params and asset params
                    // This is a candidate for a UI table
                    // Ui objects have naked assetVars and uiassetVars
                    // a naked setvar is always a string value pair
                    single |= 0x1000;
                }
                // we'll let it go anyway
                single |= 0x0100;
            }
            else if (cj->type == cJSON_True)
            {
                if (0)
                    FPS_PRINT_INFO("cj OK  found true confirmed SINGLE  body [{}] type {}", body, cj->type);
                single |= 0x0101;
            }
            else if (cj->type == cJSON_False)
            {
                if (0)
                    FPS_PRINT_INFO("cj OK  found false confirmed SINGLE  body [{}] type {}", body, cj->type);
                single |= 0x0101;
            }
            else if (cj->type == cJSON_Number)
            {
                if (0)
                    FPS_PRINT_INFO("cj OK  found number confirmed SINGLE  body [{}] type {}", body, cj->type);
                single |= 0x0101;
            }
            else if (cj->type == cJSON_String)
            {
                if (0)
                    FPS_PRINT_INFO("cj OK  found string confirmed SINGLE  body [{}] type {}", body, cj->type);
                single |= 0x0101;
            }
            else
            {
                single |= 0x0100;
            }
        }
        if (vv1)
            free(vv1);
        if (vv2)
            free(vv2);
        vv1 = nullptr;
        vv2 = nullptr;
    }

    // if we are a single get a new URI
    // get num parts
    // get the last part
    // remove the last part and we have a comp plus a var name
    if (single & 0x0001)
    {
        char* vv1 = my.pullPfrag(my.nfrags - 1);  // /a/b/c  -> /a/b
        char* vv2 = my.pullPvar(my.nfrags - 1);   // /a/b/c  ->c

        if (0)
            FPS_PRINT_INFO("got single, varName [{}] uriName [{}]", vv2, vv1);
        *vName = vv2;
        *cName = vv1;
    }
    else
    {
        *vName = nullptr;
        *cName = strdup(uri);
    }
    // if(cj)cJSON_Delete(cj);
    return cj;
}

// check for a blocked URI
// also filter options
// check for a blocked URI
// also filter options
// TODO review checkdBlockedUri after MVP
bool VarMapUtils::checkedBlockedUri(varsmap& vmap, char** cName, int& opts, const char* method, const char* uri)
{
    bool blocked = false;
    bool bypass = false;
    auto essstr = fmt::format("/{}", getSysName(vmap));
    char* ess = (char*)essstr.c_str();
    const char* naked = "/naked";
    const char* full = "/full";
    const char* ui = "/ui";
    const char* useid = "/useid";
    const char* noid = "/noid";
    const char* components = "/components";
    const char* site = "/site";
    char* sp = (char*)uri;

    // block gets on components // bypass with /ess/components
    if ((strcmp(method, "get") == 0) && (strncmp(uri, components, strlen(components)) == 0))
    {
        blocked = true;
        return blocked;
    }
    // block pubs on site // bypass with /ess/site
    if ((strcmp(method, "pub") == 0) && (strncmp(uri, site, strlen(site)) == 0))
    {
        blocked = true;
        return blocked;
    }

    if (strncmp(uri, ess, strlen(ess)) == 0)
    {
        blocked = false;
        bypass = true;
        sp += strlen(ess);
        if (0)
            FPS_PRINT_INFO("removed prefix [{}] remaining [{}]", ess, sp);
    }
    if (!blocked)
    {
        // TODO use string replace
        if (strncmp(sp, noid, strlen(noid)) == 0)
        {
            sp += strlen(noid);
            useId = 0;
            // return cj;
        }
        if (strncmp(sp, useid, strlen(useid)) == 0)
        {
            sp += strlen(useid);
            useId = 1;
            // return cj;
        }
        // TODO fixup this old opts stuff 10.2
        if (strncmp(sp, naked, strlen(naked)) == 0)
        {
            sp += strlen(naked);
            opts |= 0x0001;
            // return cj;
        }
        else if (strncmp(sp, full, strlen(full)) == 0)
        {
            sp += strlen(full);
            opts |= 0x0010;
            opts &= ~0x0001;  // this one wins
                              // return cj;
        }
        else if (strncmp(sp, ui, strlen(ui)) == 0)
        {
            sp += strlen(ui);
            opts |= 0x1010;
            opts &= ~0x0001;  // this one wins
                              // return cj;
        }
    }
    // fix in 10.2 use string ref
    *cName = strdup(sp);
    bool tblocked = false;
    const char* blockeduri = nullptr;

    // This is now back  may have to be a vector
    //{
    //  "/blockeduris/set":
    //      {
    //         "/components/cell": true,
    //         "/components/hvac": true
    //      }
    if (!bypass)
    {
        auto blkstr = fmt::format("/blockeduris/{}", method);
        blockeduri = blkstr.c_str();
        if (0)
            FPS_PRINT_INFO("checking blockeduris [{}] uri [{}] ", blockeduri, uri);

        auto ix = vmap.find(blockeduri);
        if (ix != vmap.end())
        {
            for (auto iy : vmap[blockeduri])
            {
                if (0)
                    FPS_PRINT_INFO("checking uri [{}] buri [{}] name [{}]  bval [{}]", uri, blockeduri, iy.first,
                                   iy.second->getbVal());

                // we can block /components/cell
                // but except out of /components/cell_temp
                if (strncmp(uri, iy.first.c_str(), strlen(iy.first.c_str())) == 0)
                {
                    if (0)
                        FPS_PRINT_INFO("checking uri [{}] buri [{}] name [{}]  bval [{}]", uri, blockeduri, iy.first,
                                       iy.second->getbVal());
                    if (!iy.second->getbVal())
                    {
                        if (0)
                            FPS_PRINT_INFO(" uri [{}] (unblocked  here) [{}]) name [{}]  is NOT blocked", uri,
                                           blockeduri, iy.first);
                        return false;
                    }
                    tblocked = iy.second->getbVal();
                }
            }
            if (0)
                FPS_PRINT_INFO(" uri [{}] after checking buri [{}] tblocked [{}]", uri, blockeduri, tblocked);
            if (tblocked)
            {
                if (0)
                    FPS_PRINT_INFO(" uri [{}] (blocked  [{}])  is blocked", uri, blockeduri);
                return true;
            }
        }
    }
    if (blocked)
    {
        if (0)
            FPS_PRINT_INFO("blocked uri  method [{}] uri [{}] -> [{}]", method, uri, *cName);
    }
    else
    {
        if (0)
            FPS_PRINT_INFO("NOT blocked method [{}] uri [{}] ->[{}]", method, uri, *cName);
    }

    return blocked;
}

// // the main fims message processor
bool VarMapUtils::runFimsMsg(varsmap& vmap, fims_message* msg, fims* p_fims, cJSON** cjri)
{
    return runFimsMsgAm(vmap, msg, nullptr, p_fims, cjri);
}
// // the main fims message processor
bool VarMapUtils::runFimsMsgAm(varsmap& vmap, fims_message* msg, asset_manager* am, fims* p_fims, cJSON** cjri)
{
    cJSON* cjr = nullptr;
    char* vName = nullptr;   // new var name
    char* newUri = nullptr;  // new comp (uri) name
    char* cName2 = nullptr;  // new comp (uri) name
    // std::string newBody;   // new comp (uri) name
    int opts = 0;

    // auto tvar1 =  getVar(vmap, "/assets/bms/summary:bms_test2", nullptr);
    // auto tvar2 =  getVar(vmap, "/status/bms:bms_test2", nullptr);
    // if (0||process_fims_debug)FPS_PRINT_INFO("before checkBlocked uri [{}]
    // method [{}] tvar1 [{}] tvar2 [{}]"
    //         , msg->uri
    //         , msg->method
    //         , fmt::ptr(tvar1)
    //         , fmt::ptr(tvar2)
    //         );
    bool reject = checkedBlockedUri(vmap, &newUri, opts, msg->method, msg->uri);
    if (0 || process_fims_debug)
        FPS_PRINT_INFO("after checkBlocked uri [{}] reject [{}]", msg->uri, reject);
    if (strcmp(msg->method, "pub") != 0)
    {
        if (0 || process_fims_debug)
            FPS_PRINT_INFO("after checkBlocked  method [{}] newUri [{}] reject [{}] opts {:#04x}"

                           ,
                           msg->method, newUri, reject ? "Rejected" : "OK to run", opts);
    }

    if (strcmp(msg->method, "get") == 0)
    {
        if (0 || process_fims_debug)
            FPS_PRINT_INFO(
                "after checkBlocked  method [{}] orig uri [{}] newUri "
                "[{}] reject [{}] opts {:#04x}",
                msg->method, msg->uri, newUri, reject ? "Rejected" : "OK to run", opts);
        process_fims_debug = 0;
    }
    else
    {
        process_fims_debug = 0;
    }
    if (!reject)
    {
        int single = opts;
        // send in the new uri
        // FPS_PRINT_INFO("... check x1 ");
        cJSON* cj = checkSingleUri(vmap, single, &vName, &cName2, msg->method, newUri, msg->body);

        if (0 || process_fims_debug)
            FPS_PRINT_INFO(
                "after checkSingle single {:#04x} vName [{}] cName2 [{}] "
                "newUri [{}] newBod [{}] cj {}"

                ,
                single, vName ? vName : "no Vname", cName2 ? cName2 : "no cName2", newUri ? newUri : "no newUri",
                msg->body ? msg->body : "no body", fmt::ptr(cj));
        // single 0x0100 must be set if this one is valid
        // FPS_PRINT_INFO("... check x2 ");

        if (single & 0x1000)
        {
            if (process_fims_debug)
                FPS_PRINT_INFO(
                    "ready for get/load uivars {:#04x} vName [{}] cName2 "
                    "[{}] newUri [{}] newBod [{}] cj {}"

                    ,
                    single, vName, cName2, newUri, msg->body, fmt::ptr(cj));
            char* newBod = (char*)msg->body;
            if (strcmp(msg->method, "get") == 0)
                cjr = getUimap(vmap, single, cName2, vName);
            else
                cjr = loadUimap(vmap, single, cName2, vName, newBod, am);
            if (0 || process_fims_debug)
                FPS_PRINT_INFO(
                    "ready for get/load uivars {:#04x} vName [{}] cName2 "
                    "[{}] newUri [{}] cj {}"

                    ,
                    single, vName, cName2, newUri, fmt::ptr(cjr));
        }
        else if (single & 0x0100)
        {
            if (0 || process_fims_debug)
                FPS_PRINT_INFO(
                    "ready for get/load method [{}] single {:#04x} vName "
                    "[{}] cName2 [{}] newUri [{}] newBod [{}] cj {}"

                    ,
                    msg->method, single, vName, cName2, newUri, msg->body, fmt::ptr(cj));
            char* newBod = (char*)msg->body;  // for single adjust    /x/y/x 123 =>
                                              // /x/y '{z:{value"123}}'
            if (strcmp(msg->method, "get") == 0)
            {
                if (0)
                    FPS_PRINT_INFO("... check 11  cName2 [{}] vName [{}] single [{:04x}]",
                                   cName2 ? cName2 : " no cName2", vName ? vName : " no vName", single);

                if (cName2)
                {
                    cjr = getVmap(vmap, single, cName2, vName, single);  // opts);
                }
                else
                {
                    cjr = cj;
                    cj = nullptr;
                }
                // FPS_PRINT_INFO("... check 21 ");
            }
            else
            {
                // FPS_PRINT_INFO("... check 1 ");
                cjr = loadVmap(vmap, single, cName2, vName, newBod, am, newUri);
                // FPS_PRINT_INFO("... check 2 ");
            }
            if (0 || process_fims_debug)
                FPS_PRINT_INFO(
                    "after get/load uivars {:#04x} vName [{}] cName2 [{}] "
                    "newUri [{}]  cjr {}"

                    ,
                    single, vName, cName2, newUri, fmt::ptr(cjr));
            // vm.processRawMsg(vmap, msg->method, cName2, msg->replyto, newBod,
            // &cjr);
            if (0)
                FPS_PRINT_INFO("cj result {}", fmt::ptr(cjr));
        }
        else
        {
            // TODO runFimsMsgAm after MVP we need an option to remove the baseuri
            //
            if (strcmp(msg->method, "get") == 0)
            {
                // FPS_PRINT_INFO("... check 1 uri [{}]", msg->uri);

                cjr = getVmap(vmap, single, cName2, vName, single, msg->uri);  // opts);
                                                                               // FPS_PRINT_INFO("... check 2 ");
            }
        }

        // for now limit the scope of this to gets and sets
        if ((strcmp(msg->method, "get") == 0) || (strcmp(msg->method, "set") == 0))
        {
            if (cjr && cjri)
            {
                *cjri = cjr;
                cjr = nullptr;
            }
            if (cjr)
            {
                if (useId)
                {
                    if (!idVec)
                    {
                        idVec = new std::vector<std::string>;
                    }
                    cJSON_StringsToId(cjr, idVec);
                }
                char* tmp = cJSON_PrintUnformatted(cjr);
                if (tmp)
                {
                    auto rsize = strlen(tmp);
                    if (rsize > MAX_FIMS_MSG_SIZE)
                    {
                        if (1)
                            FPS_PRINT_INFO("cj result size {}", rsize);
                        free(tmp);
                        vmlen = asprintf(&tmp, "Fims Error :: Message size too large [%ld]", rsize);
                    }
                    // if (0)FPS_PRINT_INFO("cj result {}", tmp);
                    if (p_fims && (cjr->string || cjr->child))
                    {
                        if (msg->replyto)
                        {
                            p_fims->Send("set", msg->replyto, nullptr, tmp);
                        }
                    }
                    else
                    {
                        FPS_PRINT_INFO("cj result no string or child", NULL);
                        if (0)
                            FPS_PRINT_INFO("cj result {}", tmp);
                    }
                    free(tmp);
                }
                cJSON_Delete(cjr);
                cjr = nullptr;
            }
        }
        if (cj)
            cJSON_Delete(cj);
        if (cjr)
            cJSON_Delete(cjr);
        cjr = nullptr;
        cj = nullptr;
    }
    if (p_fims)
        p_fims->free_message(msg);
    if (newUri)
        free(newUri);
    if (cName2)
        free(cName2);
    if (vName)
        free(vName);
    process_fims_debug = 0;
    // FPS_PRINT_INFO("... done ");
    return true;
}

cJSON* VarMapUtils::getMapsCjFixed(varsmap& vmap, cJSON* cji)
{
    int opts = 0;
    // cJSON* cj = nullptr;  //cJSON_CreateObject();

    cJSON* cj = cJSON_CreateObject();

    if (0)
        FPS_PRINT_INFO("get them all opts [{:#04x}]", opts);
    int found = 0;
    opts |= 0x0110;

    for (auto& x : vmap)
    {
        // if(1)FPS_PRINT_INFO("get them all comp [{}]", x.first.c_str());

        // cJSON* cjx = cJSON_CreateObject();//
        cJSON* cjx = getMapsCj(vmap, x.first.c_str(), nullptr, opts, nullptr, cji);
        if ((cjx->string == nullptr) && (cjx->child == nullptr))
        {
            // do it the old way...
            {
                for (auto& y : vmap[x.first])
                {
                    if (y.second != nullptr)
                    {
                        if (1)
                            FPS_PRINT_INFO("NOTE running for var [{}:{}] av {} cjx {}", x.first, y.first,
                                           fmt::ptr(y.second), fmt::ptr(cjx));

                        y.second->showvarCJ(cjx, 0x1110);  // opts);

                        {
                            char* spx = cJSON_Print(cjx);
                            FPS_PRINT_INFO("spx [{}]", spx);
                            free(spx);
                        }
                        {
                            char* spx = cJSON_Print(cjx->child);
                            FPS_PRINT_INFO("spx->child [{}]", spx);
                            free(spx);
                        }
                        found++;
                    }
                    else
                    {
                        FPS_PRINT_INFO("NOTE no map for var [{}]", y.first);
                    }
                }
            }
        }

        // cJSON_AddItemToObject(cj, x.first, cjx->child->child);
        cJSON_AddItemToObject(cj, x.first.c_str(), cjx);
        // cjx->child = nullptr;
        // cJSON_Delete(cjx);
    }

    return cj;
}

void VarMapUtils::processMsgGet(varsmap& vmap, const char* method, const char* uri, const char* body, cJSON** cjr,
                                asset_manager* am, asset* ai)
{
    UNUSED(method);
    UNUSED(body);
    UNUSED(am);
    UNUSED(ai);
    assetUri my(uri);
    // TODO review after MVP allow multi level base uris (/assets/bms/bms_1/thing)
    int nfrags = my.nfrags;  // get_nfrags(uri);
    // int nbase = 0;
    int opts = 0;
    // hack for assets
    if (strncmp(uri, "/assets", strlen("/assets") == 0))
    {
        opts |= 0x0100;
    }

    if (0)
        FPS_PRINT_INFO(
            "we're working on this one uri [{}] nfrags {} my.Uri [{}] "
            "my.Var [{}] my.Param [{}]",
            uri, nfrags, my.Uri, my.Var, my.Param);

    *cjr = getMapsCj(vmap, my.Uri, my.Var, opts);
}

// // this is a little different
// // we have naked assetVars
void VarMapUtils::processMsgSetPubUi(varsmap& vmap, const char* method, const char* uri, int& single, const char* body,
                                     cJSON** cjr, asset_manager* am, asset* ai)
{
    setTime();
    int nfrags = get_nfrags(uri);

    if (0)
        FPS_PRINT_INFO("we're working on this one uri [{}] nfrags {}", uri, nfrags);

    if (!cjr)
    {
        FPS_PRINT_INFO("requires a cJson reply var", NULL);
        return;
    }
    // char* smap = pull_pfrag(uri, 0);// pfrags[0]);

    cJSON* cj = cJSON_Parse(body);
    assetVar* av;

    // if cj->type == cJSON_OBJECT it must have a child
    if (cj && cj->type == cJSON_Object)
    {
        if (!cj->child)
        {
            FPS_ERROR_PRINT(
                "%s >> REJECT method %s body [%s] string [%s] cj->type "
                "%d , cj->child %p cj->next %p\n",
                __func__, method, body, cj->string ? cj->string : " No cj string", cj->type, (void*)cj->child,
                (void*)cj->next);
            cJSON_Delete(cj);
            cj = nullptr;
        }
    }

    if (cj)
    {
        // run the config system.
        if (strncmp(uri, "/cfg/", strlen("/cfg/")) == 0)
        {
            if (0)
                FPS_PRINT_INFO(" >>>>>> before cfile  uri [{}] ", uri);
            handleCfile(vmap, cj, method, uri, single, body, cjr, am, ai);
            if (0)
                FPS_PRINT_INFO(" >>>>>> after cfile  uri [{}] ", uri);
            return;
        }
    }

    if (cj)
    {
        if (*cjr == nullptr)
        {
            *cjr = cJSON_CreateObject();
        }
        if (0 || process_fims_debug)
            FPS_PRINT_INFO("method {} [{}] cj->type {}, cj->child {} cj->next {}"

                           ,
                           method, cj->string, cj->type, fmt::ptr(cj->child), fmt::ptr(cj->next));
        cJSON* cjrx = cj;
        if (cj->child)
            cjrx = cj->child;

        while (cjrx)
        {
            // if this has a child it is a normal ui assetvar
            // the uiasset var will have naked params etc.
            // else it is a naked assetvar
            if (0 || setvar_debug)
                FPS_PRINT_INFO(
                    "uri [{}] string [{}] cj->type {} (str {}), cj->child "
                    "{} cj->next {}"

                    ,
                    uri, cjrx->string, cjrx->type, cJSON_String, fmt::ptr(cjrx->child), fmt::ptr(cjrx->next));

            // setval from cj must accept naked params
            if (cjrx->child)
            {
                av = setValfromCj(vmap, uri, cjrx->string, cjrx->child, true);

                if (am && av)
                    av->am = am;
                if (ai && av)
                    av->ai = ai;
            }
            else
            {
                av = setValfromCj(vmap, uri, cjrx->string, cjrx, true);
                if (am && av)
                    av->am = am;
                if (ai && av)
                    av->ai = ai;
            }
            if (av)
            {
                // Rescue an orphan.
                if (am && !av->am && !av->ai)
                {
                    av->am = am;
                }
                else if (ai && !av->ai)
                {
                    av->ai = ai;
                }
            }
            cJSON* cji2 = cJSON_Duplicate(cjrx, true);
            cJSON_AddItemToObject(*cjr, cjrx->string, cji2);

            cjrx = cjrx->next;
        }
        cJSON_Delete(cj);
    }
    return;
}

// char* vv1 = my.pullPfrag(my.nfrags-1); // /a/b/c  -> /a/b
//    char* vv2 = my.pullPvar(my.nfrags-1);  // /a/b/c  ->c
void VarMapUtils::processMsgSetPub(varsmap& vmap, const char* method, const char* uri, int& single, const char* body,
                                   cJSON** cjr, asset_manager* am, asset* ai)
{
    assetUri my(uri);

    setTime();
    if (!cjr)
    {
        FPS_PRINT_INFO("requires a cJson reply var", NULL);
        return;
    }

    cJSON* cj = cJSON_Parse(body);
    if (cj)
    {
        // run the config system.
        if (strncmp(uri, "/cfg/", strlen("/cfg/")) == 0)
        {
            if (0)
                FPS_PRINT_INFO(" >>>>>> before cfile #2  uri [{}] ", uri);
            return handleCfile(vmap, cj, method, uri, single, body, cjr, am, ai);
        }
    }

    if (0)
        FPS_ERROR_PRINT("%s >> got cj %p  cj->type [%d]\n", __func__, (void*)cj, cj ? cj->type : -1);
    if (cj && cj->type == cJSON_Object)
    {
        if (!cj->child)
        {
            FPS_ERROR_PRINT(
                "%s >> REJECT method %s body [%s] string [%s] cj->type "
                "%d , cj->child %p cj->next %p\n",
                __func__, method, body, cj->string ? cj->string : " No cj string", cj->type, (void*)cj->child,
                (void*)cj->next);
            cJSON_Delete(cj);
            cj = nullptr;
        }
    }
    // this handles the dbi thing we skip to the child->child
    // here is the response
    // the child->child is what is needed as a body for the uri...
    // Method:  pub
    // Uri:     /ess/dbi/status/pcs
    // ReplyTo: (null)
    // Body:    {"status_pcs":{
    //"MaxDeratedChargeCmd":{"value":-3510,"EnableDbiUpdate":true,"UpdateTimeCfg":5,"UpdateTimeRemain":5,"dbiStatus":"OK","tLast":104.52318300004117},
    //"MaxDeratedDischargeCmd":{"value":3510,"EnableDbiUpdate":true,"UpdateTimeCfg":5,"UpdateTimeRemain":0,"dbiStatus":"OK","tLast":104.52335699996911}}}
    // Timestamp:   2021-07-16 12:56:13.451481

    if (cj)
    {
        if (strncmp(uri, "/xxdbi/", strlen("/xxdbi/")) == 0)
        {
            if (0)
                FPS_ERROR_PRINT("%s >> detected  a DBI Object uri [%s] string [%s] child %p\n", __func__, uri,
                                cj->string ? cj->string : " no string 1", (void*)cj->child);
            if (cj->child)
            {
                if (0)
                    FPS_ERROR_PRINT(
                        "%s >> detected  a DBI Object Child  uri [%s] string "
                        "[%s] child %p\n",
                        __func__, uri, cj->child->string ? cj->child->string : " no string 2",
                        (void*)(cj->child->child));
                if (cj->child->child)
                {
                    cJSON* cjt = cj->child->child;
                    cj->child->child = nullptr;
                    cJSON_Delete(cj);
                    cj = cjt;
                }
            }
        }
    }

    if (cj)
    {
        // Evaluate this at the head of the table
        // watch out for ui_type"control" they also may have a name but no value...
        int uiObject = 0;
        cJSON* cjname = cJSON_GetObjectItem(cj, "name");
        cJSON* cjvalue = cJSON_GetObjectItem(cj, "value");
        if (cj->child && cjname && !cjvalue)
        {
            FPS_PRINT_INFO("detected a uiObject name [{}]",
                           cjname->valuestring ? cjname->valuestring : " No Name found");
            uiObject = 1;
        }

        if (*cjr == nullptr)
        {
            *cjr = cJSON_CreateObject();
        }
        if (0 || process_fims_debug)
            FPS_PRINT_INFO("method {} string [{}] cj->type {} , cj->child {} cj->next {}"

                           ,
                           method, cj->string ? cj->string : "no string", cj->type, fmt::ptr(cj->child),
                           fmt::ptr(cj->next));
        cJSON* cjrx = cj;
        assetVar* av = nullptr;
        if (uiObject > 0)
        {
            av = setValfromCj(vmap, my.Uri, cjname->string, cjname, uiObject);

            // Rescue an orphan. Undoing Do not rescue if av already has an am
            if (am && av)
                av->am = am;
            if (ai && av)
                av->ai = ai;

            if (av)
            {
                av->setNaked = true;
            }
            FPS_PRINT_INFO("set  uiObject  name [{}] av {} naked [{}]",
                           cjname->valuestring ? cjname->valuestring : " No Name found", fmt::ptr(av),
                           av ? av->setNaked ? "true" : "false" : "no AV");
        }
        // here we go  with a config list headed into     the uri table

        if (cj->child)
        {
            cjrx = cj->child;
            if (0)
                FPS_PRINT_INFO("starting  import name [{}] ", cjrx->string ? cjrx->string : " No String found");
        }
        assetList* alist = nullptr;
        bool newAv = false;

        if (uiObject > 0)
        {
            alist = getAlist(vmap, my.Uri);
            // //Now process the list of objects.

            if (!alist)
            {
                newAv = true;
                alist = setAlist(vmap, my.Uri);

                if (0)
                    FPS_PRINT_INFO("new assetList", NULL);
            }
        }
        if (alist && av)
        {
            alist->add(av);
            av = nullptr;
        }
        // // we may need to clean out the old avec but for now just leave it.
        // DONE keep the order for uiObjects in a vector
        while (cjrx)
        {
            // if uiObject populate order list...
            // what happened to the ui controls ??   they had name but no value ....
            // allow name and ui_type = control why is "name" corrupted ?? see above

            // we should do this for each av we'll still have the timeout in place.
            if (schedActions > 0)
            {
                if (0)
                    FPS_PRINT_INFO("resetting actions {}  for [{}] elapsed time(mS) {}", schedActions,
                                   cjrx->string ? cjrx->string : " No String found",
                                   get_time_dbl() - schedTime * 1000.0);

                schedActions = 0;
            }
            auto tmp = cJSON_Print(cjrx);

            if (0)
                FPS_PRINT_INFO("Calling setValfromCj for [{}]", cjrx->string ? cjrx->string : " No String found");
            av = setValfromCj(vmap, my.Uri, cjrx->string, cjrx, uiObject);
            if (0)
                FPS_PRINT_INFO("called setValfromCj uri [{}] name [{}]  cj [{}] ui [{}] av [{}]", my.Uri,
                               cjrx->string ? cjrx->string : "no name", tmp ? tmp : "no cj", uiObject, fmt::ptr(av));
            if (tmp)
                free(tmp);

            if (av)
            {
                // Rescue an orphan. Undoing Do not rescue if av already has an am
                if (am)
                    av->am = am;
                if (ai)
                    av->ai = ai;

                if (alist && newAv)
                    alist->add(av);
            }

            if (0 || setvar_debug)
                FPS_PRINT_INFO(
                    "uri [{}] string [{}] cj->type {} (str {}), cj->child "
                    "{} cj->next {} av {}"

                    ,
                    uri, cjrx->string, cjrx->type, cJSON_String, fmt::ptr(cjrx->child), fmt::ptr(cjrx->next),
                    fmt::ptr(av));
            cJSON* cji2 = cJSON_Duplicate(cjrx, true);
            cJSON_AddItemToObject(*cjr, cjrx->string, cji2);

            cjrx = cjrx->next;
        }
        cJSON_Delete(cj);
        //// no no no dont delete this
        // if(alist) delete alist;
    }
    return;
}

void VarMapUtils::processMsgSetReply(varsmap& vmap, const char* method, const char* uri, const char* replyto,
                                     const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    assetUri my(uri);

    // char* var = nullptr;
    // char* uri = strdup(inuri);
    int nfrags = my.nfrags;
    cJSON* cj = cJSON_Parse(body);
    // look for var in last frag

    // The var may be the last frag
    // if (my.Var && nfrags > 2)
    //{
    //    var = pull_pfrag(uri, nfrags);
    //}

    if (*cjr == nullptr)
    {
        *cjr = cJSON_CreateObject();
    }

    assetVar* av = nullptr;
    cJSON* cji = nullptr;
    if (cj)
    {
        if (0)
            FPS_PRINT_INFO(
                "uri [{}] method {} uri [{}] frgs {} body [{}] va [{}] "
                "cj->string [{}] cj->type {} , cj->child {} cj->next {}",
                uri, method, my.Uri, nfrags, body, my.Var ? my.Var : "noVar", cj->string, cj->type, fmt::ptr(cj->child),
                fmt::ptr(cj->next));
        if (my.Var)
        {
            if (0)
                FPS_PRINT_INFO("[{}] cj->type {}, cj->child {} cj->next {}", my.Var, cj->type, fmt::ptr(cj->child),
                               fmt::ptr(cj->next));

            av = setValfromCj(vmap, my.Uri, my.Var, cj, false);

            // get ai / am from amap we'll do that in my temp/merge branch

            // Rescue an orphan.
            if (am && !av->am && !av->ai)
            {
                av->am = am;
            }
            else if (ai && !av->ai)
            {
                av->ai = ai;
            }

            if (0)
                FPS_PRINT_INFO("MSGSetReply we got a VAR set with a replyto [{}]", replyto ? replyto : "No Reply");
            cJSON* cji2 = cJSON_Duplicate(cj, true);
            cJSON_AddItemToObject(*cjr, my.Var, cji2);
            // free((void*)var);
        }
        else
        {
            cji = cj;
            int uiObject = 0;
            // look for "registers"
            cJSON* cjname = cJSON_GetObjectItem(cj, "name");
            cJSON* cjvalue = cJSON_GetObjectItem(cj, "value");
            if (cj->child && cjname && !cjvalue)
            {
                uiObject = 1;
            }

            if (cj->child)
                cji = cj->child;
            // this processes the list
            // if this is  UI object we can detect it here.
            // before we head off with the kids

            while (cji)
            {
                if (0)
                    FPS_PRINT_INFO("[{}] cj->type {} , cj->child {} cj->next {}", cji->string, cji->type,
                                   fmt::ptr(cji->child), fmt::ptr(cji->next));

                av = setValfromCj(vmap, my.Uri, cji->string, cji, uiObject);
                // Rescue an orphan.
                if (am && !av->am && !av->ai)
                {
                    av->am = am;
                }
                else if (ai && !av->ai)
                {
                    av->ai = ai;
                }
                if (0)
                    FPS_PRINT_INFO("MSGSetReply we got a Multi set with a replyto [{}]", replyto);
                cJSON* cji2 = cJSON_Duplicate(cji, true);
                cJSON_AddItemToObject(*cjr, cji->string, cji2);
                cji = cji->next;
            }
        }
        cJSON_Delete(cj);
    }
    else
    {
        if (1)
            FPS_PRINT_INFO("unable to parse body [{}]", body);
    }
    //    if (uri) free((void*)uri);
    return;
}

// TODO review addCjFrags after MVP
void VarMapUtils::addCjFrags(cJSON* cj, const char* uri, cJSON* junk)
{
    assetUri my(uri);
    int nfrags = my.nfrags;
    // char* suri = strdup(my.Uri);
    cJSON* cji = cj;
    cJSON* cjf = cj;
    for (int i = 0; i < nfrags; i++)
    {
        if (nfrags == 1)
        {
            cjf = junk;
        }
        else
        {
            cjf = cJSON_GetObjectItem(cji, my.uriVec[i]);
        }

        if (!cjf)
        {
            cjf = cJSON_CreateObject();
        }
        cJSON_AddItemToObject(cji, my.uriVec[i], cjf);

        cji = cjf;
    }
    // free((void*)suri);
}

// here are the rules for set

// simple /set a/b/c {"component":{"value":1234},.....}  Yes
// naked /set a/b/c {"component":1234, ...}  YES
// single    set /a/b/c/component 1234        NO
// /a/b/c/component is mot a table /a/b/c is sor set/add component to /a/b/c
// perhaps       /a/b/c/component '{"value":1234}'    YES

//   get     /a/b/c/component      NO Error
//   get     /a/b/c        Ok but we should search anyway  Error

// get /a/b/c/ should return just /a/b/c

// get one or get them all
// added assetList concept for uiObjects to keep order

// global search  given a uri get a list of comps that match it
// /assets  -> /assets/ess /assets/bms /assets/bms_1    etc

// opts 0x0000     default , full comps , value
// opts 0x0001     default , full comps , naked
// opts 0x0010     dump object , full comps , value
// opts 0x0100     dump object , reduced , value
// tis is crossed into scheduler.cpp
cJSON* getSchList();
cJSON* getAmap(VarMapUtils* vm, varsmap& vmap, char* uri, int opts);

cJSON* getAlistCj(VarMapUtils* vm, varsmap& vmap, char* uri)
{
    // createAssetListCj
    if (0)
        FPS_PRINT_INFO("get alistcj uri [{}]", uri);
    cJSON* cj = nullptr;
    cJSON* cjm = nullptr;
    // assetList* alist = nullptr;
    char* tmp = &uri[strlen("/alist")];
    // // // no leading "/" so its hard to find
    // if(uri)vm->vmlen = asprintf(&tmp, "%s", uri);
    std::vector<std::string> nVec;
    std::string bs;
    int opts = 0;

    auto x = vmap.find("_assetList");
    if (x != vmap.end())
    {
        cj = cJSON_CreateArray();
        if (*tmp)
        {
            cJSON_AddItemToArray(cj, cJSON_CreateString(tmp));

            if (strncmp(tmp, "/assets", strlen("/assets") == 0))
            {
                opts |= 0x0100;
            }
            opts = 0x100;
            vm->createAssetListCj(vmap, tmp, vm->syscVec, opts, nVec);
            cjm = vm->createUriListCj(vmap, bs, tmp, nullptr, opts, nVec);
            for (auto xx : nVec)
            {
                cJSON_AddItemToArray(cj, cJSON_CreateString(xx.c_str()));
            }
            cJSON_AddItemToArray(cj, cJSON_CreateString("###done with nVec"));
            if (cjm)
            {
                cJSON_AddItemToArray(cj, cjm);
            }
        }
        auto xz = vmap["_assetList"];
        cJSON_AddItemToArray(cj, cJSON_CreateString(uri));

        for (auto xx : xz)
        {
            // xx.first is the uri
            cJSON_AddItemToArray(cj, cJSON_CreateString(xx.first.c_str()));
            // cJSON_AddStringToObject(cj, xx.first.c_str() , "none");
        }
    }
    // if (tmp)
    // {
    //     assetVar* av = getVar(vmap, "_assetList", tmp);
    //     if (av)
    //     {
    //         alist = (assetList*)av->aVar;
    //     }
    //     if (vmdebug)FPS_PRINT_INFO("looking for alist [{}] av [{}] alist [{}]"

    //         , tmp
    //         , fmt::ptr(av)
    //         , fmt::ptr(alist)
    //     );

    //     free(tmp);
    // }
    // return alist;
    return cj;
}

cJSON* VarMapUtils::getMapsCj(varsmap& vmap, const char* inuri, const char* var, int opts, const char* origuri,
                              cJSON* cji)
{
    UNUSED(origuri);
    // psw
    if (0 || process_fims_debug)
        FPS_PRINT_INFO("getting cj #1 maps uri [{}] var [{}] opts {:#04x}", inuri ? inuri : "noURI",
                       var ? var : "noVar", opts);
    int found = 0;
    cJSON* cj = nullptr;  // cJSON_CreateObject();
    char* uri = (char*)inuri;

    // specials are
    // /amap , /schlist. /sysvec
    // add alist to the mix
    // get /assets/bms should retun
    //     {
    // "summary": {...},
    // "bms_1": {...},
    // "bms_2": {...},
    // "bms_3": {...},
    // "bms_4": {...},
    // )
    // path 1 uri and no var
    // NOTE this will return the table name as the first object.
    // if(uri)FPS_PRINT_INFO("%s >>>>>> lookin for amap [{}] cmp %d \n", uri,
    // strncmp(uri,"/amap/", strlen("/amap/")));
    if (uri && strncmp(uri, "/schlist", strlen("/schlist")) == 0)
    {
        // typedef std::vector<schedItem*>schlist;
        return getSchList();
    }
    if (uri && strncmp(uri, "/amap", strlen("/amap")) == 0)
    {
        return getAmap(this, vmap, uri, opts);
    }
    if (uri && strncmp(uri, "/alist", strlen("/alist")) == 0)
    {
        return getAlistCj(this, vmap, uri);
    }

    if (uri && strncmp(uri, "/sysvec", strlen("/sysvec")) == 0)
    {
        cJSON* cj = cJSON_CreateObject();
        FPS_PRINT_INFO("found sysvec", NULL);
        cJSON* cja = cJSON_CreateArray();
        for (auto x : *syscVec)
        {
            cJSON_AddItemToArray(cja, cJSON_CreateString(x));
        }
        cJSON_AddItemToObject(cj, "sysvec", cja);
        return cj;
    }

    if (uri && !var)
    {
        std::vector<std::string> nVec;
        std::string bs;
        // int rc =
        // Hack for assets
        // if (strncmp(uri, "/assets", strlen("/assets")==0))
        // {
        //     opts |= 0x0100;
        // }
        createAssetListCj(vmap, uri, syscVec, opts, nVec);
        cJSON* cjm = createUriListCj(vmap, bs, uri, nullptr, opts, nVec);

        if (0 || process_fims_debug)
        {
            char* tmp = cJSON_Print(cjm);

            // FPS_PRINT_INFO("used createUriList from uri [{}] basevec [{}] opts
            // 0x%04x tmp \n>>%s<<\n", uri, bs, opts, tmp);
            FPS_PRINT_INFO("used createUriList from uri [{}] basevec [{}] opts {:#04x} tmp [{}]", uri, bs, opts,
                           tmp ? tmp : " no tmp");
            // if (uri != inuri) free((void*)uri);
            if (tmp)
                free((void*)tmp);
        }

        if (bs.length() > 0)
        {
            cJSON* cjx = cJSON_CreateObject();
            cJSON_AddItemToObject(cjx, bs.c_str(), cjm);
            cjm = cjx;
        }

        if (uri != inuri)
            free((void*)uri);
        return cjm;
        // cJSON_Delete(cjm);
    }
    // path 2 we have a uri and a var
    else if (uri && var)
    {
        auto x = vmap.find(uri);
        if (x != vmap.end())
        {
            auto y = vmap[uri].find(var);
            if (y != vmap[uri].end())
            {
                found++;
                cJSON* cji = cJSON_CreateObject();
                if (0)
                    FPS_PRINT_INFO("getting cj for uri [{}] var [{}] found {} opts {:#04x}", uri, y->first, found,
                                   opts);
                // TODO review ues of opts after MVP need opts here
                if (opts & 0x0010)
                {
                    if (0)
                        FPS_PRINT_INFO("showvarCJ  for uri [{}] var [{}] found {} opts {:#04x}", uri, y->first, found,
                                       opts);
                    y->second->showvarCJ(cji, opts);
                }
                else
                {
                    y->second->showvarValueCJ(cji, opts);
                }

                cJSON* cj = cji;
                // naked
                if (opts & 0x0001 && cji->child)
                {
                    cj = cJSON_Duplicate(cji->child, true);
                    cJSON_Delete(cji);
                }
                if (uri != inuri)
                    free((void*)uri);
                return cj;
            }
        }
    }
    else
    // get all the objects
    {
        cJSON* cj = cJSON_CreateObject();

        if (0)
            FPS_PRINT_INFO("get them all opts [{:#04x}]"

                           ,
                           opts);
        int found = 0;
        opts |= 0x0110;

        for (auto& x : vmap)
        {
            if (0)
                FPS_PRINT_INFO("get them all comp [{}]"

                               ,
                               x.first.c_str());

            cJSON* cjx = getMapsCj(vmap, x.first.c_str(), nullptr, opts, nullptr, cji);
            if ((cjx->string == nullptr) && (cjx->child == nullptr))
            {
                // do it the old way...
                {
                    for (auto& y : vmap[x.first])
                    {
                        if (y.second != nullptr)
                        {
                            if (0)
                                FPS_PRINT_INFO("NOTE running  for var [{}:{}]  av {} cjx {}", x.first, y.first,
                                               fmt::ptr(y.second), fmt::ptr(cjx));

                            y.second->showvarCJ(cjx, 0x1110);  // opts);
                            found++;
                        }
                        else
                        {
                            FPS_PRINT_INFO("NOTE no map for var [{}]", y.first);
                        }
                    }
                }
            }

            if (0)
            {
                // cJSON* cjx = loadAssetList(vmap, x.first, found, opts);
                if (0)
                    FPS_PRINT_INFO("get them all comp [{}] assetlists"

                                   ,
                                   x.first);

                cJSON* cji = cJSON_CreateObject();

                {
                    for (auto& y : vmap[x.first])
                    {
                        if (y.second != nullptr)
                        {
                            if (0)
                                FPS_PRINT_INFO("NOTE running  for var [{}:{}]  av {} cji {}", x.first, y.first,
                                               fmt::ptr(y.second), fmt::ptr(cji));

                            y.second->showvarCJ(cji, opts);
                            found++;
                        }
                        else
                        {
                            FPS_PRINT_INFO("NOTE no map for var [{}]", y.first);
                        }
                    }
                }
                {
                    cJSON_AddItemToObject(cj, x.first.c_str(), cji);
                }
                found++;
            }
            // Terrible Way to do this look at CJSON_DetachObjectPointer.
            cJSON* cjxc = cJSON_Duplicate(cjx->child, true);
            cJSON_Delete(cjx);
            cJSON_AddItemToObject(cj, x.first.c_str(), cjxc);
        }

        return cj;
    }

    if (0)
        FPS_PRINT_INFO("<<<<<<<<<<<<<<<<<<<<<<<<<<<got cj {} maps found [{}]", fmt::ptr(cj), found);
    if (found == 0)
    {
        if (cj)
            cJSON_Delete(cj);
        cj = nullptr;
    }
    if (cj)
    {
        char* tmp = cJSON_PrintUnformatted(cj);
        if (tmp)
        {
            if (0)
                FPS_PRINT_INFO("getting cj for uri [{}] var [{}] as [{}]", uri, var ? var : "noVar", tmp);
            free((void*)tmp);
        }
    }
    if (uri != inuri)
        free((void*)uri);
    return cj;
}

assetVar* VarMapUtils::getaVParam(varsmap& vmap, assetVar* aV, const char* pname)
{
    assetVar* av = nullptr;
    if (0)
        FPS_PRINT_INFO("called here name [{}]", pname);
    char* avName = nullptr;
    if (aV->gotParam(pname))
    {
        avName = aV->getcParam(pname);
        if (avName)
        {
            av = getVar(vmap, avName, nullptr);
        }
    }
    // we wont make it here keep it simple
    return av;
}
// gets a version of an assetlist
int VarMapUtils::getAssetListVersion(varsmap& vmap, const char* alistname, const char* alistVersion)
{
    int rc = 1;
    char* aload;
    vmlen = asprintf(&aload, "saved_configs/%s/%s", alistname, alistVersion);
    if (aload)
    {
        // now load
        configure_vmap(vmap, aload);
        free((void*)aload);
        rc = 0;
    }
    return rc;
}

// DEFFERED  lock varmap
// get one or get them all
void VarMapUtils::getMapsVm(varsmap& vmap, varsmap& vmr, const char* uri, const char* var)
{
    if (0)
        FPS_PRINT_INFO("getting vm maps uri [{}] var [{}]", uri ? uri : "noURI", var ? var : "noVar");

    if (uri && !var)
    {
        auto x = vmap.find(uri);
        if (x != vmap.end())
        {
            // auto x = vmap.find(uri);
            for (auto y : vmap[uri])
            {
                vmr[x->first.c_str()][y.first.c_str()] = y.second;
            }
        }
    }
    else if (uri && var)
    {
        auto x = vmap.find(uri);
        if (x != vmap.end())
        {
            //                auto x = vmap.find(uri);
            auto y = vmap[uri].find(var);
            if (y != vmap[uri].end())
            {
                //                  auto y = vmap[uri].find(var);
                vmr[x->first.c_str()][y->first.c_str()] = y->second;
            }
        }
    }
    else
    // get all the objects
    {
        for (auto& x : vmap)
        {
            for (auto& y : vmap[x.first])
            {
                if (y.second != nullptr)
                {
                    vmr[x.first.c_str()][y.first.c_str()] = y.second;
                }
                else
                {
                    FPS_PRINT_INFO("NOTE no map for var [{}]", y.first);
                }
            }
        }
    }
    return;
}

bool VarMapUtils::strMatch(const char* str, const char* key)
{
    // char* key2=nullptr;
    if (0)
        FPS_PRINT_INFO(" Match test for str [{}] key [{}]", str, key);

    // char rep = 0;
    int mlen = 0;
    if (key)
        mlen = strlen(key);
    if (mlen == 0 || (mlen > 0 && key[0] == '*'))
    {
        if (0)
            FPS_PRINT_INFO("Match true for str [{}] key [{}]", str, key);
        return true;
    }
    if (!str)
    {
        FPS_PRINT_INFO("no str for match  key [{}]", key);
        return false;
    }
    if (strncmp(str, key, mlen) == 0)
    {
        if (0)
            FPS_PRINT_INFO("Match true  for str [{}] key [{}] mlen {}", str, key, mlen);
        return true;
    }

    return false;
}
// DEFERRED lock varmap
// get one or get them all
cJSON* VarMapUtils::getCompsCj(varsmap& vmap, const char* key, const char* var)
{
    if (0)
        FPS_PRINT_INFO("getting cj comps key [{}] var [{}]"

                       ,
                       key ? key : "noKey", var ? var : "noVar");

    cJSON* cj = cJSON_CreateObject();

    if (key && !var)
    {
        for (auto& x : vmap)
        {
            // if (strncmp(x.first, key, strlen(key))== 0)
            if (strMatch(x.first.c_str(), (char*)key))
            {
                if (0)
                    FPS_PRINT_INFO(
                        " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
                        "getting cj map for [{}]",
                        x.first);

                cJSON_AddStringToObject(cj, x.first.c_str(), "none");
                if (0)
                    FPS_PRINT_INFO(" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<got cj map for [{}]", x.first);
            }
        }
    }
    else if (key && var)
    {
        for (auto& x : vmap)
        {
            if (strMatch(x.first.c_str(), (char*)key))
            {
                cJSON* cjx = cJSON_CreateObject();

                for (auto& y : vmap[x.first.c_str()])
                {
                    if (y.second != nullptr)
                    {
                        if (strMatch(y.first.c_str(), (char*)var))
                        {
                            cJSON* cjy = cJSON_CreateObject();

                            y.second->showvarValueCJ(cjy);
                            cJSON_AddItemToObject(cjx, y.first.c_str(), cjy);
                        }
                    }
                }
                cJSON_AddItemToObject(cj, x.first.c_str(), cjx);
            }
        }
    }
    else
    // get all the objects
    {
        for (auto& x : vmap)
        {
            if (0)
                FPS_PRINT_INFO(
                    " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting "
                    "cj map for [{}]",
                    x.first);

            cJSON_AddStringToObject(cj, x.first.c_str(), "none");
            if (0)
                FPS_PRINT_INFO(" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<got cj map for [{}]", x.first);
        }
    }
    if (0)
        FPS_PRINT_INFO(" <<<<<<<<<<<<<<<<<<<<<<<<<<<got cj maps", NULL);

    return cj;
}

void VarMapUtils::getCompsVm(varsmap& vmap, varsmap& rmap, const char* key, const char* var)
{
    varmap rvmap;
    if (key && !var)
    {
        for (auto& x : vmap)
        {
            if (strMatch(x.first.c_str(), (char*)key))
            {
                rmap[x.first] = rvmap;
            }
        }
    }
    else if (key && var)
    {
        for (auto& x : vmap)
        {
            if (strMatch(x.first.c_str(), (char*)key))
            {
                for (auto& y : vmap[x.first.c_str()])
                {
                    if (y.second != nullptr)
                    {
                        if (strMatch(y.first.c_str(), (char*)var))
                        {
                            rmap[x.first][y.first] = y.second;
                        }
                    }
                }
            }
        }
    }
    else
    // get all the objects
    {
        for (auto& x : vmap)
        {
            for (auto& y : vmap[x.first.c_str()])
            {
                if (y.second != nullptr)
                {
                    rmap[x.first][y.first] = y.second;
                }
            }
        }
    }
    return;
}

void VarMapUtils::processRawMsg(varsmap& vmap, const char* method, const char* uri, const char* replyto,
                                const char* body, cJSON** cjr, asset_manager* am, asset* ai)
{
    int single = 0;
    if (strcmp(method, "set") == 0)
    {
        FPS_PRINT_INFO("{}: process set body [{}].", program_invocation_short_name, body);
        if (replyto != nullptr)
            return processMsgSetReply(vmap, method, uri, replyto, body, cjr, am, ai);
        else
            return processMsgSetPub(vmap, method, uri, single, body, cjr, am, ai);
    }
    else if (strcmp(method, "pub") == 0)
        return processMsgSetPub(vmap, method, uri, single, body, cjr, am, ai);

    else if (strcmp(method, "get") == 0)
        return processMsgGet(vmap, method, uri, body, cjr, am, ai);

    return;
}

// all these assume nfrags == 2 except get
// vmaps is really a name space , vm is an accessor to that name space
void VarMapUtils::processFims(varsmap& vmap, fims_message* msg, cJSON** cjr, asset_manager* am, asset* ai)
{
    return processRawMsg(vmap, msg->method, msg->uri, msg->replyto, msg->body, cjr, am, ai);
}

void VarMapUtils::free_fims_message(fims_message* msg)
{
    if (msg)
    {
        if (msg->method)
            free(msg->method);
        if (msg->uri)
            free(msg->uri);
        if (msg->replyto)
            free(msg->replyto);
        if (msg->body)
            free(msg->body);
        if (msg->pfrags)
            free(msg->pfrags);

        free(msg);
    }
}
// turns body: method: replyto: uri: into a fims message ( we also need to go
// the other way.
fims_message* VarMapUtils::bufferToFims(const char* buffer)
{
    // pull out fields for return
    cJSON* root = cJSON_Parse(buffer);
    if (root == nullptr)
    {
        FPS_PRINT_INFO("{}: Failed to parse message.", program_invocation_short_name);
        return nullptr;
    }

    fims_message* message = (fims_message*)malloc(sizeof(fims_message));

    if (message == nullptr)
    {
        FPS_PRINT_INFO("{}: Allocation error.", program_invocation_short_name);
        return nullptr;
    }
    //    return nullptr;
    message->uri = nullptr;
    message->method = nullptr;
    message->replyto = nullptr;
    message->body = nullptr;
    message->nfrags = 0;
    message->pfrags = nullptr;

    cJSON* body = cJSON_GetObjectItem(root, "body");
    if (body != nullptr)
    {
        if (body->valuestring == nullptr)
        {
            message->body = cJSON_PrintUnformatted(body);
        }
        else
        {
            message->body = strdup(body->valuestring);
        }
    }

    cJSON* method = cJSON_GetObjectItem(root, "method");
    if (method != nullptr)
    {
        message->method = strdup(method->valuestring);
    }

    cJSON* replyto = cJSON_GetObjectItem(root, "replyto");
    if (replyto != nullptr)
    {
        message->replyto = strdup(replyto->valuestring);
    }

    // return nullptr;
    cJSON* uri = cJSON_GetObjectItem(root, "uri");
    if (uri != nullptr)
    {
        message->uri = strdup(uri->valuestring);
        // build pfrags
        int count = 0;
        int offset[MAX_URI_DEPTH];
        for (int i = 0; message->uri[i] != '\0' && count < MAX_URI_DEPTH; i++)
        {
            if (message->uri[i] == '/')
            {
                offset[count] = i;
                count++;
            }
        }
        message->nfrags = count;
        if (count > 0 && count < MAX_URI_DEPTH)
            message->pfrags = (char**)calloc(count, sizeof(char*));
        else
        {
            FPS_PRINT_INFO("{}: count {} Invalid number of segments in URI [{}]", program_invocation_short_name, count,
                           uri->valuestring);
        }
        for (int i = 0; i < count; i++)
        {
            message->pfrags[i] = message->uri + (offset[i] + 1);
        }
    }

    cJSON_Delete(root);
    // xxx
    return message;
}

// This function will create a  FIMS message buffer
char* VarMapUtils::fimsToBuffer(const char* method, const char* uri, const char* replyto, const char* body)
{
    if (method == nullptr || uri == nullptr)
    {
        printf("[%s] xxx Can't transmit message body [%s] without method or uri.", __func__,
               body ? body : " No body either");
        return nullptr;
    }
    // build json object
    cJSON* message = cJSON_CreateObject();
    if (message == nullptr)
    {
        FPS_PRINT_INFO("{}: Memory allocation error.", program_invocation_short_name);
        return nullptr;
    }
    cJSON_AddStringToObject(message, "method", method);
    cJSON_AddStringToObject(message, "uri", uri);
    if (replyto != nullptr)
        cJSON_AddStringToObject(message, "replyto", replyto);
    if (body != nullptr)
        cJSON_AddStringToObject(message, "body", body);

    // create message buffer
    char* tmp_str = cJSON_PrintUnformatted(message);
    cJSON_Delete(message);
    return tmp_str;
}

void VarMapUtils::clearVm(varmap& vmap)
{
    for (auto& y : vmap)
    {
        if (y.second)
        {
            delavMap.insert(std::pair<assetVar*, void*>(y.second, nullptr));
            vmap[y.first] = nullptr;
        }
        // if(y.second->users- <= 0)
        // {
        //     FPS_PRINT_INFO(" deleting [{}]\n", y.first.c_str());
        //     delete y.second;

        // }
    }
    vmap.clear();
}

void VarMapUtils::clearVmap(varsmap& vmap)
{
    for (auto& x : vmap)
    {
        if (vmdebug)
            FPS_PRINT_INFO("delete table [{}]", x.first);
        for (auto& y : vmap[x.first])
        {
            if (vmdebug)
                FPS_PRINT_INFO(">>>> delete table [{}] {}"

                               ,
                               y.first, fmt::ptr(y.second));
            assetVar* av = y.second;

            if (y.second)
            {
                delavMap.insert(std::pair<assetVar*, void*>(y.second, nullptr));

                // if(0)FPS_PRINT_INFO("%s >> >>>> delete av [{}] {} comp [{}:{}] \n "
                //
                //     , y.first.c_str()
                //     , y.second
                //     , av->comp.c_str()
                //     , av->name.c_str()
                //     );
                // if(y.second->users--<= 0)
                // {
                //     delete y.second;
                // }
                vmap[x.first][y.first] = nullptr;
            }
            else
            {
                if (vmdebug)
                    FPS_PRINT_INFO("skipping delete for av {}", fmt::ptr(av));
            }
        }
        zapAlist(vmap, x.first.c_str());
        vmap[x.first].clear();
    }
    vmap.clear();
}

// configure_vmap
// use a list of reps to modify the file
int VarMapUtils::configure_vmap(varsmap& vmap, const char* fname,
                                std::vector<std::pair<std::string, std::string>>* reps, asset_manager* am, asset* ai)
{
    // int rc = 0;
    cJSON* cjbase = get_cjson(fname, reps);
    return configure_vmapCJ(vmap, cjbase, am, ai);
}

int VarMapUtils::configure_vmapCJ(varsmap& vmap, cJSON* cjbase, asset_manager* am, asset* ai, bool delCJ)
{
    int rc = 0;
    if (!cjbase)
        rc = -1;
    cJSON* cj = nullptr;
    if (cjbase)
        cj = cjbase->child;
    // const char* vname;
    while (cj)
    {
        // uri  cj->string
        // uri->body  cj->child
        // FPS_PRINT_INFO(" cj->string [{}] child [{}]\n", cj->string, (void *)
        // cj->child);
        char* body = cJSON_Print(cj);
        if (0)
            FPS_PRINT_INFO("cj->string [{}] child [{}] body [{}]", cj->string, fmt::ptr(cj->child), body);

        char* buf = fimsToBuffer("set", cj->string, nullptr, body);
        free((void*)body);
        fims_message* msg = bufferToFims(buf);
        free((void*)buf);
        cJSON* cjb = nullptr;
        processFims(vmap, msg, &cjb, am, ai);
        free_fims_message(msg);
        if (0)
        {
            buf = cJSON_Print(cjb);
            FPS_PRINT_INFO("configured [{}]", buf);
            free((void*)buf);
        }
        if (cjb)
            cJSON_Delete(cjb);
        cj = cj->next;
    }
    if (delCJ && cjbase)
        cJSON_Delete(cjbase);
    return rc;
}

void VarMapUtils::CheckLinks(varsmap& vmap, varmap& amap, const char* aname)
{  // vmap["/links/bms_1"]
    UNUSED(amap);
    auto ix = vmap.find(aname);
    if (ix != vmap.end())
    {
        // if this works no need to run the init function below
        if (0)
            FPS_PRINT_INFO("We found our links, we should be able to set up our link amap", NULL);
        for (auto iy : ix->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                assetVar* av = iy.second;
                assetVal* aVal = av->linkVar ? av->linkVar->aVal : av->aVal;

                FPS_PRINT_INFO("linking [{}] to [{}]"

                               ,
                               iy.first.c_str(), aVal->valuestring);
                // for example lets link [AcContactor] to the var defined for
                // [/status/bms_1:AcContactor]

                // amap[iy.first] = vm.getVar (vmap, y.second->aVal->valuestring);//
                // getVar(varsmap &vmap, const char* comp, const char* var=nullptr)

                // amap["AcContactor"]               = linkVal(vmap, link,
                // "AcContactor",            fval);
            }
        }
    }
}

// dbvl
// DEFERRED  add timeout
// template<class T>
bool VarMapUtils::valueChanged(double one, double theother, double deadb)
{
    double diff = one - theother;

    if (diff > deadb || diff < -deadb)
        return true;
    else
        return false;
}

// template<class T>
bool VarMapUtils::valueChanged(double one, double theother)
{
    double diff = one - theother;

    if (diff)
        return true;
    else
        return false;
}

// return when one or the other has changed
// or when we have passed a time
// DEFERRED  test valueChangedtimeout
// template<class T>
bool VarMapUtils::valueChanged(assetVar* one, assetVar* theother, assetVar* deadb, double vtype, double timeout)
{
    bool resp;
    // T dval;
    resp = valueChanged(one->getVal(vtype), theother->getVal(vtype), deadb->getVal(vtype));
    if (!resp)
    {
        assetVal* aVal = one->linkVar ? one->linkVar->aVal : one->aVal;

        if (timeout > 0.0 && aVal->getsTime() + timeout > getTime())
        {
            return true;
        }
    }
    return resp;
}

// template<class T>
bool VarMapUtils::valueChangednodb(assetVar* one, assetVar* theother, double vtype, double timeout)
{
    bool resp;
    // T dval;
    resp = valueChanged(one->getVal(vtype), theother->getVal(vtype));
    if (!resp)
    {
        assetVal* aVal = one->linkVar ? one->linkVar->aVal : one->aVal;
        if (timeout > 0.0 && aVal->getsTime() + timeout > getTime())
        {
            return true;
        }
    }
    return resp;
}

// deprecated
// TODO is getSubCount still used?  check after MVP
int VarMapUtils::getSubCount(char* scopy)
{
    int i = 0;
    char* sp = scopy;
    while (*sp && *sp == ' ')
        sp++;
    if (*sp)
        i++;
    while (*sp)
    {
        if (*sp == ',')
        {
            if (0)
                FPS_PRINT_INFO("found comma in [{}] ccnt {}", sp, i);
            i++;
        }
        sp++;
    }
    if (0)
        FPS_PRINT_INFO("found comma in [{}] ccnt {}", scopy, i);

    return i;
}

// free
int VarMapUtils::showList(char** subs, const char* aname, int& ccnt)
{
    int i;
    for (i = 0; i < ccnt; i++)
    {
        FPS_PRINT_INFO("subs [{}] [{}] = [{}]", aname, i, subs[i]);
    }
    return i;
}

int VarMapUtils::addListToVec(vecmap& vecs, char** subs, const char* vname, int& ccnt)
{
    int i;
    for (i = 0; i < ccnt; i++)
    {
        vecMapAddChar(vecs, vname, subs[i]);
        //            FPS_PRINT_INFO("subs [%d] = [{}]\n",i, subs[i]);
    }
    return i;
}

int VarMapUtils::clearList(char** subs, const char* aname, int& ccnt)
{
    UNUSED(aname);
    int i;
    for (i = 0; i < ccnt; i++)
    {
        free(subs[i]);
    }
    free(subs);
    return 0;
}

void VarMapUtils::getVList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt)
{
    char** slist = getList(vecs, vmap, amap, aname, vname, ccnt);
    clearList(slist, aname, ccnt);
}

char** VarMapUtils::getVmapList(vecmap& vecs, varsmap& vmap, int& ccnt)
{
    UNUSED(vecs);
    char** vars = nullptr;
    ccnt = 0;
    for (auto x : vmap)
    {
        for (auto y : vmap[x.first])
        {
            ccnt++;
        }
    }
    if (ccnt > 0)
    {
        vars = (char**)calloc(ccnt + 1, sizeof(char*));
        ccnt = 0;
        for (auto x : vmap)
        {
            for (auto y : vmap[x.first])
            {
                vmlen = asprintf(&vars[ccnt++], "%s:%s", x.first.c_str(), y.first.c_str());
            }
        }
    }
    return vars;
}
// sets up the internal structure for an assetList from a string right now but
// we'll add the template file option real soon.
int VarMapUtils::setupAssetList(varsmap& vmap, const char* aname, const char* alistname, const char* alistVersion,
                                asset_manager* am)
{
    UNUSED(alistVersion);
    UNUSED(am);
    int ccnt = 0;
    vecmap vecs;  // may not be used
    varmap amap;
    // amap not used here
    char** assets = getList(vecs, vmap, amap, aname, alistname, ccnt);

    // then look for a template file
    if (ccnt == 0)
    {
        varsmap tmap;
        char* aload = nullptr;
        vmlen = asprintf(&aload, "saved_configs/%s/%s", alistname, "template");
        if (aload)
        {
            configure_vmap(tmap, aload);
            assets = getVmapList(vecs, tmap, ccnt);
            free((void*)aload);
        }
    }

    if (0)
        FPS_PRINT_INFO("seeking [{}] Found ccnt as {}", alistname, ccnt);
    if (ccnt > 0)
    {
        if (0)
            showList(assets, aname, ccnt);
        assetList* alist = setAlist(vmap, alistname);
        // now we need to add the assets mentioned.  we make have to make the
        // assets.
        //
        // one for each ccnt in assets
        //
        for (int iy = 0; iy < ccnt; iy++)
        {
            double dval = 0.0;  // this will set up the av a a double
            assetVar* av = setVal(vmap, assets[iy], nullptr, dval);
            alist->add(av);
        }
        clearList(assets, nullptr, ccnt);
    }
    return ccnt;
}

char** VarMapUtils::getVecListbyName(vecmap& vecs, const char* vname, int& ccnt)
{
    ccnt = 0;
    char** sret = nullptr;
    std::vector<std::string>* vx = vecMapGetVec(vecs, vname, std::string("dummy"));
    if (!vx)
    {
        FPS_PRINT_INFO("no entry in vecs for [{}]"

                       ,
                       vname);
        return sret;
    }

    ccnt = vx->size();
    sret = (char**)calloc(ccnt + 1, sizeof(char*));
    int idx = 0;
    for (auto vi : *vx)
    {
        sret[idx++] = strdup(vi.c_str());
    }
    sret[idx] = nullptr;
    return sret;
}

// char** getListStr(vecmap &vecs, varsmap &vmap, varmap &amap, const char*
// aname, const char* vname,  int &ccnt, char* dbsubs)
char** VarMapUtils::getListStr(vecmap& vecs, varsmap& vmap, const char* vname, int& ccnt, char* dbsubs)
{
    UNUSED(vecs);
    UNUSED(vmap);
    char** sret = nullptr;
    char* scopy;

    ccnt = 0;
    if (dbsubs)
    {
        FPS_PRINT_INFO("recovered [{}] as [{}]", vname, dbsubs);
        char* sp = dbsubs;
        while (*sp && *sp == ' ')
            sp++;
        scopy = strdup(sp);

        ccnt = getSubCount(scopy);
        sret = (char**)calloc(ccnt + 1, sizeof(char*));
        sp = scopy;
        int idx = 0;
        int retcc = 0;
        while (idx < ccnt)
        {
            while (*sp && (*sp == ',' || *sp == ' '))
                sp++;
            if (strlen(sp) > 0)
            {
                sret[retcc] = strdup(sp);
                sp = sret[retcc];
                while (*sp && *sp != ',' && *sp != ' ')
                    sp++;
                *sp++ = 0;
                retcc++;
            }
            idx++;
        }
        free((void*)scopy);
        // addListToVec(vecs, sret, aname, retcc);
        // showList(sret,aname, retcc);

        sret[retcc] = nullptr;
        ccnt = retcc;
    }
    return sret;
}

char** VarMapUtils::getList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname, int& ccnt)
{
    char** sret;

    char* dbsubs = nullptr;
    amap[vname] = setLinkVal(vmap, aname, "/config", vname, dbsubs);  //        vmap.clear();
    dbsubs = amap[vname]->getcVal();
    //     exit(0);
    // }

    sret = getListStr(vecs, vmap, vname, ccnt, dbsubs);
    if (sret)
    {
        if (0)
            FPS_PRINT_INFO("found list string dbsubs [{}] ccnt [{}] vname [{}]", dbsubs, ccnt, vname);
        addListToVec(vecs, sret, vname, ccnt);
        showList(sret, aname, ccnt);
    }
    else
    {
        ccnt = 0;
        FPS_PRINT_INFO("No List recovered for aname [{}]  [{}]", aname, vname);
    }
    return sret;
}

char** VarMapUtils::getDefList(vecmap& vecs, varsmap& vmap, varmap& amap, const char* aname, const char* vname,
                               int& ccnt)
{
    char** sret;

    char* dbsubs = nullptr;
    amap[vname] = setLinkVal(vmap, aname, "/config", vname, dbsubs);
    dbsubs = (char*)"/components, /assets, /params ";
    sret = getListStr(vecs, vmap, vname, ccnt, dbsubs);
    if (sret)
    {
        addListToVec(vecs, sret, aname, ccnt);
        showList(sret, aname, ccnt);
    }
    else
    {
        ccnt = 0;
        FPS_PRINT_INFO("No dbsubs recovered for [{}]", vname);
    }
    return sret;
}

// vecMap utils    template<class T>
// vecMaps are vectors of things like pubs and subs
// given a name and a type create or add an entry
template <class T>
void VarMapUtils::vecMapAddEntry(vecmap& vecm, const char* name, T val)
{
    auto ix = vecm.find(name);
    if (ix == vecm.end())
    {
        vecm[name] = (new std::vector<T>);
    }
    std::vector<T>* ve = vecm[name];
    ve->push_back(val);
}

void VarMapUtils::vecMapAddChar(vecmap& vecm, const char* name, const char* val)
{
    return vecMapAddEntry(vecm, name, std::string(val));
}
// user has to decode vector type
// vecMapAddEntry(vecm, "Subs",std::string("/assets"));
// void * ve = vecMapGetVec(vecm, "Subs");
// std::vector<std::string> ve = (std::vector<std::string>)*vx;

template <class T>
std::vector<T>* VarMapUtils::vecMapGetVec(vecmap& vecm, const char* name, T val)
{
    UNUSED(val);
    auto ix = vecm.find(name);
    if (ix != vecm.end())
    {
        return vecm[name];
    }
    return nullptr;
}

void VarMapUtils::testvecMap(void)
{
    vecmap vcmap;
    vecMapAddChar(vcmap, "test", "testchar1");
    vecMapAddChar(vcmap, "test", "testchar2");
    std::vector<std::string>* vx = vecMapGetVec(vcmap, "test", std::string("dummy"));
    int idx = 0;
    for (auto ix : *vx)
    {
        FPS_PRINT_INFO("entry {} is [{}]", idx++, ix);
    }
}

void VarMapUtils::showvecMap(vecmap& vcmap, const char* key)
{
    // vecMapAddChar(vcmap,"test","testchar1");
    // vecMapAddChar(vcmap,"test", "testchar2");
    if (key)
    {
        std::vector<std::string>* vx = vecMapGetVec(vcmap, key, std::string("dummy"));
        int idx = 0;
        if (vx)
        {
            for (auto ix : *vx)
            {
                FPS_PRINT_INFO("key [{}] > entry [{}] is [{}]", key, idx++, ix);
            }
        }
        else
        {
            FPS_PRINT_INFO("no entries for key [{}]", key);
        }
    }
    else
    {
        for (auto x : vcmap)
        {
            showvecMap(vcmap, x.first.c_str());
        }
    }
}

cJSON* loadAmap(VarMapUtils* vm, varsmap& vmap, int single, const char* comp, const char* var, const char* body,
                asset_manager* am, char* uri);
// do this for singles
cJSON* VarMapUtils::loadVmap(varsmap& vmap, int single, const char* comp, const char* var, const char* body,
                             asset_manager* am, char* uri)
{
    char* xsp = nullptr;
    cJSON* cjr = nullptr;
    if (uri && strncmp(uri, "/amap", strlen("/amap")) == 0)
    {
        return loadAmap(this, vmap, single, comp, var, body, am, uri);
    }
    if (single & 1)
    {
        if ((single && 0x0010) == 0)
        {
            vmlen = asprintf(&xsp, "{\"%s\":{\"value\":%s}}", var, body);
        }
        else
        {
            vmlen = asprintf(&xsp, "{\"%s\":%s}", var, body);
        }
        if (0)
            FPS_PRINT_INFO("running single value [{}]", xsp);
        processMsgSetPub(vmap, "set", comp, single, xsp, &cjr, am);
    }
    else
    {
        processMsgSetPub(vmap, "set", comp, single, body, &cjr, am);
    }

    if (xsp)
        free((void*)xsp);

    return cjr;
}

// if we supply a baseUri it needs to be removed from the component name
cJSON* VarMapUtils::getVmap(varsmap& vmap, int& single, const char* key, const char* var, int opts, const char* baseUri)
{
    // varsmap vmr;
    // get matching components
    // does not work yet getCompsVm(vmap, vmr, key, var);

    // Hack for assets
    if (strncmp(key, "/assets", strlen("/assets")) == 0)
    {
        opts |= 0x0100;
    }

    if (0 | process_fims_debug)
        FPS_PRINT_INFO("RUNNING key [{}] var [{}] single {:#04x} opts {:#04x} baseUri {}", key ? key : "no key",
                       var ? var : " no Var", single, opts, baseUri ? baseUri : "No BaseURI");
    cJSON* cj = nullptr;

    // TODO review use of single and opts after MVP merge single and opts
    if (single & 0x0001)
    {
        if (opts & 0x0010)
        {
            opts &= ~0x0001;
        }

        if (0 || process_fims_debug)
            FPS_PRINT_INFO("Running getMapsCj #1 key [{}] var [{}] opts {:#04x}", key, var, opts);
        cj = getMapsCj(vmap, key, var, opts);
    }
    else
    {
        if (single & 0x1000)
        {
            opts |= 0x0001;
        }
        if (0 || process_fims_debug)
            FPS_PRINT_INFO("Running getMapsCj #2 key [{}] var [{}] opts {:#04x}", key, var ? var : "no Var", opts);
        cJSON* cjall = getMapsCj(vmap, key, nullptr, opts);
        if (0)
            FPS_PRINT_INFO("Ran key [{}] var [{}] single {:#04x} cjall {}", key ? key : "no Key", var ? var : "no Var",
                           single, fmt::ptr(cjall));

        if (cjall)
        {
            if (opts & 0x0001)
            {
                cj = cjall;
            }
            else
            {
                cJSON* cjd = cJSON_DetachItemFromObject(cjall, key);

                if (cjd)
                {
                    cj = cjd;
                    cJSON_Delete(cjall);
                    auto tmp = nullptr;  // cJSON_Print(cj);

                    if (0)
                        FPS_PRINT_INFO("Detached cjd  key [{}] tmp [{}]", key ? key : "no Key", tmp ? tmp : "no cjson");
                    if (tmp)
                        free(tmp);
                }
                else
                {
                    cj = cjall;
                    auto tmp = nullptr;  // cJSON_Print(cj);
                    if (0)
                        FPS_PRINT_INFO("Used  cjall  key [{}] tmp [{}]", key ? key : "no Key", tmp ? tmp : " no cj");
                    if (tmp)
                        free(tmp);
                }
            }
        }
    }
    process_fims_debug = 0;
    return cj;
}

// This is the UI map work.
// do this for singles
cJSON* VarMapUtils::loadUimap(varsmap& vmap, int single, const char* comp, const char* var, const char* body,
                              asset_manager* am)
{
    char* xsp = nullptr;
    cJSON* cjr = nullptr;
    // Dummy case ( I think)
    if (single & 0x0001)
    {
        if ((single && 0x0010) == 0)
        {
            vmlen = asprintf(&xsp, "{\"%s\":{\"value\":%s}}", var, body);
        }
        else
        {
            vmlen = asprintf(&xsp, "{\"%s\":%s}", var, body);
        }
        processMsgSetPubUi(vmap, "set", comp, single, xsp, &cjr, am);
    }
    else
    {
        processMsgSetPubUi(vmap, "set", comp, single, body, &cjr, am);
    }

    if (xsp)
        free((void*)xsp);

    return cjr;
}

cJSON* VarMapUtils::getUimap(varsmap& vmap, int& single, const char* key, const char* var)
{
    // varsmap vmr;
    // get matching components
    // does not work yet getCompsVm(vmap, vmr, key, var);

    if (0)
        FPS_PRINT_INFO("RUNNING  key   [{}] var [{}] single {:#04x}", key, var, single);
    int opts = 0;
    cJSON* cj = nullptr;
    if (single & 0x0001)
    {
        cj = getMapsCj(vmap, key, var);
    }
    else
    {
        if (single & 0x1000)
        {
            opts = 1;
        }
        cJSON* cjall = getMapsCj(vmap, key, nullptr, opts);
        if (cjall)
        {
            if (opts == 1)
            {
                cj = cjall;
            }
            else
            {
                cJSON* cjd = cJSON_DetachItemFromObject(cjall, key);

                if (cjd)
                {
                    cj = cjd;
                }
                cJSON_Delete(cjall);
            }
        }
    }
    return cj;
}

// int CheckReload(varsmap& vmap, varmap& amap, const char* aname, const char*
// fname, void* func = nullptr);

// standard check reload function
int VarMapUtils::CheckReload(varsmap& vmap, varmap& amap, const char* aname, const char* fname, void* func)
{
    int reload;
    assetVar* av = amap[fname];

    if (!av || (reload = av->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload == 0)
    {
        // reload = 0;
        amap[fname] = setLinkVal(vmap, aname, "/reload", fname, reload);
    }

    if (func)
    {
        setFunc(vmap, aname, fname, func);
    }
    return reload;
}

int cJSON_GetNumObjects(cJSON* cjl)
{
    int num = 0;
    cJSON* cji = cjl->child;
    while (cji)
    {
        num++;
        cji = cji->next;
    }
    return num;
}

// uiObject
// if its name look for name= no objet
// all other items have objects "active_thing":{or they are params
//
// How do we designate a alarm/fault object  We see ui_type as an alarm in
// loadmap does NOT correctly decode a bool parm
assetVar* VarMapUtils::setValfromCj(varsmap& vmap, const char* comp, const char* var, cJSON* cj, int uiObject)
{
    assetList* alist = nullptr;
    // bool newAv = false;

    // setvar_debug = 0;
    if ((setvar_debug) || !var)
    {
        if (0 || setvar_debug)
        {
            char* tcj = nullptr;
            if (cj)
                tcj = cJSON_PrintUnformatted(cj);
            FPS_PRINT_INFO("Seeking Variable comp 1 [{}] var [{}] cj is {} [{}]", comp, var, fmt::ptr(cj),
                           cj ? tcj : "NoCj");
            if (tcj)
                free((void*)tcj);
        }
    }
    assetUri my(comp, var);

    // see if we have an alist
    alist = getAlist(vmap, my.Uri);

    if (setvar_debug)
        FPS_PRINT_INFO("Seeking Variable comp 2 [{}] var [{}]  param [{}] uiObject {} alist {}"

                       ,
                       my.Uri, my.Var ? my.Var : " no Var", my.Param ? my.Param : " no Param", uiObject,
                       fmt::ptr(alist));

    // //Now process the list of objects.

    assetVar* av = nullptr;
    cJSON* cjval = cJSON_GetObjectItem(cj, "value");
    cJSON* cjname = cJSON_GetObjectItem(cj, "name");
    if (cjname && !cjval)
    {
        uiObject = 1;
    }

    if (setvar_debug)
        FPS_PRINT_INFO("Suspected UI Object comp [{}] my.Uri [{}] uiObject {}", comp, my.Uri, uiObject);

    if (uiObject > 0)
    {
        alist = getAlist(vmap, my.Uri);
        // //Now process the list of objects.

        if (!alist)
        {
            // newAv = true;
            alist = setAlist(vmap, my.Uri);

            if (0)
                FPS_PRINT_INFO("new assetList for [{}]", my.Uri);
        }
    }
    // just a URI , process the list of assetsVars
    if ((my.Var == nullptr) && (cjval == nullptr) && (cjname == nullptr))
    {
        cJSON* cj3 = cj;
        if (cj->child)
            cj3 = cj3->child;
        while (cj3)
        {
            if (0)
                FPS_PRINT_INFO("new assetVar ##1  [{}:{}] uiObject {}"

                               ,
                               my.Uri, cj3->string, uiObject);

            av = setValfromCj(vmap, my.Uri, cj3->string, cj3, uiObject);
            if (alist && av)
            {
                alist->add(av);
                av = nullptr;
            }
            if (av->valueChanged())
            {
                if (vmdebug)
                    FPS_PRINT_INFO("ValueChanged #2 for [{}] [{}]", av->getfName(), av);
            }
            cj3 = cj3->next;
        }
        return av;
    }
    // try it this way
    // no my.Var means no av which is used later to set value
    // we can search for an existing av anyway if not we'll have to make it
    // but we still have to solve for my.Var being nullptr
    if (my.Var)
    {
        av = getVar(vmap, my.Uri, my.Var);
    }
    if (av)
    {
        if (0 || setvar_debug)
        {
            char* tmp = nullptr;
            if (cj)
                tmp = cJSON_PrintUnformatted(cj);
            FPS_PRINT_INFO(
                "Careful here Setting Old Vari comp [{}] var [{}] param "
                "[{}] cj is {}  extras [{}] cjisObject [{}] cj [{}]"

                ,
                my.Uri, my.Var, my.Param ? my.Param : "no Param", fmt::ptr(cj), fmt::ptr(av->extras),
                cJSON_IsObject(cj), tmp);
            if (tmp)
                free(tmp);
        }
        // this does not setup detailed components .. but note we may want to update
        // params.
        if (my.Param)
        {
            setParamfromCj(vmap, my, cj);
            // setParamfromCj(vmap, my.Uri, my.Var, my.Param, cj);
            return av;
        }
    }
    else
    {
        if (0 || setvar_debug)
            FPS_PRINT_INFO("Setting New Variable comp [{}] var [{}] Param [{}] cj is {}"

                           ,
                           my.Uri, my.Var ? my.Var : "No Var", my.Param ? my.Param : "no Param", fmt::ptr(cj));
    }

    // we have to add a new variable perhaps we need special flags to enable this

    // this sections adds a new variable, looks for a single value
    // if(!av)
    //{
    // examine what we have
    if (av)
    {
        if (av->lock)
        {
            if (1 || setvar_debug)
                FPS_PRINT_INFO("AssetVar is locked. Value  [{}] will not be changed\n", av);
            return av;
        }
    }
    if (cJSON_IsObject(cj))
    {
        int numcj = cJSON_GetNumObjects(cj);
        cJSON* cjval = cJSON_GetObjectItem(cj, "value");

        cJSON* cjname = cJSON_GetObjectItem(cj, "name");
        cJSON* cjui_type = cJSON_GetObjectItem(cj, "ui_type");
        bool ui_control = false;
        bool ui_alarm = false;
        bool ui_status = false;
        const char* ui_type = "None";

        if (cjui_type)
        {
            if (strcmp(cjui_type->valuestring, "control") == 0)
            {
                ui_control = true;
                ui_type = "control";
            }
            if (strcmp(cjui_type->valuestring, "alarm") == 0)
            {
                ui_alarm = true;
                ui_type = "alarm";
            }
            if (strcmp(cjui_type->valuestring, "status") == 0)
            {
                ui_status = true;
                ui_type = "status";
            }
        }
        if (av == nullptr)
        {
            if (0 || setvar_debug)
                FPS_PRINT_INFO(
                    "Adding a new Variable from OBJECT comp [{}] var [{}] "
                    "cj is {} cjval is {} cjname is {} cjui_type is {}"

                    ,
                    my.Uri, my.Var ? my.Var : "No Var", fmt::ptr(cj), fmt::ptr(cjval ? cjval : nullptr),
                    fmt::ptr(cjname ? cjname : nullptr), fmt::ptr(cjui_type ? cjui_type : nullptr));
        }
        else
        {
            if (0 || setvar_debug)
                FPS_PRINT_INFO(
                    "Using existing Variable from OBJECT comp [{}] var [{}] "
                    "cj is {} cjval is {} cjname is {} cjui_type is {}"

                    ,
                    my.Uri, my.Var ? my.Var : "No Var", fmt::ptr(cj), fmt::ptr(cjval ? cjval : nullptr),
                    fmt::ptr(cjname ? cjname : nullptr), fmt::ptr(cjui_type ? cjui_type : nullptr));
        }
        // av = nullptr;
        // HMM ui_controls do NOT have a value if they are boolean ....
        // So we have to create one to receive the incoming data
        // UI Alarms we have to show the options vec as alarms.
        // test for no value in cj
        // we did not have a value but we still may have params .. its just like
        // that !!!
        if (!cjval)
        {
            // bool vv = cJSON_IsTrue(cjval);  ??
            if (!av && my.Var)
            {
                bool vv = false;
                av = makeVar(vmap, my.Uri, my.Var, vv);
                av->ui_type = ui_control ? 1 : ui_alarm ? 2 : ui_status ? 3 : 0;
                if (0 || setvar_debug)
                    FPS_PRINT_INFO("Adding a new ui control [{}]  OBJECT Uri [{}] var [{}]"

                                   ,
                                   ui_type, my.Uri, my.Var ? my.Var : "No Var");
                if (alist && av)
                {
                    alist->add(av);
                    // av = nullptr;
                }
            }
            // this is a special for the ui name  cjname->string is used instead of
            // my.Var
            else if (!av && cjname)
            {
                av = setValfromCj(vmap, my.Uri, cjname->string, cjname, uiObject);
                if (av)
                    av->setNaked = true;
                if (0 || setvar_debug)
                    FPS_PRINT_INFO("Adding name  ui  [{}]  OBJECT Uri [{}] var [{}] val [{}]"

                                   ,
                                   ui_type, my.Uri, cjname->string, cjname->valuestring);
                // this adds the name in at the top
                if (alist && av)
                {
                    alist->add(av);
                    // av = nullptr;
                }
                cJSON* cji = cjname->next;
                while (cji)
                {
                    av = setValfromCj(vmap, my.Uri, cji->string, cji, uiObject);
                    if (0)
                        FPS_PRINT_INFO("added uiObject [{}]", cji->string);
                    cji = cji->next;
                }
            }
            // debug show alist
            if (0 || setvar_debug)
            {
                if (alist)
                {
                    unsigned int ix = alist->size();
                    for (unsigned int i = 0; i < ix; i++)
                    {
                        if (alist->avAt(i))
                        {
                            if (0)
                                FPS_PRINT_INFO("alist item [{}] [{}]", i, alist->avAt(i)->name);
                        }
                    }
                }
            }
        }
        // did we find a value ??
        // if so, we have to create the right kind of av
        // perhaps we need makexVar(vmap, uri,var,cj);
        // this may be too early to set the value we may need the params
        if (cjval)
        {
            av = getVar(vmap, my.Uri, my.Var);
            if (0)
                FPS_PRINT_INFO("for sure setting value for [{}:{}] av {}", my.Uri, my.Var, fmt::ptr(av));
            if (0)
                FPS_PRINT_INFO("double check for more than one object  numcj {}", numcj);
            // Did not work
            // debug later
            // if (!av)
            // {
            //     av = makexVar(vmap, my.Uri, my.Var, cjval);
            // }
            // if(av)
            // {
            //     av->setVal(cjval);
            // }

            // if av does not exist we'll have to make it here but we nee the right
            // type but do not run setVal yet or at least do not run any actions we
            // need the whole cj structure in place.

            if (cJSON_IsNumber(cjval))
            {
                double vv = cjval->valuedouble;
                // av = setVal(vmap, my.Uri, my.Var, vv);
                if (!av)
                    av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            else if (cJSON_IsBool(cjval))
            {
                bool vv = cJSON_IsTrue(cjval);
                if (!av)
                    av = makeVar(vmap, my.Uri, my.Var, vv);
            }
            else if (cJSON_IsString(cjval))
            {
                const char* vv = cjval->valuestring;
                // av = setVal(vmap, my.Uri, my.Var, vv);
                if (!av)
                    av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            // add it to any running alist
            if (alist && av)
            {
                alist->add(av);
                // av = nullptr;
            }
            if (0)
                FPS_PRINT_INFO("av created if needed setval later", NULL);
        }
        if (av)
        {
            if (av->lock)
            {
                if (1 || setvar_debug)
                    FPS_PRINT_INFO("AssetVar is locked. Value [{}] will not be changed\n", av);
                return av;
            }
        }
        // now look for these dudes
        cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
        cJSON* cjparam = cJSON_GetObjectItem(cj, "params");
        cJSON* cjopts = cJSON_GetObjectItem(cj, "options");
        cJSON* cjnopts = cJSON_GetObjectItem(cj, "new_options");
        cJSON* cjfuncs = cJSON_GetObjectItem(cj, "functions");
        cJSON* cjvars = cJSON_GetObjectItem(cj, "variables");
        cJSON* cjact2 = cJSON_GetObjectItem(cj, "$$action");  // true or false to trigger actions
        if (cjact2)
        {
            cjact2 = cJSON_DetachItemFromObject(cj, "$$action");
        }

        // cJSON*
        cjname = cJSON_GetObjectItem(cj, "name");

        if (0 || setvar_debug)
            FPS_PRINT_INFO(
                "Adding a new Variable from OBJECT actions [{}] "
                "params[{}] options[{}]]"

                ,
                fmt::ptr(cjact), fmt::ptr(cjparam), fmt::ptr(cjopts));
        // // what about setfunc
        if (cjact)
        {
            if (setvar_debug)
                FPS_PRINT_INFO("new Adding actions starting for [{}] av [{}]", var, fmt::ptr(av));
            if (av)
            {
                // this will delete old actions as well
                setActfromCj(av, cjact);  // note this must delete any old actions
            }
            if (setvar_debug)
                FPS_PRINT_INFO("new Adding actions done for [{}] av [{}]", var, fmt::ptr(av));
        }
        // may need to add these to basedict
        if (cjparam)
        {
            if (0)
                FPS_PRINT_INFO("Adding params for [{}] av [{}]", var, fmt::ptr(av));
            if (av)
            {
                if (!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (!av->extras->featDict)
                {
                    av->extras->featDict = new assetFeatDict;
                }
                av->extras->featDict->addCj(cjparam);
            }
        }
        //        if (ui_control || ui_alarm || (uiObject > 0))
        // TODO after MVP make "options":{ ...} and "options":[....]  do the same
        // sort of thing (options:[..] just stored cjson encoded values)
        {
            if (0)
                FPS_PRINT_INFO("Adding base params starting for [{}] av [{}]", var, fmt::ptr(av));
            if (av)
            {
                if (!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (!av->extras->baseDict)
                {
                    av->extras->baseDict = new assetFeatDict;
                }
                // do not add value as a baseparam
                av->extras->baseDict->addCj(cj, 1 /*uiObject*/, false /*skip name */);
            }
            if (0)
                FPS_PRINT_INFO("Adding base params done for [{}] av [{}]", var, fmt::ptr(av));
        }
        if (cjnopts)
        {
            cjopts = cjnopts;
        }

        if (cjopts || cjfuncs || cjvars)
        {
            if (!av->extras)
            {
                av->extras = new assetExtras;
            }
            av->extras->optName = "options";
        }

        if (cjfuncs)
        {
            cjopts = cjfuncs;
            av->extras->optName = "functions";
        }
        if (cjvars)
        {
            cjopts = cjvars;
            av->extras->optName = "variables";
        }
        if (cjopts)
        {
            if (0)
                FPS_PRINT_INFO("Adding options starting for [{}] av [{}] cjopts type {}", var, fmt::ptr(av),
                               cjopts->type);
            // it may be an array
            // in any case addCj should handle it.
            if (av)
            {
                if (ui_control)
                    av->ui_type = 1;
                if (ui_alarm)
                    av->ui_type = 2;
                if (ui_status)
                    av->ui_type = 3;

                if (cjopts->type == cJSON_Array)
                {
                    if (0)
                        FPS_PRINT_INFO("Adding options Vec for [{}] av [{}] cjopts type {}", var, fmt::ptr(av),
                                       cjopts->type);

                    if (av->extras->optVec && cjnopts)
                    {
                        delete av->extras->optVec;
                        av->extras->optVec = nullptr;
                    }

                    if (!av->extras->optVec)
                    {
                        if (0)
                            FPS_PRINT_INFO("New Options Vec for [{}] av [{}] cjopts type {}", var, fmt::ptr(av),
                                           cjopts->type);
                        av->extras->optVec = new assetOptVec;
                    }
                    av->extras->optVec->addCj(cjopts);
                    if (cjfuncs)
                    {
                        av->extras->optVec->name = "functions";
                    }
                    if (cjvars)
                    {
                        av->extras->optVec->name = "variables";
                    }
                    if (0)
                        FPS_PRINT_INFO("New Options Vec  name [{}] cjopts {}", av->extras->optVec->name,
                                       fmt::ptr(av->extras->optVec->cjopts));
                }
                else
                {
                    if (!av->extras->optDict)
                    {
                        av->extras->optDict = new assetFeatDict;
                    }
                    av->extras->optDict->addCj(cjopts);
                }
            }
            if (0)
                FPS_PRINT_INFO("Adding options done for [{}] av [{}] cjopts type {}", var, fmt::ptr(av), cjopts->type);
        }

        if (cjval)
        {
            if (0)
                FPS_PRINT_INFO("possibly 1 setting value for [{}:{}]", my.Uri, my.Var);
            av = getVar(vmap, my.Uri, my.Var);
            // bool setvalue = true;
            if (av)
            {
                if (av->lock)
                {
                    if (1 || setvar_debug)
                        FPS_PRINT_INFO("AssetVar is locked. Value [{}] will not be changed\n", av);
                    return av;
                }
            }
            if (cJSON_IsString(cjval))
            {
                const char* vv = cjval->valuestring;
                // this is now in a better place
                // stops the actions triggering
                if (strcmp(vv, "$$") == 0)
                {
                    my.setValue = false;
                    // setValue = false;
                }
                if (cjact2)
                {
                    my.setValue = cJSON_IsTrue(cjact2);
                }

                // av = setVal(vmap, my.Uri, my.Var, vv);
                // if(!av)
                //     av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            // Did not work
            // debug later
            // if (!av)
            // {
            //     av = makeVar(vmap, my.Uri, my.Var, cjval);
            // }
            // if(av)
            // {
            //     av->setVal(cjval);
            // }

            // if av does not exist we'll have to make it here but we nee the right
            // type but do not run setVal yet or at least do not run any actions we
            // need the whole cj structure in place.

            if (cJSON_IsNumber(cjval))
            {
                double vv = cjval->valuedouble;
                av = setVal(vmap, my.Uri, my.Var, vv);
                // if(!av)
                //    av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            else if (cJSON_IsBool(cjval))
            {
                bool vv = cJSON_IsTrue(cjval);
                if (my.index == -1)
                {
                    av = setVal(vmap, my.Uri, my.Var, vv);
                }
                else
                {
                    if (av)
                    {
                        if (0)
                            FPS_PRINT_INFO("set index var [{}] [{}] [{}]", av->name, my.index, vv);
                        av->setVal(my.index, vv);
                    }
                }
                // if(!av)
                //     av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            else if (cJSON_IsString(cjval))
            {
                const char* vv = cjval->valuestring;
                // this may be in the wrong place
                if (strcmp(vv, "$$") == 0)
                {
                    my.setValue = false;
                }
                if (cjact2)
                {
                    my.setValue = cJSON_IsTrue(cjact2);
                }
                av = setVal(vmap, my, vv);
                // av = setVal(vmap, my.Uri, my.Var, vv);
                // if(!av)
                //     av = makeVar(vmap, my.Uri, my.Var, vv);
                // av->setVal(vv);
            }
            if (0)
                FPS_PRINT_INFO("after setting value for [{}:{}] av {}", my.Uri, my.Var, fmt::ptr(av));
            if (cjact2)
            {
                cJSON_Delete(cjact2);
                cjact2 = nullptr;
            }
        }
        return av;
    }
    // NOT an object   but we still may need to trigger actions
    // will setVal do that o we have to use varmaputils::setval
    // assetVar* VarMapUtils::setValfromCj(varsmap& vmap, const char* comp, const
    // char* var, cJSON* cj, int uiObject)  assetVar* VarMapUtils::setVal(varsmap&
    // vmap, const char* comp, const char* var, T& value)
    //
    if (0 || setvar_debug)
        FPS_PRINT_INFO("NOT an OBJECT Var comp [{}] var [{}] cj is {} type {}"

                       ,
                       my.Uri, my.Var ? my.Var : "No Var", fmt::ptr(cj), cj ? cj->type : -1);
    // again we need makeVar(vmap,uri,var,cj)

    if (cJSON_IsNumber(cj))
    {
        double vv = cj->valuedouble;
        // if(!av)
        //     av = makeVar(vmap, my.Uri, my.Var, vv);
        av = setVal(vmap, my.Uri, my.Var, vv);
    }
    else if (cJSON_IsBool(cj))
    {
        bool vv = cJSON_IsTrue(cj);
        if (my.index == -1)
        {
            av = setVal(vmap, my.Uri, my.Var, vv);
        }
        else
        {
            if (av)
            {
                av->setVal(my.index, vv);
            }
        }
        // if(!av)
        //     av = makeVar(vmap, my.Uri, my.Var, vv);
        // av->setVal(vv);
        // av = setVal(vmap, my.Uri, my.Var, vv);
    }
    else if (cJSON_IsString(cj))
    {
        const char* vv = cj->valuestring;
        av = setVal(vmap, my.Uri, my.Var, vv);
    }
    else
    {
        // PSW 11/11/2021 skip a value "$$"
        // can we ever get here ????
        // Yes if we have an array to deal with
        cJSON* cjval = cJSON_GetObjectItem(cj, "value");
        cJSON* cjact = cJSON_GetObjectItem(cj, "actions");
        cJSON* cjparam = cJSON_GetObjectItem(cj, "params");
        cJSON* cjopts = cJSON_GetObjectItem(cj, "options");

        if (cjval)
        {
            if (0)
                FPS_PRINT_INFO("possibly 2 setting value for [{}:{}]", my.Uri, my.Var);
        }

        // TODO review after MVP possibly add more options here like deadband
        // test turn it off
        // cjact = nullptr;
        // if (setvar_debug)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (0 || setvar_debug)
                FPS_PRINT_INFO(
                    "NOT AN OBJECT  setValfrom_Cj >>Adding NEW cj >>{}<< "
                    "cjval {} cjact {}"

                    ,
                    tmp, fmt::ptr(cjval), fmt::ptr(cjact));
            free((void*)tmp);
        }
        assetVar* av = nullptr;
        ///
        if (cJSON_IsArray(cj))
        {
            int asize = cJSON_GetArraySize(cj);
            if (0)
                FPS_PRINT_INFO("setValfrom_Cj >>Setval for cj  array size {}", asize
                               //, (void *)cjact
                );

            if (asize == 1)
            {
                //"ems_cmd":[{"value":0,"string":"Initial"}]
                cJSON* cji = cJSON_GetArrayItem(cj, 0);
                if (cji)
                    cjval = cJSON_GetObjectItem(cji, "string");
            }
        }
        // This is called when we have an array not an object.
        if (cjval && my.Var)
        {
            if (0)
                FPS_PRINT_INFO("Out of Band Adding CJ value for [{}]", var);
            av = setValfromCj(vmap, my.Uri, my.Var, cjval, uiObject);
            // cJSON_Delete(cjval);
        }
        if (cjact)
        {
            if (0)
                FPS_PRINT_INFO("Out of Band Adding actions for [{}] av [{}]", var, fmt::ptr(av));
            if (av)
            {
                setActfromCj(av, cjact);
            }
        }

        if (cjparam)
        {
            if (0)
                FPS_PRINT_INFO("Out of Band Adding params for [{}] av [{}]", var, fmt::ptr(av));
            if (av)
            {
                if (!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (!av->extras->featDict)
                {
                    av->extras->featDict = new assetFeatDict;
                }
                av->extras->featDict->addCj(cjparam);
            }
        }
        if (cjopts)
        {
            if (1)
                FPS_PRINT_INFO("FUNNY STUFF Adding options for [{}] av [{}] cjopts type {}", var, fmt::ptr(av),
                               cjopts->type);
            // it may be an array
            // in any case addCj should handle it.
            if (av)
            {
                if (!av->extras)
                {
                    av->extras = new assetExtras;
                }
                if (cjopts->type == cJSON_Array)
                {
                    if (1)
                        FPS_PRINT_INFO("FUNNY STUFF Adding options for [{}] av [{}] cjopts type {}", var, fmt::ptr(av),
                                       cjopts->type);

                    if (!av->extras->optVec)
                    {
                        if (1)
                            FPS_PRINT_INFO(
                                "FUNNY STUFF Adding New options for [{}] av [{}] "
                                "cjopts type {}",
                                var, fmt::ptr(av), cjopts->type);
                        av->extras->optVec = new assetOptVec;
                    }
                    av->extras->optVec->addCj(cjopts);
                }
                else
                {
                    if (!av->extras->optDict)
                    {
                        av->extras->optDict = new assetFeatDict;
                    }
                    av->extras->optDict->addCj(cjopts);
                }
            }
        }
        // NOW we need to deal with actions ...
    }
    // if (mycomp != comp)free((void*)mycomp);
    return av;
}

// How do we designate a alarm/fault object  We see ui_type as an alarm in
// loadmap

// setAlarm(varsmap &vmap,"/assets" ,"bms_1","alarms_1","Battery Voltage Alarm",
// 2);
int VarMapUtils::setAlarm(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* atext,
                          int retval)
{
    return 0;
    char* body;
    char* auri;
    cJSON* cj = nullptr;
    vmlen = asprintf(&body,
                     "{\"value\":1,\"ui_type\":\"alarm\",\"options\":[{\"id\":%d,"
                     "\"time\":%f,\"name\":\"%s\",\"return_value\":%d}]}",
                     alarmId++, get_time_dbl(), atext, retval);
    vmlen = asprintf(&auri, "/%s/%s", base, aname);

    if (0)
        FPS_PRINT_INFO("Alarm uri [{}] Body [{}]", auri, body ? body : "aint got no body");

    if (body)
    {
        cj = cJSON_Parse(body);
    }
    if (auri && cj)
    {
        setValfromCj(vmap, auri, vname, cj, 1);
    }
    if (auri)
        free((void*)auri);
    if (body)
        free((void*)body);
    if (cj)
        cJSON_Delete(cj);
    return 0;
}

// av.sendAlarm(vmap,  "srcUri", "destAv","atype","msg, severity)
asset_log* VarMapUtils::sendAlarm(varsmap& vmap, const char* srcUri, const char* destUri, const char* atype,
                                  const char* msg, int severity)
{
    // return nullptr;
    assetVar* srcAv = getVar(vmap, srcUri, nullptr);
    if (!srcAv)
    {
        int ival = 1;
        srcAv = makeVar(vmap, srcUri, nullptr, ival);
    }
    if (!srcAv)
    {
        if (1)
            FPS_PRINT_INFO("falied to make var for [{}]", srcUri);
        return nullptr;
    }
    return sendAlarm(vmap, srcAv, destUri, atype, msg, severity);
}

// av.sendAlarm(vmap, srcAv, "atype", "destAv", "msg", severity)
asset_log* VarMapUtils::sendAlarm(varsmap& vmap, assetVar* srcAv, const char* destUri, const char* atype,
                                  const char* msg, int severity)
{
    assetVar* destAv = getVar(vmap, destUri, nullptr);
    if (!destAv)
    {
        int ival = 1;
        destAv = makeVar(vmap, destUri, nullptr, ival);
    }
    if (0)
        FPS_PRINT_INFO("Alarm dest Uri [{}] av {}", destUri, fmt::ptr(destAv));

    return srcAv->sendAlarm(destAv, atype, msg, severity);
}

int VarMapUtils::clearAlarm(varsmap& vmap, assetVar* srcAv, assetVar* destAv, const char* atype, const char* msg,
                            int severity)
{
    UNUSED(vmap);
    UNUSED(msg);
    UNUSED(severity);
    return srcAv->clearAlarm(destAv, atype);
}

int VarMapUtils::clearAlarm(varsmap& vmap, const char* srcUri, const char* destUri, const char* atype, const char* msg,
                            int severity)
{
    assetVar* srcAv = getVar(vmap, srcUri, nullptr);
    if (!srcAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to find srcVar for [{}]", srcUri);
        return -1;
    }
    return clearAlarm(vmap, srcAv, destUri, atype, msg, severity);
}

int VarMapUtils::clearAlarm(varsmap& vmap, assetVar* srcAv, const char* destUri, const char* atype, const char* msg,
                            int severity)
{
    assetVar* destAv = getVar(vmap, destUri, nullptr);
    if (!destAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to find destVar for [{}]", destUri);
        return -1;
    }
    return clearAlarm(vmap, srcAv, destAv, atype, msg, severity);
}

int VarMapUtils::clearAlarms(varsmap& vmap, const char* destUri)
{
    assetVar* destAv = getVar(vmap, destUri, nullptr);
    if (!destAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to find destVar for [{}]", destUri);
        return -1;
    }
    return destAv->clearAlarms();
}

template <class T>
void VarMapUtils::setParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname,
                           T val)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to make var for [{}]", suri);
        return;
    }
    return srcAv->setParam(pname, val);
}

int VarMapUtils::getiParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        int val = 1;
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to make var for [{}]", suri);
        return 0;
    }
    return srcAv->getiParam(pname);
}

double VarMapUtils::getdParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        double val = 1;
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to make var for [{}]", suri);
        return 0;
    }
    return srcAv->getdParam(pname);
}

bool VarMapUtils::getbParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;

    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        bool val = true;
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to make var for [{}]", suri);
        return 0;
    }
    return srcAv->getbParam(pname);
}

char* VarMapUtils::getcParam(varsmap& vmap, const char* base, const char* aname, const char* vname, const char* pname)
{
    std::string suri = base;
    suri += "/";
    suri += aname;
    suri += ":";
    suri += vname;
    assetVar* srcAv = getVar(vmap, suri.c_str(), nullptr);
    if (!srcAv)
    {
        char* val = (char*)"noVal";
        srcAv = makeVar(vmap, suri.c_str(), nullptr, val);
    }
    if (!srcAv)
    {
        if (0)
            FPS_PRINT_INFO("failed to make var for [{}]", suri);
        return 0;
    }
    return srcAv->getcParam(pname);
}

int VarMapUtils::setAvFunc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* am,
                           const char* vname,
                           int (*func)(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* am))
{
    //*funMap[aname][vname] = (assetVar*)func;
    assetFunc* SetAvCmd = (assetFunc*)new assetFunc(aname);
    SetAvCmd->setupRavFunc(func, vmap, amap, aname, p_fims, am);
    if (!amap[vname]->extras)
    {
        amap[vname]->extras = new assetExtras;
    }
    amap[vname]->extras->SetFunc = (assetVar*)SetAvCmd;
    return 0;
}

assetList* VarMapUtils::setAlist(varsmap& vmap, const char* uri)
{
    assetList* alist = nullptr;
    char* tmp = nullptr;
    // // no leading "/" so its hard to find
    vmlen = asprintf(&tmp, "%s", uri);
    if (tmp)
    {
        alist = new assetList(tmp);
        if (0)
            FPS_PRINT_INFO("creating alist [{}]", tmp);
        assetVar* av = makeVar(vmap, (const char*)"_assetList", (const char*)tmp, alist);
        if (av)
        {
            av->aVar = (assetVar*)alist;
        }
        if (0)
            FPS_PRINT_INFO("creating alist [{}] av[{}] alist[{}]", tmp, fmt::ptr(av), fmt::ptr(alist));
        free(tmp);
    }
    return alist;
}
// zap the asset list if we have one
void VarMapUtils::zapAlist(varsmap& vmap, const char* uri)
{
    if (0)
        FPS_PRINT_INFO("get alist uri [{}]", uri);

    assetList* alist = nullptr;
    char* tmp = nullptr;
    // // no leading "/" so its hard to find
    if (uri)
        vmlen = asprintf(&tmp, "%s", uri);
    if (tmp)
    {
        assetVar* av = getVar(vmap, "_assetList", tmp);
        if (av)
        {
            alist = (assetList*)av->aVar;
            if (alist)
            {
                delete alist;
                av->aVar = nullptr;
                if (1)
                    FPS_PRINT_INFO("zapped alist [{}] av [{}] alist [{}]"

                                   ,
                                   tmp, fmt::ptr(av), fmt::ptr(alist));
            }
        }
        free(tmp);
    }
    return;
}
// get the asset list if we have one
assetList* VarMapUtils::getAlist(varsmap& vmap, const char* uri)
{
    if (0)
        FPS_PRINT_INFO("get alist uri [{}]", uri);

    assetList* alist = nullptr;
    char* tmp = nullptr;
    // // no leading "/" so its hard to find
    if (uri)
        vmlen = asprintf(&tmp, "%s", uri);
    if (tmp)
    {
        assetVar* av = getVar(vmap, "_assetList", tmp);
        if (av)
        {
            alist = (assetList*)av->aVar;
        }
        if (vmdebug)
            FPS_PRINT_INFO("looking for alist [{}] av [{}] alist [{}]"

                           ,
                           tmp, fmt::ptr(av), fmt::ptr(alist));

        free(tmp);
    }
    return alist;
}

// Time stuff
long int VarMapUtils::get_time_us()
{
    long int ltime_us;
    timespec c_time;
    clock_gettime(CLOCK_MONOTONIC, &c_time);
    ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
    return ltime_us;
}
TSCNS tn;
double tsc_ghz;

long int get_tsc_us()
{
    int64_t ltime_ns = tn.rdns();
    return ltime_ns / 1000;
}

std::mutex timeDoubleMutex;
double VarMapUtils::get_time_dbl()
{
    if (base_time == 0.0)
    {
        std::lock_guard<std::mutex> lock(timeDoubleMutex);
        if (base_time != 0.0)
            return (double)get_tsc_us() / 1000000.0 - base_time;
        tsc_ghz = tn.init();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        tsc_ghz = tn.calibrate();
        std::cout << std::setprecision(17) << "tsc_ghz:" << tsc_ghz << std::endl;
        int64_t rdns_latency;
        {
            const int N = 100;
            int64_t before = tn.rdns();
            int64_t tmp = 0;
            for (int i = 0; i < N - 1; i++)
            {
                tmp += tn.rdns();
            }
            int64_t after = tn.rdns();
            rdns_latency = (after - before) / N;
            std::cout << "rdns_latency: " << rdns_latency << " tmp: " << tmp << std::endl;
        }

        int idx = 5;
        while (0 && --idx)
        {
            int64_t a = tn.rdns();
            int64_t b = tn.rdsysns();
            int64_t c = tn.rdns();
            int64_t a2b = b - a;
            int64_t b2c = c - b;
            bool good = a2b >= 0 && b2c >= 0;
            int64_t rdsysns_latency = c - a - rdns_latency;
            std::cout << "a: " << a << ", b: " << b << ", c: " << c << ", a2b: " << a2b << ", b2c: " << b2c
                      << ", good: " << good << ", rdsysns_latency: " << rdsysns_latency << std::endl;
            // std::this_thread::sleep_for(std::chrono::miliseconds(1000));
            auto expire = tn.rdns() + 1000000000;
            while (tn.rdns() < expire)
                ;
        }
        // base_time = get_time_us()/ 1000000.0;
        base_time = get_tsc_us() / 1000000.0;
    }

    // return  (double)get_time_us() / 1000000.0 - base_time;
    double rval;
    {
        std::lock_guard<std::mutex> lock(timeDoubleMutex);
        rval = (double)get_tsc_us() / 1000000.0 - base_time;
    }

    return rval;
}

double VarMapUtils::get_time_ref()
{
    return (double)(get_time_us() / 1000000.0) - ref_time;
}

double VarMapUtils::set_time_ref(int year, int month, int day, int hour, int min, int sec)
{
    tm ltm;
    memset(&ltm, 0, sizeof(tm));
    ltm.tm_year = year;
    ltm.tm_mon = month;
    ltm.tm_mday = day;
    ltm.tm_hour = hour;
    ltm.tm_min = min;
    ltm.tm_sec = sec;
    ref_time = (double)mktime(&ltm);
    return ref_time;
}

tm* VarMapUtils::get_local_time_now()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t tnow = std::chrono::system_clock::to_time_t(now);
    return localtime(&tnow);
}

// void VarMapUtils::set_base_time()
// {
//     if (1)base_time = get_time_us();
// }

// do this before a var update
void VarMapUtils::setTime()
{
    g_setTime = get_time_dbl();
}
// do this before a var update
double VarMapUtils::getTime()
{
    return g_setTime;  // = get_time_dbl();
}

time_t VarMapUtils::getTNow(double tNow)
{
    UNUSED(tNow);
    time_t tnow = time(nullptr);  //(time_t)(tNow + g_base_time);
    return tnow;
}

varsmap* VarMapUtils::createVlist()
{
    varsmap* vl = new varsmap;
    return vl;
}

cJSON* VarMapUtils::getVmapCJ(varmap& vm)
{
    cJSON* cji = cJSON_CreateObject();

    for (auto iy : vm)
    {
        if (0)
            FPS_PRINT_INFO("getting cj for [{}]", iy.first);
        iy.second->showvarCJ(cji);
        if (0)
            FPS_PRINT_INFO("got cj for [{}]", iy.first);
    }
    return cji;
}

bool VarMapUtils::notMissing(varmap& amap, const char* func, const char* vname)
{
    if (amap.find(vname) == amap.end())
    {
        FPS_PRINT_INFO("[{}] is Missing", func, vname);
        return false;
    }
    return true;
}

int VarMapUtils::vListAddVar(varsmap& vmap, assetVar* av, const char* comp)
{
    // allow /comp/c/v:name /components/ess/some/system:varname
    // if(vmap.find(av->comp) == vmap.end())
    if (av)
    {
        if (comp)
            vmap[comp][av->name] = av;
        else
            vmap[av->comp][av->name] = av;
    }
    return 0;
}

int VarMapUtils::addVlist(varsmap* vl, assetVar* av, const char* comp)
{
    vListAddVar(*vl, av, comp);
    return 0;
}

// new wrinkle send to another assetvar
int VarMapUtils::sendVlist(fims* p_fims, const char* method, varsmap* vl, bool sim, assetVar* avd)
{
    vListSendFims(*vl, method, p_fims, nullptr, sim, avd);
    return 0;
}

// new wrinkle send to another assetvar
int VarMapUtils::sendDbiVlist(fims* p_fims, const char* method, varsmap* vl, bool sim, assetVar* avd)
{
    DbivListSendFims(*vl, method, p_fims, nullptr, sim, avd);
    return 0;
}
void VarMapUtils::clearVmapxx(varsmap& vmap, bool delAv)
{
    for (auto& x : vmap)
    {
        for (auto& y : vmap[x.first])
        {
            // only delete the vars you own..
            assetVar* av = y.second;
            if (delAv && av)
            {
                delavMap.insert(std::pair<assetVar*, void*>(y.second, nullptr));

                // if(av->users- <= 0)
                // {
                //     delete y.second;
                // }
                vmap[x.first][y.first] = nullptr;
            }
        }
        zapAlist(vmap, x.first.c_str());
        vmap[x.first].clear();
    }
    vmap.clear();
}

void VarMapUtils::clearVlist(varsmap* vl)
{
    if (vmdebug)
        FPS_PRINT_INFO("running ...", NULL);
    clearVmapxx(*vl, false);
    delete vl;
}
// TODO is get_nfrags used ?? we should use assetUri, review after MVP
int VarMapUtils::get_nfrags(const char* uri)
{
    int nfrags = 0;
    const char* sp = uri;
    while (*sp)
    {
        if (*sp++ == '/')
            nfrags++;
    }
    return nfrags;
}

void assetList::add(assetVar* av)
{
    if (0)
        FPS_PRINT_INFO("2 aList {} add av [{}] size {}", fmt::ptr(&aList), av->name, aList.size());

    unsigned int ix = aList.size();
    for (unsigned int i = 0; i < ix; i++)
    {
        if (aList[i] == av)
            return;
    }
    aList.push_back(av);
}

int assetList::size(void)
{
    return (int)aList.size();
}

assetVar* assetList::avAt(unsigned int ix)
{
    if (0)
        FPS_PRINT_INFO("1 aList {} ix {}", fmt::ptr(&aList), ix);
    // if(0)FPS_PRINT_INFO("%s >> 1 name [{}] aList {} ix %d \n", name, &aList,
    // ix);
    if (0)
        FPS_PRINT_INFO("2 aList {} size {}", fmt::ptr(&aList), aList.size());
    if (ix < aList.size())
        return aList.at(ix);
    return nullptr;
}

template <class T>
assetVar* VarMapUtils::makeVar(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetVar* av = nullptr;
    assetUri my(comp, var);

    if (my.Var)
    {
        av = new assetVar(my.Var, my.Uri, value);
        delavMap.insert(std::pair<assetVar*, void*>(av, nullptr));
        vmap[my.Uri][my.Var] = av;
        if (0)
            FPS_PRINT_INFO("seeking compFunc for Av input av [{}:{}]", my.Uri, my.Var);
        void* compFunc = nullptr;

        if (!funMapp)
            funMapp = &funMap;

        if (funMapp)
        {
            compFunc = getFunc(vmap, "comp", my.Uri,
                               av);  //   go find this when we make the var
        }
        if (compFunc)
        {
            if (!av->extras)
            {
                av->extras = new assetExtras;
            }
            av->extras->compFunc = compFunc;

            if (0)
                FPS_PRINT_INFO("FOUND compFunc {} for Av {} my.Uri [{}] input av [{}:{}]", fmt::ptr(compFunc),
                               fmt::ptr(av), my.Uri, av->comp, av->name);
        }

        if (0)
            FPS_PRINT_INFO("created [{}:{}] fname {} av {}", my.Uri, my.Var, av->getfName(), fmt::ptr(av));
    }
    else
    {
        if (1)
            FPS_PRINT_ERROR("FAILED [{}:{}]", comp, var);
    }

    return av;
}

assetVar* VarMapUtils::setVal(varsmap& vmap, const char* comp, const char* var, assetVar* aV, const char* pname)
{
    if (!aV->gotParam(pname))
    {
        return nullptr;
    }
    auto param = aV->extras->baseDict->featMap[pname];
    switch (param->type)
    {
        case assFeat::AINT:
            return setVal(vmap, comp, var, param->valueint);
        case assFeat::AFLOAT:
            return setVal(vmap, comp, var, param->valuedouble);
        case assFeat::ASTRING:
            return setVal(vmap, comp, var, param->valuestring);
        case assFeat::ABOOL:
            return setVal(vmap, comp, var, param->valuebool);
        default:
        {
            if (0)
                FPS_PRINT_ERROR("unable to determine av [{}]  af::type [{}]", aV->getfName(), (int)param->type);
            return nullptr;
        }
    }
    return nullptr;
}
template <class T>
assetVar* VarMapUtils::setVal(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetUri my(comp, var);
    return setVal(vmap, my, value);
}

template <class T>
assetVar* VarMapUtils::setVal(varsmap& vmap, assetUri& my, T& value)
{
    assetVar* av = nullptr;
    bool varCreated = false;  // GB
    if (my.Var)
    {
        av = getVar(vmap, my.Uri, my.Var);
        if (!av)
        {
            av = makeVar(vmap, my.Uri, my.Var, value);
            // auto lt = vmap.find("/config/locks"); //GB through 6952
            // if(lt != vmap.end())
            // {
            //     for(auto& xx : vmap["/config/locks"])
            //     {
            //         if(xx.first == my.Uri) //my.Uri exists in /config/locktable and
            //         is set to locked
            //         {
            //             av->lock = xx.second; //xx.second is lock state.
            //             varCreated = true; //allow newly created variable to have
            //             value changed but still add lock for next time.
            //         }
            //     }
            // }
        }
        // now we need to do this
        if (av)
        {
            if (av->lock && !varCreated)
            {
                if (true)
                    FPS_ERROR_PRINT(
                        "[%s] variable [%s:%s] is locked. Please unlock to "
                        "change values or parameters\n",
                        __func__, my.Uri, my.Var);
                return av;
            }
            else
            {
                if (my.Param)
                {
                    if (vmdebug)
                        FPS_PRINT_INFO("running set Param for [{}:{}@{}] (av->extras {})", my.Uri, my.Var, my.Param,
                                       fmt::ptr(av->extras));
                    av->setParam(my.Param,
                                 value);  // needs knowledge of locking status - GB
                    return av;
                }
                if (my.setValue)  // add check for locked status here - GB && !my.varLock
                                  // ?
                {
                    av->setVal(value);
                }
                assetVal* aVal = av->aVal;
                if (av->linkVar)
                {
                    aVal = av->linkVar->aVal;
                    if (vmdebug)
                        FPS_PRINT_INFO("detected link Value for [{}:{}@{}] (av->extras {})", my.Uri, my.Var, my.Param,
                                       fmt::ptr(av->extras));
                }
                if (0 && av->extras)
                    FPS_PRINT_INFO("running  for [{}:{}@{}] (av->extras {}) compFunc {}", my.Uri, my.Var, my.Param,
                                   fmt::ptr(av->extras), fmt::ptr(av->extras ? av->extras->compFunc : nullptr));

                if (av->valueChanged())
                {
                    if (vmdebug)
                        FPS_PRINT_INFO("ValueChanged  for [{}] [{}]", av->getfName(), av);
                }
                // skip all this if the value was "$$"
                if (my.setValue && av->extras)
                {
                    //  if(av->extras->locked){ // Gate 6985 with this condition - GB
                    //          if(vmdebug)FPS_PRINT_INFO("Variable is locked for [{}]"
                    //          av->getFname()) // Added for locking - GB
                    //         continue
                    //     }
                    if (av->extras->SetFunc)
                    {
                        if (vmdebug)
                            FPS_PRINT_INFO("running SetFunc for [{}:{}]", my.Uri, my.Var);
                        setTime();
                        assetFunc* af = (assetFunc*)av->extras->SetFunc;
                        if (af->ramFunc)
                        {
                            af->ramFunc(*af->vmap, *af->amap, af->aname, af->p_fims, af->am);
                        }
                        else if (af->raiFunc)
                        {
                            af->raiFunc(*af->vmap, *af->amap, af->aname, af->p_fims, af->ai);
                        }
                        else if (af->ravFunc)
                        {
                            af->ravFunc(*af->vmap, *af->amap, af->aname, af->p_fims, av);
                        }
                    }
                    if (av->extras->actVec.size() > 0)
                    {
                        // 1.2.0 we will use the actSet Param to select the actions
                        if (av->extras->actVec.find("onSet") != av->extras->actVec.end())
                        {
                            if (0)
                                FPS_PRINT_INFO(
                                    "After Setting  value for [{}] aval (float) {}  "
                                    "(int) {} (char) [{}]"

                                    ,
                                    my.Var, aVal->valuedouble, aVal->valueint,
                                    aVal->valuestring ? aVal->valuestring : "noval");

                            if (0)
                                FPS_PRINT_INFO("setActValfromCj onSet value for [{}]", my.Var);
                            setActVecfromCj(vmap, av);  //, cjact);
                        }
                    }
                    if (av->extras->compFunc)
                    {
                        using myCompFunc_t = int (*)(varsmap & vmap, varmap & amap, const char* aname, fims* p_fims,
                                                     assetVar* av);
                        myCompFunc_t fcn = myCompFunc_t(av->extras->compFunc);
                        if (0)
                            FPS_PRINT_INFO("setActValfromCj run compFunc for [{}] fcn 0x{}", av->name,
                                           av->extras->compFunc);

                        if (av->am)
                        {
                            fcn(vmap, av->am->amap, av->am->name.c_str(), av->am->p_fims, av);
                        }
                        else
                        {
                            if (1)
                                FPS_PRINT_INFO("NO AM for compFunc for [{}] fcn 0x{}", av->name, av->extras->compFunc);
                        }
                    }
                }
            }
        }
    }
    return av;
}

// just keeping this for the template evel.
void VarMapUtils::setMonitorList2(varsmap& vmap, const char* comp, const char* wname)
{
    char* tm = nullptr;
    vmlen = asprintf(&tm, "/schedule/%s/%s", wname, comp);
    if (0)
        FPS_PRINT_INFO(" Running func  [{}]", __func__);
    if (0)
        FPS_PRINT_INFO(" Running for item [{}]", tm);
    char* essName = getSysName(vmap);
    if (vmap.find(tm) != vmap.end())
    {
        auto x = vmap[tm];
        for (auto y : x)
        {
            bool eok = false;
            bool enabled = false;
            bool vecOk = false;
            int funOk = 0;
            char* func = nullptr;
            char* amap = nullptr;
            double rate = 0.0;
            double roff = 0.0;

            assetVar* mav = y.second;
            if (mav)
            {
                eok = mav->gotParam("enabled");
                enabled = mav->getbParam("enabled");
                rate = mav->getdParam("rate");
                roff = mav->getdParam("offset");
            }
            assetVar* av = getVar(vmap, y.first.c_str(), nullptr);
            if (av)
            {
                int ival = 10;
                av->setParam("debug", ival);
            }
            // now find the function in the actions onSet
            //"actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap":
            //"bms"}]}]}
            // extras->actMap["onSet"]->func
            void* res1 = nullptr;
            if (av && av->extras && (av->extras->actVec.find("onSet") != av->extras->actVec.end()))
            {
                vecOk = true;

                auto aa = av->extras->actVec["onSet"];
                for (auto x1 : aa)
                {
                    // x1 = an assetAction

                    // for (auto x2 : x1)
                    // {
                    if (x1->name == "func")
                    {
                        vecOk = true;
                        for (auto& x : x1->Abitmap)
                        {
                            // Set up the bitMap Number
                            // av->abNum = x.first;
                            assetBitField* abf = x.second;
                            func = abf->getFeat("func", &func);
                            amap = abf->getFeat("amap", &amap);
                            res1 = nullptr;
                            if (!amap)
                            {
                                amap = (char*)av->am->name.c_str();
                            }
                            if (func)
                            {
                                // myAvfun_t amFunc;
                                res1 = getFunc(vmap, amap, func);  // added change here -> should
                                                                   // run func for arbitrary
                                                                   // asset managers
                                if (!res1)
                                {
                                    res1 = getFunc(vmap, essName, func);  // added change here ->
                                                                          // should run func for
                                                                          // arbitrary asset
                                                                          // managers
                                }
                                if (res1)
                                {
                                    funOk++;

                                    // setRunAvFunc(vmap, aname, av, func, rate [,offset]);
                                    char* tmp1 = nullptr;
                                    vmlen = asprintf(&tmp1, "/schedule/%s:wake_monitor",
                                                     essName);  //,rate, roff, av->name.c_str());
                                    assetVar* avf = getVar(vmap, tmp1, nullptr);
                                    if (!avf)
                                    {
                                        if (0)
                                            FPS_PRINT_INFO(" test 1 running [{}]", __func__);
                                        FPS_PRINT_INFO("creating AV [{}]", tmp1);
                                        avf = setVal(vmap, tmp1, nullptr, rate);
                                        if (0)
                                            FPS_PRINT_INFO(" test 1 done [{}]", __func__);
                                        avf = getVar(vmap, tmp1, nullptr);

                                        if (avf)
                                        {
                                            avf->setParam("offset", roff);
                                            avf->setParam("enabled", true);
                                            avf->setParam("rate", rate);
                                            avf->setParam("amap", amap);
                                        }
                                    }
                                    if (avf)
                                    {
                                        av->extras->monFunc = res1;  // reinterpret_cast<myAvfun_t>res1);

                                        // now we need to add an option
                                        if (0)
                                            FPS_PRINT_INFO("do we have an optvec {}", fmt::ptr(avf->extras->optVec));
                                        if (!avf->extras->optVec)
                                        {
                                            avf->extras->optVec = new assetOptVec;
                                        }
                                        avf->extras->optVec->name = "functions";
                                        if (!avf->extras->optVec->cjopts)
                                        {
                                            avf->extras->optVec->cjopts = cJSON_CreateArray();
                                        }

                                        char* tmp2 = nullptr;
                                        cJSON* cj = nullptr;
                                        cJSON* cjopts = avf->extras->optVec->cjopts;
                                        vmlen = asprintf(&tmp2,
                                                         "{\"%s:%s\":{"
                                                         "\"%s\":\"%s\","
                                                         "\"%s\":%ld,"
                                                         "\"%s\":\"%s\","
                                                         "\"%s\":\"%s\","
                                                         "\"%s\":\"%s\","
                                                         "\"%s\":%s"
                                                         "}}",
                                                         (const char*)av->comp.c_str(), (const char*)av->name.c_str(),
                                                         "func", func, "funcaddr", (long int)res1, "aname", comp,
                                                         "amap", amap, "test", "this is a test", "enabled", "true");
                                        if (tmp2)
                                        {
                                            cj = cJSON_Parse(tmp2);
                                            if (0)
                                                FPS_PRINT_INFO("added array item from [{}] cj {}", tmp2, fmt::ptr(cj));
                                            free(tmp2);
                                            tmp2 = nullptr;
                                        }
                                        if (cj)
                                        {
                                            cJSON_AddItemToArray(cjopts, cj);
                                        }
                                        // if(tmp1)free(tmp1);
                                    }
                                    if (tmp1)
                                        free(tmp1);
                                }
                            }
                        }
                    }
                }
            }

            if (0)
                FPS_PRINT_INFO(
                    "mvar [{}] eok [{}] av [{}] amap  [{}] enabled [{}] "
                    "vecOk [{}] funOk [{}] func [{}] res1 {} rate {:2.3f}"

                    ,
                    y.first, eok, av ? av->getfName() : "noAV", amap ? amap : "no Amap", enabled, vecOk, funOk,
                    func ? func : "no Func", fmt::ptr(res1), rate);
        }
    }
    else
    {
        FPS_PRINT_INFO("unable to find key [{}]", tm ? tm : " no key");
    }

    if (tm)
        free(tm);
}

// "/schedule/ess/wake1":{
//     "rate":0.1,  used by the scheduler to control when to run a wake1
//     "offset":0.0,
//     "channel": 1,
//     "functions":[                   // this is going to be a new array av
//     used by the wakeup function to work out what to run.
//                                     // simply put an assetVec into extras but
//                                     ,, wait we have options can we use them
//         "CheckComms":{
//             "enabled":true,
//             "oneshot":false,
//             "av":"avname",
//             "func":"0x12345676"
//         },
//         "CheckHB"::{
//             "enabled":true,
//             "av":"avname",
//             "func":"0x12345676"
//         }
//         ....
//     ]
// }
// "options": [
//                 {
//                     "fname": "CheckComms",
//                     "avname":"/a/b/c:d",
//                      "enabled":true,
//                      "oneshot":true,
//                      "func":0x1234
//                 },
//                 {
//                     "name": "Off",
//                     "return_value": false
//                 }
//             ]

//  wname is the desired wakeup
void VarMapUtils::runMonitorList2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, const char* wname,
                                  bool debug)
{
    UNUSED(amap);
    char* essName = getSysName(vmap);
    // this is av....
    auto tm = fmt::format("/schedule/{}/{}", wname, aname);
    if (vmap.find(tm.c_str()) != vmap.end())
    {
        if (debug)
            FPS_PRINT_INFO("found monitor list [{}]", tm);
        auto x = vmap[tm.c_str()];
        for (auto y : x)
        {
            if (debug)
                FPS_PRINT_INFO("found monitor list [{}] item [{}]", tm, y.first);
            assetVar* av = y.second;

            // this is the monitored var
            assetVar* avm = getVar(vmap, y.first.c_str(), nullptr);
            if (debug)
                FPS_PRINT_INFO(" mon [{}] seeking item  [{}] avm found [{}] aname [{}]", tm, y.first, fmt::ptr(avm),
                               cstr{ aname });

            // is this correct ????

            if (0 && avm)
                FPS_PRINT_INFO("avm [{} : {}] asset manager [{}]", avm->name, fmt::ptr(avm),
                               avm->am ? avm->am->name : "null");

            char* amap = av->getcParam("amap");
            if (av->extras && !av->extras->monFunc)
            {
                av->extras->monFunc = getFunc(vmap, amap, av->getcParam("func"));
                if (!av->extras->monFunc)
                {
                    av->extras->monFunc = getFunc(vmap, essName, av->getcParam("func"));

                    if (!av->extras->monFunc)
                    {
                        if (1)
                            FPS_PRINT_ERROR(
                                "no function [{}] for [{}] for avm [{}] trying "
                                "[ess] found {}",
                                av->getcParam("func"), amap ? amap : " no amap", avm ? avm->name.c_str() : "null",
                                fmt::ptr(av->extras->monFunc));
                    }
                    else
                    {
                        if (debug)
                            FPS_PRINT_ERROR(
                                "OK function [{}] for [{}] for avm [{}] trying "
                                "[ess] found {}",
                                av->getcParam("func"), amap ? amap : " no amap", avm ? avm->name.c_str() : "null",
                                fmt::ptr(av->extras->monFunc));
                    }
                }
            }
            // Flexbase may fix up the am
            if (avm && !avm->am)
            {
                avm->am = getaM(vmap, aname);
                FPS_PRINT_INFO("try to fix up  am for [{}]  aname [{}] av->am {}", avm ? avm->getfName() : "No AV",
                               aname, fmt::ptr(avm->am));
            }

            if (!av || !avm)
            {
                if (!avm)
                    FPS_PRINT_INFO(
                        "Missing avm av [{}] avm [{}] amap [{}] aname [{}] func [{}]  "
                        "av->monFunc {} base {}",
                        av ? av->name.c_str() : "No AV", avm ? avm->name.c_str() : "No Avm", amap, aname,
                        av ? av->getcParam("func") : " still no Av",
                        av ? av->extras ? av->extras->monFunc : (void*)0 : (void*)1,
                        av ? getFunc(vmap, essName, av->getcParam("func")) : (void*)2);
                // special just for dbi this can go I think we use the loader now
                if (!avm)
                {
                    // special for DBI
                    if ((strncmp(y.first.c_str(), "/dbi/", strlen("/dbi/")) == 0))
                    {
                        FPS_PRINT_ERROR(" DBI >>create avm [{}] \n", y.first);
                        double dval = 0.0;
                        avm = setVal(vmap, y.first.c_str(), nullptr, dval);
                        avm->am = getaM(vmap, aname);
                        FPS_PRINT_ERROR(" DBI >>create avm [{}] {} aname [{}]\n", y.first, fmt::ptr(avm), aname);
                    }
                }
            }
            if (avm && av->extras && av->extras->monFunc)
            {
                myAvfun_t myfun = reinterpret_cast<myAvfun_t>(av->extras->monFunc);
                if (avm->ai)
                {
                    // We'll need to make sure we're using the right asset instance based
                    // on the asset instance name we're working with (amap)
                    asset* fixed_ai = getaI(vmap, aname);
                    if (fixed_ai)
                        avm->ai = fixed_ai;

                    // Need to check if the assetVar (avm) has an asset manager
                    // If not, then assign avm the asset manager (am) that is in the asset
                    // instance (ai)
                    if (!avm->am)
                    {
                        FPS_PRINT_WARN(
                            "avm [{}] is missing an asset manager (am). "
                            "Assigning asset manager [{}] to avm",
                            avm->getfName(), avm->ai->am ? avm->ai->am->name : "null");
                        avm->am = avm->ai->am;
                    }
                    myfun(vmap, avm->ai->amap, aname, p_fims, avm);
                }
                else
                {
                    // We'll need to make sure we're using the right asset manager based
                    // on the asset manager name we're working with (amap)
                    asset_manager* fixed_am = getaM(vmap, aname);
                    if (fixed_am)
                        avm->am = fixed_am;
                    myfun(vmap, avm->am->amap, aname, p_fims, avm);
                }
            }
        }
    }
    else
    {
        if (0)
            FPS_PRINT_ERROR("No Monitor list for  aname [{}] wname[{}]", aname, wname);
    }
    return;
}

//
int VarMapUtils::schedStop(varsmap& vmap, varmap& amap, const char* vname, double eTime)
{
    UNUSED(vmap);
    amap[vname]->setParam("endTime", eTime);
    amap[vname]->setVal(vname);
    return 0;
}

// vm->setVecDepth(vmap,amap, "MaxBMSDischargePowerEstFiltIn","vecAv",120,
// double* lVal);
int VarMapUtils::setVecDepth(varsmap& vmap, varmap& amap, const char* vname, const char* vecName, int depth,
                             double* lValp)
{
    assetVar* Av = amap[vname];
    if (Av)
    {
        char* Avuri = Av->getcParam(vecName);
        if (Avuri)
        {
            // vAv value is not used just the vector
            double dval = 0.0;
            assetVar* vAv = setVal(vmap, Avuri, nullptr, dval);
            if (vAv)
            {
                vAv->setVecDepth(1);
                vAv->setVecDepth(depth);

                if (lValp)
                {
                    for (int i = 0; i < depth; i++)
                    {
                        vAv->setVecVal(*lValp, depth);
                    }
                    Av->setVal(*lValp);
                }
            }
        }
    }
    return 0;
}

int VarMapUtils::setVecHold(varsmap& vmap, varmap& amap, const char* vname, const char* vecName, bool hold)
{
    assetVar* Av = amap[vname];
    if (Av)
    {
        char* Avuri = Av->getcParam(vecName);
        if (Avuri)
        {
            // vAv value is not used just the vector
            assetVar* vAv = getVar(vmap, Avuri, nullptr);
            if (vAv)
            {
                vAv->setParam("hold", hold);
            }
        }
    }
    return 0;
}

assetVar* VarMapUtils::setDefault(varsmap& vmap, const char* vcomp, const char* vname, const char* ltype)
{
    assetVar* av = nullptr;

    if (!ltype || (strcmp(ltype, "double") == 0))
    {
        double dval = 0.0;
        av = setVal(vmap, vcomp, vname, dval);
    }
    if (strcmp(ltype, "int") == 0)
    {
        int dval = 0.0;
        av = setVal(vmap, vcomp, vname, dval);
    }
    if (strcmp(ltype, "bool") == 0)
    {
        bool dval = false;
        av = setVal(vmap, vcomp, vname, dval);
    }
    if (strcmp(ltype, "string") == 0)
    {
        char* dval = (char*)"unknown";
        av = setVal(vmap, vcomp, vname, dval);
    }
    return av;
}

// assetVar* VarMapUtils::setVal(varsmap& vmap, const char* comp, const char* var, T& value)
template assetVar* VarMapUtils::setVal<double>(varsmap& vmap, const char* comp, const char* var, double& value);
template assetVar* VarMapUtils::setVal<const char*>(varsmap& vmap, const char* comp, const char* var,
                                                    const char*& value);
template assetVar* VarMapUtils::setVal<char*>(varsmap& vmap, const char* comp, const char* var, char*& value);
template assetVar* VarMapUtils::setVal<int>(varsmap& vmap, const char* comp, const char* var, int& value);
int forceVmTemplates()
{
    using namespace std::chrono;

    // ess_man = new asset_manager("ess_controller");
    varsmap vmap;
    VarMapUtils vm;
    asset_manager* am = new asset_manager("test");
    assetVar* av;
    assetVar* av2;
    cJSON* cj = nullptr;

    am->am = nullptr;
    am->running = 1;
    char* cval = (char*)"1234";
    // vm->syscVec = &syscVec;

    am->vmap = &vmap;
    am->vm = &vm;

    bool bval = false;
    double dval = 0.0;
    int ival = 0;
    // New one
    char* tmp1 = (char*)"1234";
    char* func = (char*)"4567";
    am->vm->setVal(*am->vmap, tmp1, nullptr, func);
    am->amap["bval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "bval", bval);
    am->amap["ival"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "ival", ival);
    am->amap["dval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "dval", dval);
    am->amap["cval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "cval", cval);
    am->amap["tval"] = am->vm->setLinkVal(*am->vmap, "test", "/build", "cval", cval);
    av = am->amap["dval"];
    av2 = am->amap["tval"];
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
    av2->setVal((const char*)cval);

    av->setLVal(dval);
    av->setLVal(bval);
    av->setLVal(ival);
    av->setLVal((const char*)cval);

    // av->setParam((const char*)"i1",ival);
    // av->setParam((const char*)"d1",dval);
    // av->setParam((const char*)"b1",bval);
    // av->setParam((const char*)"t1",cval);
    // av->setParam((const char*)"av",av);

    ival = av->getiParam("i1");
    bval = av->getbParam("b1");
    dval = av->getdParam("d1");
    cval = av->getcParam("c1");

    // bval = av->valueChanged(dval,ival);
    bval = av->valueChanged();
    bval = am->vm->valueChanged(dval, dval);
    // bval = am->vm->valueChanged(dval,ival);
    bval = am->vm->valueChanged(av, av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, dval, dval);
    bval = am->vm->valueChangednodb(av, av, bval, dval);
    am->vm->makeVar(vmap, "/my/asset:var", nullptr, cj);
    // bval = am->av->valueChangednodb(dval,dval);
    // bval = am->av->valueChangednodb(dval,bval);
    // av->setParam("d1",dval);
    // av->setParam("b1",bval);
    // av->setParam("c1",tval);
    // assetBitField bf(1,2,nullptr,nullptr,nullptr);
    assetBitField bf2(nullptr);
    ival = bf2.getFeat("d", &ival);
    cval = bf2.getFeat("d", &cval);
    system_clock::time_point now = system_clock::now();
    time_t tnow = system_clock::to_time_t(now);
    tm* local_tm = localtime(&tnow);
    char tbuffer[80];
    strftime(tbuffer, 80, "%c.", local_tm);
    am->amap["timeString"] = am->vm->setLinkVal(vmap, "test", "/status", "timeString", tbuffer);
    av->sendAlarm(av, nullptr, nullptr, 1);
    char* ss = strdup("some string");
    char const* ssc = (char const*)ss;
    am->vm->setVal(vmap, "/build/ess", "cval", ss);
    am->vm->setVal(vmap, "/build/ess", "cval", ssc);
    am->vm->setVal(vmap, "/build/ess", "bval", bval);
    am->vm->setVal(vmap, "/build/ess", "dval", dval);
    am->vm->setVal(vmap, "/build/ess", "dval", dval);
    am->vm->setVal(vmap, "/build/ess", "ival", ival);
    char* tval = (char*)"thisisfoo";
    am->vm->setVal(vmap, "/build/ess", "dval", tval);
    free(ss);

    return 0;
}

/**
 * @brief process log !!
 * @param timeptr the pointer to time struct
 * @return char* the string representation of date and time
 */
int process_log(varsmap& vmap, assetVar* av)
{
    if (0)
        FPS_PRINT_INFO("av {} av->am {}", fmt::ptr(av), fmt::ptr(av ? av->am : nullptr));
    if (av)
    {
        char* dest = nullptr;
        char* msg = nullptr;
        char* avVal = av->getcVal() ? av->getcVal() : (char*)"noval";
        char* avLVal = av->getcLVal() ? av->getcLVal() : (char*)"noLval";
        VarMapUtils* vm = av->am->vm;

        if (0)
            FPS_PRINT_INFO("name [{}] value [{}] lastValue[{}]"

                           ,
                           av->name, avVal, avLVal);

        char* almsg = av->getcVal() ? av->getcVal() : (char*)"Normal";

        if (almsg && (strcmp(almsg, "Clear") == 0))
        {
            // TODO get /assets/xxx/summary:faults  from an av param "faults" review
            // after MVP
            vm->vmlen = asprintf(&dest, "/assets/%s/summary:faults", av->am->name.c_str());
            vm->clearAlarms(vmap, dest);
            if (1)
                FPS_PRINT_INFO("Clear Faults dest [{}]", dest);
        }
        else if (strcmp(avVal, avLVal) != 0)
        {
            av->setLVal(avVal);

            double tNow = vm->get_time_dbl();
            vm->vmlen = asprintf(&dest, "/assets/%s/summary:faults", av->am->name.c_str());
            vm->vmlen = asprintf(&msg, "%s fault [%s] at %2.3f ", av->name.c_str(), almsg, tNow);
            {
                if (av->am && av->am->vm)
                {
                    vm->sendAlarm(vmap, av, dest, nullptr, msg, 2);
                    if (1)
                        FPS_PRINT_INFO("Fault Sent dest [{}] msg [{}] am {}", dest, msg, fmt::ptr(av->am));
                }
            }
            // av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
            if (0)
                FPS_PRINT_INFO("dest [{}] msg [{}] am {}", dest, msg, fmt::ptr(av->am));
        }
        if (dest)
            free(dest);
        if (msg)
            free(msg);
    }
    else
    {
        FPS_PRINT_INFO("running No av !!", NULL);
    }
    return 0;
}

/**
 * @brief Converts the tm struct to string without new line. Referenced from
 * http://www.cplusplus.com/reference/ctime/asctime/
 *
 * @param timeptr the pointer to time struct
 * @return char* the string representation of date and time
 */
char* strtime(const struct tm* timeptr)
{
    static char result[64];

    snprintf(result, sizeof(result), "%02d/%02d/%d %02d:%02d:%02d", 1 + timeptr->tm_mon, timeptr->tm_mday,
             1900 + timeptr->tm_year, timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);

    return result;
}

// some string test functions the fast compare only checks the first 8 bytes
// but its 3x faster on our test strings

bool oldstrcmp(char* sp1, char* sp2)
{
    if (strcmp(sp1, sp2) == 0)
        return true;
    return false;
}
// sp1 and sp2 min 8 chars
bool faststrcmp(char* sp1, char* sp2)
{
    unsigned long lv1 = *(unsigned long*)sp1;
    unsigned long lv2 = *(unsigned long*)sp2;

    return (lv1 == lv2);
}

void essPerf::runePerfAv2(asset_manager* am, assetVar* logav, const char* lname, double tVal)
{
    UNUSED(am);
    std::string vtot(lname);
    vtot += "_tot";
    std::string vavg(lname);
    vavg += "_avg";
    std::string vmax(lname);
    vmax += "_max";
    std::string vcnt(lname);
    vcnt += "_cnt";
    int ival = logav->getiParam(vcnt.c_str());
    logav->setParam(vcnt.c_str(), ival + 1);
    double dval = logav->getdParam(vtot.c_str());
    logav->setParam(vtot.c_str(), dval + tVal);
    dval = (dval + tVal) / (ival);
    logav->setParam(vavg.c_str(), dval);
    dval = logav->getdParam(vmax.c_str());
    if (tVal > dval)
    {
        logav->setParam(vmax.c_str(), tVal);
    }
}

void essPerf::runePerfAv()  // asset_manager* am, assetVar* logav, double tNow)
{
    double tTime;
    if (inval)
    {
        tTime = *inval;
        runePerfAv2(am, logav, "value", tTime);
    }
    else
    {
        tTime = (am->vm->get_time_dbl() - tNow) * 1000.0;
        runePerfAv2(am, logav, "timemS", tTime);
    }
}

assetVar* essPerf::getePerfAv(const char* aname, const char* lname)
{
    bool bval = true;
    assetVar* logav = nullptr;
    if (am->lmap.find(lname) == am->lmap.end())
    {
        logav = am->vm->setLinkVal(*am->vmap, aname, "/perf", lname, bval);
        if (0)
            FPS_PRINT_INFO("new lmap name {} Logging aname [{}] lname [{}] data", fmt::ptr(logav), aname, lname);
        int ival = 0;
        std::string vcnt(lname);
        vcnt += "_cnt";
        logav->setParam(vcnt.c_str(), ival);
        am->lmap[lname] = logav;
    }
    else
    {
        logav = am->lmap[lname];
        if (0)
            FPS_PRINT_INFO("old lmap name {} Logging aname [{}] lname [{}] data", fmt::ptr(logav), aname, lname);
    }
    return logav;
}

essPerf::essPerf(asset_manager* _am, const char* _aname, const char* _lname, double* inVal)
{
    am = _am;
    if (inVal)
    {
        tNow = *inVal;
    }
    else
    {
        tNow = am->vm->get_time_dbl();
    }
    aname = strdup(_aname);
    am->vm->vmlen = asprintf(&lname, "perf_%s", _lname);

    logav = getePerfAv(aname, lname);
    bval = logav->getbVal();
    inval = inVal;
}

essPerf::~essPerf()
{
    if (bval)
    {
        runePerfAv();
    }
    free(lname);
    free(aname);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Measures the current (and peak) resident and virtual memories
 * usage of your linux C process, in kB
 */

void getMemory(int* currRealMem, int* peakRealMem, int* currVirtMem, int* peakVirtMem)
{
    // stores each word in status file
    char buffer[1024] = "";
    int vmlen = 0;
    // linux file contains this-process info
    FILE* file = fopen("/proc/self/status", "r");
    if (file)
    {
        // read the entire file
        while (fscanf(file, " %1023s", buffer) == 1)
        {
            if (strcmp(buffer, "VmRSS:") == 0)
            {
                if (currRealMem)
                    vmlen = fscanf(file, " %d", currRealMem);
            }
            if (strcmp(buffer, "VmHWM:") == 0)
            {
                if (peakRealMem)
                    vmlen = fscanf(file, " %d", peakRealMem);
            }
            if (strcmp(buffer, "VmSize:") == 0)
            {
                if (currVirtMem)
                    vmlen = fscanf(file, " %d", currVirtMem);
            }
            if (strcmp(buffer, "VmPeak:") == 0)
            {
                if (peakVirtMem)
                    vmlen = fscanf(file, " %d", peakVirtMem);
            }
        }
        fclose(file);
    }
    if (vmlen)
        vmlen = 0;  // happy compiler
}

/*
 *
 * Measures the current  cpu temps
 *
 */
double getTempFile(const char* fname)
{
    double tmp = 0.0;
    int in = 0;
    int vmlen = 0;
    FILE* file;
    char buffer[1024] = "";

    file = fopen(fname, "r");
    if (file)
    {
        // read the entire file
        vmlen = fscanf(file, " %1023s", buffer);
        {
            vmlen = fscanf(file, " %d", &in);
            tmp = (double)(in / 1000.0);
        }
        fclose(file);
        if (vmlen)
            vmlen = 0;  // happy compiler
    }

    return tmp;
}

void getTemps(double* package, double* core0, double* core1, double* core2, double* core3)
{
    if (package)
    {
        *package = getTempFile("/sys/class/hwmon/hwmon1/temp1_input");
    }
    if (core0)
    {
        *core0 = getTempFile("/sys/class/hwmon/hwmon1/temp2_input");
    }
    if (core1)
    {
        *core1 = getTempFile("/sys/class/hwmon/hwmon1/temp3_input");
    }
    if (core2)
    {
        *core2 = getTempFile("/sys/class/hwmon/hwmon1/temp4_input");
    }
    if (core3)
    {
        *core3 = getTempFile("/sys/class/hwmon/hwmon1/temp5_input");
    }
}

asset_manager* setupEssManager(VarMapUtils* vm, varsmap& vmap, vecmap& vecs, std::vector<char*>* syscpVec,
                               const char* aname, scheduler* sc)
{
    UNUSED(sc);
    asset_manager* am = new asset_manager(aname);

    am->am = nullptr;
    am->vm = vm;
    am->vecs = &vecs;        // used to store pubs , subs and blocks
    am->syscVec = syscpVec;  // used to store UI publist
    vm->syscVec = syscpVec;

    am->setVmap(&vmap);

    am->running = 1;
    am->reload = 0;
    // am->wakeChan = &sc->wakeChan;
    // am->reqChan = (void*)&sc->reqChan;
    // sc->am = am;
    return am;
}

int loadEssConfig(VarMapUtils* vm, varsmap& vmap, const char* cname, asset_manager* am, scheduler* sc)
{
    UNUSED(sc);
    int rc = 0;
    char* cfgname = vm->getFileName(cname);
    if (cfgname == nullptr)
    {
        FPS_PRINT_ERROR("Failed to get initial configuration from [{}]", cname);
        cfgname = vm->getFileName("test_config.json");
    }
    if (cfgname)
    {
        FPS_PRINT_INFO("Getting initial configuration from [{}] aname [{}]", cfgname, am->name);

        rc = vm->configure_vmap(vmap, cfgname, nullptr, am);
        free(cfgname);
        if (rc < 0)
        {
            FPS_PRINT_ERROR("Failed to parse initial configuration from [{}]", cname);
            return rc;
        }
        // OK all well up to this point
        // now get the base subs.. after setting up the veclist
        // int ccntam = 0;
    }
    return rc;
}

// show subs vecs and dump config to an initial file.
int debugSystemLoad(VarMapUtils* vm, varsmap& vmap, vecmap& vecs, std::vector<char*>* syscpVec, const char* aname,
                    const char* logdir)
{
    UNUSED(logdir);
    // syscVec holds the assets in order

    // Call initFunc here to initialize functions for all assets/asset managers
    // initFuncs(ess_man);

    if (0)
    {
        FPS_PRINT_INFO("list of assets in syscVec [{}]", fmt::ptr(syscpVec));
        if (syscpVec)
        {
            for (int i = 0; i < (int)syscpVec->size(); i++)
            {
                FPS_PRINT_INFO("idx [{}] name [{}]", i, syscpVec->at(i));
            }
        }
    }
    // now inspect vecs we want the pubs and subs
    if (0)
    {
        int idx = 0;
        for (auto xv : vecs)
        {
            FPS_PRINT_INFO("vecs idx [{}] name [{}]", idx++, xv.first);

            // xx.second->clear();
            // delete xx.second;
        }
        vm->showvecMap(vecs, "Subs");
    }
    {
        char* fname = nullptr;
        vm->vmlen = asprintf(&fname, "%s/%s_after_assets.json", vm->runCfgDir ? vm->runCfgDir : "run_configs", aname);
        // this a test for our config with links
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        vm->write_cjson(fname, cjbm);

        // cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        if (0)
            FPS_PRINT_INFO("Maps (should be ) with links and assets  cjbm {} {} <<< done", fmt::ptr(cjbm), res);
        free(res);
        free(fname);
        cJSON_Delete(cjbm);
    }
    return 0;
}
// moved here
int checkAv(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    int rc = 0;
    if (!aV)
    {
        FPS_PRINT_ERROR("Error no aV ", NULL);
        return rc;
    }
    asset_manager* am = aV->am;
    // int debug = 1;

    if (!am)
    {
        FPS_PRINT_ERROR("Error no am for  [{}]", aV->getfName());
        return rc;
    }
    if (!am->vm)
    {
        FPS_PRINT_ERROR(" Error no am->vm for  [{}]", aV->getfName());
        return rc;
    }

    if (!am->vmap)
    {
        FPS_PRINT_ERROR(" Setting am->vmap for  [{}] value [{}]", aV->getfName(), aV->getcVal());
        am->vmap = &vmap;
    }
    rc = 1;
    return rc;
}

bool getLoggingEnabled(varsmap& vmap, VarMapUtils& vm)
{
    bool logging_enabled = false;

    char* essName = vm.getSysName(vmap);
    const auto config_name = fmt::format("/config/{}", essName);

    assetVar* avlog_global = vm.getVar(vmap, "/config/ess", "logging_enabled");
    assetVar* avlog = vm.getVar(vmap, config_name.c_str(), "logging_enabled");

    // Priority is /config/ess then /config/ess_x in that order:
    if (avlog_global)
        logging_enabled = avlog_global->getbVal();
    else if (avlog)
        logging_enabled = avlog->getbVal();

    return logging_enabled;
}

bool getLoggingTimestamp(varsmap& vmap, VarMapUtils& vm)
{
    bool add_timestamp = false;

    char* essName = vm.getSysName(vmap);
    const auto config_name = fmt::format("/config/{}", essName);

    assetVar* avlog_global = vm.getVar(vmap, "/config/ess", "logging_timestamp");
    assetVar* avlog = vm.getVar(vmap, config_name.c_str(), "logging_timestamp");

    // Priority is /config/ess then /config/ess_x in that order:
    if (avlog_global)
        add_timestamp = avlog_global->getbVal();
    else if (avlog)
        add_timestamp = avlog->getbVal();

    return add_timestamp;
}

char* getLogDir(varsmap& vmap, VarMapUtils& vm)
{
    char* LogDir = nullptr;

    char* essName = vm.getSysName(vmap);
    const auto config_name = fmt::format("/config/{}", essName);

    assetVar* avlog_global = vm.getVar(vmap, "/config/ess", "LogDir");
    assetVar* avlog = vm.getVar(vmap, config_name.c_str(), "LogDir");

    // NOTE(WALKER): Priority is /config/ess then /config/ess_x in that order:
    if (avlog_global)
        LogDir = avlog_global->getcVal();
    else if (avlog)
        LogDir = avlog->getcVal();

    // NOTE(WALKER): if NO LogDir exists then one is created for this particular
    // ess_x:
    if (!LogDir)
    {
        LogDir = (char*)"/var/log/ess_controller";
        avlog = vm.setVal(vmap, config_name.c_str(), "LogDir", LogDir);
        LogDir = avlog->getcVal();
    }
    return LogDir;
}

void setLoggingSize(varsmap& vmap, VarMapUtils& vm)
{
    char* essName = vm.getSysName(vmap);
    const auto config_name = fmt::format("/config/{}", essName);

    assetVar* avlog_global = vm.getVar(vmap, "/config/ess", "logging_size");
    assetVar* avlog = vm.getVar(vmap, config_name.c_str(), "logging_size");

    // NOTE(WALKER): Priority is /config/ess then /config/ess_x in that order:
    int logging_size = 0;

    if (avlog_global)
        logging_size = avlog_global->getiVal();
    else if (avlog)
        logging_size = avlog->getiVal();

    if (logging_size > 0)
        ESSLogger::get().setBacktrace(logging_size);
}

// cJSON*getSchListcJ(schlist&schreqs);
// cJSON*getSchList()
// {
//     return getSchListcJ(schreqs);
// }
#endif
