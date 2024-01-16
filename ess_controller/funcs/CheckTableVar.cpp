#ifndef CHECKTABLEVAR_CPP
#define CHECKTABLEVAR_CPP

#include <string.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <utility>

#include "asset.h"

extern "C++" {
    int CheckTableVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
}


// Type alias for map representing two-way-table
using two_way_tbl = std::map<std::pair<double, double>, double>;

/**
 * @brief Parses the CSV file and adds data to the two-way-table
 * 
 * @param fname the csv file to parse
 * @return two_way_tbl the two-way-table after parsing the csv file
 */
std::shared_ptr<two_way_tbl> parseCSVFile(const char* fileName)
{
    std::string fn = fileName;
    std::ifstream myFile(fn);
    if (!myFile.is_open())
    {
        FPS_PRINT_INFO("File {} cannot be opened", fileName);
        return nullptr;
    }
    else FPS_PRINT_DEBUG("File {} is opened", fileName);
    
    std::shared_ptr<two_way_tbl> map(new two_way_tbl);  // Two-way table
    std::vector<int> varOneKeys;                        // Store list of keys related to the first variable

    std::string line, field;
    getline(myFile, line);                              // skip the 1st line

    bool firstField = true;                             // firstField used to determine var one keys
    while (getline(myFile, line))
    {
        std::istringstream iss(line);

        // Retrieve the first row of data first. This will be used to help make
        // the pair of keys to then add to the map
        if (firstField)
        {
            getline(iss, field, ',');                   // skip first field
            while (getline(iss, field, ','))
            {
                varOneKeys.push_back(stod(field));
            }
            firstField = false;
        }
        // Create key pairs and add key-value pairs to the two-way table
        else
        {
            getline(iss, field, ',');
            int varTwoKey = stod(field);                // Store current var two key
            int idx = 0;
            while (getline(iss, field, ','))
            {
                FPS_PRINT_DEBUG("{} ", field.c_str());
                std::pair<int, int> keyPair = std::make_pair(varOneKeys[idx++], varTwoKey);
                map->insert(std::pair<std::pair<double, double>, double>(keyPair, stod(field)));
            }
        }
    }
    return map;
}

/**
 * @brief Creates the two-way-table from the given file
 * 
 * @param fileName the file to parse
 * @param extension the 
 * @return two_way_tbl 
 */
std::shared_ptr<two_way_tbl> SetupTableVar(VarMapUtils *vm, const char* fileName, const char* ext = "csv")
{
    if (!fileName) return nullptr;

    std::string fn = fileName;

    // Check if file matches the extension or if it is a csv file (default)
    if (fn.substr(fn.find_last_of(".") + 1) != ext)
    {
        FPS_PRINT_INFO("{} does not contain extension {}", fileName, ext);
        return nullptr;
    }
    // find the file in the configs area.
    char* cfgname = vm->getFileName(fileName);
    // Parse the file (csv)
    if (!cfgname) return nullptr;
    std::shared_ptr<two_way_tbl> tbl = nullptr;
    if (!strcmp(ext, "csv")) tbl = parseCSVFile(cfgname);
    if(cfgname) free(cfgname);
    return tbl;
}

/**
 * @brief Retrieves the value from the two-way-table, given two values that serve
 * as key-pair values
 * 
 * @param var1 the first key value
 * @param var2 the second key value
 * @param tbl the two-way-table
 * @return the value from the table or 0 if the value cannot be found
 */
double getTableValue(double var1, double var2, std::shared_ptr<two_way_tbl> tbl)
{
    double varOneKey = 0;
    double varTwoKey = 0;
    double varOneLowerBound = 0;
    double varOneUpperBound = 0;
    double varTwoLowerBound = 0;
    double varTwoUpperBound = 0;
    bool keysInit = false;

    for (auto& var : *tbl)
    {
        FPS_PRINT_DEBUG("varOneKey [{}] varTwoKey [{}] var value [{}]", var.first.first, var.first.second, var.second);
        if (!keysInit)
        {
            varOneLowerBound = var.first.first;
            varTwoLowerBound = var.first.second;
            keysInit = true;
        }
        if (var.first.first <= var1)
        {
            varOneKey = var.first.first;
        }
        if (var.first.second <= var2)
        {
            varTwoKey = var.first.second;
        }
        varOneUpperBound = var.first.first;
        varTwoUpperBound = var.first.second;
    }

    // If var1 or var2 is outside the range in the table, set value to 0 by default
    if (var1 > varOneUpperBound || var1 < varOneLowerBound || var2 > varTwoUpperBound || var2 < varTwoLowerBound)
    {
        FPS_PRINT_DEBUG("Value not found (due to key value). Value is now set to 0 by default");
        FPS_PRINT_DEBUG("var1 [{}] varOnekey [{}]  var2 [{}] varTwoKey [{}]", var1, varOneKey, var2, varTwoKey);
        return 0.0;
    }
    else
    {
        std::pair<double, double> keyvar = std::make_pair(varOneKey,varTwoKey);
        FPS_PRINT_DEBUG("varOneKey found [{}] varTwoKey found [{}] var value found [{}]", varOneKey, varTwoKey, tbl->find(keyvar)->second);
        return tbl->find(keyvar)->second;
    }
}

