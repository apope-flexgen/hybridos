#include "../SL_files/DC_Augmentation.h"
#include "dataMap.h"



std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<DC_Augmentation_class::ExtUPointer>>> DC_AugmentationInputs;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<DC_Augmentation_class::ExtYPointer>>> DC_AugmentationOutputs;
std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<DC_Augmentation_class>>>	DC_AugmentationObjects;


uint8_t* getDC_AugmentationInputs(std::string uri, int instance)
{
	std::unique_ptr<DC_Augmentation_class::ExtUPointer>& uqInputPtr = DC_AugmentationInputs[uri][instance];

	DC_Augmentation_class::ExtUPointer* dmDC_AugmentationInputs = uqInputPtr.get();

	return reinterpret_cast<uint8_t*>(dmDC_AugmentationInputs);
}

uint8_t* getDC_AugmentationOutputs(std::string uri, int instance)
{
	std::unique_ptr<DC_Augmentation_class::ExtYPointer>& uqOutputPtr = DC_AugmentationOutputs[uri][instance];

	DC_Augmentation_class::ExtYPointer* dmDC_AugmentationOutputs = uqOutputPtr.get();

	return reinterpret_cast<uint8_t*>(dmDC_AugmentationOutputs);
}

void DC_AugmentationRun(std::string uri, int instance)
{
	std::unique_ptr<DC_Augmentation_class>& uqObjPtr = DC_AugmentationObjects[uri][instance];

	DC_Augmentation_class* dmDC_AugmentationObject = uqObjPtr.get();

	dmDC_AugmentationObject->step();

	FPS_PRINT_INFO("aV [{}] instance [{}]", uri, instance);
}

void createNewDC_AugmentationInstance(std::string uri, int instance)
{
	// create a new instance of the DC_Augmentation inputs struct, output struct, and object
	DC_Augmentation_class::ExtUPointer* dmDC_AugmentationInput = new DC_Augmentation_class::ExtUPointer;
	DC_Augmentation_class::ExtYPointer* dmDC_AugmentationOutput = new DC_Augmentation_class::ExtYPointer;
	DC_Augmentation_class* dmDC_AugmentationObject = new DC_Augmentation_class(dmDC_AugmentationInput, dmDC_AugmentationOutput);

	// transfer memory management of "new" calls to unique pointers
	std::unique_ptr<DC_Augmentation_class::ExtUPointer> dmDC_AugmentationInputPtr(dmDC_AugmentationInput);
    std::unique_ptr<DC_Augmentation_class::ExtYPointer> dmDC_AugmentationOutputPtr(dmDC_AugmentationOutput);
	std::unique_ptr<DC_Augmentation_class> dmDC_AugmentationObjectPtr(dmDC_AugmentationObject);

	// move the ownership of the unique pointers to a global map of instances to unique pointers. the map key needs to be the aV it is running on and its instance
	DC_AugmentationInputs[uri][instance] = std::move(dmDC_AugmentationInputPtr);
	DC_AugmentationOutputs[uri][instance] = std::move(dmDC_AugmentationOutputPtr);
	DC_AugmentationObjects[uri][instance] = std::move(dmDC_AugmentationObjectPtr);


	// use a function to get modelInputs and modelOutputs when in CoreAmapAcces and store a pointer to that function in our global external map
	uint8_t* (*getInputsPtr)(std::string, int) = &getDC_AugmentationInputs;
	modelFcnRef["DC_AugmentationInputs"] = reinterpret_cast<void(*)>(getInputsPtr);

	uint8_t* (*getOutputsPtr)(std::string, int) = &getDC_AugmentationOutputs;
	modelFcnRef["DC_AugmentationOutputs"] = reinterpret_cast<void(*)>(getOutputsPtr);

	// set reference to DC_Augmentation's run function using a global external
	void (*runFuncPtr)(std::string, int) = &DC_AugmentationRun;
	modelFcnRef["DC_Augmentation"] = reinterpret_cast<void(*)>(runFuncPtr);
}

