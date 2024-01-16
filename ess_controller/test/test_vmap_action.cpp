/*
 * vmap test creating an object with an action.
 * Then setting a value to trigger that actions
 * 
 * 
 */

#include "asset.h"
//#include "bms.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"


int main(int argc, char *argv[])
{
    char* res;
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory
    VarMapUtils vm;
    //bms_manager* bms_man = new bms_manager("bms_man");

    asset* bm = new asset("bms_1");
    //bms_man->setVmap(&vmap);
    //bm = bms_man->addInstance("bms_1");
    bm->setVmap(&vmap);

    printf("bms asset test OK\n");
    bm->configure("configs/test_vmap_action.json");

    cJSON* cjbm = bm->getConfig();
    res = cJSON_Print(cjbm);
    printf("Maps at beginning \n%s\n", res);
    free((void *)res) ;
    //cJSON_Delete(cjbm);


    // const char*method = "set";
    // const char*uri = "/components/vmap_test";
    // const char*body = "{\"test_int\": 123,\"test_float\":456.78,\"test_string\":\"some string thing\" }";
    // cJSON* cjr = cJSON_CreateObject();

    // //vm.processRawMsg(vmap, method, uri, nullptr, body, &cjr);

    // if(cjr) cJSON_Delete(cjr);



    // is this one bust ??
    // cJSON *cj = vm.getMapsCj(vmap);
    // res = cJSON_Print(cj);
    // printf("vmap at end \n%s\n", res);
    // free((void *)res) ;
    // cJSON_Delete(cj);

    // monitor M;
    // M.configure("configs/monitor.json");

    cJSON *cjbm2 = bm->getConfig();
    res = cJSON_Print(cjbm2);
    printf("Maps at end \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cjbm);
    cJSON_Delete(cjbm2);


    vm.clearVmap(vmap);
    //  delete bms_man should delete all instances
    //delete bms_man;
    delete bm;
    return 0;

}