/**
 * @brief Checks the two-way-table for a value based on key-pair values and updates the table value to the assetVar
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the assetVar to check the table for
 */
int CheckTableVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    FPS_PRINT_DEBUG("Running for assetVar [{}] with aname [{}]", av->name.c_str(), aname);

	VarMapUtils* vm = av->am->vm;
    std::string reloadStr = "CheckTableVar_" + av->name;
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        FPS_PRINT_DEBUG("Reload val for asset manager {} reload name {} is %d. Reloading...", aname, reloadStr.c_str(), reload);
        if (reload < 1)
        {

            // Set up table var assetVars if they don't exist yet
            char* tblVar1Name = av->getcParam("tableVar1");
            if(tblVar1Name)
            {
                double dval = 0.0;
                if (!amap[tblVar1Name])
                {
                    FPS_PRINT_DEBUG("Creating assetVar [{}] for {} in tableVar1", tblVar1Name, aname);
                    amap[tblVar1Name] = vm->setLinkVal(vmap, aname, "/status", tblVar1Name, dval);
                }
            }

            char* tblVar2Name = av->getcParam("tableVar2");
            if (tblVar2Name)
            {
                double dval = 0.0;
                if (!amap[tblVar2Name])
                {
                    FPS_PRINT_DEBUG("Creating assetVar [{}] for {} in tableVar2", tblVar2Name, aname);
                    amap[tblVar2Name] = vm->setLinkVal(vmap, aname, "/status", tblVar2Name, dval);
                }
            }

            // // Check if the table exists. If the table exists, delete the table since SetupTableVar dynamically allocates the table
            // void* tblPtr = av->getvParam("tbl");
            // if (tblPtr)
            // {
            //     FPS_PRINT_DEBUG("We're setting up a new table, but table %p in {} already exists. Deleting current table now...", av->name.c_str(), tblPtr);
            //     two_way_tbl* tbl = (two_way_tbl*)tblPtr;
            //     tbl->clear();
            //     delete tbl;
            // }

            // // Set up the table and store that as an asset param
            // two_way_tbl* tbl = SetupTableVar(vm, av->getcParam("tableName"));
            // if (tbl) av->setParam("tbl", (void *)tbl);

            char* tblName = av->getcParam("tableName");
            if (tblName)
            {
                // Check if table already exists in the table map. If so, replace the existing table with a new one
                if (vm->tblMap.find(tblName) != vm->tblMap.end())
                {
                    FPS_PRINT_DEBUG("We're setting up a new table for {} to use, but table {} already exists in the table map. Deleting current table now...", av->name.c_str(), tblName);
                    vm->tblMap.erase(tblName);
                }

                // Create new table and add to the table map
                std::shared_ptr<two_way_tbl> tbl = SetupTableVar(vm, tblName);
                if (tbl) vm->tblMap.insert(std::pair<std::string, std::shared_ptr<two_way_tbl>>(tblName, tbl));
            }
        }

        int ival = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), ival);
        amap[reloadStr]->setVal(ival);
    }

    // Make sure the table var params are defined in the config file or database and the table exists.
    //two_way_tbl* tbl = (two_way_tbl *)(av->getvParam("tbl"));
    char* tblName = av->getcParam("tableName");
    auto tblIter = vm->tblMap.find(tblName);
    if (tblIter != vm->tblMap.end())
    {
        char* tblVar1Name = av->getcParam("tableVar1");
        char* tblVar2Name = av->getcParam("tableVar2");
        if (!tblVar1Name || !tblVar2Name)
        {
            FPS_PRINT_DEBUG("tableVar param(s) are null. tableVar1: {}  tableVar2: {}. Skipping table check.", tblVar1Name ? tblVar1Name : "null", tblVar2Name ? tblVar2Name : "null");
            return 0;
        }
        assetVar* tblVar1 = amap[tblVar1Name];
        assetVar* tblVar2 = amap[tblVar2Name];
        if (!tblVar1 || !tblVar2)
        {
            FPS_PRINT_DEBUG("tableVar assetVar(s) are null. tableVar1: {}  tableVar2: {}. Skipping table check.", (void*)tblVar1, (void*)tblVar2);
            return 0;
        }

        // Retrieve the table and look for the values
        double val1 = tblVar1->getdVal();
        double val2 = tblVar2->getdVal();
        double tblVal = getTableValue(val1, val2, tblIter->second);
        FPS_PRINT_DEBUG("Table [{}] --> tblVar1 [{}] val1 [{}]   tblVar2 [{}] val2 [{}] tblVal [{}]", av->getcParam("tableName") ? av->getcParam("tableName") : "null", tblVar1->name.c_str(), tblVar2->name.c_str(), val1, val2, tblVal);

        // Update av with the value retrieved from the table
        av->setVal(tblVal);
    }

    return 0;
}

#endif