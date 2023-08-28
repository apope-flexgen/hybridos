
#include "dataMap.h"

extern "C++" {
    int runDataMaps(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV);
};

struct DataMap;
std::unordered_map<std::string, DataMap*> dataMaps;     // a map of all dataMaps and their names. used to access dataMaps outside of their creation function


void DataMap::addDataItem(char *name, int offset, char *type)
{
    auto dataItem = new DataItem;
    dataItem->name = name;
    dataItem->offset = offset;
    dataItem->type = type;
    dataItems[std::string(name)] = dataItem;
}

void DataMap::addTransferItem(std::string bname, std::string aname, std::string dname)
{
    std::pair<std::string, std::string> item = std::make_pair(aname, dname);
    transferBlocks[bname].push_back(item);
}

void DataMap::showTransferItems(std::string bname)
{
    if (transferBlocks.find(bname) != transferBlocks.end())
    {
        FPS_PRINT_INFO(" Transfer Block name [{}] ", bname);
        FPS_PRINT_INFO("  [amap name]  <=> [dmap name]");
        FPS_PRINT_INFO("______________________________");
        for (auto xx : transferBlocks[bname])
        {
            FPS_PRINT_INFO("  [{}]  <=> [{}]", xx.first, xx.second);
        }
        std::cout << std::endl;
    }
}

void DataMap::getFromAmap(std::string bname, asset_manager* am, DataMap *dataMap, uint8_t* dataArea)
{
    if (transferBlocks.find(bname) != transferBlocks.end())
    {
        // FPS_PRINT_INFO("Get values from Amap using Transfer Block name [{}] ", bname);
        for (auto xx : transferBlocks[bname])
        {
            
            if(!dataMap->getDataItemFromAmap(am, dataMap, xx.first, xx.second, dataArea))
            {
                FPS_PRINT_ERROR("getDataItemFromAmap returned false. Exiting");
                exit(0);
                // send to error state
            }
        }
    }
    else 
    {
        FPS_PRINT_ERROR("Cannot find transfer block name: {}. Exiting program", bname);
        exit(0);
        // in threaded version, this should send to error state
    }
}

void DataMap::sendToAmap(std::string bname, asset_manager* am, DataMap *dataMap, uint8_t* dataArea)
{
    if (transferBlocks.find(bname) != transferBlocks.end())
    {
        // FPS_PRINT_INFO("Send values to Amap using Transfer Block name [{}] ", bname);
        for (auto xx : transferBlocks[bname])
        {
            if(!dataMap->setDataItemToAmap(am, dataMap, xx.first, xx.second, dataArea))
            {
                FPS_PRINT_ERROR("setDataItemToAmap returned false. Exiting");
                exit(0);
                // send to error state
            }
        }
    }
    else 
    {
        FPS_PRINT_ERROR("Cannot find transfer block name: {}. Exiting program", bname);
        exit(0);
        // in threaded version, this should send to error state
    }
}

