/*
*  functions.cpp
 * these are going to be fun functions
 * This is probably a great example on how NOT to di this sort of thing.
 * Lots of specific code to wrap around this operation.
 * After writing this test code my ideas changed.
 * 
 * NEW 10/6/2020 Added a test store to of the function address to the varsmap
 * 
  * void func(vector<assVars*> &outputs, vector<assVars *>&inputs)
  * keep em in a map by name
  * define at config time
  *
   function group (name)
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))

     example sum, max, min, avg
     if (i0) {
         for each i1 ...in)
             if(0=o0)o0 += ix
            if(o1)o1 = max (o1,ix)
            if(o2)o2 = min(o2,ix)
            if(o0 && o3) o3 = o0/n

     }
  this is cool stuff , all the processing maths can be configured at run time in the text config file.
                {
                    "id": "dc_volts_summary",
                    "inputs": [
                        {"var":"/components/bss_1","id":"dc_volts"},
                        {"var":"/components/bss_2","id":"dc_volts"},
                        {"var":"/components/bss_3","id":"dv_volts"}
                    ],
                    "outputs": [
                        {"op":"max","var":"/features/bss_status","id":"max_dc_volts"},
                        {"op":"min","var":"/features/bss_status","id":"min_dc_volts"}
                        {"op":"avg","var":"/features/bss_status","id":"avg_dc_volts"}
                    ],
                    "operation": "add_dbl",
                    "params": [
                      {"falseCase": 0}
                    ]
                }
*/
//#include <vector>

#include "asset.h"
#include "functions.h"


// struct funVec {
//     funVec(const char *fname) {
//         id = fname;
//         iface = new Interface;

//     }
//     ~funVec() {
//         delete iface;
//     };

//     void setFname(const char *_fname) {
//         fname = _fname;
//     }

//     Interface* iface;
//     std::string id;
//     std::string fname;

// };

// for each in vector add sum
bool func_add_dbl(struct Interface *iface)
{
    bool ok = true;
    std::vector<assetVar *>*ins = &iface->ins;
    //std::map<std::string,assetVar *>*outs = &iface->outs;

    int insize = ins->size();
    //int outsize = outs->size();
    double vals;
    double sum;
    double max;
    double min;
    if(!ins->at(0)->getbVal())
    {
        if (iface->outAt("enabled"))
            iface->outAt("enabled")->setVal(false);
        printf(" function not enabled\n");
        return false;
    }
    printf(" function  enabled\n");

    if (iface->outAt("enabled"))
        iface->outAt("enabled")->setVal(true);

    //pull in first value
    vals = ins->at(1)->getdVal();
    sum = vals;
    max = sum;
    min = sum;
    printf(" got a value %f sum %f max %f\n", vals,  sum, max);

    for (int i = 2 ; i < insize; i++)
    {
        sum += ins->at(i)->getdVal();
        if (vals > max) max = vals;
        if (vals < min) min = vals;
        printf(" got a value %f sum %f max %f\n", vals,  sum, max);

    }
    if (iface->outAt("sum"))
    {
        iface->outAt("sum")->setVal(sum); // save sum
    }
    if (iface->outAt("avg"))
    {
        iface->outAt("avg")->setVal(sum/(insize-1));
        iface->outAt("avg")->getdVal();

        printf(" set avg %f  from var  %f\n", sum/(insize-1), vals);
    }
    else
    {
        printf(">>>>>no avg op var found\n");

    }
    if (iface->outAt("max"))
        iface->outAt("max")->setVal(max);
    if (iface->outAt("min"))
        iface->outAt("min")->setVal(min);
    return ok;
}

//
// the objective it to be able to specify functions in the config text
// {
//     "id": "dc_volts_summary",
//     "inputs": [
//         {"var":"/components/bss_1","id":"dc_volts"},
//         {"var":"/components/bss_2","id":"dc_volts"},
//         {"var":"/components/bss_3","id":"dv_volts"}
//     ],
//     "outputs": [
//         {"op":"max","var":"/features/bss_status","id":"max_dc_volts"},
//         {"op":"min","var":"/features/bss_status","id":"min_dc_volts"}
//         {"op":"avg","var":"/features/bss_status","id":"avg_dc_volts"}
//     ],
//     "operation": "add_dbl",
//     "params": [
//       {"falseCase": 0}
//     ]
// }



// // here is the function type
// typedef bool (*boolFunType)(Interface *);


