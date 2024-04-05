#include "../SL_files/HighLevelController.h"
#include "dataMap.h"



std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<HighLevelController>>> HighLevelControllerObjects;


uint8_t* getHighLevelControllerInputs(std::string uri, int instance)
{
	std::unique_ptr<HighLevelController>& uqObjPtr = HighLevelControllerObjects[uri][instance];

    HighLevelController* dmHLCObject = uqObjPtr.get();

    return reinterpret_cast<uint8_t*>(&dmHLCObject->HighLevelController_U);
}

uint8_t* getHighLevelControllerOutputs(std::string uri, int instance)
{
	std::unique_ptr<HighLevelController>& uqObjPtr = HighLevelControllerObjects[uri][instance];

    HighLevelController* dmHLCObject = uqObjPtr.get();

    return reinterpret_cast<uint8_t*>(&dmHLCObject->HighLevelController_Y);
}

void HighLevelControllerRun(std::string uri, int instance)
{
	std::unique_ptr<HighLevelController>& uqObjPtr = HighLevelControllerObjects[uri][instance];

    HighLevelController* dmHLCObject = uqObjPtr.get();

    dmHLCObject->step();

    FPS_PRINT_INFO("aV [{}]  HLC[{}] In: [{}]    HLC Out: [{}]", uri, instance, dmHLCObject->HighLevelController_U.In1, dmHLCObject->HighLevelController_Y.Out1);
}

