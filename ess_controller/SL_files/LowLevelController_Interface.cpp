#include "../SL_files/LowLevelController.h"
#include "dataMap.h"



std::unordered_map<int, std::unique_ptr<LowLevelController>> LowLevelControllerObjects;


uint8_t* getLowLevelControllerInputs(int instance)
{
	std::unique_ptr<LowLevelController>& uqObjPtr = LowLevelControllerObjects[instance];

	LowLevelController* dmLLCObject = uqObjPtr.get();

	return reinterpret_cast<uint8_t*>(&dmLLCObject->rtU);
}

uint8_t* getLowLevelControllerOutputs(int instance)
{
	std::unique_ptr<LowLevelController>& uqObjPtr = LowLevelControllerObjects[instance];

	LowLevelController* dmLLCObject = uqObjPtr.get();

	return reinterpret_cast<uint8_t*>(&dmLLCObject->rtY);
}

void LowLevelControllerRun(int instance)
{
	std::unique_ptr<LowLevelController>& uqObjPtr = LowLevelControllerObjects[instance];

	LowLevelController* dmLLCObject = uqObjPtr.get();

	dmLLCObject->step();

    FPS_PRINT_INFO("  LLC[{}] Common IN: [{}] Unique IN: [{}] LLOut: [{}]", instance, dmLLCObject->rtU.Common_IN, dmLLCObject->rtU.Unique_IN, dmLLCObject->rtY.LowLevelOut);
}

void createNewLowLevelControllerInstance(int instance)
{
	// set up new instance of LowLevelController

	// create a new instance of the LowLevelController  object
	LowLevelController* dmLowLevelControllerObject = new LowLevelController();

	// transfer memory management of "new" call to a unique pointer
	std::unique_ptr<LowLevelController> dmLowLevelControllerObjectPtr(dmLowLevelControllerObject);

	// move the ownership of the unique pointer to a global map of instances to unique pointers
	LowLevelControllerObjects[instance] = std::move(dmLowLevelControllerObjectPtr);

	// set key for modelInputs and modelOutputs
	std::string inputBlock = "LowLevelController_" + std::to_string(instance) + "Inputs";
	std::string outputBlock = "LowLevelController_" + std::to_string(instance) + "Outputs";

	// use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in our global external map
	uint8_t* (*getInputsPtr)(int) = &getLowLevelControllerInputs;
	modelFcnRef[inputBlock] = reinterpret_cast<void(*)>(getInputsPtr);

	uint8_t* (*getOutputsPtr)(int) = &getLowLevelControllerOutputs;
	modelFcnRef[outputBlock] = reinterpret_cast<void(*)>(getOutputsPtr);

	// set refernce to LowLevelController's run function using a global external
	void (*runFuncPtr)(int) = &LowLevelControllerRun;
	modelFcnRef["LowLevelController"] = reinterpret_cast<void(*)>(runFuncPtr);

}

void setupLowLevelControllerDM(assetVar* aV, int instance)
{
	if (!aV->gotParam("datamapName"))
	{
		// we should never get here because RunThread will set a default datamap name if none exits
		// if datamap_name somehow gets deleted, use another default datamap name
		int num_datamap = dataMaps.size() + 1;
        std::string num = std::to_string(num_datamap);

        std::string dflt = "Default_Datamap_" + num;
        aV->setParam("datamapName", (char*)dflt.c_str());

		FPS_PRINT_ERROR("Could not find \"datamap_name\" parameter in assetVar [{}]. Using default name [{}]", __func__, aV->name, dflt);
	}
	std::string name = aV->getcParam("datamapName");

	DataMap* dm = dataMaps[name];
	if (!dm)
	{
		dm = new DataMap;
	}
	dm->name = name;

	std::string instanceStr = std::to_string(instance);

	std::string inputBlock = "LowLevelController_" + instanceStr + "Inputs";
	std::string outputBlock = "LowLevelController_" + instanceStr + "Outputs";

	//Input data items and transfer blocks
	std::string inputName = "Common_IN";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(LowLevelController::ExtU, Common_IN),  DataMapType::INT32_T);
	dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    inputName = "Unique_IN";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(LowLevelController::ExtU, Unique_IN),  DataMapType::INT32_T);
	dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());


	//Output data items and transfer blocks
	std::string outputName = "LowLevelOut";
	dm->addDataItem((char*)outputName.c_str(),  offsetof(LowLevelController::ExtY, LowLevelOut),  DataMapType::INT32_T);
	dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

	dataMaps[dm->name] = dm;
}

