#include "BatteryBalancingUtility.hpp"
#include "dataMap.h"

// NOTE: this interface file's setup function is called by BatteryBalancing's setup function
// this object has no step function yet, and can only get send inputs from the amap or send outputs to the amap

extern "C++" {
std::string setupLowPassFilterfromBRB(varsmap& vmap, assetVar* parent_AV, assetVar* aV, int instance, double repTime,
                                      double fc, std::string inputURI);
}

using namespace BatteryBalancingUtility;

std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<RackInfo::ExtU>>> RackInputs;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<RackInfo::ExtY>>> RackOutputs;

uint8_t* getRackInputs(std::string uri, int instance)
{
    RackInfo::ExtU* dmRackInputs = RackInputs[uri][instance].get();

    return reinterpret_cast<uint8_t*>(dmRackInputs);
}

uint8_t* getRackOutputs(std::string uri, int instance)
{
    RackInfo::ExtY* dmRackOutputs = RackOutputs[uri][instance].get();

    return reinterpret_cast<uint8_t*>(dmRackOutputs);
}

void RackRun(std::string uri, int instance)
{
    UNUSED(uri);
    UNUSED(instance);

    return;
}

// Racks should not have a Run function or a getOuputs function

void createNewRackInstance(BatteryBalancingUtility::BatteryBalancing* Battery, std::string uri, int instance)
{
    // create a new instance of the RackInfo inputs struct
    RackInfo::ExtU* dmRackInput = new RackInfo::ExtU;
    RackInfo::ExtY* dmRackOutput = new RackInfo::ExtY;

    // transfer memory management of struct to a unique pointer
    std::unique_ptr<RackInfo::ExtU> dmRackInputPtr(dmRackInput);
    std::unique_ptr<RackInfo::ExtY> dmRackOutputPtr(dmRackOutput);

    // move the ownership of the unique pointer to a global map of instances to unique pointers. need a different map
    // for each type of unique pointer
    RackInputs[uri][instance] = std::move(dmRackInputPtr);
    RackOutputs[uri][instance] = std::move(dmRackOutputPtr);

    // make a new unique pointer to a RackInfo object with the RackInput struct
    std::unique_ptr<BatteryBalancingUtility::RackInfo> rackPtr = std::make_unique<RackInfo>(dmRackInput, dmRackOutput);

    // assign this rack a rackNum
    rackPtr.get()->rackNum = instance;

    // add this rack to Battery's vector of racks
    Battery->racks.push_back(std::move(rackPtr));

    // use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in
    // our global external map
    uint8_t* (*getInputsPtr)(std::string, int) = &getRackInputs;
    modelFcnRef["RackInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

    // even tho send and run functions do nothing, the system still needs a reference to them
    uint8_t* (*getOutputsPtr)(std::string, int) = &getRackOutputs;
    modelFcnRef["RackOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

    // set refernce to BatteryBalancing's run function using a global external
    void (*runFuncPtr)(std::string, int) = &RackRun;
    modelFcnRef["Rack"] = reinterpret_cast<void(*)>(runFuncPtr);
}

void setupRackDM(assetVar* aV, int instance)
{
    if (!aV->gotParam("datamapName"))
    {
        // we should never get here because RunThread will set a default datamap name if none exits
        // if datamap_name somehow gets deleted, use another default datamap name
        int num_datamap = dataMaps.size() + 1;
        std::string num = std::to_string(num_datamap);

        std::string dflt = "Default_Datamap_" + num;
        aV->setParam("datamapName", (char*)dflt.c_str());

        FPS_PRINT_ERROR("Could not find \"datamap_name\" parameter in assetVar [{}]. Using default name [{}]", __func__,
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

    std::string inputBlock = "Rack_" + instanceStr + "Inputs";
    std::string outputBlock = "Rack_" + instanceStr + "Outputs";

    // Input data items and transfer blocks
    std::string inputName = "voltage";
    dm->addDataItem((char*)inputName.c_str(), offsetof(RackInfo::ExtU, voltage), DataMapType::DOUBLE_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "soc";
    dm->addDataItem((char*)inputName.c_str(), offsetof(RackInfo::ExtU, soc), DataMapType::DOUBLE_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "contactorStatus";
    dm->addDataItem((char*)inputName.c_str(), offsetof(RackInfo::ExtU, contactorStatus), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "ignoreExternal";
    dm->addDataItem((char*)inputName.c_str(), offsetof(RackInfo::ExtU, ignoreExternal), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "enableFeedback";
    dm->addDataItem((char*)inputName.c_str(), offsetof(RackInfo::ExtU, enableFeedback), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    // Rack outputs
    std::string outputName = "ignoreInternal";
    dm->addDataItem((char*)outputName.c_str(), offsetof(RackInfo::ExtY, ignoreInternal), DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "enableCmd";
    dm->addDataItem((char*)outputName.c_str(), offsetof(RackInfo::ExtY, enableCmd), DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());
}

void setupRackAmap(VarMapUtils* vm, varsmap& vmap, asset_manager* am, int instance, std::string uri)
{
    int debug = 0;
    double dVal = -1.0;
    bool bVal = false;

    std::string instanceStr = std::to_string(instance);

    // we want the amap entry to use underscores when displaying the comp instead of slashes
    std::string underscoreURI = replaceSlashAndColonWithUnderscore(uri);
    std::string ctrlRack = "/control" + underscoreURI + "/rack_" + instanceStr;

    if (debug)
        FPS_PRINT_INFO("Setting up datamap to amap interface for {} using amap of asset manager: [{}]", ctrlRack,
                       am->name);

    // inputs amap vals
    std::string inputAmap = "voltage";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)inputAmap.c_str(), dVal);

    inputAmap = "soc";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)inputAmap.c_str(), dVal);

    inputAmap = "contactorStatus";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)inputAmap.c_str(), bVal);

    inputAmap = "ignoreExternal";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)inputAmap.c_str(), bVal);

    inputAmap = "enableFeedback";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)inputAmap.c_str(), bVal);

    // Output amap vals
    std::string outputAmap = "ignoreInternal";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)outputAmap.c_str(), bVal);

    outputAmap = "enableCmd";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRack.c_str(), (char*)outputAmap.c_str(), bVal);
}