void setupDC_AugmentationDM(assetVar* aV, int instance)
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

	std::string inputBlock = "DC_Augmentation_" + instanceStr + "Inputs";
	std::string outputBlock = "DC_Augmentation_" + instanceStr + "Outputs";

	//Input data items and transfer blocks
	std::string inputName = "Pcmd";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, Pcmd),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());

	inputName = "Qcmd";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, Qcmd),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "StartVal";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, StartVal),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "StopVal";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, StopVal),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "FltClr";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, FltClr),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "MaintCmd";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, MaintCmd),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DcaFltActv";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DcaFltActv),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_SOC";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_SOC),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_Volts";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_Volts),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_Current";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_Current),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_Capacity";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_Capacity),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_ChargePwrLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_ChargePwrLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_DischargePwrLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_DischargePwrLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_ChargeCurrentLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_ChargeCurrentLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_DischargeCurrentLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_DischargeCurrentLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_SOC";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_SOC),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_Volts";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_Volts),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_Current";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_Current),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_Capacity";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_Capacity),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_ChargePwrLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_ChargePwrLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_DischargePwrLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_DischargePwrLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_ChargeCurrentLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_ChargeCurrentLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_DischargeCurrentLim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_DischargeCurrentLim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_V1";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_V1),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_V2";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_V2),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_Current";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_Current),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_Power";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_Power),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_Plim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_Plim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_DcVlt";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_DcVlt),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_DcCurrent";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_DcCurrent),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_ActivePower";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_ActivePower),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_Plim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_Plim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_Qlim";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_Qlim),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());

	inputName = "PCS_1_PRamp";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_PRamp),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_Qramp";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_Qramp),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_PRamp";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_PRamp),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_PNom";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_PNom),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_PNom";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_PNom),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_PNom";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_PNom),  DataMapType::REAL_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "StartStop";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, StartStop),  DataMapType::BOOLEAN_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_1_DCDisconnectStt";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_1_DCDisconnectStt),  DataMapType::BOOLEAN_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "BMS_2_DCDisconnectStt";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, BMS_2_DCDisconnectStt),  DataMapType::BOOLEAN_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "DCDC_1_Status";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, DCDC_1_Status),  DataMapType::BOOLEAN_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());
	
	inputName = "PCS_1_Status";
	dm->addDataItem((char*)inputName.c_str(),  offsetof(DC_Augmentation_class::ExtUPointer, PCS_1_Status),  DataMapType::BOOLEAN_T);
	dm->addTransferItem(inputBlock , inputName.c_str(), inputName.c_str());


	//Output data items and transfer blocks
	std::string outputName = "DCDC_1_Pcmd";
	dm->addDataItem((char*)outputName.c_str(),  offsetof(DC_Augmentation_class::ExtYPointer, DCDC_1_Pcmd),  DataMapType::REAL_T);
	dm->addTransferItem(outputBlock , outputName.c_str(), outputName.c_str());
	
	outputName = "PCS_1_Pcmd";
	dm->addDataItem((char*)outputName.c_str(),  offsetof(DC_Augmentation_class::ExtYPointer, PCS_1_Pcmd),  DataMapType::REAL_T);
	dm->addTransferItem(outputBlock , outputName.c_str(), outputName.c_str());
	
	outputName = "PCS_1_Qcmd";
	dm->addDataItem((char*)outputName.c_str(),  offsetof(DC_Augmentation_class::ExtYPointer, PCS_1_Qcmd),  DataMapType::REAL_T);
	dm->addTransferItem(outputBlock , outputName.c_str(), outputName.c_str());
	
	outputName = "DcaMode";
	dm->addDataItem((char*)outputName.c_str(),  offsetof(DC_Augmentation_class::ExtYPointer, DcaMode),  DataMapType::REAL_T);
	dm->addTransferItem(outputBlock , outputName.c_str(), outputName.c_str());
	
	outputName = "RunMode";
	dm->addDataItem((char*)outputName.c_str(),  offsetof(DC_Augmentation_class::ExtYPointer, RunMode),  DataMapType::REAL_T);
	dm->addTransferItem(outputBlock , outputName.c_str(), outputName.c_str());

	dataMaps[dm->name] = dm;
}