void setupLowLevelControllerAmap(VarMapUtils *vm, varsmap &vmap, asset_manager *am, int instance)
{
	int debug = 0;
	if(debug)FPS_PRINT_INFO("Setting up datamap to amap interface for /control/LowLevelController_{} using amap of asset manager: [{}]", instance, am->name);
	double dVal = 0.0;

	std::string instanceStr = std::to_string(instance);
	std::string ctrlLLC = "/control/LowLevelController_" + instanceStr;

	// inputs amap vals
	std::string inputAmap = "Common_IN";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlLLC.c_str(), (char*)inputAmap.c_str(), dVal);

    inputAmap = "Unique_IN";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlLLC.c_str(), (char*)inputAmap.c_str(), dVal);
	
	
	// Output amap vals
	std::string outputAmap = "LowLevelOut";
	am->amap[(char*)outputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlLLC.c_str(), (char*)outputAmap.c_str(), dVal);

}

void setupLowLevelController(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
{
	// create datamap and its asset manager
	asset_manager *am = aV->am;
	VarMapUtils *vm = am->vm;

	// use parent AV to run everything
	if (!aV->gotParam("parentAV"))
	{
		FPS_PRINT_ERROR("There is no \"parent_AV\" parameter in [{}]. Cannot signal back to the thread. Timing out", aV->name);
		return;
	}
	std::string parent_uri = aV->getcParam("parentAV");
    assetVar *parent_AV = vm->getVar(vmap, (char*)parent_uri.c_str(), nullptr);
	if (!parent_AV)
	{
		FPS_PRINT_ERROR("Could not find parent AV of [{}] using comp [{}]. Cannot signal back to the thread. Timing out", aV->name, parent_uri);
        return;
	}
	// determine which instance we are setting up and instantiate it
	if (!aV->gotParam("LowLevelController_instance"))
	{
		aV->setParam("LowLevelController_instance", 0);
	}
	int instance = aV->getiParam("LowLevelController_instance") + 1;
	createNewLowLevelControllerInstance(instance);

	// setup the datamap for the aV we are going to run our function on
	setupLowLevelControllerDM(parent_AV, instance);

	// get or make the asset manager for our instance
	std::string instanceAMname = "LowLevelController_" + std::to_string(instance) + "_asset_manager";
	asset_manager *datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

	// setup amap for this instance
	setupLowLevelControllerAmap(vm, vmap, datamapInstanceAM, instance);

	// tell our parent AV that we are done by setting the setup flag for this function instance high
	std::string thisFunction = "LowLevelController_" + std::to_string(instance);

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

				if (0) FPS_PRINT_INFO(" we found {} from the [{}] param. setting [{}] to true", thisFunction, funcNum, setupNum);

				aV->setParam("LowLevelController_instance", instance);
				return;
			}
			
		} 
		else
		{
			// if we get here we have no way to signal back to RunThread that we are done setting up. Setting error to our parentAV
			FPS_PRINT_ERROR("Could not find \"{}\" in [{}]'s list of functions. Signaling error", thisFunction, parent_AV->name);

			std::string errorMsg = fmt::format("Could not find \"{}\" in [{}]'s list of functions.", thisFunction, parent_AV->name);
			parent_AV->setParam("errorType", (char*)"fault");
			parent_AV->setParam("errorMsg", (char*)errorMsg.c_str());

			bool logging_enabled = parent_AV->getbParam("logging_enabled");
    		char* LogDir = parent_AV->getcParam("LogDir");

			ESSLogger::get().critical("While trying to set up function [{}] on assetVar [{}], we got this error: [{}] ", thisFunction, parent_AV->name, errorMsg);
			if (logging_enabled)
			{
				std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "datamap_errors", "txt");
				ESSLogger::get().logIt(dirAndFile);
			}
			
			signalThread(parent_AV, ERROR);
			return;
		}
		
	}

}

