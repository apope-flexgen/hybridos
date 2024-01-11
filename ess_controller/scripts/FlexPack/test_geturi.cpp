#include "asset.h"
#include "varMapUtils.h"
#include "chrono_utils.hpp"

// get options

// /flex/components   get all components , value only
// /components/bms_info: {...}
// /components/bms_status: {...}
// /components/bms_system: {...}
// /components/pcs_info: {...}
// /components/pcs_status: {...}
// /components/pcs_system: {...}

// /flex/components/b   get all components that match b, value only
// /components/bms_info: {...}
// /components/bms_status: {...}
// /components/bms_system: {...}

// /flex/components/bms_i   get all components that match bms_i, value only
// /components/bms_info: {...}

// Then we have to do the following

// /flex/components/bms_info/bms_voltage
// in this case we first look for "bms_voltage" in 
// /components/bms_info and if found return
// 1234

// get /flex/full/components/bms_info/bms_voltage
// {"value":1234}

// if /components/bms_info:bms_voltage does not exist then we look for 
// the table  ( or comp) /components/bms_info/bms_voltage

// set /flex/full/components/bms_info/bms_volts '{"ac":280}' 
// set /flex/full/components/bms_info/bms_volts '{"dc":1280}' 
// get /flex/full/components/bms_info/bms_volts
// '{"ac":"280","dc":1280}' 

// With the extended features, added after the original code release,
//    some of this should be easier.
// for example we did not have the uri parsing /components/bms_info:bms_voltage@param
// at that time.
// flexpack_basics is a bit broken at the momnt . I'll patch it up but a real rewrite 
// is needed.

// In addition we have to keep all the uri stuff in order of loading.
// In addition we have to manage the segmented query from the UI
        
// get /assets
// { "ess":{ 
//         "summary"{...},
//            "bms":{"summary":[...],
//                   "sbmu_1":{...},
//                   "sbmu_2":{...}
//                  },
//            "pcs" :{"summary":[...],
//                     "rack_1":{...},
//                     "rack_2":{...}
//                  },
//           }
//  }
// get /assets/ess
// get /assets/ess/bms
// get /assets/ess/pcs
//  -m get -r /$$ -u /assets/bms | jq | more
// this gets all the subcomponents under bms
// {
//   "summary": {
//     "name": "BMS Manager",
//     "alarms": {
//       "value": 1,
//       "options": [
//         {
//           "name": "[bms_heartbeat] Alarm   (value [0] is not changing) for 5.00 seconds at Thu Jun  3 09:19:10 2021",
//           "return_value": 2
//         }
//       ],
//       "enabled": true,
//       "name": "Alarms",
//       "scaler": 0,
//       "type": "number",
//       "ui_type": "alarm",
//       "unit": ""
//     },
///   "sbmu_1": {
//     "name": "Battery Rack 01",
//     "alarms": {
//       "value": 0,
//       "options": [],
//       "enabled": true,
//       "name": "Alarms",
//       "scaler": 0,
//       "type": "number",
//       "ui_type": "alarm",
//       "unit": ""
//     },
//     "faults": {
//       "value": 0,
//       "options": [],
//       "enabled": true,
//       "name": "Faults",
//       "scaler": 0,
//       "type": "number",
//       "ui_type": "alarm",
//       "unit": ""
//     },
// this is incorrect it should just have the low level object.
//-m get -r /$$ -u /assets/bms/sbmu_1 | jq | more
// {
//   "sbmu_1": {
//     "name": "Battery Rack 01",
//     "alarms": {
//       "value": 0,
//       "options": [],
//       "enabled": true,
//       "name": "Alarms",
//       "scaler": 0,
//       "type": "number",
//       "ui_type": "alarm",
//       "unit": ""
//     },
//     "faults": {
//       "value": 0,
//       "options": [],
//       "enabled": true,
//       "name": "Faults",
//       "scaler": 0,
//       "type": "number",
//       "ui_type": "alarm",
//       "unit": ""
//     },

// all have to return the correct objects.
//
//
// the aList keeps track of the order of elements of a component as they are loaded 
// the sysVec keeps track of the compoents as they are loaded.
//
 


// I'll start a test_geturi.cpp in the tests dir so we can work on it.
#include "scheduler.h"

