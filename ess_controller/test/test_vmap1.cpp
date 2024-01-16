/*
 * vmap basic test
 * more advance use wher we pull a reference to a data point an run some calculations on it
 * just for kicks we'll do a fims type calculation.
 * this will not use the fully named vmap access after the initial startup to get the references.
 *   
 * Here is the twins code we are going to replicate
	if e.On && !e.Standby {
		socloss := e.Idleloss + e.pesr*math.Pow(p, 2)
		soc, plimited, under, over := getIntegral(e.Soc/100, -(p+socloss)/e.Cap, dt/3600, 0, 1)
		e.Pcharge = e.Phigh
		e.Pdischarge = e.Plow
		if over {
			p = -plimited*e.Cap - socloss // limit power output for this tick to bring SOC to 100%
			e.Pcharge = 0
		} else if under {
			p = -plimited*e.Cap - socloss // limit power output for this tick to bring SOC to 0%
			e.Pdischarge = 0
		}
		// Actually update SOC
		e.Soc = soc * 100
	} else {
		p, q = 0, 0
	}
    //function calculates delta accumulation at given rate over time (deltaT).  returns new accumulated value and rate value, limited by maximum and minimum, and under/over bool flags when limited
    func getIntegral(accumulatedIn, rateIn, deltaT, minimum, maximum float64) (accumulatedOut, rateOut float64, under, over bool) {
	//initialize accumulated value, rate, and flags
	accumulatedOut = accumulatedIn + (rateIn * deltaT)
	rateOut = rateIn
	under = false
	over = false

	//if max or min exceeded, return limited accumulation, rate, and flags
	if (accumulatedOut > maximum) {
		rateOut = (maximum - accumulatedIn) / deltaT
		accumulatedOut = maximum
		over = true
	} else if (accumulatedOut < minimum) {
		rateOut = (minimum - accumulatedIn) / deltaT
		accumulatedOut = minimum
		under = true
	}
	return accumulatedOut, rateOut, under, over
}
 */


/*
varsmap is simply a typedef
We have a two level key

typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;

Thise will become unordered_maps sometime soon they are 10X faster but use 2X space.

instead of providing a complex class to contain the structure I provide a set of tools to manipulate it.
VarMapUtils
Lot of junk code in ther but I'll clean int up over the next few weeks (10/2/2020)

The other BIG key to the system is the variable structure
<comp name>:<var name>->assetVar.
In the past (like many years) I was trying to be too clever with the layering
/asset/ess/ess_1:Soc
I store the entire /asset/ess/ess_1 as a component name instead of trying to break it down

asset/ 
       ess/
           ess_1

It we need assess to all /assets 
or all /assets/ess    

typedef std::map<std::string, assetVar*> varmap;
then we'll create a structure to hold the query results
typedef std::map<std::string, std::vector<varmap*> varsdict;
But that is not needed at the moment.
I will create test_vmap2.cpp real soon with some performance measurements.

*/
#include <math.h>
#include "asset.h"
#include "varMapUtils.h"
#include "chrono_utils.hpp"

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

// int CheckAssetDisable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
// {
//     // this will filter up the comms stats to the manager
//     int ival = 0;
//     //bool bval = true;
//     double tNow = am->vm->get_time_dbl();
//     double dval = 0.0;
//     int reload = am->vm->CheckReload(vmap, amap, aname, __func__);

