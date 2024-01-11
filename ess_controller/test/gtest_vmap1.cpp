#include <math.h>
#include "asset.h"
#include "varMapUtils.h"
#include "chrono_utils.hpp"
#include "gtest/gtest.h"

namespace flex
{
    const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
    const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}

//#include "assetFunc.cpp"
cJSON*getSchList()
{
    return nullptr;
}
void printVars(VarMapUtils &vm , varsmap &vmap)
{
    cJSON *cj = vm.getMapsCj(vmap);
    char* res = cJSON_Print(cj);
    printf("vmap at end \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);
}


class Vmap1 : public ::testing::Test {
    protected:
    virtual void SetUp(){
        av = vm.makeVar(vmap, "/comp/ess", "myvar", nullptr);
        int ixval = 1234;
        vm.setVal(vmap,"/system/status","StandbyTestI", ixval);
        double dxval = 1234.5;
        vm.setVal(vmap, "/system/status", "StandbyTestD", dxval);
        char* cxval = (char*) "some_string";
        vm.setVal(vmap, "/system/status", "StandbyTestS", cxval);
    }

    virtual void TearDown(){
        delete av->am;
    }

    varsmap vmap;
    VarMapUtils vm;
    assetVar* av;
};

TEST_F(Vmap1, vmap1test){
    
    cJSON* cjxval = cJSON_Parse("{1235.6}");
    vm.setValfromCj(vmap,"/system/status","StandbyTestCJD", cjxval);
    //EXPECT_EQ(cjxval, 1235.6);
    cJSON_Delete(cjxval);
    cjxval = cJSON_Parse("{\"value\":1235}");
    vm.setValfromCj(vmap,"/system/status","StandbyTestCJI",cjxval);
    //EXPECT_EQ(cjxval, 1235);
    cJSON_Delete(cjxval);
}

TEST_F(Vmap1, testbval){
    EXPECT_TRUE(vm.getVar(vmap,"/system/test","bvaltest")->getbVal());
    EXPECT_FALSE(vm.getVar(vmap,"/system/test","bvalOK")->getbVal());
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

bool testbval(VarMapUtils &vm , varsmap &vmap)
{
    bool OK = true;

    vm.setVal(vmap,"/system/test","bvaltest",OK);
    OK = false;
    vm.setVal(vmap,"/system/test","bvalOK",OK);

    bool bvaltest  =    vm.getVar(vmap,"/system/test","bvaltest")->getbVal();
    bool bvalOK  =      vm.getVar(vmap,"/system/test","bvalOK")->getbVal();
    if(bvaltest)
    {
        printf(" bvaltest OK : true\n");
    }
    else
    {
        printf(" bvaltest ERROR : false\n");
    }
    if(bvalOK)
    {
        printf(" bvalOK ERROR: true\n");
    }
    else
    {
        printf(" bvalOK OK : false\n");
    }

    return OK;

}

// int main(int argc, char *argv[])
// {

//     cJSON* cjxval = cJSON_Parse("{1235.6}");
//     printf( "result of cJSON Parse {12345.6} %p\n", cjxval);
//     cJSON_Delete(cjxval);
    
//     cjxval = cJSON_Parse("{\"value\":1235.6}");
//     printf( "result of cJSON Parse {\"value\":1235.6} %p\n", cjxval);
//     //cJSON_Delete(cjxval);
    
//     char* jxvals = cJSON_PrintUnformatted(cjxval);
//     printf( "result of cJSON Parse {\"value\":1235.6} >>%s<<\n", jxvals);
//     free(jxvals);
//     //cJSON_Delete(cjxval);
//     // OK here vm.clearVmap(vmap);return 0;

//     //assetVar* StandbyTestCJD  = 
//     vm.setValfromCj(vmap,"/system/status","StandbyTestCJD", cjxval);
//     cJSON_Delete(cjxval);
//     cjxval = cJSON_Parse("{\"value\":1235}");
//     //assetVar* StandbyTestCJI  = 
//     vm.setValfromCj(vmap,"/system/status","StandbyTestCJI",cjxval);
//     cJSON_Delete(cjxval);
 
//     cjxval = cJSON_Parse("{\"value\":\"my text\"}");
//     //assetVar* StandbyTestCJS  = 
//     vm.setValfromCj(vmap,"/system/status","StandbyTestCJS",cjxval);
//     cJSON_Delete(cjxval);
//     // OK here vm.clearVmap(vmap);return 0;

//     cjxval = cJSON_Parse("{\"value\":\"my text2\"}");
//     //assetVar* StandbyTestCJSV  = 
//     vm.setValfromCj(vmap,"/system/status","StandbyTestCJS",cjxval);
//     cJSON_Delete(cjxval);

// this is all OK
    // const char *xsp = "{\"On\": false,\"Standby\":true,\"Idleloss\":0.1,\"Idlestr\":{\"value\":\"soc0.1\"},"
    //                     "\"Idlenum\":{\"value\":0.134},"
    //                     "\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Pd_str\":\"300.3\","
    //                     "\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}";
    // int single = 0;
    // vm.processMsgSetPub(vmap, "set", "/big/pub", single, xsp,  nullptr);//cJSON **cjr)
    // // OK here vm.clearVmap(vmap);return 0;

    // cJSON* cjr = nullptr;
    // vm.processMsgSetPub(vmap, "set", "/big/pub", single, xsp,  &cjr);
    
    // printVars(vm, vmap);
    // if(cjr)cJSON_Delete(cjr);
    // // OK here vm.clearVmap(vmap);return 0;

    // //return 0;

    // int rc;

    // rc = vm.testRes(" Test 1", vmap
    //         ,"set"
    //         ,"/system/status"
    //         ,"{\"On\": false,\"Standby\":true,\"Idleloss\":0.1,\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}"
    //         ,"{\"On\":false,\"Standby\":true,\"Idleloss\":0.1,\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Ploss\":45,\"Phigh\":100,\"Plow\":0.01,\"Soc\":0.1}"
    //     );

    // assetVar* On       = vm.getVar(vmap,"/system/status:On", nullptr); //vmap["/system/status"]["On"];

    // assetVar* Standby  = vm.getVar(vmap,"/system/status","Standby");
    // assetVar* Idleloss = vm.getVar(vmap,"/system/status","Idleloss");
    // assetVar* pesr     = vm.getVar(vmap,"/system/status","pesr");
    
    // // use val to force the type of the return vaue through the template class
    // //bool val;
    // printf(" On = %p, Standby = %p\n", (void *)On , (void *)Standby);

    // printf(" On = %s, standby = %s\n", On->getbVal()?"true":"false", Standby->getbVal()?"true":"false");
    // //double dval;
    // //socloss := e.Idleloss + e.pesr*math.Pow(p, 2)
    // double socloss = Idleloss->getdVal() + pow(pesr->getdVal(),2);

    // printf(" socloss = %f, Idleloss = %f pesr = %f \n", socloss, Idleloss->getdVal(), pesr->getdVal());
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
    // free(tmp);
    // free(rtmp);
    // assetVar* SocLoss   =  vm.getVar(vmap,"/system/status:socloss", nullptr);//vmap["/system/status"]["socloss"];

    // // giving direct access 
    // // but note this bypasses all the clever stuff to manage the variable
    // // way 2
    // SocLoss->setVal(socloss+1);

    // // Way 3 will also cret the var and run through all the bells and whistles.
    // vm.setVal(vmap,"/system/status","Socloss2", socloss);
    // vm.setVal(vmap,"/system/status:Socloss3", nullptr, socloss);
    
    // cJSON *cj = vm.getMapsCj(vmap);
    // char* res = cJSON_Print(cj);
    // rc = 0;
    // if(rc == 0) printf("vmap at end \n%s\n", res);
    // free((void *)res) ;
    // cJSON_Delete(cj);
    
    // testbval(vm, vmap);
    // varmap amap;
    // const char *aname= "ess1";
    // asset* ass = new asset(aname);;
    // ass->setVmap(&vmap);
    // ass->setVm(&vm);
    // ass->vm->set_base_time();
    // ass->vm->setTime();

    // delete ass;
    // vm.clearVmap(vmap);

    // return 0;

//}
