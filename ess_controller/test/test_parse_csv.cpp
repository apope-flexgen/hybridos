#include "../funcs/CheckTableVar.cpp"
#include "chrono_utils.hpp"

// Test valid case - both vars are within the range of key-pair values in the
// table
void testValidVars(two_way_tbl* tbl)
{
    FPS_ERROR_PRINT(
        "%s >> Testing valid case - vars are within range of "
        "key-pair values in table...\n",
        __func__);

    // Both vars == key-pair values
    FPS_ERROR_PRINT("Test 1: Both vars == key-pair values\n");
    double soc = 30;
    double temp = 5;
    double expResults = 0.120000;
    double actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    // One var in between key value ranges (ex.: 35 between 30 and 40)
    FPS_ERROR_PRINT("Test 2: One var in between key value ranges\n");
    soc = 35;
    expResults = 0.120000;
    actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT("\n");
}

// Test valid case - both vars are within the range of key-pair values in the
// table (edge case)
void testValidVars2(two_way_tbl* tbl)
{
    FPS_ERROR_PRINT(
        "%s >> Testing valid case - vars are within the range of "
        "key-pair values in the table (edge case)...\n",
        __func__);

    FPS_ERROR_PRINT(
        "Test 1: Var one and var two values are in the lower bound "
        "in p_charge_tbl.csv\n");
    double soc = 0;
    double temp = -30;
    double expResults = 0.000000;
    double actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT(
        "Test 2: Var one and var two values are in the upper bound "
        "in p_charge_tbl.csv\n");
    soc = 100;
    temp = 65;
    expResults = 0.000000;
    actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT("Test 3: One var is in the lower bound in p_charge_tbl.csv\n");
    soc = 0;
    temp = 54;
    expResults = 1.000000;
    actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT("\n");
}

// Test invalid case - one var is outside the range of key-pair values in the
// table
void testInvalidVars(two_way_tbl* tbl)
{
    FPS_ERROR_PRINT(
        "%s >> Testing invalid valid case - vars are outside the "
        "range of key-pair values in the table...\n",
        __func__);

    FPS_ERROR_PRINT(
        "Test 1: Var one and var two are less than the lower bound "
        "in p_charge_tbl.csv\n");
    double soc = -1;
    double temp = -36;
    double expResults = 0.000000;
    double actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT(
        "Test 2: Var one and var two are greater than the upper "
        "bound in p_charge_tbl.csv\n");
    soc = 101;
    temp = 66;
    expResults = 0.000000;
    actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT("Test 3: One var is less than the lower bound in p_charge_tbl.csv\n");
    soc = -1;
    temp = 54;
    expResults = 0.000000;
    actResults = getTableValue(soc, temp, tbl);
    if (expResults == actResults)
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] == actResults [%f] "
            "--> SUCCESS\n\n",
            soc, temp, expResults, actResults);
    else
        FPS_ERROR_PRINT(
            "soc [%f] temp [%f]    expResults [%f] != actResults [%f] "
            "--> FAILURE\n\n",
            soc, temp, expResults, actResults);

    FPS_ERROR_PRINT("\n");
}

int main()
{
    VarMapUtils vm;
    two_way_tbl* p_charge_tbl = SetupTableVar(&vm, "p_charge_tbl.csv");
    testValidVars(p_charge_tbl);
    testValidVars2(p_charge_tbl);
    testInvalidVars(p_charge_tbl);

    return EXIT_SUCCESS;
}