#ifndef FUNCTIONUTILITY_HPP
#define FUNCTIONUTILITY_HPP

#include "asset.h"
#include "assetVar.h"
#include "formatters.hpp"
#include <iostream>

namespace FunctionUtility
{
 
    struct AssetVarInfo {
        const char* uri;
        const char* varName;
        assetVar::ATypes type;

        AssetVarInfo(const char* u, const char* vn, assetVar::ATypes t) 
            : uri(u), varName(vn), type(t) {}

        // Add a virtual function to make the class polymorphic
        virtual void doSomething() {}
    };

    struct AssetVarInfoWithManagerId : public AssetVarInfo {
        const char* assetManagerId;

        AssetVarInfoWithManagerId(const char* u, const char* vn, const char* ami, assetVar::ATypes t) 
            : AssetVarInfo(u, vn, t), assetManagerId(ami) {}
    };
 
    int FunctionSharedResultFinder
    (
        int (*function)(varsmap&, varmap&, 
        const char*, fims*, assetVar*), 
        varsmap& vmap, 
        varmap& amap, 
        const char* aname, 
        fims* p_fims, 
        assetVar* aV
    );

    void FunctionResultHandler
    (
        int returnValue, 
        varsmap& vmap, 
        varmap& amap, 
        const char* aname, 
        fims* p_fims, 
        assetVar* aV,
        const char* scheduledFuncName,
        const char* inputHandlerFuncName,
        std::string outputHandlerFuncName,
        const char* updatedAssetVarName,
        const char* infoMessage
    ); 

    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo& info);

    varmap& PopulateAmapWithManyAvs(varsmap& vmap, varmap& amap, VarMapUtils* vm, std::vector<AssetVarInfo>& assetVarVector);


    
}



#endif
