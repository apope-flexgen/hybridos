/*
 * vmap deadband test
 *   set a double value
 *    add 0.1 to it and check valueChangedReset
 * should detect a change
 * set deadband to 0.2
 * add 0.1 and check valueChangedReset
 * should not detect a change
 * add 0.1001 and check valueChangedReset
 * should detect a change
 * repeat for ints
 *

 */

#include "asset.h"
#include "chrono_utils.hpp"
#include <math.h>

namespace flex
{
const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}  // namespace flex
//#include "../test/assetFunc.cpp"

void printVars(VarMapUtils& vm, varsmap& vmap)
{
    cJSON* cj = vm.getMapsCj(vmap);
    char* res = cJSON_Print(cj);
    printf("vmap at end \n%s\n", res);
    free((void*)res);
    cJSON_Delete(cj);
}
cJSON* getSchList()
{
    return nullptr;
}

void showResult(VarMapUtils* vm, varsmap& vmap, const char* testId, const char* res, assetVar* av, double tval)
{
    vm->setTime();
    double lastdval = av->getdLVal();
    double thisdval = av->getdVal();

    printf("[%s] Seting  Value %2.3f current  %2.6f last %2.6f dbV %2.6f\n", testId, tval, thisdval, lastdval, av->dbV);

    vm->setVal(vmap, av->comp.c_str(), av->name.c_str(), tval);
    lastdval = av->getdLVal();
    thisdval = av->getdVal();
    bool bval = av->valueChangedReset();
    printf(
        "[%s] Set Value %2.3f changedReset   [%5s] expect [%5s] last %2.6f "
        "this %2.6f dbV %2.6f\n\n",
        testId, tval, bval ? "true" : "false", res, lastdval, thisdval, av->dbV);
}