void createNewHighLevelControllerInstance(std::string uri, int instance)
{
    // set up new instance of HighLevelController

    // create a new instance of the HighLevelController object
    HighLevelController* dmHighLevelControllerObject = new HighLevelController();

    // transfer memory management of "new" call to a unique pointer
    std::unique_ptr<HighLevelController> dmHighLevelControllerObjectPtr(dmHighLevelControllerObject);

	// move the ownership of the unique pointer to a global map of instances to unique pointers
	HighLevelControllerObjects[uri][instance] = std::move(dmHighLevelControllerObjectPtr);


	// use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in our global external map
	uint8_t* (*getInputsPtr)(std::string, int) = &getHighLevelControllerInputs;
	modelFcnRef["HighLevelControllerInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

	uint8_t* (*getOutputsPtr)(std::string, int) = &getHighLevelControllerOutputs;
	modelFcnRef["HighLevelControllerOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

	// set refernce to HighLevelController's run function using a global external
	void (*runFuncPtr)(std::string, int) = &HighLevelControllerRun;
	modelFcnRef["HighLevelController"] = reinterpret_cast<void(*)>(runFuncPtr);

    // set refernce to HighLevelController's run function using a global external
    void (*runFuncPtr)(int) = &HighLevelControllerRun;
    modelFcnRef["HighLevelController"] = reinterpret_cast<void(*)>(runFuncPtr);
}

void setupHighLevelControllerDM(assetVar* aV, int instance)
{
    if (!aV->gotParam("datamapName"))
    {
        // we should never get here because RunThread will set a default datamap
        // name if none exits if datamap_name somehow gets deleted, use another
        // default datamap name
        int num_datamap = dataMaps.size() + 1;
        std::string num = std::to_string(num_datamap);

        std::string dflt = "Default_Datamap_" + num;
        aV->setParam("datamapName", (char*)dflt.c_str());

        FPS_PRINT_ERROR(
            "Could not find \"datamap_name\" parameter in assetVar "
            "[{}]. Using default name [{}]",
            __func__, aV->name, dflt);
    }
    std::string name = aV->getcParam("datamapName");

    DataMap* dm = dataMaps[name];
    if (!dm)
    {
        dm = new DataMap;
    }
    dm->name = name;

    std::string instanceStr = std::to_string(instance);

    std::string inputBlock = "HighLevelController_" + instanceStr + "Inputs";
    std::string outputBlock = "HighLevelController_" + instanceStr + "Outputs";

    // Input data items and transfer blocks
    std::string inputName = "In1";
    dm->addDataItem((char*)inputName.c_str(), offsetof(HighLevelController::ExtU_HighLevelController_T, In1),
                    DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    // Output data items and transfer blocks
    std::string outputName = "Out1";
    dm->addDataItem((char*)outputName.c_str(), offsetof(HighLevelController::ExtY_HighLevelController_T, Out1),
                    DataMapType::INT32_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    dataMaps[dm->name] = dm;
}

void setupHighLevelControllerAmap(VarMapUtils *vm, varsmap &vmap, asset_manager *am, int instance, std::string uri)
{
	int debug = 0;
	bool bVal = false;
	double dVal = 0.0;

	std::string instanceStr = std::to_string(instance);

	std::string underscoreURI = replaceSlashAndColonWithUnderscore(uri);
	std::string ctrlHLC = "/control" + underscoreURI + "/HighLevelController_" + instanceStr;

	if(debug)FPS_PRINT_INFO("Setting up datamap to amap interface for {} using amap of asset manager: [{}]", ctrlHLC, am->name);

    // inputs amap vals
    std::string inputAmap = "In1";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlHLC.c_str(), (char*)inputAmap.c_str(), bVal);

    // Output amap vals
    std::string outputAmap = "Out1";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlHLC.c_str(), (char*)outputAmap.c_str(), dVal);
}

void setupHighLevelController(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // create datamap and its asset manager
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

	// use parent AV to run everything
	if (!aV->gotParam("parentAV"))
	{
		FPS_PRINT_ERROR("There is no \"parent_AV\" parameter in [{}]. Cannot signal back to the thread. Timing out", aV->name);
		return;
	}
	std::string parent_uri = aV->getcParam("parentAV");

	// get parent AV from vmap
    assetVar *parent_AV = vm->getVar(vmap, (char*)parent_uri.c_str(), nullptr);
	if (!parent_AV)
	{
		FPS_PRINT_ERROR("Could not find parent AV of [{}] using comp [{}]. Cannot signal back to the thread. Timing out", aV->name, parent_uri);
        return;
	}
	
	// determine which instance we are setting up and instantiate it
	if (!aV->gotParam("HighLevelController_instance"))
	{
		aV->setParam("HighLevelController_instance", 0);
	}
	int instance = aV->getiParam("HighLevelController_instance") + 1;

	// create new instance and set references to it for the rest of the system
	createNewHighLevelControllerInstance(parent_uri, instance);

    // setup the datamap for the aV we are going to run our function on
    setupHighLevelControllerDM(parent_AV, instance);

	// get or make the asset manager for our instance
	std::string instanceAMname = parent_uri + "_HighLevelController_" + std::to_string(instance) + "_asset_manager";
	asset_manager *datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

	// setup amap for this instance
	setupHighLevelControllerAmap(vm, vmap, datamapInstanceAM, instance, parent_uri);

    // tell our parent AV that we are done by setting the setup flag for this
    // function instance high
    std::string thisFunction = "HighLevelController_" + std::to_string(instance);

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

                if (0)
                    FPS_PRINT_INFO(" we found {} from the [{}] param. setting [{}] to true", thisFunction, funcNum,
                                   setupNum);

				aV->setParam("HighLevelController_instance", instance);
				return;
			}
			
		} 
		else
		{
			// if we get here we have no way to signal back to RunThread that we are done setting up. Setting error to our parentAV
			FPS_PRINT_ERROR("Could not find \"{}\" in [{}]'s list of functions. Signaling error", thisFunction, parent_uri);

			std::string errorMsg = fmt::format("Could not find \"{}\" in [{}]'s list of functions.", thisFunction, parent_uri);
			parent_AV->setParam("errorType", (char*)"fault");
			parent_AV->setParam("errorMsg", (char*)errorMsg.c_str());

            bool logging_enabled = parent_AV->getbParam("logging_enabled");
            char* LogDir = parent_AV->getcParam("LogDir");

			ESSLogger::get().critical("While trying to set up function [{}] on assetVar [{}], we got this error: [{}] ", thisFunction, parent_uri, errorMsg);
			if (logging_enabled)
			{
				std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
				ESSLogger::get().logIt(dirAndFile);
			}
			
			signalThread(parent_AV, ERROR);
			return;
		}
		
	}

            signalThread(parent_AV, ERROR);
            return;
        }
    }
}