int setupRack(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // this is the "setup" function that gets called by the thread but this is not where the racks actually get set up.
    // Each rack's setup function is called in setupBatteryBalancing so that the rack setup can have access to the BB
    // object. this "function" still needs to exists so that the rack functions still work with the datamaps system see
    // below for the real setupRack function
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(aV);

    return 0;
}

// given a templated uri string as an input, this function parses the templatedURI and swaps the bms and rack key's with
// their rep value
std::string expandRackTemplate(assetVar* parent_AV, std::string templatedURI, int bmsNum, int rackNum)
{
    bool debug = parent_AV->getbParam("debug");

    // get all the template fields as strings
    std::string rackKey = parent_AV->getcParam("RackTmpl");
    std::string bmsKey = parent_AV->getcParam("BmsTmpl");
    std::string rackReplace = parent_AV->getcParam("RackRep");
    std::string bmsReplace = parent_AV->getcParam("BmsRep");

    // put the actual numbers in rackReplace and bmsReplace
    try
    {
        bmsReplace = formatTemplateNumber(bmsReplace, bmsNum);
        rackReplace = formatTemplateNumber(rackReplace, rackNum);
    }
    catch (const std::invalid_argument& e)
    {
        std::string errorMsg = e.what();
        signalError(parent_AV, "setupRack_" + std::to_string(rackNum), errorMsg, true);
        // return an error string if we get an error
        return "error";
    }

    // replace the key in the uri string with the replace value we just calculated
    templatedURI = replaceKeyInURI(templatedURI, bmsKey, bmsReplace);
    templatedURI = replaceKeyInURI(templatedURI, rackKey, rackReplace);

    if (debug)
        FPS_PRINT_INFO("expanded uri to [{}]", templatedURI);

    return templatedURI;
}

