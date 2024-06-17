#include "BatteryBalancingUtility.hpp"
#include "LowPassFilter.hpp"
#include "dataMap.h"

/*

Why this file exists:
    This file allows for configuration of filters on the inputs of the BRB racks.
    In order to ensure that the filter functions run before the rack functions, they must be on the same aV.
    This is a problem since the original LPF_Interface file already has its own set up function.
    To work around the issue of trying to get the LPF function to not use its original set up function, this file was
made This file does a similar thing to the Battery Balancing Racks where the "setup function" that is called by
dataMapThread does not actually do anything The real setup of the filter objects is done in the rack class -> the rack
class calls "setupLowPassFilterfromBRB" where it is needed This allows us to do the set up we need while having all the
functions (filter functions, rack functions, and BB func) on the same aV

*/

// this function must be different from the original createNewLPFInstance because we need to store these objects in the
// map using the key "LowPassFilterBRB"
void createNewLowPassFilterBRBInstance(std::string uri, int instance)
{
    // create a new instance of the LowPassFilter inputs struct, output struct, and object
    LowPassFilter::ExtU* dmLPFInput = new LowPassFilter::ExtU;
    LowPassFilter::ExtY* dmLPFOutput = new LowPassFilter::ExtY;
    LowPassFilter* dmLowPassFilterObject = new LowPassFilter(dmLPFInput, dmLPFOutput);

    // transfer memory management of "new" calls to a unique pointer
    std::unique_ptr<LowPassFilter::ExtU> dmLPFInputPtr(dmLPFInput);
    std::unique_ptr<LowPassFilter::ExtY> dmLPFOutputPtr(dmLPFOutput);
    std::unique_ptr<LowPassFilter> dmLowPassFilterObjectPtr(dmLowPassFilterObject);

    // move the ownership of the unique pointer to a global map of instances to unique pointers. need a different map
    // for each type of unique pointer
    LowPassFilterInputs[uri][instance] = std::move(dmLPFInputPtr);
    LowPassFilterOutputs[uri][instance] = std::move(dmLPFOutputPtr);
    LowPassFilterObjects[uri][instance] = std::move(dmLowPassFilterObjectPtr);

    // use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in
    // our global external map
    uint8_t* (*getInputsPtr)(std::string, int) = &getLowPassFilterInputs;
    modelFcnRef["LowPassFilterBRBInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

    uint8_t* (*getOutputsPtr)(std::string, int) = &getLowPassFilterOutputs;
    modelFcnRef["LowPassFilterBRBOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

    // set refernce to LowPassFilter's run function using a global external
    void (*runFuncPtr)(std::string, int) = &LowPassFilterRun;
    modelFcnRef["LowPassFilterBRB"] = reinterpret_cast<void(*)>(runFuncPtr);
}

// this function needs to be different than the original setupLPFDM because the transfer blocks need to include "BRB" in
// the name
void setupLowPassFilterBRBDM(assetVar* aV, int instance)
{
    if (!aV->gotParam("datamapName"))
    {
        // we should never get here because RunThread will set a default datamap name if none exits
        // if datamap_name somehow gets deleted, use another default datamap name
        int num_datamap = dataMaps.size() + 1;
        std::string num = std::to_string(num_datamap);

        std::string dflt = "Default_Datamap_" + num;
        aV->setParam("datamapName", (char*)dflt.c_str());

        FPS_PRINT_WARN("Could not find \"datamap_name\" parameter in assetVar [{}]. Using default name [{}]", __func__,
                       aV->name, dflt);
    }
    std::string name = aV->getcParam("datamapName");

    DataMap* dm = nullptr;
    auto it = dataMaps.find(name);
    if (it != dataMaps.end())
    {
        dm = it->second.get();
    }
    else
    {
        dataMaps[name] = std::make_unique<DataMap>();
        dataMaps[name].get()->name = name;
        dm = dataMaps[name].get();
    }

    std::string instanceStr = std::to_string(instance);

    std::string inputBlock = "LowPassFilterBRB_" + instanceStr + "Inputs";
    std::string outputBlock = "LowPassFilterBRB_" + instanceStr + "Outputs";

    // Input data items and transfer blocks
    std::string inputName = "input";
    dm->addDataItem((char*)inputName.c_str(), offsetof(LowPassFilter::ExtU, input), DataMapType::DOUBLE_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "debug";
    dm->addDataItem((char*)inputName.c_str(), offsetof(LowPassFilter::ExtU, debug), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    // Output data items and transfer blocks
    std::string outputName = "output";
    dm->addDataItem((char*)outputName.c_str(), offsetof(LowPassFilter::ExtY, output), DataMapType::DOUBLE_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());
}

int setupLPFDummy(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // this is the "setup" function that gets called by the dataMaps system on the thread but this is not where the
    // filters for BRB actually get set up. Each filters's setup function is called in setupRack so that the filter
    // setup can have access to the info it needs. this "function" still needs to exists so that the datamaps system can
    // still "get, run, send" the LPF function without setting it up itself see below for the real
    // setupLowPassFilterfromBRB function
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(aV);
    return 0;
}

// this is the setup function that will be called by BRB to set up a low pass filter for its input
std::string setupLowPassFilterfromBRB(varsmap& vmap, assetVar* parent_AV, assetVar* aV, int instance, double repTime,
                                      double fc, std::string inputURI)
{
    // create datamap and its asset manager
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = aV->getbParam("debug");

    std::string parent_uri = parent_AV->comp + ":" + parent_AV->name;
    if (debug)
        FPS_PRINT_INFO("Setting up LowPassFilter_{} on [{}]", instance, parent_uri);

    // create new instance and set references to it for the rest of the system
    createNewLowPassFilterBRBInstance(parent_uri, instance);

    // setup the datamap for the aV we are going to run our function on
    setupLowPassFilterBRBDM(parent_AV, instance);

    // get or make the asset manager for our instance
    std::string tmp = replaceSlashAndColonWithUnderscore(parent_uri);
    tmp.erase(tmp.begin());
    std::string instanceAMname = tmp + "_LowPassFilterBRB_" + std::to_string(instance) + "_asset_manager";
    asset_manager* datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

    // setup amap for this instance
    setupLowPassFilterAmap(vm, vmap, datamapInstanceAM, instance, parent_uri);

    // we have set up the LowPassFilter object and now we need to set up all of its parameters
    std::string errorMsg;

    // get our LPF object
    std::unique_ptr<LowPassFilter>& uqObjPtr = LowPassFilterObjects[parent_uri][instance];
    LowPassFilter* LPF = uqObjPtr.get();

    // set all the values to our LPF class
    LPF->Configs.fs = 1 / repTime;
    LPF->Configs.fc = fc;

    // URI PARAMS
    bool debugRemaps = true;
    if (parent_AV->gotParam("fims"))
    {
        debugRemaps = parent_AV->getbParam("fims");
    }

    // determine the uri of our amap /control/ space
    std::string controlURI = "/control" + replaceSlashAndColonWithUnderscore(parent_uri) + "/LowPassFilter_" +
                             std::to_string(instance);

    // only add remap if our input URI is real. no input URI is a valid option for this function
    if (isValidURI(inputURI))
    {
        // get the aV from our input uri
        assetVar* inputAV = vm->getVar(vmap, (char*)inputURI.c_str(), nullptr);
        if (!inputAV)
        {
            // if this aV doesnt exist already, make it
            double dval = 0;
            inputAV = vm->makeVar(vmap, inputURI.c_str(), nullptr, dval);
            FPS_PRINT_WARN("aV for InputURI [\"{}\"] doesnt exist, creating new aV [{}:{}] and adding remap to it",
                           inputURI, inputAV->comp, inputAV->name);
        }

        // create a remap in our inputAV that will remap its value to our /control/ amap space when set to
        if (debugRemaps)
        {
            inputAV->addRemap(controlURI + ":input", 0, (void*)0, (void*)0, (void*)0, "set");
        }
        inputAV->addRemap(controlURI + ":input", 0, (void*)0, (void*)0, true);
    }

    // tell our parent AV that we are done by setting the setup flag for this function instance high
    std::string thisFunction = "LowPassFilterBRB_" + std::to_string(instance);

    // check every function param in our parent AV to find our function number
    int num = 0;
    while (++num)
    {
        // determine which function number we are setting up
        std::string numStr = std::to_string(num);
        std::string funcNum = "func" + numStr;
        if (parent_AV->gotParam((char*)funcNum.c_str()))
        {
            // function is the name we got off our parent AV
            std::string iterativeFunction = parent_AV->getcParam((char*)funcNum.c_str());

            if (iterativeFunction == thisFunction)
            {
                // we have found our function, set its done flag high
                std::string setupNum = "setup" + numStr;
                parent_AV->setParam((char*)setupNum.c_str(), true);

                if (debug)
                    FPS_PRINT_INFO(" we found {} from the [{}] param. setting [{}] to true", thisFunction, funcNum,
                                   setupNum);

                aV->setParam("LowPassFilterBRB_instance", instance);

                // set our alreadyConfigured flag high on our parentAV
                parent_AV->setParam("alreadyConfigured", true);

                // return this filters uri
                return controlURI + ":output";
            }
        }
        else
        {
            // if we get here we have no way to signal back to RunThread that we are done setting up. Setting error to
            // our parentAV
            FPS_PRINT_ERROR("Could not find \"{}\" in [{}]'s list of functions. Signaling error", thisFunction,
                            parent_uri);

            std::string errorMsg = fmt::format("Could not find \"{}\" in [{}]'s list of functions.", thisFunction,
                                               parent_uri);
            signalError(parent_AV, "setupLowPassFilterfromBRB_" + std::to_string(instance), errorMsg, true);
            return "error";
        }
    }

    return 0;
}