funVec* parse_funItem(cJSON*cj, varsmap &varMaps)
{
    funVec* funItem = nullptr;
    cJSON* cji = cJSON_GetObjectItem(cj,"id");
    if(cji)
    {
        funItem = new funVec(cji->valuestring);
    }
    if(!funItem)
    {
        return nullptr;
    }

    cji = cJSON_GetObjectItem(cj,"operation");
    if(cji)
    {
        funItem->setFname(cji->valuestring);
    }
    cji = cJSON_GetObjectItem(cj,"inputs");
    if(cji)
    {

        funItem->iface->addIn(varMaps["base"]["d1"]);

        cJSON*ci;
        cJSON_ArrayForEach(ci, cji)
        {
            cJSON* civ = cJSON_GetObjectItem(ci, "var");
            cJSON* cii = cJSON_GetObjectItem(ci, "id");

            if(1)printf (" found input array item [%s] [%s]\n"
                             , civ?civ->valuestring:"no var"
                             , cii?cii->valuestring:"no id");

            funItem->iface->addIn(varMaps[civ->valuestring][cii->valuestring]);

        }
    }
    cji = cJSON_GetObjectItem(cj,"outputs");
    if (cji)
    {
        cJSON*ci;
        cJSON_ArrayForEach(ci, cji)
        {
            cJSON* civ = cJSON_GetObjectItem(ci, "var");
            cJSON* cii = cJSON_GetObjectItem(ci, "id");
            cJSON* cio = cJSON_GetObjectItem(ci, "op");

            if(1)printf (" found out array item [%s] [%s] [%s]\n"
                             , cio?cio->valuestring:"no op"
                             , civ?civ->valuestring:"no var"
                             , cii?cii->valuestring:"no id"
                            );

            // outs have the op feature
            // we have to make sure that we have this tite,
            auto it1 = varMaps.find(civ->valuestring);
            if (it1 == varMaps.end())
            {
                FPS_ERROR_PRINT("unable to find component map [%s] \n" , civ->valuestring);
            }
            else
            {
                auto it2 = varMaps[civ->valuestring].find(cii->valuestring);
                if(it2 == varMaps[civ->valuestring].end())
                {
                    FPS_ERROR_PRINT("unable to find item id [%s] \n" , cii->valuestring);
                }
                else
                {
                    funItem->iface->addOut(cio->valuestring, varMaps[civ->valuestring][cii->valuestring]);
                }
            }
        }
    }
    return funItem;
}

void addSampleItem(cJSON *cj, const char *arr, const char* var, const char *id, const char *op=nullptr)
{
    cJSON* cja = cJSON_GetObjectItem(cj, arr);
    if(!cja)
    {
      cja = cJSON_CreateArray();
      cJSON_AddItemToObject(cj,arr,cja);
    }
    cJSON* cji = cJSON_CreateObject();
    cJSON_AddStringToObject(cji,"var",var);
    cJSON_AddStringToObject(cji,"id",id);
    if(op)
      cJSON_AddStringToObject(cji,"op",op);
    
    cJSON_AddItemToArray(cja, cji);
}

cJSON* createSampleFunction(const char *id, const char* operation)
{
    //cJSON* cja;
    //cJSON* cji;

    cJSON* cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj,"id",id);
    cJSON_AddStringToObject(cj,"operation",operation);
    addSampleItem(cj, "inputs","/components/bss_1", "dc_volts");
    addSampleItem(cj, "inputs", "/components/bss_2", "dc_volts");
    addSampleItem(cj, "inputs", "/components/bss_3", "dc_volts");
    addSampleItem(cj, "outputs", "/features/bss_status", "max_dc_volts", "max");
    addSampleItem(cj, "outputs", "/features/bss_status", "min_dc_volts", "min");
    addSampleItem(cj, "outputs", "/features/bss_status", "avg_dc_volts", "avg");
    addSampleItem(cj, "params", "/site/bss_params", "max_dc_volts", "maxVal");
    addSampleItem(cj, "params", "/site/bss_params", "min_dc_volts", "minVal");
    addSampleItem(cj, "params", "/site/bss_params", "check_dc_volts", "enable");
    addSampleItem(cj, "alarms", "/site/bss_alarms", "over_dc_volts", "maxVal");
    addSampleItem(cj, "alarms", "/site/bss_alarms", "under_dc_volts", "minVal");

    return cj;
}

void mYTestFunc(void * data)
{
    printf("%s running\n",__func__);

}