// adds a remap to the inputURI gotten from the paramName
void addRemapToRackInputAV(varsmap& vmap, VarMapUtils* vm, assetVar* parent_AV, std::string paramName,
                           std::string remapURI, int batteryNum, int rackNum, std::string essSystemName, bool debug)
{
    if (parent_AV->gotParam((char*)paramName.c_str()))
    {
        // do rack remap setup
        std::string templatedURI = parent_AV->getcParam((char*)paramName.c_str());

        if (!templatedURI.empty() && isValidURI(templatedURI))
        {
            std::string expandedTemplateURI = expandRackTemplate(parent_AV, templatedURI, batteryNum, rackNum);
            if (expandedTemplateURI == "error")
                return;

            // use our new expandedTemplateURI to get the sourceAV for our remaps
            assetVar* sourceAV = vm->getVar(vmap, (char*)expandedTemplateURI.c_str(), nullptr);
            if (!sourceAV)
            {
                // if this aV doesnt exist already, make it
                double dval = 0;
                sourceAV = vm->makeVar(vmap, expandedTemplateURI.c_str(), nullptr, dval);
                FPS_PRINT_WARN("[\"{}\"] doesnt exist, creating new aV [{}:{}] and adding remap to it", paramName,
                               sourceAV->comp, sourceAV->name);
            }

            // create a remap in our sourceAV that will remap its value to our /control/ amap space when set to
            if (debug)
            {
                sourceAV->addRemap("/" + essSystemName + remapURI, 0, (void*)0, (void*)0, (void*)0, "set");
            }
            sourceAV->addRemap(remapURI, 0, (void*)0, (void*)0, true);
        }
        else
        {
            FPS_PRINT_WARN(
                "\"{}\" was not configured or is an invalid uri. No automatic remap set up to output value of [{}]",
                paramName, remapURI);
        }
    }
    else
    {
        FPS_PRINT_WARN("\"{}\" was not configured. No automatic remap set up to output value of [{}]", paramName,
                       remapURI);
    }
}

// add a remap to the /control/ internal assetVar remapping out to our uri gotten from param name
// returns false if an error occurs
bool addRemapToRackOutput(varsmap& vmap, VarMapUtils* vm, assetVar* parent_AV, std::string paramName,
                          std::string controlURI, int batteryNum, int rackNum, std::string essSystemName, bool debug)
{
    if (parent_AV->gotParam((char*)paramName.c_str()))
    {
        // get our /control/.../enableCmd aV
        assetVar* controlOutputAV = vm->getVar(vmap, (char*)controlURI.c_str());
        if (!controlOutputAV)
        {
            FPS_PRINT_ERROR("Could not find aV [{}]. Please retry amap setup", controlURI);
            std::string errorMsg = fmt::format(
                "Could not find aV [{}]. Please ensure this aV is set up properly in setupRackAmap", controlURI);

            signalError(parent_AV, "setupRack_" + std::to_string(rackNum), errorMsg, true);
            return false;
        }

        // get the URI we want to remap to
        std::string templatedURI = parent_AV->getcParam((char*)paramName.c_str());

        if (!templatedURI.empty() && isValidURI(templatedURI))
        {
            std::string expandedTemplateURI = expandRackTemplate(parent_AV, templatedURI, batteryNum, rackNum);

            // add our expanded uri as a remap action to the /control/.../enableCmd aV
            if (debug)
            {
                controlOutputAV->addRemap("/" + essSystemName + expandedTemplateURI, 0, (void*)0, (void*)0, (void*)0, "set");
            }
            controlOutputAV->addRemap(expandedTemplateURI, 0, (void*)0, (void*)0, true);
        }
        else
        {
            FPS_PRINT_WARN("\"{}\" is an invalid URI. No automatic remap set up to output value of [{}]", paramName,
                           controlURI);
        }
    }

    return true;
}

