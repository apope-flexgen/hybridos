#include "BatteryBalancingUtility.hpp"
#include "dataMap.h"

extern "C++" {
void setupRackFromBattery(BatteryBalancingUtility::BatteryBalancing* Battery, int batteryNum, int rackNum,
                          assetVar* parent_AV, assetVar* aV, varsmap& vmap);
}

using namespace BatteryBalancingUtility;

std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<BatteryBalancing>>> BatteryBalancingObjects;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<BatteryBalancing::ExtU>>>
    BatteryBalancingInputs;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<BatteryBalancing::ExtY>>>
    BatteryBalancingOutputs;

uint8_t* getBatteryBalancingInputs(std::string uri, int instance)
{
    BatteryBalancing::ExtU* dmBBInputs = BatteryBalancingInputs[uri][instance].get();

    return reinterpret_cast<uint8_t*>(dmBBInputs);
}

uint8_t* getBatteryBalancingOutputs(std::string uri, int instance)
{
    BatteryBalancing::ExtY* dmBBOutputs = BatteryBalancingOutputs[uri][instance].get();

    return reinterpret_cast<uint8_t*>(dmBBOutputs);
}

void BatteryBalancingRun(std::string uri, int instance)
{
    BatteryBalancing* dmBBObject = BatteryBalancingObjects[uri][instance].get();

    dmBBObject->step();

    if (dmBBObject->InputRef->debug)
    {
        FPS_PRINT_INFO("Ran step() for BatteryBalancing model on aV [{}]", uri);

        // get the bms number
        int bms_i = uri[uri.size() - 1] - '0';

        FPS_PRINT_INFO("Inputs: StartCmd: {} | Stop: {} | OpenContRes: {} | CloseContRes: {} | debug: {} | reset: {}",
                       dmBBObject->InputRef->StartCmd, dmBBObject->InputRef->StopCmd,
                       dmBBObject->InputRef->OpenContactorResult, dmBBObject->InputRef->CloseContactorResult,
                       dmBBObject->InputRef->debug, dmBBObject->InputRef->reset);
        FPS_PRINT_INFO("Outputs: State: {} | Pcmd: {} | OpenReq: {} | CloseReq: {}",
                       dmBBObject->OutputRef->StateVariable, dmBBObject->OutputRef->PCmd,
                       dmBBObject->OutputRef->OpenContactorReq, dmBBObject->OutputRef->CloseContactorReq);
        FPS_PRINT_INFO("    errStr: [{}]", dmBBObject->OutputRef->errStr);

        for (int i = 0; i < (int)dmBBObject->racks.size(); i++)
        {
            FPS_PRINT_INFO(
                "bms_{}_rack_{}: voltage: {} | soc: {} | contactorStatus: {} | ignoreExternal: {} | ignoreInternal: {} | enableFeedback: {} | enableCmd: {}",
                bms_i, i + 1, dmBBObject->racks.at(i)->RackInputRef->voltage,
                dmBBObject->racks.at(i)->RackInputRef->soc, dmBBObject->racks.at(i)->RackInputRef->contactorStatus,
                dmBBObject->racks.at(i)->RackInputRef->ignoreExternal,
                dmBBObject->racks.at(i)->RackOutputRef->ignoreInternal,
                dmBBObject->racks.at(i)->RackInputRef->enableFeedback,
                dmBBObject->racks.at(i)->RackOutputRef->enableCmd);
        }

        std::cout << std::endl;
    }
}

void createNewBatteryBalancingInstance(std::string uri, int instance)
{
    // create a new instance of the BatteryBalancing inputs struct, output struct, and object
    BatteryBalancing::ExtU* dmBBInput = new BatteryBalancing::ExtU;
    BatteryBalancing::ExtY* dmBBOutput = new BatteryBalancing::ExtY;
    BatteryBalancing* dmBatteryBalancingObject = new BatteryBalancing(dmBBInput, dmBBOutput);

    // transfer memory management of "new" calls to a unique pointer
    std::unique_ptr<BatteryBalancing::ExtU> dmBBInputPtr(dmBBInput);
    std::unique_ptr<BatteryBalancing::ExtY> dmBBOutputPtr(dmBBOutput);
    std::unique_ptr<BatteryBalancing> dmBatteryBalancingObjectPtr(dmBatteryBalancingObject);

    // move the ownership of the unique pointer to a global map of instances to unique pointers. need a different map
    // for each type of unique pointer
    BatteryBalancingInputs[uri][instance] = std::move(dmBBInputPtr);
    BatteryBalancingOutputs[uri][instance] = std::move(dmBBOutputPtr);
    BatteryBalancingObjects[uri][instance] = std::move(dmBatteryBalancingObjectPtr);

    // use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in
    // our global external map
    uint8_t* (*getInputsPtr)(std::string, int) = &getBatteryBalancingInputs;
    modelFcnRef["BatteryBalancingInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

    uint8_t* (*getOutputsPtr)(std::string, int) = &getBatteryBalancingOutputs;
    modelFcnRef["BatteryBalancingOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

    // set refernce to BatteryBalancing's run function using a global external
    void (*runFuncPtr)(std::string, int) = &BatteryBalancingRun;
    modelFcnRef["BatteryBalancing"] = reinterpret_cast<void(*)>(runFuncPtr);
}

void setupBatteryBalancingDM(assetVar* aV, int instance)
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

    std::string inputBlock = "BatteryBalancing_" + instanceStr + "Inputs";
    std::string outputBlock = "BatteryBalancing_" + instanceStr + "Outputs";

    // Input data items and transfer blocks
    std::string inputName = "StartCmd";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, StartCmd), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "StopCmd";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, StopCmd), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "FineBalanceEnabled";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, FineBalanceEnabled),
                    DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "debug";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, debug), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "MaxBalancePower";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, MaxBalancePower), DataMapType::DOUBLE_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "PActl";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, PActl), DataMapType::DOUBLE_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "OpenContactorResult";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, OpenContactorResult),
                    DataMapType::INT32_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "CloseContactorResult";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, CloseContactorResult),
                    DataMapType::INT32_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "reset";
    dm->addDataItem((char*)inputName.c_str(), offsetof(BatteryBalancing::ExtU, reset), DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    // Output data items and transfer blocks
    std::string outputName = "StateVariable";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, StateVariable), DataMapType::INT32_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "PCmd";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, PCmd), DataMapType::DOUBLE_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "OpenContactorReq";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, OpenContactorReq),
                    DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "CloseContactorReq";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, CloseContactorReq),
                    DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "PcsStartReq";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, PcsStartReq), DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "PcsStopReq";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, PcsStopReq), DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "errStr";
    dm->addDataItem((char*)outputName.c_str(), offsetof(BatteryBalancing::ExtY, errStr), DataMapType::STRING);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());
}

void setupBatteryBalancingAmap(VarMapUtils* vm, varsmap& vmap, asset_manager* am, std::string uri)
{
    int debug = 0;
    double dVal = 0.0;
    bool bVal = false;
    int iVal = 0;
    int one = 1;
    char* sVal = (char*)"";

    // we want the amap entry to use underscores when displaying the comp instead of slashes
    std::string underscoreURI = replaceSlashAndColonWithUnderscore(uri);
    std::string ctrlBB = "/control" + underscoreURI + "/BatteryBalancing";

    if (debug)
        FPS_PRINT_INFO("Setting up datamap to amap interface for {} using amap of asset manager: [{}]", ctrlBB,
                       am->name);

    // inputs amap vals
    std::string inputAmap = "StartCmd";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), bVal);

    inputAmap = "StopCmd";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), bVal);

    inputAmap = "FineBalanceEnabled";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), bVal);

    inputAmap = "debug";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), bVal);

    inputAmap = "MaxBalancePower";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), dVal);

    inputAmap = "PActl";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), dVal);

    inputAmap = "OpenContactorResult";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), one);

    inputAmap = "CloseContactorResult";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), one);

    inputAmap = "reset";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)inputAmap.c_str(), bVal);

    // Output amap vals
    std::string outputAmap = "StateVariable";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), iVal);

    outputAmap = "PCmd";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), dVal);

    outputAmap = "OpenContactorReq";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), bVal);

    outputAmap = "CloseContactorReq";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), bVal);

    outputAmap = "PcsStartReq";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), bVal);

    outputAmap = "PcsStopReq";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), bVal);

    outputAmap = "errStr";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlBB.c_str(), (char*)outputAmap.c_str(), sVal);
}