//     if (reload < 2)
//     {
//         //char * cval = (char *)"HeartBeat Init";
//         //setAmapAi(am,  amap,          HeartBeatState,              am->name.c_str(),      /status,       cval);
//         //int ival = 0;
//         bool bval = false;
//         //setAmapAi(am,  amap,          CheckAssetDisable,        am->name.c_str(),      /reload,       ival);
//         setAmapAi(am,  amap,          DisableCmd,               am->name.c_str(),      /controls,     ival);
//         setAmapAi(am,  amap,          EnableCmd,                am->name.c_str(),      /controls,     ival);
//         setAmapAi(am,  amap,          EnableCnt,                am->name.c_str(),      /status,     ival);
//         setAmapAi(am,  amap,          DisableCnt,               am->name.c_str(),      /status,     ival);
//         setAmapAi(am,  amap,          Enabledxx,                am->name.c_str(),      /controls,     bval);
//         setAmapAi(am,  amap,          CheckAssetInit,           am->name.c_str(),      /status,     tNow);
//         setAmapAi(am,  amap,          CheckAssetCmdRun,         am->name.c_str(),    /status,     tNow);
//         setAmapAi(am,  amap,          CheckAssetCmdRuns,        am->name.c_str(),    /status,     dval);
//         setAmapAi(am,  amap,          DisableCmdRun,            am->name.c_str(),    /status,     tNow);
//         setAmapAi(am,  amap,          EnableCmdRun,             am->name.c_str(),    /status,     tNow);
//         ival = 2; amap[__func__]->setVal(ival); 
//     }
//     //bool tval2;
//     bool tval = true;
//     amap["Enabledxx"]->setVal(tval);
//     if(amap["Enabledxx"]->getbVal())
//     {
//         printf(" Enabledxx OK : true\n");
//         tval = false; amap["Enabledxx"]->setVal(tval);
//     }
//     else
//     {
//         printf(" Enabledxx ERROR : false\n");
//     }
//     if(amap["Enabledxx"]->getbVal())
//     {
//         printf(" Enabledxx ERROR : true\n");
//         tval = false; amap["Enabledxx"]->setVal(tval);
//     }
//     else
//     {
//         printf(" Enabledxx OK : false\n");
//     }

    
//     return 0;
// }

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