void setupRackFromBattery(BatteryBalancingUtility::BatteryBalancing* Battery, int batteryNum, int rackNum,
                          assetVar* parent_AV, assetVar* aV, varsmap& vmap)
{
    // create datamap and its asset manager
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = aV->getbParam("debug");

    std::string essSystemName = vm->getSysName(vmap);

    // the parameter rackNum is the index which the rack sits at in the racks vector, which starts at 0 and increments
    // from there the rack instance is the rackNum+1 because the function instances, the way the racks are used, start
    // at 1 and increment
    int instance = rackNum + 1;
    std::string parent_uri = parent_AV->comp + ":" + parent_AV->name;
    createNewRackInstance(Battery, parent_uri, instance);

    // setup the datamap for the aV we are going to run our function on
    setupRackDM(parent_AV, instance);

    // get or make the asset manager for our instance
    std::string tmp = replaceSlashAndColonWithUnderscore(parent_uri);
    tmp.erase(tmp.begin());
    std::string instanceAMname = tmp + "_Rack_" + std::to_string(instance) + "_asset_manager";
    asset_manager* datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

    // with this config, the racks exist in the their own space in the amap and they are not under the batteryBalancing
    // URI
    setupRackAmap(vm, vmap, datamapInstanceAM, instance, parent_uri);

    bool debugRemaps = true;
    if (parent_AV->gotParam("fims"))
    {
        debugRemaps = parent_AV->getbParam("fims");
    }

    // this is the comp (not including var name) of our /control/ amap space for this rack
    std::string controlURI = "/control" + replaceSlashAndColonWithUnderscore(parent_uri) + "/rack_" +
                             std::to_string(instance);

    // check if our parent AV has the correct params for template expansion
    if (parent_AV->gotParam("RackTmpl") && parent_AV->gotParam("BmsTmpl") && parent_AV->gotParam("RackRep") &&
        parent_AV->gotParam("BmsRep"))
    {
        // this is the uri that we will store the templated uri in that was gotten off the aV
        std::string templatedURI;
        std::string expandedTemplateURI;

        // set up remaps on rack input aV's if we have the right params
        addRemapToRackInputAV(vmap, vm, parent_AV, "RackSoCURI", controlURI + ":soc", batteryNum, instance, essSystemName, debugRemaps);

        addRemapToRackInputAV(vmap, vm, parent_AV, "RackEnableFeedbackURI", controlURI + ":enableFeedback", batteryNum,
                              instance, essSystemName, debugRemaps);

        addRemapToRackInputAV(vmap, vm, parent_AV, "RackFaultURI", controlURI + ":ignoreExternal", batteryNum,
                              instance, essSystemName, debugRemaps);

        addRemapToRackInputAV(vmap, vm, parent_AV, "RackContactorStatusURI", controlURI + ":contactorStatus", batteryNum,
                              instance, essSystemName, debugRemaps);

        // rack voltage cannot be remapped using the same function bc it has special logic due to filtering
        if (parent_AV->gotParam("RackVoltURI"))
        {
            // do rack volt remap setup
            templatedURI = parent_AV->getcParam("RackVoltURI");

            if (!templatedURI.empty() && isValidURI(templatedURI))
            {
                // expand the param string we just got
                expandedTemplateURI = expandRackTemplate(parent_AV, templatedURI, batteryNum, instance);

                // now that we have our expanded template URI, determine if our input needs to be filtered
                if (parent_AV->gotParam("FilterRackVoltage"))
                {
                    if (parent_AV->getbParam("FilterRackVoltage"))
                    {
                        // if we are here, we need to filter our input rack voltage
                        if (!parent_AV->gotParam("VoltageFilterFC"))
                        {
                            parent_AV->setParam("VoltageFilterFC", Battery->Configs.VoltageFilterFC);
                        }
                        else
                        {
                            // if we have the VoltageFilterFC param, set it to our configs struct
                            Battery->Configs.VoltageFilterFC = parent_AV->getdParam("VoltageFilterFC");
                        }

                        // pass in parent av, av, instance (same as rack instance), repTime from our Battery object's
                        // Configs struct,  and cutoff freq for thisinput
                        expandedTemplateURI = setupLowPassFilterfromBRB(
                            vmap, parent_AV, aV, instance, Battery->Configs.repTime, Battery->Configs.VoltageFilterFC,
                            expandedTemplateURI);
                    }
                }

                if (expandedTemplateURI == "error")
                {
                    // if this string is empty, we got an error during template expansion and have already and signaled
                    // an error to the thread
                    return;
                }

                // use our new expandedTemplateURI to get the sourceAV for our remaps
                assetVar* sourceAV = vm->getVar(vmap, (char*)expandedTemplateURI.c_str(), nullptr);
                if (!sourceAV)
                {
                    // if this aV doesnt exist already, make it
                    double dval = 0;
                    sourceAV = vm->makeVar(vmap, expandedTemplateURI.c_str(), nullptr, dval);
                    FPS_PRINT_WARN("RackVoltURI doesnt exist, creating new aV [{}:{}] and adding remap to it",
                                   sourceAV->comp, sourceAV->name);
                }

                // create a remap in our inputAV that will remap its value to our /control/ amap space when set to
                if (debugRemaps)
                {
                    sourceAV->addRemap("/" + essSystemName + controlURI + ":voltage", 0, (void*)0, (void*)0, (void*)0, "set");
                }
                sourceAV->addRemap(controlURI + ":voltage", 0, (void*)0, (void*)0, true);
            }
            else
            {
                FPS_PRINT_WARN(
                    "\"RackVoltURI\" is an invalid URI. No automatic remap set up to output value of [{}:voltage]",
                    controlURI);
            }
        }
        else
        {
            // if RackVoltURI doesnt exist there is no rack filtering either
            FPS_PRINT_WARN(
                "\"RackVoltURI\" is not configured. No automatic remap set up to output value of [{}:voltage]",
                controlURI);
        }

        // do the same for remapping our racks output to the output uri's
        if (!addRemapToRackOutput(vmap, vm, parent_AV, "RackEnableCmdURI", controlURI + ":enableCmd", batteryNum,
                                  instance, essSystemName, debugRemaps))
            return;

        if (!addRemapToRackOutput(vmap, vm, parent_AV, "RackBalanceFailureURI", controlURI + ":ignoreInternal",
                                  batteryNum, instance, essSystemName, debugRemaps))
            return;
    }
    else if (parent_AV->gotParam("RackTmpl") || parent_AV->gotParam("BmsTmpl") || parent_AV->gotParam("RackRep") ||
             parent_AV->gotParam("BmsRep"))
    {
        // determine which fields we are missing
        bool rackTmpl = parent_AV->gotParam("RackTmpl");
        bool bmsTmpl = parent_AV->gotParam("BmsTmpl");
        bool rackRep = parent_AV->gotParam("RackRep");
        bool bmsRep = parent_AV->gotParam("BmsRep");

        // create a string that includes all our missing params
        std::string missing;
        if (!bmsTmpl)
        {
            missing += missing.empty() ? "\"BmsTmpl\"" : ", \"BmsTmpl\"";
        }

        if (!rackTmpl)
        {
            missing += missing.empty() ? "\"RackTmpl\"" : ", \"RackTmpl\"";
        }

        if (!bmsRep)
        {
            missing += missing.empty() ? "\"BmsRep\"" : ", \"BmsRep\"";
        }

        if (!rackRep)
        {
            missing += missing.empty() ? "\"RackRep\"" : ", \"RackRep\"";
        }

        // dont set up any remaps and print a warning, but otherwise continue
        FPS_PRINT_WARN(
            "aV [{}:{}] is missing parameters [{}] and cannot perform template expansion or set up remaps. Automatic template expansion and remapping requires parameters [\"BmsTmpl\", \"RackTmpl\", \"BmsRep\", and \"RackRep\"]",
            parent_AV->comp, parent_AV->name, missing);
    }

    // tell our parent AV that we are done by setting the setup flag for this function instance high
    std::string thisFunction = "Rack_" + std::to_string(instance);

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

                aV->setParam("Rack_instance", instance);
                return;
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

            signalError(parent_AV, thisFunction, errorMsg, true);
            return;
        }
    }
}