void setupDC_AugmentationAmap(VarMapUtils *vm, varsmap &vmap, asset_manager *am, int instance, std::string uri)
{
	int debug = 0;
	bool bVal = false;
	double dVal = 0.0;

	std::string instanceStr = std::to_string(instance);

	std::string underscoreURI = replaceSlashAndColonWithUnderscore(uri);
	std::string ctrlDCA = "/control" + underscoreURI + "/DC_Augmentation_" + instanceStr;

	if(debug)FPS_PRINT_INFO("Setting up datamap to amap interface for {} using amap of asset manager: [{}]", ctrlDCA, am->name);

	// input amap vals
	std::string inputAmap = "Pcmd";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "Qcmd";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "StartVal";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "StopVal";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "FltClr";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "MaintCmd";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DcaFltActv";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_SOC";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_Volts";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_Current";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_Capacity";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_ChargePwrLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_DischargePwrLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_ChargeCurrentLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_DischargeCurrentLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_SOC";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_Volts";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_Current";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_Capacity";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_ChargePwrLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_DischargePwrLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_ChargeCurrentLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_DischargeCurrentLim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_V1";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_V2";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_Current";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_Power";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_Plim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_DcVlt";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_DcCurrent";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_ActivePower";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_Plim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_Qlim";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_PRamp";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "PCS_1_Qramp";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_PRamp";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "DCDC_1_PNom";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_1_PNom";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "BMS_2_PNom";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), dVal);
	
	inputAmap = "StartStop";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), bVal);
	
	inputAmap = "BMS_1_DCDisconnectStt";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), bVal);
	
	inputAmap = "BMS_2_DCDisconnectStt";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), bVal);
	
	inputAmap = "DCDC_1_Status";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), bVal);
	
	inputAmap = "PCS_1_Status";
	am->amap[(char*)inputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)inputAmap.c_str(), bVal);


	// Output amap vals
	std::string outputAmap = "DCDC_1_Pcmd";
	am->amap[(char*)outputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)outputAmap.c_str(), dVal);
	
	outputAmap = "PCS_1_Pcmd";
	am->amap[(char*)outputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)outputAmap.c_str(), dVal);
	
	outputAmap = "PCS_1_Qcmd";
	am->amap[(char*)outputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)outputAmap.c_str(), dVal);
	
	outputAmap = "DcaMode";
	am->amap[(char*)outputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)outputAmap.c_str(), dVal);
	
	outputAmap = "RunMode";
	am->amap[(char*)outputAmap.c_str()]    = vm->setVal(vmap, (char*)ctrlDCA.c_str(), (char*)outputAmap.c_str(), dVal);
}

void setupDC_Augmentation(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* aV)
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

	// get parent AV from vmap
    assetVar *parent_AV = vm->getVar(vmap, (char*)parent_uri.c_str(), nullptr);
	if (!parent_AV)
	{
		FPS_PRINT_ERROR("Could not find parent AV of [{}] using comp [{}]. Cannot signal back to the thread. Timing out", aV->name, parent_uri);
        return;
	}

	// determine which instance we are setting up and instantiate it
	if (!aV->gotParam("DC_Augmentation_instance"))
	{
		aV->setParam("DC_Augmentation_instance", 0);
	}
	int instance = aV->getiParam("DC_Augmentation_instance") + 1;

	// create new instance and set references to it for the rest of the system
	createNewDC_AugmentationInstance(parent_uri, instance);

	// setup the datamap for the aV we are going to run our function on
	setupDC_AugmentationDM(parent_AV, instance);

	// get or make the asset manager for our instance
	std::string instanceAMname = parent_uri + "_DC_Augmentation_" + std::to_string(instance) + "_asset_manager";
	asset_manager *datamapInstanceAM = getOrMakeAm(vm, vmap, am->name.c_str(), instanceAMname.c_str());

	// setup amap for this instance
	setupDC_AugmentationAmap(vm, vmap, datamapInstanceAM, instance, parent_uri);

	std::string thisFunction = "DC_Augmentation_" + std::to_string(instance);

	// tell our parent AV that we are done by setting the setup flag for this function high
	int num = 0;
	while (++num)
	{
		// determine which function number we are setting up
		std::string numStr = std::to_string(num);
		std::string funcNum = "func" + numStr;
		if (parent_AV->gotParam((char*)funcNum.c_str()))
		{
			std::string iterativeFunction = parent_AV->getcParam((char*)funcNum.c_str());
			
			if (iterativeFunction == thisFunction)
			{
				// we have found our function, set its "setupX" flag high to signify that we are done
				std::string setupNum = "setup" + numStr;
				parent_AV->setParam((char*)setupNum.c_str(), true);

				if (0) FPS_PRINT_INFO(" we found {} from the [{}] param. setting [{}] to true", thisFunction, funcNum, setupNum);

				aV->setParam("DC_Augmentation_instance", instance);
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
}