// this function adds the remapURI to the assetVar gotten from inURI. creates a new one if necessary
void addRemapToInputURI(varsmap& vmap, VarMapUtils* vm, std::string inURI, std::string remapURI, std::string paramName,
                        std::any typeVal, std::string essSystemName, bool debug)
{
    if (!inURI.empty() && isValidURI(inURI))
    {
        assetVar* sourceAV = vm->getVar(vmap, (char*)inURI.c_str());
        if (!sourceAV)
        {
            // if this aV doesnt exist already, make it with the given input type
            if (typeVal.type() == typeid(bool))
            {
                bool bval = false;
                sourceAV = vm->makeVar(vmap, inURI.c_str(), nullptr, bval);
            }
            else if (typeVal.type() == typeid(int))
            {
                int val = 0;
                sourceAV = vm->makeVar(vmap, inURI.c_str(), nullptr, val);
            }
            else if (typeVal.type() == typeid(double))
            {
                double val = 0.0;
                sourceAV = vm->makeVar(vmap, inURI.c_str(), nullptr, val);
            }
            else if (typeVal.type() == typeid(std::string) || typeVal.type() == typeid(char*) ||
                     typeVal.type() == typeid(const char*))
            {
                char* val = (char*)"";
                sourceAV = vm->makeVar(vmap, inURI.c_str(), nullptr, val);
            }
            else
            {
                // if the type is unknown, default to using an int
                int val = 0;
                sourceAV = vm->makeVar(vmap, inURI.c_str(), nullptr, val);
                FPS_PRINT_WARN("{} was called to make [{}] av with unknown type. Defaulting to int type", __func__,
                               inURI);
            }

            FPS_PRINT_WARN("[\"{}\"] doesnt exist, creating new aV [{}:{}] and adding remap to it", paramName,
                           sourceAV->comp, sourceAV->name);
        }

        if (debug)
        {
            sourceAV->addRemap("/" + essSystemName + remapURI, 0, (void*)0, (void*)0, true, "set");
        }
        sourceAV->addRemap(remapURI, 0, (void*)0, (void*)0, true);
    }
    else
    {
        FPS_PRINT_WARN("\"{}\" was not configured or is an invalid uri. No automatic input remap set up for [{}]",
                       paramName, remapURI);
    }
}

// this function adds the remapURI to the /control/ datamap amap space assetVar that is passed in
// returns false if an error occurs
bool addRemapToControlOutputAV(varsmap& vmap, VarMapUtils* vm, std::string comp, std::string var, std::string remapURI,
                               std::string paramName, assetVar* parent_AV, int bmsNum, std::string essSystemName, bool debug)
{
    if (!remapURI.empty() && isValidURI(remapURI))
    {
        assetVar* controlOutputAV = vm->getVar(vmap, (char*)comp.c_str(), (char*)var.c_str());
        if (!controlOutputAV)
        {
            FPS_PRINT_ERROR("Could not find aV [{}:{}]. Please retry amap setup", comp, var);
            std::string errorMsg = fmt::format(
                "Could not find aV [{}:{}}]. Please ensure this aV is set up properly in setupRackAmap", comp, var);

            signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(bmsNum), errorMsg, true);
            return false;
        }

        // all BRB outputs should have the ifChanged flag true
        FPS_PRINT_INFO("Remapping [{}:{}] to [{}]", comp, var, remapURI);

        if (debug)
        {
            controlOutputAV->addRemap("/" + essSystemName + remapURI, 0, (void*)0, (void*)0, true, "set");
        }
        controlOutputAV->addRemap(remapURI, 0, (void*)0, (void*)0, true);
    }
    else
    {
        FPS_PRINT_WARN(
            "\"{}\" was not configured or is an invalid uri. No automatic remap set up to output value of [{}:{}]",
            paramName, comp, var);
    }

    return true;
}