int debug = 0;
namespace flex
{
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

cJSON*getSchList()
{
    return nullptr; //getSchListcJ(schreqs);
}

void SplitUriName (std::string&uri, std::string&var, const std::string& str)
{
  //std::cout << "Splitting: " << str << '\n';
  std::size_t found = str.find_last_of("/\\");
  uri = str.substr(0,found);
  var = str.substr(found+1);
}

int SplitUriBase (std::string&uri, std::string&var, const std::string& str)
{
  //std::cout << "Splitting: " << str << '\n';
  std::size_t found = str.find_first_of("/\\");
  if(found != str.npos)
  {
    uri = str.substr(0,found);
    var = str.substr(found+1);
    found = 1;
  }
  else
  {
      found  = 0;
  }
  return (int)found;
}

cJSON* segmentCj(varsmap& vmap, VarMapUtils*vm,  cJSON* cj, int options)
{
    bool first = true;
    cJSON* cjr =  nullptr;
    cJSON* cjri =  nullptr;
    if(options & 0x10000)
    {
        if(debug)FPS_ERROR_PRINT(
            "   %s >> segmenting the result cj->next %p cj->child %p cnext %p\n"
            ,__func__, cj->next, cj->child, cj->child->next);
        cJSON* cji = cj->child;
        while(cji)
        {
            if(first)
            {
                first = false;
                cjr = cJSON_CreateObject();
            }

            if(debug)FPS_ERROR_PRINT("   %s >>>> segmenting [%s] cjr %p cji->next %p\n"
                ,__func__, cji->string, cjr, cji->next);
            std::string uri;
            std::string var;

            int found = SplitUriBase (uri, var, cji->string);
            if(found)
            {
                cjri = cjr;
            }
            while (found)
            {
                found = SplitUriBase (uri, var, var);
                if(debug)FPS_ERROR_PRINT(
                    "   %s >> >> >>    segmented uri [%s] var [%s] found %d\n"
                    ,__func__, uri.c_str(), var.c_str(), found);
                //cjri = cJSON_GetObjectItem(cjri, uri);
                if(found)
                {
                    if (!cJSON_GetObjectItem(cjri, uri.c_str()))
                    {
                        if (debug) FPS_ERROR_PRINT("   %s >> create tree [%s] \n"
                            , __func__, uri.c_str());

                        cJSON* cjii = cJSON_CreateObject();
                        cJSON_AddItemToObject(cjri, uri.c_str(), cjii);
                    }
                    cjri = cJSON_GetObjectItem(cjri, uri.c_str());
                }
                else
                {
                    if (!cJSON_GetObjectItem(cjri, var.c_str()))
                    {
                        if (debug) FPS_ERROR_PRINT("   %s >> create tree [%s] \n"
                            , __func__, var.c_str());

                        //cJSON* cjii = cJSON_CreateObject();
                        cJSON* cjii = vm->getMapsCj(vmap, cji->string, nullptr, options);
                        cJSON* cjd = cJSON_DetachItemFromObject(cjii, cji->string);

                        //cJSON_AddItemToObject(cjri, var.c_str(), cjd);
                        ////cjri->child = cjd->child;
                        //if(cj)cJSON_Delete(cj);

                        cJSON_Delete(cjii);
                        //cj = cjd->child;
                        //cjd->child = nullptr;
                        //cJSON_Delete(cjd);
                        //cJSON_Delete(cjr);
                        //return cjr;

                        cJSON_AddItemToObject(cjri, var.c_str(), cjd->child);
                        cjd->child = nullptr;
                        cJSON_Delete(cjd);

                    }
                    // for (auto& y : vmap[cji->string])
                    // {
                    //     if (1)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                    //     y.second->showvarCJ(cjri, options);
                    //     if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                    //     //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                    // }
                }
            }
            cji = cji->next;            
        }
    }
    if(cjr)
    {
        cJSON_Delete(cj);
        cj = cjr;
    }
    return cj;
}

char* getUri(varsmap &vmap, VarMapUtils *vm, const char* uri, int options = 0x100)
{
    char* stuff=nullptr;
    cJSON* cj = nullptr;
    //assetUri()
    assetUri my(uri);

    if(vmap.find(my.Uri) == vmap.end())
    {
        // this is the worst case 
        // what about partial matches.
        // something like this
        std::vector<const char*>mvec;

        if(vm->syscVec)
        {
            
            int idx = 0;
            for (auto x : *vm->syscVec)
            {
                if (debug) FPS_ERROR_PRINT(" %s >> syscVec idx [%d] entry [%s] \n", __func__, idx, x);
                if(strncmp(my.Uri, x, strlen(my.Uri)) == 0) 
                {
                    mvec.push_back(x);
                }
                idx++;

            }
        }
        else
        {
            for (auto x: vmap)
            {
                if(strncmp(my.Uri, x.first.c_str(), strlen(my.Uri))== 0)
                {
                    mvec.push_back(x.first.c_str());
                }

                //assetVar*av = x.second;
                //av->getCjVal(&cj);
            }
        }
        int idx  = 0;
        if(mvec.size() > 0)
        {
            cj = cJSON_CreateObject();
            while(idx < (int)mvec.size())
            {
                FPS_ERROR_PRINT(" %s >>  [%03d]->[%s]\n", __func__, idx, mvec[idx]);
                // do something to add mvec to the overall cj
                cJSON* cji = vm->getMapsCj(vmap, mvec[idx], nullptr, options);
                cJSON* cjd = cJSON_DetachItemFromObject(cji, mvec[idx]);

                cJSON_AddItemToObject(cj, mvec[idx], cjd);
                cJSON_Delete(cji);
                idx++;
            }
            if(debug)
            {
                char *tmp = cJSON_Print(cj);
                FPS_ERROR_PRINT(" %s >>  cj so far\n[%s]\n", __func__, tmp);
                free(tmp);
            }
            printf(" segment 1\n");

            cj = segmentCj(vmap, vm, cj, options);

            stuff = cJSON_Print(cj);
            cJSON_Delete(cj);
        }
        else
        {
            FPS_ERROR_PRINT(" got nothing from [%s]\n", idx, my.Uri);
            std::string uri;
            std::string var; 
            SplitUriName (uri, var, my.Uri);
            if(debug)FPS_ERROR_PRINT(" Lets try [%s:%s]\n", uri.c_str(), var.c_str());
            cj = vm->getMapsCj(vmap, uri.c_str(), var.c_str(), options);
             // now we may need to split the uri
            printf(" segment 2\n");

            cj = segmentCj(vmap, vm, cj, options);

            if(cj)
            {
                stuff = cJSON_Print(cj);
                cJSON_Delete(cj);
            }
            else
            {
                stuff = strdup("{}");
            }
        }
    }
    else
    {
        cj = nullptr;
        if (my.Param)
        {
            assetVar*av = vm->getVar(vmap,my.Uri, my.Var);
            cj = av->getCjParam(my.Param, options);
        }
        else
        {
            cj = vm->getMapsCj(vmap, my.Uri, my.Var, options);
        
        }
        // now we may need to split the uri
        printf(" segment 3\n");
        cj = segmentCj(vmap, vm, cj, options);

        if(cj)
        {
            stuff = cJSON_Print(cj);
            cJSON_Delete(cj);
        }

    }
    return stuff;
}

// this is used to maintain the object order.
// UI object is detected by "name" with no value.
// but we may as well create the alist regardless
// if (uiObject > 0)
 //   varsmap vmap;
    // // this is our map utils factory
//         {
//assetVar* av = makeVar(vmap,  (const char*)"_assetList", (const char*)tmp, alist);
void addtoAlist(VarMapUtils* vm, varsmap &vmap, assetVar *av)
{
    assetList* alist = nullptr;
    const char* uri = av->comp.c_str();
    alist = vm->getAlist(vmap, uri);

    if (!alist)
    {
        alist = vm->setAlist(vmap, uri);

        if(debug) FPS_ERROR_PRINT("%s >>  new assetList\n", __func__);
    }
    if (alist)
    {
            alist->add(av);
    }
    return;
}

void clearAlist(VarMapUtils* vm, varsmap &vmap)
{
    if(vmap.find("_assetList")== vmap.end())
    {
        FPS_ERROR_PRINT("%s >> no aList found\n",__func__);
    }
    else
    {
        for (auto x: vmap["_assetList"])
        {
            assetVar *av = x.second;
            if(debug)FPS_ERROR_PRINT("%s >> aList [%s] found av %p\n",__func__, x.first.c_str(), av);
            assetList* aList = vm->getAlist(vmap, x.first.c_str());
            delete aList;
            delete av;

        }
    }

}

//assetVar* av = makeVar(vmap,  (const char*)"_assetList", (const char*)tmp, alist);
// adds to table vector
void addtoSysVec(VarMapUtils* vm, varsmap &vmap, assetVar *av)
{
    const char* uri = av->comp.c_str();
    if(!vm->syscVec)
    {
        vm->syscVec = new std::vector<char*>;//&vm.syscharVec;
    }
    int idx = 0;
    for (auto x : *vm->syscVec)
    {
        if (debug) FPS_ERROR_PRINT(" %s >> syscVec idx [%d] entry [%s] \n", __func__, idx, x);
        if(strcmp(x, uri) == 0) 
        {
            return;
        }
        idx++;
    }
    vm->syscVec->push_back((char*)uri);
}

void clearSysVec(VarMapUtils* vm)
{
    if(vm->syscVec)
    {
        vm->syscVec->erase (vm->syscVec->begin(),vm->syscVec->begin()+vm->syscVec->size());
        vm->syscVec->clear();
        delete vm->syscVec;
        vm->syscVec=nullptr;
    }
}
int test_geturi(int argc, char*argv[])
{
    const char* uri = "/components/bms_info/bms_volts";
    const char*sopts = nullptr;
    unsigned int opts = 0x000;
    bool asort = false;
    // // this is our main data map
    varsmap vmap;
    // // this is our map utils factory
    VarMapUtils vm;

    FPS_ERROR_PRINT(" hello from [%s]\n", __func__);
    if(argc > 1)
    {
        uri = argv[1];
    }

    if(argc > 2)
    {
        sopts = argv[2];
        std::string fstr;
        std::string remstr(sopts);
        int found;// = SplitUriBase (fstr, remstr, remstr);
        do 
        {
            found = SplitUriBase (fstr, remstr, remstr);
            printf(" fstr [%s]  remstr[%s] found %d \n", fstr.c_str(), remstr.c_str(), found);
            if(!found) fstr = remstr;
            if (strncmp(fstr.c_str(), "0x", 2) == 0)
            {
                opts = std::stoul(fstr.c_str(), nullptr, 16);
                printf(" opts 0x%05x\n", opts);

            }
            else if (fstr == "naked")
            {
                opts |= 0x00001;
            }
            else if (fstr == "end")
            {
                found = 0;
            }
            else if (fstr == "help")
            {
                printf(" help    :  print help\n");
                printf(" end     :  stop preparsing\n");
                printf(" full    :  print options etc\n");
                printf(" naked   :  print naked uris\n");
                printf(" ui      :  user interface\n");
                printf(" hmm     :  something to do with options\n");
                printf(" sort    :  sort fields\n");
                printf(" debug   :  run local debug\n");
                printf(" vmdebug :  run VerMapUtils debug\n");
                printf(" 0xnnnn  :  set options directly\n");
                
            }
            else if (fstr == "full")
            {
                opts |= 0x00010;
            }
            else if (fstr == "ui")
            {
                opts |= 0x10000;
            }
            else if (fstr == "hmm")
            {
                opts |= 0x00100;
            }
            else if (fstr == "sort")
            {
                asort = true;
            }
            else if (fstr == "debug")
            {
                debug = 1;
            }
            else if (fstr == "vmdebug")
            {
                vm.vmdebug = 1;
            }
        } while(found);
    }

    // if(argc > 3)
    // {
    //     asort = true;
    // }
    

    //double tNow = vm.get_time_dbl();
    double dval = 1234.0;
    assetVar* av; 
    av = vm.makeVar(vmap, "/components/bms_info/bms_volts:dc", nullptr, dval);
    if(asort)addtoAlist(&vm, vmap, av);
    addtoSysVec(&vm, vmap, av);
    dval = 3.5;
    //av->setParam("max_limits",dval);


    dval = 34.5;
    //assetVar* av2 = 
    av = vm.makeVar(vmap, "/components/bms_info/bms_volts:ac", nullptr, dval);
    if(asort)addtoAlist(&vm, vmap, av);
    addtoSysVec(&vm, vmap, av);
    //assetVar* av2 = 
    dval = 3.5;
    vm.setVal(vmap, "/components/bms_info/bms_volts:dc@max_limit", nullptr, dval);

    dval = 3450;
    av = vm.makeVar(vmap, "/components/bms_info/bms_power:reactive_power", nullptr, dval);
    if(asort)addtoAlist(&vm, vmap, av);
    addtoSysVec(&vm, vmap, av);
    dval = 300500;
    av = vm.makeVar(vmap, "/components/bms_info/bms_power:active_power", nullptr, dval);
    if(asort)addtoAlist(&vm, vmap, av);
    addtoSysVec(&vm, vmap, av);

    // this should step back and find  :ac
    //build/release/test_geturi /components/bms_info/bms_volts/ac 0x1100
    //hello from [test_geturi]
    //test_geturi >> opts = 0x1100 stuff = [{}]

    // all these should work
    // char* stuff = getUri(vmap,&vm, "/components");
    // char* stuff = getUri(vmap,&vm, "/components/b");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/b");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/bms_volts/ac");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/bms_volts/dc");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/bms_volts/dc@max_limit");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/bms_volts:ac");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/bms_volts:dc");
    // char* stuff = getUri(vmap,&vm, "/components/bms_info/bms_volts:dc@max_limit");
    
    
    char* stuff = getUri(vmap,&vm, uri, opts);
    FPS_ERROR_PRINT("%s >> opts = 0x%04x asort %s \n result =>> \n[%s]\n"
        , __func__, (int)opts, asort?"true":"false", stuff);
    if(stuff)free(stuff);
    if(argc == 1)
    {
        printf( " use it like this:\n");
        printf( "build/release/test_geturi");
        printf(" /components/bms_info/bms_volts");
        printf(" full/help/naked/hmm/sort/vmdebug/debug\n");
    }

    //asset_manager* am = new asset_manager("test");
    clearAlist(&vm, vmap);
    clearSysVec(&vm);

    return 0;
}

#if defined _MAIN

int main(int argc, char *argv[])
{
 return test_geturi(argc, argv);
}
#endif