int main(int argc, char *argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory

    VarMapUtils vm;
    // low level setVar , makes it and sets the value
    int ixval = 1234;

    // disabled the assetVar defs to allow -Wall on the compiler
    //assetVar* StandbyTestI  = 
    vm.setVal(vmap,"/system/status","StandbyTestI", ixval);
    
    double dxval = 1234.5;
    //assetVar* StandbyTestD  = 
    vm.setVal(vmap,"/system/status","StandbyTestD",dxval);
 // OK here   vm.clearVmap(vmap);return 0;

    //TODO 
    const char* cxval ="some string";
    //assetVar* StandbyTestS  = 
    vm.setVal(vmap,"/system/status","StandbyTestS", cxval);

    cJSON* cjxval = cJSON_Parse("{1235.6}");
    printf( "result of cJSON Parse {12345.6} %p\n", cjxval);
    cJSON_Delete(cjxval);
    
    cjxval = cJSON_Parse("{\"value\":1235.6}");
    printf( "result of cJSON Parse {\"value\":1235.6} %p\n", cjxval);
    //cJSON_Delete(cjxval);
    
    char* jxvals = cJSON_PrintUnformatted(cjxval);
    printf( "result of cJSON Parse {\"value\":1235.6} >>%s<<\n", jxvals);
    free(jxvals);
    //cJSON_Delete(cjxval);
    // OK here vm.clearVmap(vmap);return 0;

    //assetVar* StandbyTestCJD  = 
    vm.setValfromCj(vmap,"/system/status","StandbyTestCJD", cjxval);
    cJSON_Delete(cjxval);
    cjxval = cJSON_Parse("{\"value\":1235}");
    //assetVar* StandbyTestCJI  = 
    vm.setValfromCj(vmap,"/system/status","StandbyTestCJI",cjxval);
    cJSON_Delete(cjxval);
 
    cjxval = cJSON_Parse("{\"value\":\"my text\"}");
    //assetVar* StandbyTestCJS  = 
    vm.setValfromCj(vmap,"/system/status","StandbyTestCJS",cjxval);
    cJSON_Delete(cjxval);
    // OK here vm.clearVmap(vmap);return 0;

    cjxval = cJSON_Parse("{\"value\":\"my text2\"}");
    //assetVar* StandbyTestCJSV  = 
    vm.setValfromCj(vmap,"/system/status","StandbyTestCJS",cjxval);
    cJSON_Delete(cjxval);

// this is all OK
    const char *xsp = "{\"On\": false,\"Standby\":true,\"Idleloss\":0.1,\"Idlestr\":{\"value\":\"soc0.1\"},"
                          "\"Idlenum\":{\"value\":0.134},"
                          "\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Pd_str\":\"300.3\","
                          "\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}";
    int single = 0;
    vm.processMsgSetPub(vmap, "set", "/big/pub", single, xsp,  nullptr);//cJSON **cjr)
    // OK here vm.clearVmap(vmap);return 0;

    cJSON* cjr = nullptr;
    vm.processMsgSetPub(vmap, "set", "/big/pub", single, xsp,  &cjr);
    
    printVars(vm, vmap);
    if(cjr)cJSON_Delete(cjr);
    // OK here vm.clearVmap(vmap);return 0;

    //return 0;

    int rc;

    rc = vm.testRes(" Test 1", vmap
            ,"set"
            ,"/system/status"
            ,"{\"On\": false,\"Standby\":true,\"Idleloss\":0.1,\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}"
            ,"{\"On\":false,\"Standby\":true,\"Idleloss\":0.1,\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Ploss\":45,\"Phigh\":100,\"Plow\":0.01,\"Soc\":0.1}"
        );

    assetVar* On       = vm.getVar(vmap,"/system/status:On", nullptr); //vmap["/system/status"]["On"];

    assetVar* Standby  = vm.getVar(vmap,"/system/status","Standby");
    assetVar* Idleloss = vm.getVar(vmap,"/system/status","Idleloss");
    assetVar* pesr     = vm.getVar(vmap,"/system/status","pesr");
    
    // use val to force the type of the return vaue through the template class
    //bool val;
    printf(" On = %p, Standby = %p\n", (void *)On , (void *)Standby);

    printf(" On = %s, standby = %s\n", On->getbVal()?"true":"false", Standby->getbVal()?"true":"false");
    //double dval;
    //socloss := e.Idleloss + e.pesr*math.Pow(p, 2)
    double socloss = Idleloss->getdVal() + pow(pesr->getdVal(),2);

    printf(" socloss = %f, Idleloss = %f pesr = %f \n", socloss, Idleloss->getdVal(), pesr->getdVal());
    // if we need this back in the system 
    //way 1
    char * tmp;
    asprintf(&tmp, "{\"socloss\": %f}",socloss);
    char * rtmp;
    asprintf(&rtmp, "{\"socloss\":%s}","1190.35");
    rc = vm.testRes(" Test 2", vmap
            ,"set"
            ,"/system/status"
            , tmp
            , rtmp
            );
    free(tmp);
    free(rtmp);
    //OK here vm.clearVmap(vmap);return 0;
    // now this will work
    assetVar* SocLoss   =  vm.getVar(vmap,"/system/status:socloss", nullptr);//vmap["/system/status"]["socloss"];

    // giving direct access 
    // but note this bypasses all the clever stuff to manage the variable
    // way 2
    SocLoss->setVal(socloss+1);

    // Way 3 will also cret the var and run through all the bells and whistles.
    vm.setVal(vmap,"/system/status","Socloss2", socloss);
    vm.setVal(vmap,"/system/status:Socloss3", nullptr, socloss);


    //         ,"{\"status\": \"standby\",\"soc\":100,\"active_current_setpoint\":2000 }"
    //         ,"{\"status\":\"standby\",\"soc\":100,\"active_current_setpoint\":2000}"
    //     );

    // rc = testRes(" Test 3", vmap
    //         , "get"
    //         , "/system/status"
    //         , nullptr
    //         , "{\"/system/status\":{\"active_current_setpoint\":{\"value\":2000},\"soc\":{\"value\":100},\"status\":{\"value\":\"standby\"}}}"
    //     );

    // rc = testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );
    
    cJSON *cj = vm.getMapsCj(vmap);
    char* res = cJSON_Print(cj);
    rc = 0;
    if(rc == 0) printf("vmap at end \n%s\n", res);
    free((void *)res) ;
    cJSON_Delete(cj);
    // OK here vm.clearVmap(vmap);return 0;

    // monitor M;
    // M.configure("configs/monitor.json");
    
    testbval(vm, vmap);
    varmap amap;
    const char *aname= "ess1";
    //fims* p_fims = nullptr;
    asset* ass = new asset(aname);
    //am->vm = &vm;
    //am->amap = &amap;
    ass->setVmap(&vmap);
    ass->setVm(&vm);
    //ass->vm->set_base_time();
    ass->vm->setTime();
    //amap["foo"] = nullptr;
    //ass->amap["fum"] = nullptr;

    //CheckAssetDisable(vmap, ass->amap, aname, p_fims, ass);

    delete ass;
    vm.clearVmap(vmap);

    return 0;

}
