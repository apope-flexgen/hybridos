#include "LowPassFilter.hpp"
#include "dataMap.h"

std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<LowPassFilter>>> LowPassFilterObjects;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<LowPassFilter::ExtU>>> LowPassFilterInputs;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<LowPassFilter::ExtY>>> LowPassFilterOutputs;

uint8_t* getLowPassFilterInputs(std::string uri, int instance)
{
    LowPassFilter::ExtU* dmLPFInputs = LowPassFilterInputs[uri][instance].get();

    return reinterpret_cast<uint8_t*>(dmLPFInputs);
}

uint8_t* getLowPassFilterOutputs(std::string uri, int instance)
{
    LowPassFilter::ExtY* dmLPFOutputs = LowPassFilterOutputs[uri][instance].get();

    return reinterpret_cast<uint8_t*>(dmLPFOutputs);
}

void LowPassFilterRun(std::string uri, int instance)
{
    LowPassFilter* dmLPFObject = LowPassFilterObjects[uri][instance].get();

    dmLPFObject->step();

    if (dmLPFObject->InputRef->debug)
    {
        FPS_PRINT_INFO("Ran step() for LowPassFilter model on aV [{}]", uri);
        FPS_PRINT_INFO("    input: [{}]     output: [{}]", dmLPFObject->InputRef->input,
                       dmLPFObject->OutputRef->output);
    }
}

void createNewLowPassFilterInstance(std::string uri, int instance)
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
    modelFcnRef["LowPassFilterInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

    uint8_t* (*getOutputsPtr)(std::string, int) = &getLowPassFilterOutputs;
    modelFcnRef["LowPassFilterOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

    // set refernce to LowPassFilter's run function using a global external
    void (*runFuncPtr)(std::string, int) = &LowPassFilterRun;
    modelFcnRef["LowPassFilter"] = reinterpret_cast<void(*)>(runFuncPtr);
}

void setupLowPassFilterDM(assetVar* aV, int instance)
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

    std::string inputBlock = "LowPassFilter_" + instanceStr + "Inputs";
    std::string outputBlock = "LowPassFilter_" + instanceStr + "Outputs";

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

void setupLowPassFilterAmap(VarMapUtils* vm, varsmap& vmap, asset_manager* am, int instance, std::string uri)
{
    int debug = 0;
    double dVal = 0.0;
    bool bVal = false;

    // we want the amap entry to use underscores when displaying the comp instead of slashes
    std::string underscoreURI = replaceSlashAndColonWithUnderscore(uri);
    std::string ctrlLPF = "/control" + underscoreURI + "/LowPassFilter_" + std::to_string(instance);

    if (debug)
        FPS_PRINT_INFO("Setting up datamap to amap interface for {} using amap of asset manager: [{}]", ctrlLPF,
                       am->name);

    // inputs amap vals
    std::string inputAmap = "input";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlLPF.c_str(), (char*)inputAmap.c_str(), dVal);

    inputAmap = "debug";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlLPF.c_str(), (char*)inputAmap.c_str(), bVal);

    // Output amap vals
    std::string outputAmap = "output";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlLPF.c_str(), (char*)outputAmap.c_str(), dVal);
}