int setupBatteryBalancing(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);

    // create datamap and its asset manager
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = aV->getbParam("debug");

    std::string essSystemName = vm->getSysName(vmap);

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
    if (!aV->gotParam("BatteryBalancing_instance"))
    {
        aV->setParam("BatteryBalancing_instance", 0);
    }
    int instance = aV->getiParam("BatteryBalancing_instance") + 1;

    if (debug)
        FPS_PRINT_INFO("Setting up BatteryBalancing on [{}]", parent_uri);

    // once we have our parent AV, we need to determine if we are being set up the first time or not
    if (parent_AV->gotParam("alreadyConfigured"))
    {
        if (parent_AV->getbParam("alreadyConfigured"))
        {
            // if we get here, this is the second (or more) time we have run this setup function
            // we need to delete our existing objct and recreate one
            if (debug)
                FPS_PRINT_INFO("[{}] was alreadyConfigured, creating new BatteryBalancing object and reinitializing",
                               __func__);

            // smart pointers handle the clean up of all the objects that get replaced in global maps and vectors, dont
            // need to manually delete anything

            // reset all the "setupX" flags
            int numRacks = parent_AV->getiParam("func1_instances");
            for (int i = 1; i <= numRacks + 1; i++)  // reset numRacks+1 to include the BatteryBalancing setup function
            {
                // we only want to set low the flags for the number of racks we are reconfigured to run with
                std::string numStr = std::to_string(i);
                std::string setupNum = "setup" + numStr;
                if (!parent_AV->gotParam((char*)setupNum.c_str()))
                {
                    // we have cleared all setup flags and now we have found a setup# that doesnt exist, break out of
                    // loop
                    if (debug)
                        FPS_PRINT_INFO("Could not find [{}] param. Exiting setup loop", setupNum);
                    break;
                }

                parent_AV->setParam((char*)setupNum.c_str(), false);
                if (debug)
                    FPS_PRINT_INFO("just reset [{}] to false", setupNum);
            }
        }
    }

    // create new instance and set references to it for the rest of the system
    createNewBatteryBalancingInstance(parent_uri, instance);

    // setup the datamap for the aV we are going to run our function on
    setupBatteryBalancingDM(parent_AV, instance);

    // get or make the asset manager for our instance
    std::string tmp = replaceSlashAndColonWithUnderscore(parent_uri);
    tmp.erase(tmp.begin());
    std::string instanceAMname = tmp + "_BatteryBalancing_" + std::to_string(instance) + "_asset_manager";
    asset_manager* datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

    // setup amap for this instance
    setupBatteryBalancingAmap(vm, vmap, datamapInstanceAM, parent_uri);

    // we have set up the BatteryBalancing object and now we need to set up all of its racks and parameters
    std::unique_ptr<BatteryBalancing>& uqObjPtr = BatteryBalancingObjects[parent_uri][instance];
    BatteryBalancing* BB = uqObjPtr.get();

    // PARAMS
    // set the repTime of this aV to our Battery
    if (!parent_AV->gotParam("repTime"))
    {
        FPS_PRINT_ERROR("AV [{}] does not have a repTime, please configure this av to run repeatedly", parent_AV->name);
        std::string errorMsg = fmt::format("Parent AV [{}] does not have \"repTime\" param", parent_AV->name);

        signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(instance), errorMsg, true);
        return 0;
    }
    double repTime = parent_AV->getdParam("repTime");

    // set our delta voltages
    if (!parent_AV->gotParam("RampStartDeltaVoltage"))
    {
        parent_AV->setParam("RampStartDeltaVoltage", BB->Configs.RampStartDeltaVoltage);
    }
    double rsdv = parent_AV->getdParam("RampStartDeltaVoltage");

    if (!parent_AV->gotParam("RampEndDeltaVoltage"))
    {
        parent_AV->setParam("RampEndDeltaVoltage", BB->Configs.RampEndDeltaVoltage);
    }
    double redv = parent_AV->getdParam("RampEndDeltaVoltage");

    if (!parent_AV->gotParam("RackCloseDeltaVoltage"))
    {
        parent_AV->setParam("RackCloseDeltaVoltage", BB->Configs.RackCloseDeltaVoltage);
    }
    double rcdv = parent_AV->getdParam("RackCloseDeltaVoltage");

    // battery relax time
    if (!parent_AV->gotParam("BatteryRelaxTime"))
    {
        parent_AV->setParam("BatteryRelaxTime", BB->Configs.BatteryRelaxTime);
    }
    double bct = parent_AV->getdParam("BatteryRelaxTime");

    // minimum feedback delay time
    if (!parent_AV->gotParam("MinimumFeedbackDelayTime"))
    {
        parent_AV->setParam("MinimumFeedbackDelayTime", BB->Configs.MinimumFeedbackDelayTime);
    }
    double mfdt = parent_AV->getdParam("MinimumFeedbackDelayTime");

    // active power update time minimum
    if (!parent_AV->gotParam("ActivePowerUpdateTimeMinimum"))
    {
        parent_AV->setParam("ActivePowerUpdateTimeMinimum", BB->Configs.ActivePowerUpdateTimeMinimum);
    }
    double aputm = parent_AV->getdParam("ActivePowerUpdateTimeMinimum");

    // action delay timeout
    if (!parent_AV->gotParam("ActionDelayTimeout"))
    {
        parent_AV->setParam("ActionDelayTimeout", BB->Configs.ActionDelayTimeout);
    }
    double adto = parent_AV->getdParam("ActionDelayTimeout");

    // maximum open or close contactor attempts
    if (!parent_AV->gotParam("MaxOpenContactorAttempts"))
    {
        parent_AV->setParam("MaxOpenContactorAttempts", BB->Configs.MaxOpenContactorAttempts);
    }
    double moca = parent_AV->getdParam("MaxOpenContactorAttempts");

    if (!parent_AV->gotParam("MaxCloseContactorAttempts"))
    {
        parent_AV->setParam("MaxCloseContactorAttempts", BB->Configs.MaxCloseContactorAttempts);
    }
    double mcca = parent_AV->getdParam("MaxCloseContactorAttempts");

    // rack level contactor control flag
    if (!parent_AV->gotParam("RackLevelContactorControl"))
    {
        parent_AV->setParam("RackLevelContactorControl", BB->Configs.RackLevelContactorControl);
    }
    bool rlcc = parent_AV->getbParam("RackLevelContactorControl");

    // Pcmd ramp rate
    if (!parent_AV->gotParam("ActivePowerRampRatekWps"))
    {
        parent_AV->setParam("ActivePowerRampRatekWps", BB->Configs.ActivePowerRampRatekWps);
    }
    double aprr = parent_AV->getdParam("ActivePowerRampRatekWps");

    // minimum power balancing thresholds
    if (!parent_AV->gotParam("MinRackBalancePower"))
    {
        parent_AV->setParam("MinRackBalancePower", BB->Configs.MinRackBalancePower);
    }
    double mrbp = parent_AV->getdParam("MinRackBalancePower");


    // set all the values to our BB class
    BB->Configs.repTime = repTime;
    BB->Configs.RampStartDeltaVoltage = rsdv;
    BB->Configs.RampEndDeltaVoltage = redv;
    BB->Configs.RackCloseDeltaVoltage = rcdv;
    BB->Configs.BatteryRelaxTime = bct;
    BB->Configs.MinimumFeedbackDelayTime = mfdt;
    BB->Configs.ActivePowerUpdateTimeMinimum = aputm;
    BB->Configs.ActionDelayTimeout = adto;
    BB->Configs.MaxOpenContactorAttempts = moca;
    BB->Configs.MaxCloseContactorAttempts = mcca;
    BB->Configs.RackLevelContactorControl = rlcc;
    BB->Configs.ActivePowerRampRatekWps = aprr;
    BB->Configs.MinRackBalancePower = mrbp;

    // RACKS
    // for the BatteryBalancing function, "func1_instances" will always be the number of racks
    if (!parent_AV->gotParam("func1_instances"))
    {
        // throw error if we dont have func1_instances
        FPS_PRINT_ERROR(
            "Parent AV [{}] does not have a \"bms_#_racks\" param on config wrapper av. We do not know how many racks to set up.",
            parent_uri);
        std::string errorMsg = fmt::format(
            "Parent AV [{}] does not have \"bms_#_racks\" param and we do not know how many racks to set up",
            parent_uri);

        signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(instance), errorMsg, true);
        return 0;
    }

    int num_racks = parent_AV->getiParam("func1_instances");
    if (debug)
        FPS_PRINT_INFO("func1_instances is [{}] so we have [{}] racks", num_racks, num_racks);

    // make sure we have a valid number of racks
    if (num_racks < 1)
    {
        // we need at least 1 rack to run this function
        FPS_PRINT_ERROR(
            "Parent AV [{}] is configured with [{}] racks. Please configure this assetVar with at least 1 rack",
            parent_uri, num_racks);

        std::string errorMsg = fmt::format(
            "Parent AV [{}] is configured with [{}] racks. Please configure this assetVar with at least 1 rack",
            parent_uri, num_racks);

        signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(instance), errorMsg, true);
        return 0;
    }

    // we need to get the "battery instance" from the parent URI since each aV only ever has 1 BB class
    if (!parent_AV->gotParam("BmsNumber"))
    {
        // if we dont have this param, default to 1
        parent_AV->setParam("BmsNumber", 1);
    }
    int bmsNum = parent_AV->getiParam("BmsNumber");

    // set up each rack
    for (int i = 0; i < num_racks; i++)
    {
        // setupRack is run here because the Rack needs access to the Battery object so that it can add itself to its
        // racks vector
        setupRackFromBattery(BB, bmsNum, i, parent_AV, aV, vmap);
    }

    // pass our debug flag to the BB class by setting the /control/.../BatteryBalancing/debug aV to true
    std::string ctrlBB = "/control" + replaceSlashAndColonWithUnderscore(parent_uri) + "/BatteryBalancing";
    assetVar* controlDebugAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "debug");
    if (!controlDebugAV)
    {
        // if we cant find this aV, something went wrong in set up and we likely need to restart
        FPS_PRINT_ERROR("Could not find aV [{}:debug]. Please retry amap setup", ctrlBB);
        std::string errorMsg = fmt::format(
            "Could not find aV [{}:debug]. Please ensure this aV is set up properly in setupBatteryBalancingAmap",
            ctrlBB);

        signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(instance), errorMsg, true);
        return 0;
    }
    controlDebugAV->setVal(debug);

    bool debugRemaps = true;
    if (parent_AV->gotParam("fims"))
    {
        debugRemaps = parent_AV->getbParam("fims");
    }

    // set up remaps to BB inputs on inputAV's
    // get the inputURI's from our parent_AV
    std::string startURI;
    if (parent_AV->gotParam("StartCmdURI"))
    {
        startURI = parent_AV->getcParam("StartCmdURI");
    }

    std::string bmsMaintModeURI;
    if (parent_AV->gotParam("BmsMaintModeURI"))
    {
        bmsMaintModeURI = parent_AV->getcParam("BmsMaintModeURI");
    }

    std::string pcsMaintModeURI;
    if (parent_AV->gotParam("PcsMaintModeURI"))
    {
        pcsMaintModeURI = parent_AV->getcParam("PcsMaintModeURI");
    }

    std::string clearFaultsURI;
    if (parent_AV->gotParam("ClearFaultsDoneURI"))
    {
        clearFaultsURI = parent_AV->getcParam("ClearFaultsDoneURI");
    }

    std::string stopURI;
    if (parent_AV->gotParam("StopCmdURI"))
    {
        stopURI = parent_AV->getcParam("StopCmdURI");
    }

    std::string fineBalURI;
    if (parent_AV->gotParam("FineBalancingEnableURI"))
    {
        fineBalURI = parent_AV->getcParam("FineBalancingEnableURI");
    }

    std::string maxBalPwrURI;
    if (parent_AV->gotParam("MaxBalancePowerURI"))
    {
        maxBalPwrURI = parent_AV->getcParam("MaxBalancePowerURI");
    }

    std::string pActlURI;
    if (parent_AV->gotParam("PActlURI"))
    {
        pActlURI = parent_AV->getcParam("PActlURI");
    }

    // get our outputs from our parent uris
    std::string pCmdOutput;
    if (parent_AV->gotParam("PCmdURI"))
    {
        pCmdOutput = parent_AV->getcParam("PCmdURI");
    }

    std::string stateOutput;
    if (parent_AV->gotParam("StateVariableReportURI"))
    {
        stateOutput = parent_AV->getcParam("StateVariableReportURI");
    }

    std::string errorOutput;
    if (parent_AV->gotParam("ErrorReportURI"))
    {
        errorOutput = parent_AV->getcParam("ErrorReportURI");
    }

    std::string openReqOutput;
    if (parent_AV->gotParam("OpenContactorURI"))
    {
        openReqOutput = parent_AV->getcParam("OpenContactorURI");
    }

    std::string closeReqOutput;
    if (parent_AV->gotParam("CloseContactorURI"))
    {
        closeReqOutput = parent_AV->getcParam("CloseContactorURI");
    }

    std::string pcsStartOutput;
    if (parent_AV->gotParam("PcsStartURI"))
    {
        pcsStartOutput = parent_AV->getcParam("PcsStartURI");
    }

    std::string pcsStopOutput;
    if (parent_AV->gotParam("PcsStopURI"))
    {
        pcsStopOutput = parent_AV->getcParam("PcsStopURI");
    }

    std::string siteStatusURI;
    if (parent_AV->gotParam("SiteStatusURI"))
    {
        siteStatusURI = parent_AV->getcParam("SiteStatusURI");
    }

    // perform Bms template expansion if necessary
    if (parent_AV->gotParam("BmsTmpl") != parent_AV->gotParam("BmsRep"))
    {
        // if we have only 1 of BmsTmpl and BmsRep, print a warning and attempt to use the input URI as is
        bool bmsTmpl = parent_AV->gotParam("BmsTmpl");

        // determine which uri we are missing
        std::string missing;
        if (!bmsTmpl)  // if this is true, we know the other one is false
        {
            missing = "\"BmsTmpl\"";
        }
        else
        {
            missing = "\"BmsRep\"";
        }

        // dont set up any remaps and print a warning, but otherwise continue
        FPS_PRINT_WARN(
            "aV [{}:{}] is missing parameters [{}] and cannot perform BMS template expansion. Template expansion requires parameters [\"BmsTmpl\" and \"BmsRep\"]",
            parent_AV->comp, parent_AV->name, missing);
    }
    else if (parent_AV->gotParam("BmsTmpl") && parent_AV->gotParam("BmsRep"))
    {
        // get our BmsRep and BmsTmpl values
        std::string bmsKey = parent_AV->getcParam("BmsTmpl");
        std::string bmsReplace = parent_AV->getcParam("BmsRep");

        try
        {
            bmsReplace = formatTemplateNumber(bmsReplace, bmsNum);
        }
        catch (const std::invalid_argument& e)
        {
            FPS_PRINT_ERROR("Please ensure that params \"BmsTmpl\" and \"BmsRep\" are formatted properly", nullptr);
            std::string errorMsg = e.what();
            signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(bmsNum), errorMsg, true);
            return 0;
        }

        // do template expansion for each uri we have
        if (parent_AV->gotParam("StartCmdURI"))
        {
            startURI = replaceKeyInURI(startURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("BmsMaintModeURI"))
        {
            bmsMaintModeURI = replaceKeyInURI(bmsMaintModeURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("ClearFaultsDoneURI"))
        {
            clearFaultsURI = replaceKeyInURI(clearFaultsURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("StopCmdURI"))
        {
            stopURI = replaceKeyInURI(stopURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("FineBalancingEnableURI"))
        {
            fineBalURI = replaceKeyInURI(fineBalURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("MaxBalancePowerURI"))
        {
            maxBalPwrURI = replaceKeyInURI(maxBalPwrURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("PActlURI"))
        {
            pActlURI = replaceKeyInURI(pActlURI, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("PCmdURI"))
        {
            pCmdOutput = replaceKeyInURI(pCmdOutput, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("StateVariableReportURI"))
        {
            stateOutput = replaceKeyInURI(stateOutput, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("ErrorReportURI"))
        {
            errorOutput = replaceKeyInURI(errorOutput, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("OpenContactorURI"))
        {
            openReqOutput = replaceKeyInURI(openReqOutput, bmsKey, bmsReplace);
        }

        if (parent_AV->gotParam("CloseContactorURI"))
        {
            closeReqOutput = replaceKeyInURI(closeReqOutput, bmsKey, bmsReplace);
        }
    }

    // once we get here, our uri strings are the aV's we want to add remaps to (if our URI string is empty, that means
    // we dont have its respective param and we should skip auto-remapping)
    addRemapToInputURI(vmap, vm, stopURI, ctrlBB + ":StopCmd", "StopCmdURI", false, essSystemName, debugRemaps);

    addRemapToInputURI(vmap, vm, fineBalURI, ctrlBB + ":FineBalanceEnabled", "FineBalancingEnableURI", false, essSystemName, debugRemaps);

    addRemapToInputURI(vmap, vm, maxBalPwrURI, ctrlBB + ":MaxBalancePower", "MaxBalancePowerURI", 0.0, essSystemName, debugRemaps);

    addRemapToInputURI(vmap, vm, pActlURI, ctrlBB + ":PActl", "PActlURI", 0.0, essSystemName, debugRemaps);

    // the start command cannot use the addRemapToInputURI function since special actions are needed for bms and pcs
    // maint mode uris
    if (!startURI.empty() && isValidURI(startURI))
    {
        assetVar* sourceAV = vm->getVar(vmap, (char*)startURI.c_str());
        if (!sourceAV)
        {
            // if this aV doesnt exist already, make it
            bool bval = false;
            sourceAV = vm->makeVar(vmap, startURI.c_str(), nullptr, bval);
            FPS_PRINT_WARN("[\"StartCmdURI\"] doesnt exist, creating new aV [{}:{}] and adding remap to it",
                           sourceAV->comp, sourceAV->name);
        }

        if (debugRemaps)
        {
            sourceAV->addRemap("/" + essSystemName + ctrlBB + ":StartCmd", 0, (void*)0, (void*)0, true, "set");
        }
        sourceAV->addRemap(ctrlBB + ":StartCmd", 0, (void*)0, (void*)0, true);

        // we also want to set up a remap so that when our startCmd sourceAV is true, it sets bms and pcs MaintModes
        // true
        if (!bmsMaintModeURI.empty() && isValidURI(bmsMaintModeURI))
        {
            if (debugRemaps)
            {
                sourceAV->addRemap("/" + essSystemName + bmsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            sourceAV->addRemap(bmsMaintModeURI, 0, true, true, true);
        }

        if (!pcsMaintModeURI.empty() && isValidURI(pcsMaintModeURI))
        {
            if (debugRemaps)
            {
                sourceAV->addRemap("/" + essSystemName + pcsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            sourceAV->addRemap(pcsMaintModeURI, 0, true, true, true);
        }
    }
    else
    {
        FPS_PRINT_WARN(
            "\"StartCmdURI\" was not configured or is an invalid uri. No automatic input remap set up for [{}:StartCmd]",
            ctrlBB);

        // if our StartCmdURI param was empty or invalid, check if we have maintModeURI's
        if (!bmsMaintModeURI.empty() && isValidURI(bmsMaintModeURI))
        {
            // if we have a valid bmsMaintModeURI, add it as a remap to our /control/.../BatteryBalancing/StartCmd aV
            assetVar* controlAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "StartCmd");

            if (debugRemaps)
            {
                controlAV->addRemap("/" + essSystemName + bmsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            controlAV->addRemap(bmsMaintModeURI, 0, true, true, true);
        }

        if (!pcsMaintModeURI.empty() && isValidURI(pcsMaintModeURI))
        {
            // if we have a valid pcsMaintModeURI, add it as a remap to our /control/.../BatteryBalancing/StartCmd aV
            assetVar* controlAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "StartCmd");

            if (debugRemaps)
            {
                controlAV->addRemap("/" + essSystemName + pcsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            controlAV->addRemap(pcsMaintModeURI, 0, true, true, true);
        }
    }

    // we want to remap from our /control/ uri to the output URI we have
    if (!addRemapToControlOutputAV(vmap, vm, ctrlBB, "errStr", errorOutput, "ErrorReportURI", parent_AV, bmsNum, essSystemName, debugRemaps))
        return 0;

    if (!addRemapToControlOutputAV(vmap, vm, ctrlBB, "OpenContactorReq", openReqOutput, "OpenContactorURI", parent_AV,
                                   bmsNum, essSystemName, debugRemaps))
        return 0;

    if (!addRemapToControlOutputAV(vmap, vm, ctrlBB, "CloseContactorReq", closeReqOutput, "CloseContactorURI",
                                   parent_AV, bmsNum, essSystemName, debugRemaps))
        return 0;

    if (!addRemapToControlOutputAV(vmap, vm, ctrlBB, "PcsStartReq", pcsStartOutput, "PcsStartURI", parent_AV, bmsNum, essSystemName, debugRemaps))
        return 0;

    if (!addRemapToControlOutputAV(vmap, vm, ctrlBB, "PcsStopReq", pcsStopOutput, "PcsStopURI", parent_AV, bmsNum, essSystemName, debugRemaps))
        return 0;

    if (!addRemapToControlOutputAV(vmap, vm, ctrlBB, "PCmd", pCmdOutput, "PCmdURI", parent_AV, bmsNum, essSystemName, debugRemaps))
        return 0;

    // the state variable cannot be remapped using the addRemapToControlOutputAV since special actions are needed for
    // bms and pcs maint mode uris
    assetVar* controlOutputAV;
    if (!stateOutput.empty() && isValidURI(stateOutput))
    {
        controlOutputAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "StateVariable");
        if (!controlOutputAV)
        {
            FPS_PRINT_ERROR("Could not find aV [{}:StateVariable]. Please retry amap setup", ctrlBB);
            std::string errorMsg = fmt::format(
                "Could not find aV [{}:StateVariable]. Please ensure this aV is set up properly in setupRackAmap",
                ctrlBB);

            signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(bmsNum), errorMsg, true);
            return 0;
        }

        if (debugRemaps)
        {
            controlOutputAV->addRemap("/" + essSystemName + stateOutput, 0, (void*)0, (void*)0, true, "set");
        }
        controlOutputAV->addRemap(stateOutput, 0, (void*)0, (void*)0, true);

        // if we have bms or pcs maintenace mode uri's, we want to add remaps to our StateVariableReportURI aV to turn
        // off maint mode on END or ERR
        if (!bmsMaintModeURI.empty() && isValidURI(bmsMaintModeURI))
        {
            // we want our remaps to be on the uri that is stored in stateOutput variable
            assetVar* outputAV = vm->getVar(vmap, (char*)stateOutput.c_str());
            if (!outputAV)
            {
                // if this aV doesnt exist already, make it
                int ival = 0;
                outputAV = vm->makeVar(vmap, stateOutput.c_str(), nullptr, ival);
                FPS_PRINT_WARN(
                    "[\"StateVariableReportURI\"] aV doesnt exist, creating new aV [{}:{}] and adding remap to it",
                    outputAV->comp, outputAV->name);
            }

            if (debugRemaps)
            {
                outputAV->addRemap("/" + essSystemName + bmsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            outputAV->addRemap(bmsMaintModeURI, 0, (int)BatteryBalancingUtility::States::END, false, true);
            outputAV->addRemap(bmsMaintModeURI, 0, (int)BatteryBalancingUtility::States::ERR, false, true);
        }

        if (!pcsMaintModeURI.empty() && isValidURI(pcsMaintModeURI))
        {
            // we want our remaps to be on the uri that is stored in stateOutput variable
            assetVar* outputAV = vm->getVar(vmap, (char*)stateOutput.c_str());
            if (!outputAV)
            {
                // if this aV doesnt exist already, make it
                int ival = 0;
                outputAV = vm->makeVar(vmap, stateOutput.c_str(), nullptr, ival);
                FPS_PRINT_WARN(
                    "[\"StateVariableReportURI\"] aV doesnt exist, creating new aV [{}:{}] and adding remap to it",
                    outputAV->comp, outputAV->name);
            }

            if (debugRemaps)
            {
                outputAV->addRemap("/" + essSystemName + pcsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            outputAV->addRemap(pcsMaintModeURI, 0, (int)BatteryBalancingUtility::States::END, false, true);
            outputAV->addRemap(pcsMaintModeURI, 0, (int)BatteryBalancingUtility::States::ERR, false, true);
        }
    }
    else
    {
        FPS_PRINT_WARN(
            "\"StateVariableReportURI\" was not configured or is an invalid uri. No automatic remap set up to output value of [{}:StateVariable]",
            ctrlBB);

        // if we dont have a state variable report uri, we still want to remap bms and pcs MaintModeURI's if we have
        // them
        if (!bmsMaintModeURI.empty() && isValidURI(bmsMaintModeURI))
        {
            controlOutputAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "StateVariable");
            if (!controlOutputAV)
            {
                FPS_PRINT_ERROR("Could not find aV [{}:StateVariable]. Please retry amap setup", ctrlBB);
                std::string errorMsg = fmt::format(
                    "Could not find aV [{}:StateVariable]. Please ensure this aV is set up properly in setupRackAmap",
                    ctrlBB);

                signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(bmsNum), errorMsg, true);
                return 0;
            }

            // when our state goes into END or ERROR, we want to turn bms maint mode to false
            if (debugRemaps)
            {
                controlOutputAV->addRemap("/" + essSystemName + bmsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            controlOutputAV->addRemap(bmsMaintModeURI, 0, (int)BatteryBalancingUtility::States::END, false, true);
            controlOutputAV->addRemap(bmsMaintModeURI, 0, (int)BatteryBalancingUtility::States::ERR, false, true);
        }

        if (!pcsMaintModeURI.empty() && isValidURI(pcsMaintModeURI))
        {
            controlOutputAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "StateVariable");
            if (!controlOutputAV)
            {
                FPS_PRINT_ERROR("Could not find aV [{}:StateVariable]. Please retry amap setup", ctrlBB);
                std::string errorMsg = fmt::format(
                    "Could not find aV [{}:StateVariable]. Please ensure this aV is set up properly in setupRackAmap",
                    ctrlBB);

                signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(bmsNum), errorMsg, true);
                return 0;
            }

            // when our state goes into END or ERROR, we want to turn pcs maint mode to false
            if (debugRemaps)
            {
                controlOutputAV->addRemap("/" + essSystemName + pcsMaintModeURI, 0, (void*)0, (void*)0, true, "set");
            }
            controlOutputAV->addRemap(pcsMaintModeURI, 0, (int)BatteryBalancingUtility::States::END, false, true);
            controlOutputAV->addRemap(pcsMaintModeURI, 0, (int)BatteryBalancingUtility::States::ERR, false, true);
        }
    }

    // if we have a siteStatusURI, we want to remap our state variable to it using the input reg values provided by site
    // controller
    if (!siteStatusURI.empty() && isValidURI(siteStatusURI))
    {
        controlOutputAV = vm->getVar(vmap, (char*)ctrlBB.c_str(), "StateVariable");
        if (!controlOutputAV)
        {
            FPS_PRINT_ERROR("Could not find aV [{}:StateVariable]. Please retry amap setup", ctrlBB);
            std::string errorMsg = fmt::format(
                "Could not find aV [{}:StateVariable]. Please ensure this aV is set up properly in setupRackAmap",
                ctrlBB);

            signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(bmsNum), errorMsg, true);
            return 0;
        }

        if (debugRemaps)
        {
            
            controlOutputAV->addRemap("/" + essSystemName + siteStatusURI, 0, (void*)0, (void*)0, true, "set");
        }

        // INIT remaps to Inactive. INIT = 0 which is also the default value for the StateVariable aV
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::INIT, (int)SiteControllerReporting::Inactive, true);
        // END remaps to completed
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::END, (int)SiteControllerReporting::Completed, true);
        // ERR remaps to Failed
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::ERR, (int)SiteControllerReporting::Failed, true);
        // every state other than INIT, END, or ERR remaps to "In Progress"
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::VOLTAGE_ARBITRATION, (int)SiteControllerReporting::In_Progress, true);
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::ACTIVE_POWER_BALANCING, (int)SiteControllerReporting::In_Progress, true);
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::CONTACTOR_CONTROL, (int)SiteControllerReporting::In_Progress, true);
        controlOutputAV->addRemap(siteStatusURI, 0, (int)BatteryBalancingUtility::States::FINE_BALANCE, (int)SiteControllerReporting::In_Progress, true);
}
    else
    {
        FPS_PRINT_WARN(
            "\"SiteStatusURI\" was not configured or is an invalid uri. No automatic remap set up to output value of [{}:StateVariable]",
            ctrlBB);
    }

    // if we are given a clear faults uri, create a calculate var to set our internal InputRef->reset to true if clear faults uri is true and out state variable is ERR
    if (!clearFaultsURI.empty() && isValidURI(clearFaultsURI))
    {
        // create our clear faults uri if it doesnt exist
        assetVar* sourceAV = vm->getVar(vmap, (char*)clearFaultsURI.c_str());
        if (!sourceAV)
        {
            // if this aV doesnt exist already, make it
            bool bval = false;
            sourceAV = vm->makeVar(vmap, clearFaultsURI.c_str(), nullptr, bval);
            FPS_PRINT_WARN("[\"ClearFaultsURI\"] doesnt exist, creating new aV [{}:{}] and adding remap to it",
                           sourceAV->comp, sourceAV->name);
        }

        // create a new calculateVar
        std::string calcVarName = ctrlBB + ":calculateReset";
        assetVar* calcAV = vm->getVar(vmap, (char*)calcVarName.c_str());
        if (!calcAV)
        {
            // if this aV doesnt exist already, make it
            bool bval = false;
            calcAV = vm->makeVar(vmap, calcVarName.c_str(), nullptr, bval);
            FPS_PRINT_WARN("Creating new aV [{}:{}] to calculate reset", calcAV->comp, calcAV->name);
        }

        // get our /control/.../BatteryBalancing/StateVariable uri
        std::string internalStateVar = ctrlBB + ":StateVariable";

        // add the needed params to our calcVar
        calcAV->setParam("useExpr", true);
        calcAV->setParam("numVars", 2);

        calcAV->setParam("variable1", (char*)clearFaultsURI.c_str());
        calcAV->setParam("variable2", (char*)internalStateVar.c_str());

        // our expression needs to check if clear faults is true and our state variable is ERR (enum value 6)
        calcAV->setParam("expression", (char*)"({1} == true) && ({2} == 6)");

        // remap the value of our calculate var to our internal reset variable
        if (debugRemaps)
        {
            calcAV->addRemap("/" + essSystemName + ctrlBB + ":reset", 0, (void*)0, (void*)0, true, "set");
        }
        calcAV->addRemap(ctrlBB + ":reset", 0, (void*)0, (void*)0, true);

        // NOTE: this is the monitor param for all calculate vars that will ever be set up from code
        std::string monitor = "run_monitor_from_code";

        // now add our calcAV to the wake monitor by creating a wake monitor av with our calc var URI as the wakeMonAV var name 
        std::string wakeMon = "/schedule/" + monitor + "/ess";
        assetVar* wakeMonAV = vm->getVar(vmap, (char*)(wakeMon + calcVarName).c_str());
        if (!wakeMonAV)
        {
            // if this aV doesnt exist already, make it
            bool bval = false;
            wakeMonAV = vm->makeVar(vmap, (char*)wakeMon.c_str(), (char*)calcVarName.c_str(), bval);
            FPS_PRINT_WARN("Creating new aV [{}:{}] to add to wake monitor", wakeMonAV->comp, wakeMonAV->name);

            // add params to our wakeMonAV
            wakeMonAV->setParam("func", (char*)"CalculateVar");
            wakeMonAV->setParam("amap", (char*)"ess");
        }


        // check if we have a run_monitor_from_code sched item for /sched/ess
        std::string runMon = "/sched/ess:" + monitor;
        assetVar* runMonAV = vm->getVar(vmap, (char*)runMon.c_str());
        if (!runMonAV)
        {
            // if this av doesnt exist, put it on the scheduler with the RunMonitor function
            int ival = 0;
            assetVar* runMonAV = vm->makeVar(vmap, (char*)runMon.c_str(), nullptr, ival);
        
            // add params to our runMonAV
            runMonAV->am = vm->getaM(vmap, "ess");
            runMonAV->setParam("aname", (char*)"ess");
            runMonAV->setParam("monitor", (char*)monitor.c_str());

            // schedule our runMonAV
            schedItem* schItem = new schedItem();
            schItem->putOnScheduler("RunMonitor", 0, vm->get_time_dbl(), 0.1, runMon, runMonAV);
        }
        // else, we have a run_monitor_from_code sched item for /sched/ess which is already looking for our wakeMonAV (/schedule/run_monitor_from_code uri)
    }
    else
    {
        FPS_PRINT_WARN(
            "\"ClearFaultsDoneURI\" was not configured or is an invalid uri: [{}]. No automatic calculate var set up to reset internal reset", clearFaultsURI);
    }
    

    // tell our parent AV that we are done by setting the setup flag for this function instance high
    std::string thisFunction = "BatteryBalancing_" + std::to_string(instance);

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

                aV->setParam("BatteryBalancing_instance", instance);

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

            signalError(parent_AV, "setupBatteryBalancing_" + std::to_string(instance), errorMsg, true);
            return 0;
        }
    }

    return 0;
}

// take in a string of comma separated numbers and return a vector of those numbers
std::vector<int> parseNumbers(const std::string& input)
{
    std::vector<int> result;
    std::string noSpaceInput;
    std::remove_copy_if(input.begin(), input.end(), std::back_inserter(noSpaceInput), ::isspace);

    std::stringstream ss(noSpaceInput);
    std::string token;

    // each token is the content between 2 commas
    while (std::getline(ss, token, ','))
    {
        // if our token contains "..", we want to get the range of all numbers between the ".."
        if (token.find("..") != std::string::npos)
        {
            std::regex rangeRegex(R"((\d+)\.*(\d+))");
            std::smatch match;

            if (std::regex_search(token, match, rangeRegex))
            {
                int start = std::stoi(match[1].str());
                int end = std::stoi(match[2].str());

                for (int i = start; i <= end; ++i)
                {
                    result.push_back(i);
                }
            }
        }
        // do not allow a single "."
        else if (token.find(".") != std::string::npos)
        {
            std::string err = "string cannot contain just one \".\"";
            throw std::invalid_argument(err);
        }
        // if no ".." is found, its just a number
        else
        {
            result.push_back(std::stoi(token));
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    // do not allow negative numbers
    if (result.at(0) < 0)
    {
        std::string err = fmt::format("String [{}] cannot contain negative numbers", input);
        throw std::invalid_argument(err);
    }

    return result;
}

// check if a param exists on our fromAV. if so, transfer its value (the value must be a double!) or the value of its uri to our toAV
// returns true if the param exists and we set the value to our toAV, false otherwise
bool transferParam(varsmap& vmap, assetVar* fromAV, assetVar* toAV, std::string param, DataMapType type)
{
    // check if the param exists on the fromAV
    if (fromAV->gotParam((char*)param.c_str()))
    {
        // determine if the param is a string or a number
        if (fromAV->getcParam((char*)param.c_str()) == nullptr)
        {
            // if getcParam returns a nullptr, our param is a number or a bool

            if (type == DataMapType::DOUBLE_T || type == DataMapType::INT_T)
            {
                // if our type is a double, set the param as a double
                toAV->setParam((char*)param.c_str(), fromAV->getdParam((char*)param.c_str()));
                return true;
            }
            else if (type == DataMapType::BOOLEAN_T)
            {
                // if our type is a bool, set the param as a bool
                toAV->setParam((char*)param.c_str(), fromAV->getbParam((char*)param.c_str()));
                return true;
            }
            else
            {
                FPS_PRINT_ERROR("Parameter [{}] has an invalid type; can only be DOUBLE_T or BOOLEAN_T", param);
            }
        }
        else
        {
            // if getcParam is not a nullptr, use that string to get a value from that uri
            std::string paramURI = fromAV->getcParam((char*)param.c_str());
            if (isValidURI(paramURI))
            {
                // get the value from the uri
                assetVar* paramAV = fromAV->am->vm->getVar(vmap, (char*)paramURI.c_str());
                if (paramAV)
                {
                    if (type == DataMapType::DOUBLE_T || type == DataMapType::INT_T)
                    {
                        // if our type is a double, set the param as a double
                        toAV->setParam((char*)param.c_str(), paramAV->getdVal());
                        return true;
                    }
                    else if (type == DataMapType::BOOLEAN_T)
                    {
                        // if our type is a bool, set the param as a bool
                        toAV->setParam((char*)param.c_str(), paramAV->getbVal());
                        return true;
                    }
                    else
                    {
                        FPS_PRINT_ERROR("Parameter [{}] has an invalid type", param);
                    }
                }
                else
                {
                    FPS_PRINT_ERROR("Could not find aV [{}] using URI on parameter [{}]", paramURI, param);
                }
            }
            else
            {
                FPS_PRINT_ERROR("URI: [{}] gotten from parameter [{}] is not a valid uri", paramURI, param);
            }
        }
    }

    return false;
}

// this function is meant to make configuration simpler for BatteryRackBalancing using datamaps
int BatteryRackBalancingConfigWrapper(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // This is a wrapper function that exists to run Battery Balancing in a way that is more easily configurable
    // This function takes the information from its aV, creates a new aV, configures the new aV parameters in the
    // specific way the system needs with the information from this aV, and schedules that new aV periodically This
    // function is also used to re initialize the Battery Balancing object. Calling this function a second (or more)
    // time will delete the old object, create a new one, and re init the BB state machine

    UNUSED(amap);
    UNUSED(p_fims);

    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = false;
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    // determine which bms's we need to set up from av
    std::string bmsIDsString;
    if (!aV->gotParam("BmsIDs"))
    {
        // if we dont have num bms specified, assume we are using 1
        aV->setParam("BmsIDs", (char*)"1");
        bmsIDsString = "1";
    }
    else if (aV->getcParam("BmsIDs") == nullptr)
    {
        // if getcParam returns a nullptr, our param is a int and we need it as a string
        int bmsIDsNum = aV->getiParam("BmsIDs");
        bmsIDsString = std::to_string(bmsIDsNum);
    }
    else
    {
        // if getcParam is not a nullptr, get that string
        bmsIDsString = aV->getcParam("BmsIDs");
    }

    // convert our input param string to a vector of numbers
    std::vector<int> bmsIDsVec;
    try
    {
        bmsIDsVec = parseNumbers(bmsIDsString);
    }
    catch (const std::exception& e)
    {
        FPS_PRINT_ERROR(
            "aV [{}:{}] got error [{}] because parameter [\"BmsIDs\": \"{}\"] is configured incorrectly. Please ensure this parameter is a number or list of comma separated numbers",
            aV->comp, aV->name, e.what(), bmsIDsString);
        return 0;
    }

    // the size of our bmsIDsVec is the total number of bms's we are setting up
    int bmsAmount = bmsIDsVec.size();
    if (debug)
        FPS_PRINT_INFO("Setting up [{}] different assetVars for each bms", bmsAmount);

    // create a local vector to store all of our schedItems after we've created them
    // we dont want to start any schedItems until we're sure there are no errors in the entire setup process
    std::vector<schedItem*> schedVec;

    // for each bms, we need to create an aV for it, set up its params, determine how many racks it has, create a
    // schedItem for it, and put it on the scheduler
    for (int bmsVecIdx = 0; bmsVecIdx < bmsAmount; bmsVecIdx++)
    {
        // get the actual bms number from the index of the bmsIDsVec we are "iterating" over
        int bms_i = bmsIDsVec.at(bmsVecIdx);

        // create a new assetVar for this BMS that RunThread will run on periodically
        std::string comp = aV->comp + ":" + aV->name + "_bms_" + std::to_string(bms_i);
        assetVar* bmsAV = vm->getVar(vmap, (char*)comp.c_str(), nullptr);
        if (!bmsAV)
        {
            // if this bmsAV doesnt exist already, make it
            double dval = 0;
            bmsAV = vm->makeVar(vmap, comp.c_str(), nullptr, dval);
            bmsAV->am = aV->am;
            if (debug)
                FPS_PRINT_INFO("made new bmsAV [{}:{}]", bmsAV->comp, bmsAV->name);

            // inform this aV where the actual magic is happening
            std::string note = fmt::format(
                "The BatteryRackBalancing algorithm is running on aV [{}:{}]. Please check that aV for more information",
                bmsAV->comp, bmsAV->name);
            std::string NOTE_i = "NOTE_" + std::to_string(bms_i);
            aV->setParam((char*)NOTE_i.c_str(), (char*)note.c_str());

            // put a note on the active aV about who started it
            note = fmt::format("This aV was configured and ran based on the information from [{}:{}]", aV->comp,
                               aV->name);
            bmsAV->setParam("NOTE", (char*)note.c_str());

            // default the alreadyConfigured to be false
            bmsAV->setParam("alreadyConfigured", false);
        }
        if (debug)
            FPS_PRINT_INFO("Using bmsAV [{}:{}] for bms [{}]", bmsAV->comp, bmsAV->name, bms_i);

        // let this aV know what bms number it is (since there is only ever 1 instance per av)
        bmsAV->setParam("BmsNumber", bms_i);

        // pass debug flag to new av
        if (debug)
            bmsAV->setParam("debug", true);

        transferParam(vmap, aV, bmsAV, "fims", DataMapType::BOOLEAN_T);

        // transfer all the params from the aV that triggered us to our new "_BRB_bms_i" av
        // datamap specific params
        transferParam(vmap, aV, bmsAV, "overrunLimit", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "decrementOverrun", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "incrementOverrun", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "setupTimeLimit", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "heartbeatTimeout", DataMapType::DOUBLE_T);

        // BRB state machine configurations
        transferParam(vmap, aV, bmsAV, "RampStartDeltaVoltage", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "RampEndDeltaVoltage", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "RackCloseDeltaVoltage", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "BatteryRelaxTime", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "MinimumFeedbackDelayTime", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "ActivePowerUpdateTimeMinimum", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "ActionDelayTimeout", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "MaxOpenContactorAttempts", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "MaxCloseContactorAttempts", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "RackLevelContactorControl", DataMapType::BOOLEAN_T);

        transferParam(vmap, aV, bmsAV, "ActivePowerRampRatekWps", DataMapType::DOUBLE_T);

        transferParam(vmap, aV, bmsAV, "MinRackBalancePower", DataMapType::DOUBLE_T);

        // pass our rack templating fields to our bmsAV
        // inputs
        if (aV->gotParam("RackVoltURI"))
        {
            bmsAV->setParam("RackVoltURI", aV->getcParam("RackVoltURI"));
        }

        if (aV->gotParam("RackSoCURI"))
        {
            bmsAV->setParam("RackSoCURI", aV->getcParam("RackSoCURI"));
        }

        if (aV->gotParam("RackContactorStatusURI"))
        {
            bmsAV->setParam("RackContactorStatusURI", aV->getcParam("RackContactorStatusURI"));
        }

        if (aV->gotParam("RackEnableFeedbackURI"))
        {
            bmsAV->setParam("RackEnableFeedbackURI", aV->getcParam("RackEnableFeedbackURI"));
        }

        if (aV->gotParam("RackFaultURI"))
        {
            bmsAV->setParam("RackFaultURI", aV->getcParam("RackFaultURI"));
        }

        // outputs
        if (aV->gotParam("RackEnableCmdURI"))
        {
            bmsAV->setParam("RackEnableCmdURI", aV->getcParam("RackEnableCmdURI"));
        }

        if (aV->gotParam("RackBalanceFailureURI"))
        {
            bmsAV->setParam("RackBalanceFailureURI", aV->getcParam("RackBalanceFailureURI"));
        }

        // keys
        if (aV->gotParam("RackTmpl"))
        {
            bmsAV->setParam("RackTmpl", aV->getcParam("RackTmpl"));
        }

        if (aV->gotParam("BmsTmpl"))
        {
            bmsAV->setParam("BmsTmpl", aV->getcParam("BmsTmpl"));
        }

        // replacers
        if (aV->gotParam("RackRep"))
        {
            bmsAV->setParam("RackRep", aV->getcParam("RackRep"));
        }

        if (aV->gotParam("BmsRep"))
        {
            bmsAV->setParam("BmsRep", aV->getcParam("BmsRep"));
        }

        // filter info
        bool usingFilter = transferParam(vmap, aV, bmsAV, "FilterRackVoltage", DataMapType::BOOLEAN_T);

        transferParam(vmap, aV, bmsAV, "VoltageFilterFC", DataMapType::DOUBLE_T);

        // maintenance mode uri's
        if (aV->gotParam("BmsMaintModeURI"))
        {
            bmsAV->setParam("BmsMaintModeURI", aV->getcParam("BmsMaintModeURI"));
        }

        if (aV->gotParam("PcsMaintModeURI"))
        {
            bmsAV->setParam("PcsMaintModeURI", aV->getcParam("PcsMaintModeURI"));
        }

        // clear faults uri
        if (aV->gotParam("ClearFaultsDoneURI"))
        {
            bmsAV->setParam("ClearFaultsDoneURI", aV->getcParam("ClearFaultsDoneURI"));
        }

        // site controller outputs
        if (aV->gotParam("SiteStatusURI"))
        {
            bmsAV->setParam("SiteStatusURI", aV->getcParam("SiteStatusURI"));
        }

        // Battery Balancing class inputs
        if (aV->gotParam("StartCmdURI"))
        {
            bmsAV->setParam("StartCmdURI", aV->getcParam("StartCmdURI"));
        }

        if (aV->gotParam("StopCmdURI"))
        {
            bmsAV->setParam("StopCmdURI", aV->getcParam("StopCmdURI"));
        }

        if (aV->gotParam("FineBalancingEnableURI"))
        {
            bmsAV->setParam("FineBalancingEnableURI", aV->getcParam("FineBalancingEnableURI"));
        }

        if (aV->gotParam("MaxBalancePowerURI"))
        {
            bmsAV->setParam("MaxBalancePowerURI", aV->getcParam("MaxBalancePowerURI"));
        }

        if (aV->gotParam("PActlURI"))
        {
            bmsAV->setParam("PActlURI", aV->getcParam("PActlURI"));
        }

        // BB class outputs
        if (aV->gotParam("PCmdURI"))
        {
            bmsAV->setParam("PCmdURI", aV->getcParam("PCmdURI"));
        }

        if (aV->gotParam("StateVariableReportURI"))
        {
            bmsAV->setParam("StateVariableReportURI", aV->getcParam("StateVariableReportURI"));
        }

        if (aV->gotParam("OpenContactorURI"))
        {
            bmsAV->setParam("OpenContactorURI", aV->getcParam("OpenContactorURI"));
        }

        if (aV->gotParam("CloseContactorURI"))
        {
            bmsAV->setParam("CloseContactorURI", aV->getcParam("CloseContactorURI"));
        }

        if (aV->gotParam("PcsStartURI"))
        {
            bmsAV->setParam("PcsStartURI", aV->getcParam("PcsStartURI"));
        }

        if (aV->gotParam("PcsStopURI"))
        {
            bmsAV->setParam("PcsStopURI", aV->getcParam("PcsStopURI"));
        }

        if (aV->gotParam("ErrorReportURI"))
        {
            bmsAV->setParam("ErrorReportURI", aV->getcParam("ErrorReportURI"));
        }

        // check if we've already been configured
        if (!bmsAV->gotParam("alreadyConfigured"))
        {
            // if we dont have this variable, set it to false
            bmsAV->setParam("alreadyConfigured", false);
        }

        if (bmsAV->getbParam("alreadyConfigured"))
        {
            if (debug)
                FPS_PRINT_INFO("We have already been configured and are re initializing our BatteryBalancing object",
                               nullptr);

            // if we've already been re configured, clear all our funcX flags and set to reload flag
            int num = 0;
            while (++num)
            {
                // determine which function number we are setting up
                std::string numStr = std::to_string(num);
                std::string funcNum = "func" + numStr;
                if (!bmsAV->gotParam((char*)funcNum.c_str()))
                {
                    // we have cleared all the funcs listed on our av and now we have found a func# that doesnt exist,
                    // break out of loop
                    if (debug)
                        FPS_PRINT_INFO("Could not find [{}] param on aV [{}:{}]. Done clearing all params", funcNum,
                                       bmsAV->comp, bmsAV->name);
                    break;
                }

                // clear this func#
                bmsAV->setParam((char*)funcNum.c_str(), (char*)"");
                if (debug)
                    FPS_PRINT_INFO("Just set [{}] to [{}]", funcNum, bmsAV->getcParam((char*)funcNum.c_str()));
            }

            // reset the datamaps system flags
            bmsAV->setParam("reconfigured", false);
            bmsAV->setParam("reload", 0);
            bmsAV->setParam("BatteryBalancing_instance", 0);

            // put our step function into the init state to avoid Pcmd issues
            std::string uri = bmsAV->comp + ":" + bmsAV->name;

            // there will only ever be 1 instance of a BatteryBalancing Object on each uri
            BatteryBalancing* BB = BatteryBalancingObjects[uri][1].get();

            // put state machine into safe state to re-init the system
            BB->OutputRef->StateVariable = States::INIT;
            BB->Dw.initialized = false;
            BB->Dw.configWrapperReset = true;
            BB->step();
        }

        // determine how many racks this bms has
        std::string bms_i_NumRacks = "bms_" + std::to_string(bms_i) + "_NumRacks";

        // get the number of racks this bms has
        if (!aV->gotParam((char*)bms_i_NumRacks.c_str()))
        {
            // if we dont know how many racks we have, we cant run
            FPS_PRINT_ERROR(
                "AV [{}:{}] has no \"{}\" parameter but is configured to setup bms_{}. Please configure this aV to include a \"{}\" parameter.",
                aV->comp, aV->name, bms_i_NumRacks, bms_i, bms_i_NumRacks);
            return 0;
        }

        // our bms_X_racks param can be a number or a uri string
        int racks = 0;

        // check if its a string
        if (aV->getcParam((char*)bms_i_NumRacks.c_str()) != NULL)
        {
            // if getcParam does not return NULL, it is a string and we need to get the value from this uri
            std::string comp = aV->getcParam((char*)bms_i_NumRacks.c_str());
            assetVar* numRacksAV = vm->getVar(vmap, (char*)comp.c_str(), nullptr);

            if (!numRacksAV)
            {
                FPS_PRINT_ERROR(
                    "Could not find aV [{}] from string in parameter [{}]. Please ensure that the parameter [{}] is either a number or valid uri",
                    comp, bms_i_NumRacks, bms_i_NumRacks);
                return 0;
            }

            racks = numRacksAV->getiVal();
        }
        else  // if its not a string, just get the value
        {
            racks = aV->getiParam((char*)bms_i_NumRacks.c_str());
        }

        // set our Rack and Battery Balancing functions
        if (usingFilter)
        {
            // if we are filtering input, we need to run our filter functions before running our rack functions
            bmsAV->setParam("func1", (char*)"LowPassFilterBRB");
            bmsAV->setParam("func1_instances", racks);
            bmsAV->setParam("func2", (char*)"Rack");
            bmsAV->setParam("func2_instances", racks);
            bmsAV->setParam("func3", (char*)"BatteryBalancing");
        }
        else
        {
            bmsAV->setParam("func1", (char*)"Rack");
            bmsAV->setParam("func1_instances", racks);
            bmsAV->setParam("func2", (char*)"BatteryBalancing");
        }

        if (debug)
            FPS_PRINT_INFO("BMS [{}] is configured with [{}] racks", bms_i, racks);

        // set the thread that our functions will run on
        std::string threadName = aV->name + "_Thread";
        bmsAV->setParam("threadName", (char*)threadName.c_str());

        // set up new schedItem
        std::string schedID = comp + "_schedItem";

        schedItem* schItem = new schedItem();  // when a new schedItem is a replacement schedItem, the scheduler handles
                                               // cleanup of the old schedItem
        char* myid = (char*)schedID.c_str();
        char* schedFcn = (char*)"RunThread";    // we need RunThread to be our scheduled function
        char* schedUri = (char*)comp.c_str();   // this comp should be the same as the comp of the aV param
        char* schedTarg = (char*)comp.c_str();  // our targAV is this av

        // get our repTime
        if (!aV->gotParam("every"))
        {
            aV->setParam("every", 1);  // default value is every 1 second
        }
        double every = aV->getdParam("every");

        schItem->setUp(myid, aname, schedUri, schedFcn, 0, vm->get_time_dbl(), every, 0, schedTarg);

        if (debug)
            FPS_PRINT_INFO(" Set up new schedItem [{}] to run [{}] on aV [{}]", schItem->id, schItem->func, comp);
        if (debug)
            schItem->show();

        // configure the params the assetVar needs to run on the scheduler
        schItem->av = bmsAV;                                   // our sched item should run on this av
        schItem->av->setParam("runTime", vm->get_time_dbl());  // tell scheduler to run asap
        schItem->av->setParam("repTime", every);               // this is how often RunThread will run
        schItem->av->setParam("update",
                              true);  // tell this schedItem to update so that it runs faster after the first run

        // store this schedItem in our schedItemVector and set up next bms
        schedVec.push_back(schItem);

        if (debug)
            FPS_PRINT_INFO("Finished setting up BMS [{}] with [{}] racks to run on [{}:{}]", bms_i, racks, bmsAV->comp,
                           bmsAV->name);
    }

    // after set up for each bms is done, put them all on the scheduler
    for (schedItem* schItem : schedVec)
    {
        auto reqChan = (channel<schedItem*>*)am->reqChan;
        reqChan->put(schItem);
    }

    // wake them
    if (am->wakeChan)
    {
        am->wakeChan->put(0);
    }

    return 0;
}
