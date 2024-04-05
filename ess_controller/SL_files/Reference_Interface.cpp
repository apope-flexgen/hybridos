#include "../SL_files/Reference.h"
#include "dataMap.h"



std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<Reference_class>>> ReferenceObjects;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<Reference_class::ExtUPointer_Reference_T>>> ReferenceInputs;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<Reference_class::ExtYPointer_Reference_T>>> ReferenceOutputs;


uint8_t* getReferenceInputs(std::string uri, int instance)
{
	std::unique_ptr<Reference_class::ExtUPointer_Reference_T>& uqInputPtr = ReferenceInputs[uri][instance];

    Reference_class::ExtUPointer_Reference_T* dmRefInputs = uqInputPtr.get();

    return reinterpret_cast<uint8_t*>(dmRefInputs);
}

uint8_t* getReferenceOutputs(std::string uri, int instance)
{
	std::unique_ptr<Reference_class::ExtYPointer_Reference_T>& uqOutputPtr = ReferenceOutputs[uri][instance];

    Reference_class::ExtYPointer_Reference_T* dmRefOutputs = uqOutputPtr.get();

    return reinterpret_cast<uint8_t*>(dmRefOutputs);
}

void ReferenceRun(std::string uri, int instance)
{
	std::unique_ptr<Reference_class>& uqObjPtr = ReferenceObjects[uri][instance];

    Reference_class* dmRefObject = uqObjPtr.get();

    dmRefObject->step();

	FPS_PRINT_INFO("aV [{}] running Reference_{} output: {}", uri, instance, dmRefObject->ExtYPointer_ref_Y->DMTestOut);
}