int setupLowPassFilter(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);

    // create datamap and its asset manager
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = aV->getbParam("debug");

    // use parent AV to run everything
    if (!aV->gotParam("parentAV"))
    {
        FPS_PRINT_ERROR("There is no \"parent_AV\" parameter in [{}]. Cannot signal back to the thread. Timing out",
                        aV->name);
        return 0;
    }
    std::string parent_uri = aV->getcParam("parentAV");

    // get parent AV from vmap
    assetVar* parent_AV = vm->getVar(vmap, (char*)parent_uri.c_str(), nullptr);
    if (!parent_AV)
    {
        FPS_PRINT_ERROR(
            "Could not find parent AV of [{}] using comp [{}]. Cannot signal back to the thread. Timing out", aV->name,
            parent_uri);
        return 0;
    }
    // determine which instance we are setting up and instantiate it
    if (!aV->gotParam("LowPassFilter_instance"))
    {
        aV->setParam("LowPassFilter_instance", 0);
    }
    int instance = aV->getiParam("LowPassFilter_instance") + 1;

    if (debug)
        FPS_PRINT_INFO("Setting up LowPassFilter on [{}]", parent_uri);

    // create new instance and set references to it for the rest of the system
    createNewLowPassFilterInstance(parent_uri, instance);

    // setup the datamap for the aV we are going to run our function on
    setupLowPassFilterDM(parent_AV, instance);

    // get or make the asset manager for our instance
    std::string tmp = replaceSlashAndColonWithUnderscore(parent_uri);
    tmp.erase(tmp.begin());
    std::string instanceAMname = tmp + "_LowPassFilter_" + std::to_string(instance) + "_asset_manager";
    asset_manager* datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

    // setup amap for this instance
    setupLowPassFilterAmap(vm, vmap, datamapInstanceAM, instance, parent_uri);

    // we have set up the LowPassFilter object and now we need to set up all of its parameters
    std::string errorMsg;

    // OBJECT PARAMS
    // set the repTime of this aV to our Battery
    if (!parent_AV->gotParam("repTime"))
    {
        FPS_PRINT_ERROR("AV [{}] does not have a repTime, please configure this av to run repeatedly", parent_AV->name);
        errorMsg = fmt::format("Parent AV [{}] does not have \"repTime\" param", parent_AV->name);

        signalError(parent_AV, "setupLowPassFilter_" + std::to_string(instance), errorMsg, true);
        return 0;
    }
    double repTime = parent_AV->getdParam("repTime");

    // get our cutoff freq
    if (!parent_AV->gotParam("CutoffFrequency"))
    {
        // default to 0.45 if no value is given
        parent_AV->setParam("CutoffFrequency", 0.45);
    }
    double fc = parent_AV->getdParam("CutoffFrequency");

    // get our LPF object
    std::unique_ptr<LowPassFilter>& uqObjPtr = LowPassFilterObjects[parent_uri][instance];
    LowPassFilter* LPF = uqObjPtr.get();

    // set all the values to our LPF class
    LPF->Configs.fs = 1 / repTime;
    LPF->Configs.fc = fc;

    // URI PARAMS
    // determine the uri of our amap /control/ space
    std::string controlURI = "/control" + replaceSlashAndColonWithUnderscore(parent_uri) + "/LowPassFilter_" +
                             std::to_string(instance);

    // get our input URI
    std::string inputURIstr;
    if (!parent_AV->gotParam("InputURI"))
    {
        // if we dont have an input URI, simply dont set up a remap
        if (debug)
            FPS_PRINT_INFO("LowPassFilter_{} on aV [{}:{}] is not setting up an input remap", instance, parent_AV->comp,
                           parent_AV->name);
    }
    else
    {
        inputURIstr = parent_AV->getcParam("InputURI");

        // get the aV from our input uri
        assetVar* inputAV = vm->getVar(vmap, (char*)inputURIstr.c_str(), nullptr);
        if (!inputAV)
        {
            // if this aV doesnt exist already, make it
            double dval = 0;
            inputAV = vm->makeVar(vmap, inputURIstr.c_str(), nullptr, dval);
            FPS_PRINT_WARN("aV for InputURI doesnt exist, creating new aV [{}:{}] and adding remap to it",
                           inputAV->comp, inputAV->name);
        }

        // create a remap in our inputAV that will remap its value to our /control/ amap space when set to
        inputAV->addRemap(controlURI + ":input");
    }

    // get our output URI
    std::string outputURIstr;
    if (!parent_AV->gotParam("OutputURI"))
    {
        // if we dont have an output uri, dont remap our output automatically
        if (debug)
            FPS_PRINT_INFO("LowPassFilter_{} on aV [{}:{}] is not setting up an output remap", instance,
                           parent_AV->comp, parent_AV->name);
    }
    else
    {
        outputURIstr = parent_AV->getcParam("OutputURI");

        // now we need to remap from our /control/.../output to our outputURI
        assetVar* controlOutputAV = vm->getVar(vmap, (char*)controlURI.c_str(), "output");
        if (!controlOutputAV)
        {
            FPS_PRINT_ERROR("Could not find aV [{}:output]. Please retry setup", controlURI);
            errorMsg = fmt::format(
                "Could not find aV [{}:output]. Please ensure this aV is set up properly in setupLowPassFilterAmap",
                controlURI);

            signalError(parent_AV, "setupLowPassFilter_" + std::to_string(instance), errorMsg, true);
            return 0;
        }

        controlOutputAV->addRemap(outputURIstr);
    }

    // tell our parent AV that we are done by setting the setup flag for this function instance high
    std::string thisFunction = "LowPassFilter_" + std::to_string(instance);

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

                aV->setParam("LowPassFilter_instance", instance);

                // set our alreadyConfigured flag high on our parentAV
                parent_AV->setParam("alreadyConfigured", true);
                return 0;
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
            signalError(parent_AV, "setupLowPassFilter_" + std::to_string(instance), errorMsg, true);
            return 0;
        }
    }

    return 0;
}

