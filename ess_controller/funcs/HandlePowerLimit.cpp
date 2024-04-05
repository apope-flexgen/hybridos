#ifndef HANDLEPOWERLIMIT_CPP
#define HANDLEPOWERLIMIT_CPP

#include "asset.h"
//#include "../funcs/CheckTableVar.cpp"
// Type alias for map representing two-way-table
// using two_way_tbl = std::map<std::pair<double, double>, double>;

// two_way_tbl* SetupTableVar(VarMapUtils* vm, const char* fileName, const char*
// ext = "csv");

// double getTableValue(double var1, double var2, two_way_tbl *tbl);

extern "C++" {

int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// int CheckTableVar(varsmap &vmap, varmap &amap, const char* aname, fims*
// p_fims, assetVar* av1, assetVar* av2, assetVar* tblAv);
}

/**
 * @brief Adjust the power limit based on incoming SOC and temperature
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the asset var
 */
int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    asset_manager* am = av->am;
    VarMapUtils* vm = am->vm;
    char* essName = vm->getSysName(vmap);
    essPerf ePerf(am, aname, "HandlePowerLimit");
    int reload = vm->CheckReload(vmap, amap, aname, "HandlePowerLimit");
    if (reload < 2)
    {
        int ival = 0;
        double dval = 0.0;
        char* chargeTbl = (char*)"p_charge_tbl.csv";
        char* dischargeTbl = (char*)"p_discharge_tbl.csv";
        amap["BMSChargePowerDerate"] = vm->setLinkVal(*am->vmap, "bms", "/status", "BMSChargePowerDerate", dval);
        amap["BMSDischargePowerDerate"] = vm->setLinkVal(*am->vmap, "bms", "/status", "BMSDischargePowerDerate", dval);
        amap["pChargeTbl"] = vm->setLinkVal(*am->vmap, essName, "/config", "pChargeTbl", chargeTbl);
        amap["pDischargeTbl"] = vm->setLinkVal(*am->vmap, essName, "/config", "pDischargeTbl", dischargeTbl);
        amap["mbmu_soc"] = vm->setLinkVal(*am->vmap, "bms", "/status", "mbmu_soc", dval);
        amap["mbmu_avg_cell_temperature"] = vm->setLinkVal(*am->vmap, essName, "/status", "mbmu_avg_cell_temperature",
                                                           dval);

        if (!amap["pChargeTbl"]->getcVal())
            amap["pChargeTbl"]->setVal(chargeTbl);
        if (!amap["pDischargeTbl"]->getcVal())
            amap["pDischargeTbl"]->setVal(dischargeTbl);

        // Check if the table exists. If the table exists, delete the table since
        // SetupTableVar dynamically allocates the table
        void* tblPtr = amap["pChargeTbl"]->getvParam("tbl");
        if (tblPtr)
        {
            if (1)
                FPS_ERROR_PRINT(
                    "%s >> We're setting up a new table, but table %s at "
                    "%p already exists. Deleting current table now...\n",
                    __func__, amap["pChargeTbl"]->name.c_str(), tblPtr);
            delete (two_way_tbl*)tblPtr;
        }

        // Set up the table and store that as an asset param
        two_way_tbl* tbl = SetupTableVar(vm, amap["pChargeTbl"]->getcVal());
        // printf("%s >>  done 1\n",__func__);
        amap["pChargeTbl"]->setParam("tbl", (void*)tbl);

        tblPtr = amap["pDischargeTbl"]->getvParam("tbl");
        if (tblPtr)
        {
            if (1)
                FPS_ERROR_PRINT(
                    "%s >> We're setting up a new table, but table %s at "
                    "%p already exists. Deleting current table now...\n",
                    __func__, amap["pDischargeTbl"]->name.c_str(), tblPtr);
            delete (two_way_tbl*)tblPtr;
        }

        // Set up the table and store that as an asset param
        tbl = SetupTableVar(vm, amap["pDischargeTbl"]->getcVal());
        // printf("%s >> done 2\n", __func__);
        amap["pDischargeTbl"]->setParam("tbl", (void*)tbl);

        if (reload == 0)
        {
            // Set params/other variables here
        }
        ival = 2;
        amap["HandlePowerLimit"]->setVal(ival);
    }

    double soc = amap["mbmu_soc"]->getdVal();
    double temp = amap["mbmu_avg_cell_temperature"]->getdVal();
    // printf("%s >> done 3\n"  , __func__);
    if (0)
        FPS_ERROR_PRINT("%s >> done 4 pChargeTbl %p pDischargeTbl %p\n", __func__, amap["pChargeTbl"]->getvParam("tbl"),
                        amap["pDischargeTbl"]->getvParam("tbl")

        );

    // Retrieve the table and look for the values
    two_way_tbl* pChargeTbl = (two_way_tbl*)(amap["pChargeTbl"]->getvParam("tbl"));
    two_way_tbl* pDischargeTbl = (two_way_tbl*)(amap["pDischargeTbl"]->getvParam("tbl"));
    if (0)
        FPS_ERROR_PRINT("%s >> done 5 pChargeTbl %p pDischargeTbl %p\n", __func__, pChargeTbl, pDischargeTbl

        );
    if (!pChargeTbl || !pDischargeTbl)
    {
        if (1)
            FPS_ERROR_PRINT("%s >> Unable to load tables \n", __func__);
        return 0;
    }

    double pChargeVal = getTableValue(soc, temp, pChargeTbl);
    double pDischargeVal = getTableValue(soc, temp, pDischargeTbl);

    if (0)
        FPS_ERROR_PRINT(
            "%s >> soc [%2.3f] temp [%2.3f]  pChargeVal [%2.3f] "
            "pDischargeVal [%2.3f]\n",
            __func__, soc, temp, pChargeVal, pDischargeVal);

    amap["BMSChargePowerDerate"]->setVal(pChargeVal);
    amap["BMSDischargePowerDerate"]->setVal(pDischargeVal);

    return 0;
}
#endif