int main_test_deadband(int argc, char* argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory

    VarMapUtils vm;
    // low level setVar , makes it and sets the value
    int dval = 1234.0;

    vm.setTime();

    assetVar* DeadbandTestD = vm.setVal(vmap, "/system/test", "DeadbandTestD", dval);
    showResult(&vm, vmap, "Test 1", "false", DeadbandTestD, dval + 0.00);

    showResult(&vm, vmap, "Test 2", "true", DeadbandTestD, dval + 0.1);

    double dbval = 0.1;
    DeadbandTestD->setDbVal(dbval);

    showResult(&vm, vmap, "Test 3", "false", DeadbandTestD,
               dval + 0.150);  // below deadband level

    showResult(&vm, vmap, "Test 4", "true", DeadbandTestD, dval + 0.201);
    showResult(&vm, vmap, "Test 5", "false", DeadbandTestD, dval + 0.201);
    showResult(&vm, vmap, "Test 6", "false", DeadbandTestD, dval + 0.104);
    showResult(&vm, vmap, "Test 7", "true", DeadbandTestD, dval + 0.100);

    // ival1 =StandbyTestI->getiLVal();
    // ival2 =StandbyTestI->getiVal();
    // dtime3 = StandbyTestI->aVal->chgTime;
    // dtime2 = StandbyTestI->aVal->setTime;
    // bval =StandbyTestI->valueChangedReset();
    // printf(" Second Test Value changedReset [%s] expect [true] at chg %2.6f set
    // %2.6f last %d currrent  %d  time diffuS %f\n"
    //     , bval?"true":"false"
    //     , dtime3
    //     , dtime2
    //     , ival1, ival2, (dtime2-dtime1)*1000000.0);

    // ival1 =StandbyTestI->getiLVal();
    // ival2 =StandbyTestI->getiVal();
    // bval = StandbyTestI->valueChanged();
    // printf(" Third Test Value changed       [%s] expect [false] last %d
    // currrent  %d  time diffuS %f\n"
    //     , bval?"true":"false"
    //     , ival1, ival2, (dtime2-dtime1)*1000000.0);

    // printf(" set it again to %d  and retest  we should still register no change
    // possibly also check chgtime\n", ixval);

    // vm.setTime();
    // vm.setVal(vmap,"/system/status","StandbyTestI",ixval);

    // dtime1 = StandbyTestI->lVal->setTime;
    // dtime2 = StandbyTestI->aVal->setTime;
    // dtime3 = StandbyTestI->aVal->chgTime;

    // ival1 =StandbyTestI->getiLVal();
    // ival2 =StandbyTestI->getiVal();
    // bval = StandbyTestI->valueChangedReset();
    // printf(" Third Test Value changedReset) [%s] expect [false] at chg %2.6f
    // set %2.6f last %d currrent  %d  time diffuS %f\n"
    //     , bval?"true":"false"
    //     , dtime3
    //     , dtime2
    //     , ival1, ival2, (dtime2-dtime1)*1000000.0);

    // printf("**now test doubles******\n\n");

    // double tNow = vm.get_time_dbl();
    // double dval1;
    // double dval2;
    // double dval3;

    // assetVar* av2 = vm.setVal(vmap,"/system/status","StandbyTestD2",tNow);
    // tNow = vm.get_time_dbl();
    // vm.setVal(vmap,"/system/status","StandbyTestD2",tNow);
    // dval1 =av2->getdLVal();
    // dval2 =av2->getdVal();
    // dval3 = av2->dbV;
    // dtime1 = av2->lVal->setTime;
    // dtime2 = av2->aVal->setTime;
    // dtime3 = av2->aVal->chgTime;

    // bval = av2->valueChangedReset();
    // printf(" First Test Value changedReset [%s] expect [true]  at chg %2.6f set
    // %2.6f last %2.6f currrent  %2.6f  time diffuS %f db *10^6 %2.6f\n"
    //     , bval?"true":"false"
    //     , dtime3
    //     , dtime2
    //     , dval1, dval2, (dval2-dval1)*1000000.0
    //     , dval3*1000000.0);
    // bval = av2->valueChangedReset();
    // printf(" First Test Value changedReset [%s] expect [false] at chg %2.6f set
    // %2.6f last %2.6f currrent  %2.6f  time diffuS %f\n"
    //     , bval?"true":"false"
    //     , dtime3
    //     , dtime2
    //     , dval1, dval2, (dval2-dval1)*1000000.0);

    // printf("**now test deadband ******\n\n");
    // tNow = vm.get_time_dbl();
    // vm.setVal(vmap,"/system/status","StandbyTestD2",tNow);
    // bval = av2->valueChangedReset();
    // dval3=av2->dbV;
    // dval = tNow+(dval3/2.0);
    // vm.setVal(vmap,"/system/status","StandbyTestD2",dval);
    // dval1 =av2->getdLVal();
    // dval2 =av2->getdVal();
    // bval = av2->valueChangedReset();
    // printf(" First Test Value changedReset [%s] expect [false] at chg %2.6f set
    // %2.6f last %2.6f currrent  %2.6f  time diffuS %f\n"
    //     , bval?"true":"false"
    //     , dtime3
    //     , dtime2
    //     , dval1, dval2, (dval2-dval1)*1000000.0);
    // dval = tNow+(dval3*2.0);

    // vm.setVal(vmap,"/system/status","StandbyTestD2",dval);
    // dval1 =av2->getdLVal();
    // dval2 =av2->getdVal();
    // bval = av2->valueChangedReset();
    // printf(" First Test Value changedReset [%s] expect [true] at chg %2.6f set
    // %2.6f last %2.6f currrent  %2.6f  time diffuS %f\n"
    //     , bval?"true":"false"
    //     , dtime3
    //     , dtime2
    //     , dval1, dval2, (dval2-dval1)*1000000.0);

    // printf("**now test other stuff******\n\n");
    // vm.setTime();
    // StandbyTestI->setVal(120);
    // ival1 =StandbyTestI->getiLVal();
    // ival2 =StandbyTestI->getiVal();
    // dtime1 = StandbyTestI->lVal->setTime;
    // dtime2 = StandbyTestI->aVal->setTime;

    // if (StandbyTestI->valueIsDiff(1234) )
    // {
    //     printf(" First Test Value has changed from %d to %d  time diffuS %f\n",
    //     ival1, ival2, (dtime2-dtime1)*1000000.0);
    // }
    // else
    // {
    //     printf(" First Test Value has NOT changed \n");
    // }

    // if (StandbyTestI->valueIsDiff(1234) )
    // {
    //     printf(" Second Test Value has changed \n");
    // }
    // else
    // {
    //     printf(" Second Test Value has NOT changed \n");
    // }
    // ixval = 1233; StandbyTestI->setLVal(ixval);
    // ixval = 12; StandbyTestI->setVal(ixval);
    // //StandbyTestI->setVal(12);
    // bool bxval = StandbyTestI->valueChanged();

    // printf(" Second Test ValueChanged [%s] \n\n\n\n",bxval?"true":"false");

    // delete bms_man;
    //     vm.clearVmap(vmap);

    //     //return 0;

    //     double dxval = 1234.5;
    //     //assetVar* StandbyTestD  =
    //     vm.setVal(vmap,"/system/status","StandbyTestD",dxval);
    //     const char* cxval ="some string";
    //     //assetVar* StandbyTestS  =
    //     vm.setVal(vmap,"/system/status","StandbyTestS",cxval);

    //     cJSON* cjxval = cJSON_Parse("{1235.6}");
    //     printf( "result of cJSON Parse {12345.6} %p\n", cjxval);
    //     cjxval = cJSON_Parse("{\"value\":1235.6}");
    //     printf( "result of cJSON Parse {\"value\":1235.6} %p\n", cjxval);
    //     const char * jxvals = cJSON_PrintUnformatted(cjxval);
    //     printf( "result of cJSON Parse {\"value\":1235.6} >>%s<<\n", jxvals);

    //     //assetVar* StandbyTestCJD  =
    //     vm.setValfromCj(vmap,"/system/status","StandbyTestCJD", cjxval);
    //     cjxval = cJSON_Parse("{\"value\":1235}");
    //     //assetVar* StandbyTestCJI  =
    //     vm.setValfromCj(vmap,"/system/status","StandbyTestCJI",cjxval);

    //     cjxval = cJSON_Parse("{\"value\":\"my text\"}");
    //     //assetVar* StandbyTestCJS  =
    //     vm.setValfromCj(vmap,"/system/status","StandbyTestCJS",cjxval);

    //     cjxval = cJSON_Parse("{\"value\":\"my text2\"}");
    //     //assetVar* StandbyTestCJSV  =
    //     vm.setValfromCj(vmap,"/system/status","StandbyTestCJS",cjxval);
    // // this is all OK
    //     const char *xsp = "{\"On\":
    //     false,\"Standby\":true,\"Idleloss\":0.1,\"Idlestr\":{\"value\":\"soc0.1\"},"
    //                           "\"Idlenum\":{\"value\":0.134},"
    //                           "\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Pd_str\":\"300.3\","
    //                           "\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}";
    //     int single = 0;
    //     vm.processMsgSetPub(vmap, "set", "/big/pub", single, xsp,
    //     nullptr);//cJSON **cjr)

    //     cJSON* cjr = nullptr;
    //     vm.processMsgSetPub(vmap, "set", "/big/pub", single, xsp,  &cjr);

    printVars(vm, vmap);

    // return 0;

    // int rc;

    // rc = vm.testRes(" Test 1", vmap
    //         ,"set"
    //         ,"/system/status"
    //         ,"{\"On\":
    //         false,\"Standby\":true,\"Idleloss\":0.1,\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}"
    //         ,"{\"On\":false,\"Standby\":true,\"Idleloss\":0.1,\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Ploss\":45,\"Phigh\":100,\"Plow\":0.01,\"Soc\":0.1}"
    //     );

    // assetVar* On       = vm.getVar(vmap,"/system/status:On", nullptr);
    // //vmap["/system/status"]["On"];

    // assetVar* Standby  = vm.getVar(vmap,"/system/status","Standby");
    // assetVar* Idleloss = vm.getVar(vmap,"/system/status","Idleloss");
    // assetVar* pesr     = vm.getVar(vmap,"/system/status","pesr");

    // // use val to force the type of the return vaue through the template class
    // //bool val;
    // printf(" On = %p, Standby = %p\n", (void *)On , (void *)Standby);

    // printf(" On = %s, standby = %s\n", On->getbVal()?"true":"false",
    // Standby->getbVal()?"true":"false");
    // //double dval;
    // //socloss := e.Idleloss + e.pesr*math.Pow(p, 2)
    // double socloss = Idleloss->getdVal() + pow(pesr->getdVal(),2);

    // printf(" socloss = %f, Idleloss = %f pesr = %f \n", socloss,
    // Idleloss->getdVal(), pesr->getdVal());
    // // if we need this back in the system
    // //way 1
    // char * tmp;
    // asprintf(&tmp, "{\"socloss\": %f}",socloss);
    // char * rtmp;
    // asprintf(&rtmp, "{\"socloss\":%s}","1190.35");
    // rc = vm.testRes(" Test 2", vmap
    //         ,"set"
    //         ,"/system/status"
    //         , tmp
    //         , rtmp
    //         );
    // free((void*)tmp);
    // free((void*)rtmp);

    // // now this will work
    // assetVar* SocLoss   =  vm.getVar(vmap,"/system/status:socloss",
    // nullptr);//vmap["/system/status"]["socloss"];

    // // giving direct access
    // // but note this bypasses all the clever stuff to manage the variable
    // // way 2
    // SocLoss->setVal(socloss+1);

    // // Way 3 will also cret the var and run through all the bells and whistles.
    // vm.setVal(vmap,"/system/status","Socloss2", socloss);
    // vm.setVal(vmap,"/system/status:Socloss3", nullptr, socloss);

    // //         ,"{\"status\":
    // \"standby\",\"soc\":100,\"active_current_setpoint\":2000 }"
    // // ,"{\"status\":\"standby\",\"soc\":100,\"active_current_setpoint\":2000}"
    // //     );

    // // rc = testRes(" Test 3", vmap
    // //         , "get"
    // //         , "/system/status"
    // //         , nullptr
    // //         ,
    // "{\"/system/status\":{\"active_current_setpoint\":{\"value\":2000},\"soc\":{\"value\":100},\"status\":{\"value\":\"standby\"}}}"
    // //     );

    // // rc = testRes(" Test 4", vmap
    // //         , "get"
    // //         , "/system/status/soc"
    // //         , nullptr
    // //         , "{\"value\":100}"
    // //     );

    // cJSON *cj = vm.getMapsCj(vmap);
    // char* res = cJSON_Print(cj);
    // rc = 0;
    // if(rc == 0)printf("vmap at end \n%s\n", res);
    // free((void *)res) ;
    // cJSON_Delete(cj);

    // // monitor M;
    // // M.configure("configs/monitor.json");

    // // delete bms_man;
    vm.clearVmap(vmap);

    return 0;
}

#if defined _MAIN
int main(int argc, char* argv[])
{
    return main_test_deadband(argc, argv);
}
#endif