void createNewReferenceInstance(std::string uri, int instance)
{
	// create a new instance of the Reference inputs struct, output struct, and object
	Reference_class::ExtUPointer_Reference_T* dmReferenceInput = new Reference_class::ExtUPointer_Reference_T;
	Reference_class::ExtYPointer_Reference_T* dmReferenceOutput = new Reference_class::ExtYPointer_Reference_T;
	Reference_class* dmReferenceObject = new Reference_class(dmReferenceInput, dmReferenceOutput);

    // transfer memory management of "new" calls to unique pointers
    std::unique_ptr<Reference_class::ExtUPointer_Reference_T> dmReferenceInputPtr(dmReferenceInput);
    std::unique_ptr<Reference_class::ExtYPointer_Reference_T> dmReferenceOutputPtr(dmReferenceOutput);
    std::unique_ptr<Reference_class> dmReferenceObjectPtr(dmReferenceObject);

	// move the ownership of the unique pointers to a global map of instances to unique pointers. the map key needs to be the aV it is running on and its instance
	ReferenceInputs[uri][instance] = std::move(dmReferenceInputPtr);
	ReferenceOutputs[uri][instance] = std::move(dmReferenceOutputPtr);
	ReferenceObjects[uri][instance] = std::move(dmReferenceObjectPtr);


	// use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in our global external map
	uint8_t* (*getInputsPtr)(std::string, int) = &getReferenceInputs;
	modelFcnRef["ReferenceInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

	uint8_t* (*getOutputsPtr)(std::string, int) = &getReferenceOutputs;
	modelFcnRef["ReferenceOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

	// set refernce to Reference's run function using a global external
	void (*runFuncPtr)(std::string, int) = &ReferenceRun;
	modelFcnRef["Reference"] = reinterpret_cast<void(*)>(runFuncPtr);
}

void setupReferenceDM(assetVar* aV, int instance)
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

    std::string inputBlock = "Reference_" + instanceStr + "Inputs";
    std::string outputBlock = "Reference_" + instanceStr + "Outputs";

    // Input data items and transfer blocks
    std::string inputName = "DMTestDirection";
    dm->addDataItem((char*)inputName.c_str(), offsetof(Reference_class::ExtUPointer_Reference_T, DMTestDirection),
                    DataMapType::BOOLEAN_T);
    dm->addTransferItem(inputBlock, (char*)inputName.c_str(), (char*)inputName.c_str());

    // Output data items and transfer blocks
    std::string outputName = "DMTestOutDblNoise";
    dm->addDataItem((char*)outputName.c_str(), offsetof(Reference_class::ExtYPointer_Reference_T, DMTestOutDblNoise),
                    DataMapType::REAL_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "DMTestOut";
    dm->addDataItem((char*)outputName.c_str(), offsetof(Reference_class::ExtYPointer_Reference_T, DMTestOut),
                    DataMapType::INT32_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "AdderResult";
    dm->addDataItem((char*)outputName.c_str(), offsetof(Reference_class::ExtYPointer_Reference_T, AdderResult),
                    DataMapType::INT32_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "GainResult";
    dm->addDataItem((char*)outputName.c_str(), offsetof(Reference_class::ExtYPointer_Reference_T, GainResult),
                    DataMapType::INT32_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    outputName = "DirectionFeedback";
    dm->addDataItem((char*)outputName.c_str(), offsetof(Reference_class::ExtYPointer_Reference_T, DirectionFeedback),
                    DataMapType::BOOLEAN_T);
    dm->addTransferItem(outputBlock, (char*)outputName.c_str(), (char*)outputName.c_str());

    dataMaps[dm->name] = dm;
}

void setupReferenceAmap(VarMapUtils *vm, varsmap &vmap, asset_manager *am, int instance, std::string uri)
{
	int debug = 0;
	int iVal = 0;
	bool bVal = false;
	double dVal = 0.0;

	std::string instanceStr = std::to_string(instance);

	// we want the amap entry to use underscores when displaying the comp instead of slashes
	std::string underscoreURI = replaceSlashAndColonWithUnderscore(uri);
	std::string ctrlRef = "/control" + underscoreURI + "/Reference_" + instanceStr;

	if(debug)FPS_PRINT_INFO("Setting up datamap to amap interface for {} using amap of asset manager: [{}]", ctrlRef, am->name);

    // inputs amap vals
    std::string inputAmap = "DMTestDirection";
    am->amap[(char*)inputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRef.c_str(), (char*)inputAmap.c_str(), bVal);

    // Output amap vals
    std::string outputAmap = "DMTestOutDblNoise";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRef.c_str(), (char*)outputAmap.c_str(), dVal);

    outputAmap = "DMTestOut";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRef.c_str(), (char*)outputAmap.c_str(), iVal);

    outputAmap = "AdderResult";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRef.c_str(), (char*)outputAmap.c_str(), iVal);

    outputAmap = "GainResult";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRef.c_str(), (char*)outputAmap.c_str(), iVal);

    outputAmap = "DirectionFeedback";
    am->amap[(char*)outputAmap.c_str()] = vm->setVal(vmap, (char*)ctrlRef.c_str(), (char*)outputAmap.c_str(), bVal);
}

void setupReference(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // create datamap and its asset manager
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

    // use parent AV to run everything
    if (!aV->gotParam("parentAV"))
    {
        FPS_PRINT_ERROR(
            "There is no \"parent_AV\" parameter in [{}]. Cannot "
            "signal back to the thread. Timing out",
            aV->name);
        return;
    }
    std::string parent_uri = aV->getcParam("parentAV");

	// determine which instance we are setting up and instantiate it
	if (!aV->gotParam("Reference_instance"))
	{
		aV->setParam("Reference_instance", 0);
	}
	int instance = aV->getiParam("Reference_instance") + 1;

	// create new instance and set references to it for the rest of the system
	createNewReferenceInstance(parent_uri, instance);

    // determine which instance we are setting up and instantiate it
    if (!aV->gotParam("Reference_instance"))
    {
        aV->setParam("Reference_instance", 0);
    }
    int instance = aV->getiParam("Reference_instance") + 1;
    createNewReferenceInstance(instance);

	// get or make the asset manager for our instance
	std::string instanceAMname = parent_uri + "_Reference_" + std::to_string(instance) + "_asset_manager";
	asset_manager *datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

	// setup amap for this instance
	setupReferenceAmap(vm, vmap, datamapInstanceAM, instance, parent_uri);

    // setup amap for this instance
    setupReferenceAmap(vm, vmap, datamapInstanceAM, instance);

    // tell our parent AV that we are done by setting the setup flag for this
    // function instance high
    std::string thisFunction = "Reference_" + std::to_string(instance);

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

				aV->setParam("Reference_instance", instance);
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

            std::string errorMsg = fmt::format("Could not find \"{}\" in [{}]'s list of functions.", thisFunction,
                                               parent_AV->name);
            parent_AV->setParam("errorType", (char*)"fault");
            parent_AV->setParam("errorMsg", (char*)errorMsg.c_str());

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

            ESSLogger::get().critical(
                "While trying to set up function [{}] on "
                "assetVar [{}], we got this error: [{}] ",
                thisFunction, parent_AV->name, errorMsg);
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
