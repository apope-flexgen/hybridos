#ifndef DATAUTILITY_CPP
#define DATAUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"
#include "InfoMessageUtility.hpp"
#include "OutputHandler.hpp"
#include "DataUtility.hpp"


namespace DataUtility
{

    /**
    * @brief gives the ability to populate the amap with a singular given assetVar based on the information given in the "info" variable
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param vm  VarMapUtils pointer
    * @param info The necessary info of an assetVar for the purpose of populating it within the amap
    */
    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo* info)
    {

        int ival= -1;
        double dval= 0.0;
        bool bval = false;
        char* cval = nullptr;

        char* amId = nullptr;

        // if(info->uri) FPS_PRINT_INFO("info->uri [{}]", info->uri);
        // if(info->varName) FPS_PRINT_INFO("info->varName [{}]", info->varName);
        // if(info->assetManagerId) FPS_PRINT_INFO("info->assetManagerId [{}]", info->assetManagerId);
        // if(info->type) FPS_PRINT_INFO("info->type [{}]", info->type);
        

        if (info->assetManagerId)  {
            amId = const_cast<char*>(info->assetManagerId);
        } else {
            amId = const_cast<char*>(info->varName);
        }

        assetVar* av = vm->getVar(vmap, info->uri, info->varName);
        if (!av) {
            switch (info->type){
                case assetVar::ATypes::AINT:
                    av = vm->makeVar(vmap, info->uri, info->varName, ival);
                    break;
                case assetVar::ATypes::AFLOAT:
                    av = vm->makeVar(vmap, info->uri, info->varName, dval);
                    break;
                case assetVar::ATypes::ABOOL:
                    av = vm->makeVar(vmap, info->uri, info->varName, bval);
                    break;
                case assetVar::ATypes::ASTRING:
                    av = vm->makeVar(vmap, info->uri, info->varName, cval);
                    break;
                default:
                    FPS_PRINT_ERROR("Error in Switch Statement of PopulateAmapWithAv - No known type given for AV", nullptr);

            }
        }
        amap[amId] = av;

        return amap;
    } 

    /**
    * @brief allows to populate multiple assetVars within the amap at once by calling with a vector list of the desired assetVars, this will then call the PopulateAmapWithAv for each one
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param vm  VarMapUtils pointer
    * @param assetVarVector a vector list of all the assetVars that will be populated individually in the amap
    */
    varmap& PopulateAmapWithManyAvs(varsmap& vmap, varmap& amap, VarMapUtils* vm, std::vector<AssetVarInfo>& assetVarVector)
    {
        for (const AssetVarInfo& assetVarInfo : assetVarVector) {
            amap = DataUtility::PopulateAmapWithAv(vmap, amap, vm, &assetVarInfo);
        }
        return amap;
    } 

 
    void PrintAssetVar(assetVar* aV, assetVar::ATypes assetVarValType) {
        std::string assetVarString = "";
        assetVarString += "\n{\n";
        // assetVarString += "  \"" + aV->getfName() + "\": {\n";
        if(0)FPS_PRINT_INFO("Start of PrintAssetVar");
        assetVarString += fmt::format("  \"{}\": {{\n", aV->getfName());

        switch (assetVarValType){
            case assetVar::ATypes::AINT:
                if(0)FPS_PRINT_INFO("Before iParam");
                assetVarString += fmt::format("    \"value\": {},\n", aV->getiVal());
                break;
            case assetVar::ATypes::AFLOAT:
                if(0)FPS_PRINT_INFO("Before dVal");
                assetVarString += fmt::format("    \"value\": {},\n", aV->getdVal());
                break;
            case assetVar::ATypes::ABOOL:
                if(0)FPS_PRINT_INFO("Before bVal");
                assetVarString += fmt::format("    \"value\": {},\n", aV->getbVal());
                break;
            case assetVar::ATypes::ASTRING:
                if(0)FPS_PRINT_INFO("Before cVal");
                assetVarString += fmt::format("    \"value\": \"{}\",\n", aV->getcVal());
                break;
            default:
                break;
        }

        // for (const auto& keyValuePair : paramToDataTypeDict) {
        for (auto it = paramToDataTypeDict.begin(); it != paramToDataTypeDict.end(); ++it) {

            std::string paramString = it->first;
            assetVar::ATypes paramType = it->second;

            if(0)FPS_PRINT_INFO("paramString {}", paramString);

            if(aV->gotParam(paramString.c_str())) {
                if(0)FPS_PRINT_INFO("Param was found");
                switch (paramType){
                    case assetVar::ATypes::AINT:
                        if(0)FPS_PRINT_INFO("iParam {}", aV->getiParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": {}", paramString, aV->getiParam(paramString.c_str()));
                        break;
                    case assetVar::ATypes::AFLOAT:
                        if(0)FPS_PRINT_INFO("dParam {}", aV->getdParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": {}", paramString, aV->getdParam(paramString.c_str()));
                        break;
                    case assetVar::ATypes::ABOOL:
                        if(0)FPS_PRINT_INFO("bParam {}", aV->getbParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": {}", paramString, aV->getbParam(paramString.c_str()));
                        break;
                    case assetVar::ATypes::ASTRING:
                        if(0)FPS_PRINT_INFO("cParam {}", aV->getcParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": \"{}\"", paramString, aV->getcParam(paramString.c_str()));
                        break;
                    default:
                        break;
                }

                if (std::next(it) != paramToDataTypeDict.end()) {
                    //not the last element
                    assetVarString += ",\n";
                } else {
                    //the last element
                    assetVarString += "\n";
                }
            }

        }

        assetVarString += fmt::format("  }}\n");
        assetVarString += fmt::format("}}\n\n");

        FPS_PRINT_INFO("{}", assetVarString);
    }

    std::unordered_map<std::string, std::string> controlsEnabledLogicMap = {
        {"close_contactors", ""},
        {"open_contactors", ""},
        {"start", ""},
        {"stop", ""},
        {"standby", ""}

    };

    std::string GetEnabledLogicMessage(std::string controlUri){
        return controlsEnabledLogicMap[controlUri];
    }

    void UpdateEnabledLogicMessage(std::string controlUri, std::string logicMessage){
        controlsEnabledLogicMap[controlUri] = logicMessage;
    }


}


#endif