//#ifdef  1 //FUNCTION_TEST
int main()
{
    varsmap varMaps2;

    varmap varMap2;
    VarMapUtils vm;// = new VarMapUtils;

    int idx = 0;
    assetVar *av = new assetVar("dummy", idx);
    delete av;
    // 184 bytes per assetVar
    varMap2["d21"]  = new assetVar("d21",true);
    double sval = 0.0;
    
    vm.clearVm(varMap2);
    vm.setVal(varMaps2, "/components/bss_1", "dc_volts", sval);
    vm.clearVmap(varMaps2);
    vm.setFunc(varMaps2, "ess", "MyTestFunc" , (void*) &mYTestFunc);

    //void (*tf)(void *) = (void (*tf)(void *))
    void *res = 
    vm.getFunc(varMaps2, "ess","MyTestFunc" );
    typedef void (*myFun_t)(void * data);
    myFun_t myfun = myFun_t(res);
    myfun(nullptr);
    //tf();
    vm.testvecMap();

    //delete vmap;
    return 0;

    varmap varMap;

    //std::map<std::string,assetVar*>
    //std::map<std::string,std::map<std::string,assetVar*>>
    varsmap varMaps;

// first we create a list of vars

    varMap["d1"]  = new assetVar("d1",true);
    varMap["d2"]  = new assetVar("d2",true);
    varMap["sum"] = new assetVar("sum",sval);
    varMap["avg"] = new assetVar("avg",sval);
    varMap["max"] = new assetVar("max",sval);
    varMap["min"] = new assetVar("min",sval);
    varMap["v1"]  = new assetVar("v1",1000.45);
    varMap["v2"]  = new assetVar("v2",1134.7);
    varMap["v3"]  = new assetVar("v3",1212.4);
    varMap["v4"]  = new assetVar("v4",965.0);
    varMap["v5"]  = new assetVar("v5",800.5);
    varMap["v6"]  = new assetVar("v6",900.5);

    // just for this test
    double dvalue = 900.5;
    bool bvalue = true;
    //const char *tvalue;
    vm.setVal(varMaps, "/components/bss_1", "dc_volts", dvalue);
    vm.setVal(varMaps, "/components/bss_2", "dc_volts", dvalue);
    vm.setVal(varMaps, "/components/bss_3", "dc_volts", dvalue);
    vm.setVal(varMaps, "/features/bss_status", "max_dc_volts", dvalue);
    vm.setVal(varMaps, "/features/bss_status", "min_dc_volts", dvalue);
    vm.setVal(varMaps, "/features/bss_status", "avg_dc_volts", dvalue);
    vm.setVal(varMaps, "/site/bss_params", "check_dc_volts", bvalue);
    dvalue=9000.0 ;vm.setVal(varMaps, "/site/bss_params", "min_dc_volts", dvalue);
    dvalue=10000.0 ;vm.setVal(varMaps, "/site/bss_params", "max_dc_volts", dvalue);
    //tvalue = "BSS DC Volts Over Limit";
    //vm.setVal(varMaps, "/site/bss_alarms", "over_dc_volts", tvalue);
    //tvalue = "BSS DC Volts Under Limit";
    //vm.setVal(varMaps, "/site/bss_alarms", "under_dc_volts", tvalue);
 
    vm.setVal(varMaps, "base", "d1", bvalue);


    //varMaps["/components/bss_1"]["dc_volts"]= new assetVar("dc_volts",900.5);
    //varMaps["/components/bss_2"]["dc_volts"]= new assetVar("dc_volts",900.5);
    //varMaps["/components/bss_3"]["dc_volts"]= new assetVar("dc_volts",900.5);
    //varMaps["/features/bss_status"]["max_dc_volts"]= new assetVar("max_dc_volts",900.5);
    //varMaps["/features/bss_status"]["avg_dc_volts"]= new assetVar("avg_dc_volts",901.5);
    //varMaps["/features/bss_status"]["min_dc_volts"]= new assetVar("max_dc_volts",902.5);
    //varMaps["base"]["d1"]= new assetVar("d1",true);

// then we create a list of functions
    std::map<std::string,boolFunType> funMap;
    funMap["add_dbl"] = func_add_dbl;

    // set up the interface

    // we could loose the fixed indexing since the vars are named
    // here we are addng assetVars into the interface
    //
    Interface* iface = new Interface;
    iface->addIn(varMap["d1"]);
    iface->addIn(varMap["v1"]);
    iface->addIn(varMap["v2"]);
    iface->addIn(varMap["v3"]);
    iface->addIn(varMap["v4"]);
    iface->addIn(varMap["v5"]);
    iface->addIn(varMap["v6"]);

    iface->addOut("enabled",varMap["d2"]);
    iface->addOut("sum", varMap["sum"]);
    iface->addOut("avg", varMap["avg"]);
    iface->addOut("max", varMap["max"]);
    iface->addOut("min", varMap["min"]);

    //iface->clearAlarms()
    //iface->clearWarns()
    //iface->addParam("low_min", varMap["low_min"]);


    ///std::vector<assetVar*> ins = make_vector<assetVar*>() << d1 << v1 << v2 << v3 << v4 << v5;
    //std::vector<assetVar*> outs = make_vector<assetVar*>() << d2 << sum << avg << max << min;
    //  bool ok = func_add_dbl(iface);

    //bool ok; 
    funMap["add_dbl"](iface);

    printf("after first run \n sum %f \n avg %f \n min %f\n max %f \n ",
           varMap["sum"]->getdVal(),
           varMap["avg"]->getdVal(),
           varMap["min"]->getdVal(),
           varMap["max"]->getdVal()
          );

// we have not cleaned up yet.

//  func* f = new func();

//  delete f;
// OK now do it from a json Interface
// the objective it to be able to specify functions in the config text
// {
//     "id": "dc_volts_summary",
//     "inputs": [
//         {"var":"/components/bss_1","id":"dc_volts"},
//         {"var":"/components/bss_2","id":"dc_volts"},
//         {"var":"/components/bss_3","id":"dv_volts"}
//     ],
//     "outputs": [
//         {"op":"max","var":"/features/bss_status","id":"max_dc_volts"},
//         {"op":"min","var":"/features/bss_status","id":"min_dc_volts"}
//         {"op":"avg","var":"/features/bss_status","id":"avg_dc_volts"}
//     ],
//     "operation": "add_dbl",
//     "params": [
//       {"falseCase": 0}
//     ]
// }
 // cJSON_AddStringToObject(cj,"id","dc_volts_summary");
//  cJSON_AddStringToObject(cj,"operation","add_dbl");
    cJSON* cj = createSampleFunction("dc_volts_summary","add_dbl");

    const char *cjdata = cJSON_Print(cj);
    printf(" sample cjdata = [%s]\n", cjdata);

    // next we have to parse cj into a function exec object
    // we'll have a vector of commands , eventually they will be grouped by names
    // each time will have a name
    // NOTE we'll have a goto function to jump to a name.

    // the following will parse a function contained in a cJSON object

    funVec* funItem = parse_funItem(cj, varMaps);

    dvalue = 10034.6;
    vm.setVal(varMaps, "/components/bss_1", "dc_volts", dvalue);
    dvalue = 9034.6;
    vm.setVal(varMaps, "/components/bss_2", "dc_volts", dvalue);
    dvalue = 14034.8;
    vm.setVal(varMaps, "/components/bss_3", "dc_volts", dvalue);
    //varMaps["/components/bss_1"]["dc_volts"]->setVal(10034.6);
    //varMaps["/components/bss_2"]["dc_volts"]->setVal(9034.6);
    //varMaps["/components/bss_3"]["dc_volts"]->setVal(14034.6);
    // now run this
    auto fi = funMap.find(funItem->fname);
    if(fi == funMap.end())
    {
        printf (" unable to find operation [%s]\n", funItem->fname.c_str());
    }
    else
    {
        // todo add the var map here ??
        //ok = 
        funMap[funItem->fname](funItem->iface);
        // this is fragile if the maps dont have the vars defind
        printf(" for second run \n bss_1 %f \n bss_2 %f \n bss_3 %f\n\n ",
               varMaps["/components/bss_1"]["dc_volts"]->getdVal(),
               varMaps["/components/bss_2"]["dc_volts"]->getdVal(),
               varMaps["/components/bss_3"]["dc_volts"]->getdVal()
              );

        printf(" after second run \n max %f \n min %f \n avg %f\n\n ",
               varMaps["/features/bss_status"]["max_dc_volts"]->getdVal(),
               varMaps["/features/bss_status"]["min_dc_volts"]->getdVal(),
               varMaps["/features/bss_status"]["avg_dc_volts"]->getdVal()
              );

    }

    cJSON_Delete(cj);
    free((void *)cjdata);

    vm.clearVm(varMap);

    vm.clearVmap(varMaps);
    funMap.clear();
    
    delete funItem;
    delete iface;
    

//    delete vmap;
    return 0;
}
//#endif