// Function to map data from the asset_manager to the DataMap data area
// for all simulink types, our naming convention drops the _T from each type (ex: uint32_T is the "uint32" case)
bool DataMap::getDataItemFromAmap(asset_manager *am, DataMap *dataMap, const std::string &amapName, const std::string &mapName, uint8_t* dataArea)
{
    if (am->amap.find(amapName) != am->amap.end() && dataMap->dataItems.find(mapName) != dataMap->dataItems.end())
    {
        DataItem *dataItem = dataMap->dataItems[mapName];
        if (dataItem->type == "int")
        {
            *(int *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "double")
        {
            *(double *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
        }
        else if (dataItem->type == "bool")
        {
            *(bool *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getbVal();
        }
        else if (dataItem->type == "uint")
        {
            *(uint_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "int16")
        {
            *(int16_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "uint16")
        {
            *(uint32_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "int32")
        {
            *(int32_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "uint32")
        {
            *(uint32_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "int64")
        {
            *(int64_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "uint64")
        {
            *(uint64_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "ulong")
        {
            *(ulong_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "ulonglong")
        {
            *(ulonglong_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getiVal();
        }
        else if (dataItem->type == "real")
        {
            *(real_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
        }
        else if (dataItem->type == "real32")
        {
            *(real32_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
        }
        else if (dataItem->type == "real64")
        {
            *(real64_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
        }
        else if (dataItem->type == "time")
        {
            *(time_T *)(&dataArea[dataItem->offset]) = am->amap[amapName]->getdVal();
        }
        // Add more cases for other data types as needed
    }
    else
    {
        return false;
    }

    return true;
}

// Function to map data from the DataMap data area to the asset_manager. returns false if not found
// for all simulink types, our naming convention drops the _T from each type (ex: uint32_T is the "uint32" case)
bool DataMap::setDataItemToAmap(asset_manager* am, DataMap* dataMap, const std::string& amapName, const std::string& mapName, uint8_t* dataArea) 
{
    if (am->amap.find(amapName) != am->amap.end() && dataMap->dataItems.find(mapName) != dataMap->dataItems.end() && am->amap[amapName] != nullptr) 
    {
        DataItem *dataItem = dataMap->dataItems[mapName];
        if (dataItem->type == "int" || 
            dataItem->type == "uint" ||
            dataItem->type == "int16" ||
            dataItem->type == "uint16" ||
            dataItem->type == "int32" ||
            dataItem->type == "uint32" ||
            dataItem->type == "int64" ||
            dataItem->type == "uint64" ||
            dataItem->type == "ulong" ||
            dataItem->type == "ulonglong")
        {
            am->amap[amapName]->setVal(*(int*)(&dataArea[dataItem->offset]));
        } 
        else if (dataItem->type == "double" || 
                 dataItem->type == "real" ||
                 dataItem->type == "real32" || 
                 dataItem->type == "real64" ||
                 dataItem->type == "time") 
        {
            am->amap[amapName]->setVal(*(double*)(&dataArea[dataItem->offset]));
        }
        else if (dataItem->type == "bool") 
        {
            am->amap[amapName]->setVal(*(bool*)(&dataArea[dataItem->offset]));
        }
        // char case delayed until use case is created and char is needed
    }
    else
    {
        return false;
    }

    return true;
}


// gives dataMapObject context in other functions
static Datamaps_test::ExtUPointer_Reference_T dmTestInput;      // reference to the inputs
static Datamaps_test::ExtYPointer_Reference_T dmTestOutput;     // reference to the outputs
static Datamaps_test dmTestObject{ &dmTestInput, &dmTestOutput };

asset_manager *getOrMakeAm(VarMapUtils *vm, varsmap &vmap, const char *pname, const char *amname)
{   
    char* essName = vm->getSysName(vmap);
    auto essam = vm->getaM(vmap, essName);
    asset_manager *am = nullptr;

    
    if(pname)
    {
        // get parent asset manager or make one ourselves
        auto pam = vm->getaM(vmap, pname);
        if (!pam) 
        {
            FPS_PRINT_INFO(" pname [{}]  not found, have to make one", pname );
            pam = new asset_manager(pname);
            vm->setaM(vmap, pname, pam);
            // set up the rest of the am 
            pam->setFrom(essam);            // pam's parent is ess
            essam->addManAsset(pam, pname);
            FPS_PRINT_INFO(" added {} as a new parent asset manager", pname);
        }
        // get asset manager or make one ourselves
        am = vm->getaM(vmap, amname);
        if (!am)
        {
            FPS_PRINT_INFO(" amname [{}]  not found, have to make one", amname );
            am = new asset_manager(amname);
            vm->setaM(vmap, amname, am);
            // set up the rest of the am 
            am->setFrom(pam);
            pam->addManAsset(am, amname);
            FPS_PRINT_INFO(" added {} as a new asset manager", amname);
        } 
    }
    else
    {
        // if not given a pname, add amname asset manager to ess am children
        am = vm->getaM(vmap, amname);
        if (!am)
        {
            FPS_PRINT_INFO(" amname [{}]  not found, have to make one with essam as parent", amname );
            am = new asset_manager(amname);
            vm->setaM(vmap, amname, am);
            // set up the rest of the am 
            am->setFrom(essam);
            essam->addManAsset(am, amname);
            FPS_PRINT_INFO(" added {} as a new asset manager", amname);
        } 

    }

    
    return am;
}

void setupDMTest()
{
    FPS_PRINT_INFO("running {}", __func__);

    struct DataMap *dm = new DataMap;
    dm->name = "testDM";

    // adds input field dataItems
    dm->addDataItem((char*)"DMTestDirection",  offsetof(Datamaps_test::ExtUPointer_Reference_T, DMTestDirection),   (char*)"bool");

    // output
    dm->addDataItem((char*)"DMTestOut",    offsetof(Datamaps_test::ExtYPointer_Reference_T, DMTestOut),     (char*)"int32");
    dm->addDataItem((char*)"AdderResult",  offsetof(Datamaps_test::ExtYPointer_Reference_T, AdderResult),   (char*)"int32");
    dm->addDataItem((char*)"GainResult",   offsetof(Datamaps_test::ExtYPointer_Reference_T, GainResult),    (char*)"int32");
    dm->addDataItem((char*)"DMTestOutDblNoise",   offsetof(Datamaps_test::ExtYPointer_Reference_T, DMTestOutDblNoise), (char*)"real");
    dm->addDataItem((char*)"DirectionFeedback",   offsetof(Datamaps_test::ExtYPointer_Reference_T, DirectionFeedback), (char*)"bool");

    // add this data map to map of all datamaps
    dataMaps[dm->name] = dm;

    // transfer blocks
    dm->addTransferItem("dmTestInputs",     "DMTestDirection", "DMTestDirection");

    dm->addTransferItem("dmTestOutputs", "DMTestOut", "DMTestOut");
    dm->addTransferItem("dmTestOutputs", "AdderResult", "AdderResult");
    dm->addTransferItem("dmTestOutputs", "GainResult", "GainResult");
    dm->addTransferItem("dmTestOutputs", "DMTestOutDblNoise", "DMTestOutDblNoise");
    dm->addTransferItem("dmTestOutputs", "DirectionFeedback", "DirectionFeedback");

}

void setupDMtestAmap(VarMapUtils *vm, varsmap &vmap, asset_manager *am)
{
    FPS_PRINT_INFO("setting up demo datamap amap for /control/{}", am->name);
    int ival = 0;
    double dval = 0;
    bool bval = false;
    
    am->amap["DMTestDirection"]     = vm->setVal(vmap, "/control/dataMapTest", "DMTestDirection", bval);

    am->amap["DMTestOut"]           = vm->setVal(vmap, "/control/dataMapTest", "DMTestOut", ival);
    am->amap["AdderResult"]         = vm->setVal(vmap, "/control/dataMapTest", "AdderResult", ival);
    am->amap["GainResult"]          = vm->setVal(vmap, "/control/dataMapTest", "GainResult", ival);
    am->amap["DMTestOutDblNoise"]   = vm->setVal(vmap, "/control/dataMapTest", "DMTestOutDblNoise", dval);
    am->amap["DirectionFeedback"]   = vm->setVal(vmap, "/control/dataMapTest", "DirectionFeedback", bval);

}


// on scheduler functions
int runDataMaps(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
    FPS_PRINT_INFO("\n\n\nIN RUN DATAMAPS");
    asset_manager *am = aV->am;
    VarMapUtils *vm = am->vm;

    int debug = 1;
    int reload = 0;
    essPerf ePerf(am, aname, __func__);

    auto relname = fmt::format("{}_{}", __func__, "reload");
    assetVar* reloadAV = amap[relname];
    asset_manager *testDMam = getOrMakeAm(vm, vmap, aname, "testDM_asset_manager");

    if (!reloadAV || (reload = reloadAV->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        if (debug) FPS_PRINT_INFO("Reloading ...");
        // datamap and amap init functions (what used to be setup_demo_rack and setup_demo_rack_amap)
        setupDMTest();

        setupDMtestAmap(vm, vmap, testDMam);
        if (debug) FPS_PRINT_INFO("amap set up");

        reload = 2;
        if(!reloadAV)
        {
            reloadAV = vm->setVal(vmap, "/reload", relname.c_str(), reload);
            amap[relname] = reloadAV;
        }
        else 
        {
            reloadAV->setVal(reload);
        }

        if (debug) FPS_PRINT_INFO("Finished reloading");
    }
    
    // getFromAmap
    DataMap *dm = dataMaps["testDM"];

    dm->getFromAmap("dmTestInputs", testDMam, dm, (uint8_t *)&dmTestInput);
    
    if (debug){
        FPS_PRINT_INFO("Got these values from amap:");
        FPS_PRINT_INFO("dmTestOutput.DirectionFeedback: {}", dmTestOutput.DirectionFeedback);       // NEW! testing
        FPS_PRINT_INFO("dmTestOutput.DMTestOut: {}", dmTestOutput.DMTestOut);
        FPS_PRINT_INFO("dmTestOutput.DMTestOutDblNoise: {}", dmTestOutput.DMTestOutDblNoise);       // NEW! testing
        FPS_PRINT_INFO("dmTestOutput.AdderResult: {}", dmTestOutput.AdderResult);
        FPS_PRINT_INFO("dmTestOutput.GainResult: {}", dmTestOutput.GainResult);        
    }

    FPS_PRINT_INFO("\n\nRunning step function\n");
    dmTestObject.step();

    if (debug){
        FPS_PRINT_INFO("Got these values after running step():");
        FPS_PRINT_INFO("dmTestOutput.DirectionFeedback: {}", dmTestOutput.DirectionFeedback);       // NEW! testing
        FPS_PRINT_INFO("dmTestOutput.DMTestOut: {}", dmTestOutput.DMTestOut);
        FPS_PRINT_INFO("dmTestOutput.DMTestOutDblNoise: {}", dmTestOutput.DMTestOutDblNoise);       // NEW! testing
        FPS_PRINT_INFO("dmTestOutput.AdderResult: {}", dmTestOutput.AdderResult);
        FPS_PRINT_INFO("dmTestOutput.GainResult: {}", dmTestOutput.GainResult);
    }

    dm->sendToAmap("dmTestOutputs", testDMam, dm, (uint8_t *)&dmTestOutput);

    FPS_PRINT_INFO("done");
    return 0;
}








/*
TO RUN THINGS

    This runs the func twice (change inValue to != 0 if you only want it to run once)
fims_send -m set -r /$$ -u /ess/demo/code '{
    "runExtCode":{
        "value":0,
        "actions":{
            "onSet":[
                {"func":
                    [
                        {"func":"demoExtCode", "inValue":0},
                        {"func":"demoExtCode"}
                    ]
                }
            ]
        }
    }
}'


    this wil set up two bms_racks each with 2 modules
fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":15,"cmd":"setup_demo","pname":"rack","num_racks":2, "num_modules":2}}'


    to test set_vlink
fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":0,"cmd":"set_vlink","amap":"someVar","amname":"ext_1", "uri":"/components/ext_1:someVar","val":1111}}'
    

    to test set_amap
fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":0,"cmd":"set_amap","amap":"someVar","amname":"ext_1", "uri":"/sys/status:someVar","val":2222}}'


    this sets up the parent and the asset  using set_mapping
fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":3,"cmd":"set_mapping","pname":"ext","amname":"ext_1"}}'
fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":3,"cmd":"set_mapping","pname":"ext","amname":"ext_2"}}'


    use these to inspect system status, and specific parent or asset
fims_send -m get -r /$$ -u /ess/components/ext_1 | jq
fims_send -m get -r /$$ -u /ess/full/sys/status | jq




    to view the whole amap 
fims_send -m get -r /$$ -u /ess/amap | jq
fims_send -m get -r /$$ -u /ess/amap/control | jq 
    (with objects)

    to view the values in each rack
fims_send -m get -r /$$ -u /ess/control | jq
    add /rack_x_y to look at a specific rack
    add /field to look at specific value (fims_send -m get -r /$$ -u /ess/control/rack_0/max_charge_capacity | jq for example)


    running setup_demo creates an amap of racks/modules with all zero values
    this will set values to the amap
fims_send -m set -u /ess/control/rack_X_Y/field_to_be_set 15
    only fields rn are charge_capacity, charge_current, max_charge_capacity, & soc



use /functional_ess/ scripts to runDataMaps


*/