// this function is meant to make configuration simpler for LowPassFilter using datamaps
int LowPassFilterConfigWrapper(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // This is a wrapper function that exists to run LowPass Filter in a way that is more easily configurable
    // This function takes the information from its aV, creates a new aV, configures the new aV parameters in the
    // specific way the system needs with the information from this aV, and schedules that new aV periodically This
    // function is also used to re initialize the LowPass Filter object. Calling this function a second (or more) time
    // will delete the old object and create a new one.

    UNUSED(aname);
    UNUSED(amap);
    UNUSED(p_fims);

    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = false;
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    // create a new assetVar for this BMS that RunThread will run on periodically
    std::string comp = aV->comp + ":" + aV->name + "_LPFilter";
    assetVar* lpfAV = vm->getVar(vmap, (char*)comp.c_str(), nullptr);
    if (!lpfAV)
    {
        // if this lpfAV doesnt exist already, make it
        double dval = 0;
        lpfAV = vm->makeVar(vmap, comp.c_str(), nullptr, dval);
        lpfAV->am = aV->am;
        if (debug)
            FPS_PRINT_INFO("made new lpfAV [{}:{}]", lpfAV->comp, lpfAV->name);

        // inform this aV where the actual magic is happening
        std::string note = fmt::format(
            "The LowPassFilter algorithm is running on aV [{}:{}]. Please check that aV for more information",
            lpfAV->comp, lpfAV->name);
        aV->setParam("NOTE", (char*)note.c_str());

        // put a note on the active aV about who started it
        note = fmt::format("This aV was configured and ran based on the information from [{}:{}]", aV->comp, aV->name);
        lpfAV->setParam("NOTE", (char*)note.c_str());

        // default the alreadyConfigured to be false
        lpfAV->setParam("alreadyConfigured", false);
    }
    if (debug)
        FPS_PRINT_INFO("Using lpfAV [{}:{}]", lpfAV->comp, lpfAV->name);

    // pass debug flag to new av
    if (debug)
        lpfAV->setParam("debug", true);

    // transfer all the params from the aV that triggered us to our new "_LPFilter" av
    // datamap specific params
    if (aV->gotParam("overrunLimit"))
    {
        lpfAV->setParam("overrunLimit", aV->getdParam("overrunLimit"));
    }

    if (aV->gotParam("decrementOverrun"))
    {
        lpfAV->setParam("decrementOverrun", aV->getdParam("decrementOverrun"));
    }

    if (aV->gotParam("incrementOverrun"))
    {
        lpfAV->setParam("incrementOverrun", aV->getdParam("incrementOverrun"));
    }

    if (aV->gotParam("setupTimeLimit"))
    {
        lpfAV->setParam("setupTimeLimit", aV->getdParam("setupTimeLimit"));
    }

    if (aV->gotParam("heartbeatTimeout"))
    {
        lpfAV->setParam("heartbeatTimeout", aV->getdParam("heartbeatTimeout"));
    }

    // lpf specific params
    if (aV->gotParam("CutoffFrequency"))
    {
        lpfAV->setParam("CutoffFrequency", aV->getdParam("CutoffFrequency"));
    }

    if (aV->gotParam("InputURI"))
    {
        lpfAV->setParam("InputURI", aV->getcParam("InputURI"));
    }

    if (aV->gotParam("OutputURI"))
    {
        lpfAV->setParam("OutputURI", aV->getcParam("OutputURI"));
    }

    // check if we've already been configured
    if (!lpfAV->gotParam("alreadyConfigured"))
    {
        // if we dont have this variable, set it to false
        lpfAV->setParam("alreadyConfigured", false);
    }

    if (lpfAV->getbParam("alreadyConfigured"))
    {
        if (debug)
            FPS_PRINT_INFO("We have already been configured and are re initializing our LowPassFilter object", nullptr);

        // if we've already been re configured, clear all our funcX flags and set to reload flag
        int num = 0;
        while (++num)
        {
            // determine which function number we are setting up
            std::string numStr = std::to_string(num);
            std::string funcNum = "func" + numStr;
            if (!lpfAV->gotParam((char*)funcNum.c_str()))
            {
                // we have cleared all the funcs listed on our av and now we have found a func# that doesnt exist, break
                // out of loop
                if (debug)
                    FPS_PRINT_INFO("Could not find [{}] param on aV [{}:{}]. Done clearing all params", funcNum,
                                   lpfAV->comp, lpfAV->name);
                break;
            }

            // clear this func#
            lpfAV->setParam((char*)funcNum.c_str(), (char*)"");
            if (debug)
                FPS_PRINT_INFO("Just set [{}] to [{}]", funcNum, lpfAV->getcParam((char*)funcNum.c_str()));
        }

        // reset the datamaps system flags
        lpfAV->setParam("reconfigured", false);
        lpfAV->setParam("reload", 0);
        lpfAV->setParam("LowPassFilter_instance", 0);
    }

    // set the function we want to run
    lpfAV->setParam("func1", (char*)"LowPassFilter");

    // TODO future improvement:
    // allow this function to have multiple instances to simulate higher order filters
    // 3 "instances" = 3rd order filter

    // set the thread that our functions will run on
    std::string threadName = aV->name + "_Thread";
    lpfAV->setParam("threadName", (char*)threadName.c_str());

    // set up new schedItem
    schedItem* schItem = new schedItem();

    // get our repTime
    if (!aV->gotParam("every"))
    {
        aV->setParam("every", 1);  // default value is every 1 second
    }
    double every = aV->getdParam("every");

    schItem->putOnScheduler("RunThread", 0, vm->get_time_dbl(), every, comp, lpfAV);

    return 0;
}
