/*
 * vmap basic test
 */

#include "asset.h"
#include "chrono_utils.hpp"
#include "varMapUtils.h"

// #include <iostream>
// #include <string>
// #include <map>
// #include <vector>
// #include <cstring>
// #include <mutex>
// #include <malloc.h>
// #include <pthread.h>
// #include <cmath>

// #include <cjson/cJSON.h>
//#include <fims/libfims.h>
//#include <fims/fps_utils.h>

//#include "../src/assetFunc.cpp"

int main_test_event(int argc, char* argv[])
{
    printf("%s  am running\n", __func__);

    // // this is our main data map
    varsmap vmap;
    // // this is our map utils factory
    VarMapUtils vm;

    // double tNow = vm.get_time_dbl();

    assetVar* av = vm.makeVar(vmap, "/status/ess:myevent", nullptr, nullptr);
    asset_manager* am = new asset_manager("test");

    av->am = am;
    const char* subs[] = { "/test", "/events", "/ess" };
    int sublen = sizeof subs / sizeof subs[0];

    fims* p_fims2 = new fims();
    if (!p_fims2->Connect((char*)"eventTestRx"))
    {
        printf("Unable to connect with fims ..\n");
        return 0;
    }
    fims* p_fims = new fims();
    if (!p_fims->Connect((char*)"eventTest"))
    {
        printf("Unable to connect with fims ..\n");
        return 0;
    }

    int rc = p_fims2->Subscribe(subs, sublen);
    {
        printf(" fims subscribe result sublen %d %d ..\n", sublen, rc);
        //        return 0;
    }
    am->p_fims = p_fims;
    // int assetVar::sendEvent(const char* source, int severity, const char
    // *msg,...)
    rc = av->sendEvent("TestEvent", 2, "This a test event from [%s:%s] at time %2.3f", av->comp.c_str(),
                       av->name.c_str(), vm.get_time_dbl());
    fims_message* msg = nullptr;
    bool done = false;
    printf(" waiting for fims rc %d p_fims %p \n", rc, p_fims);
    int count = 0;
    while (!done)
    {
        msg = p_fims2->Receive_Timeout(2000);
        if (msg)
        {
            printf(" message received uri [%s] method [%s] body [%s]\n", msg->uri, msg->method,
                   msg->body ? msg->body : "No Body");
            p_fims2->free_message(msg);
        }
        else
        {
            printf(" fims timed out  count %d \n", count);
            rc = av->sendEvent("TestEvent", 2, "This a test event %d  [%s:%s] at time %2.3f", count, av->comp.c_str(),
                               av->name.c_str(), vm.get_time_dbl());

            if (count++ > 10)
                done = true;
        }
    }
    //    int rc;
    //     rc = vm.testRes(" Test 1", vmap
    //             ,"set"
    //             ,"/components/vmap_test"
    //             ,"{\"test_int\":
    //             123,\"test_float\":456.78,\"test_string\":\"some string thing\"
    //             }"
    //             ,"{\"test_int\":123,\"test_float\":456.78,\"test_string\":\"some
    //             string thing\"}"
    //         );
    //     rc = vm.testRes(" Test 1a", vmap
    //             ,"set"
    //             ,"/components/vmap_test"
    //             ,"{\"test_int\":123,\"test_float\":456.78,\"test_string2\":{\"value\":\"some
    //             string value\"}}"
    //             ,"{\"test_int\":123,\"test_float\":456.78,\"test_string2\":{\"value\":\"some
    //             string value\"}}"
    //         );

    //     rc = vm.testRes(" Test 2", vmap
    //             ,"set"
    //             ,"/system/status"
    //             ,"{\"status\":
    //             \"standby\",\"soc\":100,\"active_current_setpoint\":2000 }"
    //             ,"{\"status\":\"standby\",\"soc\":100,\"active_current_setpoint\":2000}"
    //         );

    //     rc = vm.testRes(" Test 3", vmap
    //             , "get"
    //             , "/system/status"
    //             , nullptr
    //             ,
    //             "{\"active_current_setpoint\":{\"value\":2000},\"soc\":{\"value\":100},\"status\":{\"value\":\"standby\"}}"
    //         );

    //     rc = vm.testRes(" Test 4", vmap
    //             , "get"
    //             , "/system/status/soc"
    //             , nullptr
    //             , "100"
    //         );

    //     cJSON *cj = vm.getMapsCj(vmap,nullptr,nullptr,0);
    //     char* res = cJSON_Print(cj);
    //     char*eres= (char*)"{\n"
    //         "\t\"/comp/ess\":\t{\n"
    //         "\t\t\"ess\":\t{\n"
    //         "\t\t\t\"myvar\":\t{\n"
    //         "\t\t\t\t\"value\":\ttrue\n"
    //         "\t\t\t}\n"
    //         "\t\t}\n"
    //         "\t},\n"
    //         "\t\"/components/vmap_test\":\t{\n"
    //         "\t\t\"vmap_test\":\t{\n"
    //         "\t\t\t\"test_float\":\t{\n"
    //         "\t\t\t\t\"value\":\t456.78\n"
    //         "\t\t\t},\n"
    //         "\t\t\t\"test_int\":\t{\n"
    //         "\t\t\t\t\"value\":\t123\n"
    //         "\t\t\t},\n"
    //         "\t\t\t\"test_string\":\t{\n"
    //         "\t\t\t\t\"value\":\t\"some string thing\"\n"
    //         "\t\t\t},\n"
    //         "\t\t\t\"test_string2\":\t{\n"
    //         "\t\t\t\t\"value\":\t\"some string value\"\n"
    //         "\t\t\t}\n"
    //         "\t\t}\n"
    //         "\t},\n"
    //         "\t\"/system/status\":\t{\n"
    //         "\t\t\"status\":\t{\n"
    //         "\t\t\t\"active_current_setpoint\":\t{\n"
    //         "\t\t\t\t\"value\":\t2000\n"
    //         "\t\t\t},\n"
    //         "\t\t\t\"soc\":\t{\n"
    //         "\t\t\t\t\"value\":\t100\n"
    //         "\t\t\t},\n"
    //         "\t\t\t\"status\":\t{\n"
    //         "\t\t\t\t\"value\":\t\"standby\"\n"
    //         "\t\t\t}\n"
    //         "\t\t}\n"
    //         "\t}\n"
    //         "}";

    //     //rc = 0;
    //     if(0 && rc ==1 )printf("vmap at end opts 0 tNow %2.3f\n%s\n",  tNow1,
    //     res); if(strcmp(res,eres) == 0)
    //     {
    //         printf (" Test 5.0 all OK \n");
    //     }
    //     else
    //     {
    //         printf (" res failed\n");
    //         char *spr = res;
    //         char *sper = eres;
    //         int idx = 0;
    //         while (*spr && *sper)
    //         {
    //             idx++;

    //             if (*spr == *sper)
    //             {
    //                 spr++;
    //                 sper++;
    //             }
    //             else
    //             {
    //                 printf(" failed at  idx %d\nresult >>%s<<\nexpected
    //                 >>%s<<\n", idx, spr, sper); return 0;
    //             }
    //         }
    //         return 0;
    //     }

    //     free((void *)res) ;
    //     cJSON_Delete(cj);
    //     // naked 0x0001
    //     cj = vm.getMapsCj(vmap,nullptr,nullptr,1);
    //     res = cJSON_Print(cj);
    //     rc = 0;
    //     //if(rc == 0)printf("vmap at end opts 1\n%s\n", res);
    //     eres =(char*)"{\n"
    // "\t\"/comp/ess\":\t{\n"
    // "\t\t\"ess\":\t{\n"
    // "\t\t\t\"myvar\":\ttrue\n"
    // "\t\t}\n"
    // "\t},\n\t\"/components/vmap_test\":\t{\n"
    // "\t\t\"vmap_test\":\t{\n"
    // "\t\t\t\"test_float\":\t456.78,\n"
    // "\t\t\t\"test_int\":\t123,\n"
    // "\t\t\t\"test_string\":\t\"some string thing\",\n"
    // "\t\t\t\"test_string2\":\t\"some string value\"\n"
    // "\t\t}\n"
    // "\t},\n"
    // "\t\"/system/status\":\t{\n"
    // "\t\t\"status\":\t{\n"
    // "\t\t\t\"active_current_setpoint\":\t2000,\n"
    // "\t\t\t\"soc\":\t100,\n"
    // "\t\t\t\"status\":\t\"standby\"\n"
    // "\t\t}\n"
    // "\t}\n"
    // "}";
    //     if(strcmp(res,eres) == 0)
    //     {
    //         printf (" Test 5.2 all OK \n");
    //     }
    //     else
    //     {
    //         // res 7b 0a 09 22
    //         // eres 7b 0a 09 22
    //         printf (" res failed\n");
    //         char *spr = res;
    //         char *sper = eres;
    //         int idx = 0;
    //         while (*spr && *sper)
    //         {
    //             idx++;

    //             if (*spr == *sper)
    //             {
    //                 spr++;
    //                 sper++;
    //             }
    //             else
    //             {
    //                 printf(" failed at  idx %d\n>>%s<<\n>>%s<<\n", idx, spr,
    //                 sper); return 0;
    //             }
    //         }

    //         return 0;

    //     }

    //     free((void *)res) ;
    //     cJSON_Delete(cj);

    //     cj = vm.getMapsCj(vmap,nullptr,nullptr,0x01);
    //     //rc = 0;
    //     res = cJSON_Print(cj);
    //     // individual tables 0x0010
    //     if(0)printf("vmap at end opts %04x\n%s\n", 0x10, res);
    //      eres =(char*)"{\n"
    // "\t\"/comp/ess\":\t{\n"
    // "\t\t\"ess\":\t{\n"
    // "\t\t\t\"myvar\":\ttrue\n"
    // "\t\t}\n"
    // "\t},\n\t\"/components/vmap_test\":\t{\n"
    // "\t\t\"vmap_test\":\t{\n"
    // "\t\t\t\"test_float\":\t456.78,\n"
    // "\t\t\t\"test_int\":\t123,\n"
    // "\t\t\t\"test_string\":\t\"some string thing\",\n"
    // "\t\t\t\"test_string2\":\t\"some string value\"\n"
    // "\t\t}\n"
    // "\t},\n"
    // "\t\"/system/status\":\t{\n"
    // "\t\t\"status\":\t{\n"
    // "\t\t\t\"active_current_setpoint\":\t2000,\n"
    // "\t\t\t\"soc\":\t100,\n"
    // "\t\t\t\"status\":\t\"standby\"\n"
    // "\t\t}\n"
    // "\t}\n"
    // "}";
    //     if(strcmp(res,eres) == 0)
    //     {
    //         printf (" Test 5.3 all OK \n");
    //     }
    //     else
    //     {
    //         // res 7b 0a 09 22
    //         // eres 7b 0a 09 22
    //         printf (" res failed\n");
    //         char *spr = res;
    //         char *sper = eres;
    //         int idx = 0;
    //         while (*spr && *sper)
    //         {
    //             idx++;

    //             if (*spr == *sper)
    //             {
    //                 spr++;
    //                 sper++;
    //             }
    //             else
    //             {
    //                 printf(" failed at  idx %d\nresult >>%s<<\nexpected
    //                 >>%s<<\n", idx, spr, sper); return 0;
    //             }
    //         }
    //         return 0;
    //     }

    //     free((void *)res) ;
    //     cJSON_Delete(cj);
    //     double tNow = vm.get_time_dbl();
    //     bool oldstrcmp(char*sp1, char*sp2);

    //     bool faststrcmp(char*sp1, char*sp2);

    //     for (int i = 0 ; i < 1000000; i++)
    //     {
    //         oldstrcmp((char*)" test time string this is a much longer string
    //         number 1\n",(char *)" test time string this is a much loner string
    //         number 2\n");
    //     }
    //     double tNow2 = vm.get_time_dbl();
    //     printf("\nrunning speed test for string compares tNow %2.3f, tDiff
    //     %2.3fuS\n",tNow, (tNow2-tNow)*1000000.0); tNow = vm.get_time_dbl();
    //     double tRef = vm.get_time_ref();

    //     for (int i = 0 ; i < 1000000; i++)
    //     {
    //         faststrcmp((char*)" test time old string number 1\n",(char *)" test
    //         time string number 2\n");
    //     }
    //     double tRef2 = vm.get_time_ref();

    //     tNow2 = vm.get_time_dbl();
    //     printf("\nrunning speed test for fast string compares tNow %2.3f, tDiff
    //     %2.3fuS ref_time %2.3f base_time %2.3f tRef %2.3f tRef(diff) %2.3f \n"
    //         ,tNow
    //         , (tNow2-tNow)*1000000.0
    //         , vm.ref_time
    //         , vm.base_time
    //         , tRef
    //         , (tRef2-tRef)*1000000.0
    //         );
    //     timespec c_time;
    //     clock_gettime(CLOCK_MONOTONIC, &c_time);
    //     double cdtime =(double)c_time.tv_sec;
    //     printf("\n Time stuff  tNow %2.3f, cdtime %2.3f  c_time %ld \n"
    //         , tNow
    //         , cdtime
    //         , c_time.tv_sec
    //         );
    //     // // monitor M;
    //     // // M.configure("configs/monitor.json");

    // // delete bms_man;
    delete p_fims;
    delete p_fims2;

    am->p_fims = nullptr;
    delete am;

    vm.clearVmap(vmap);

    return 0;
}

#if defined _MAIN

int main(int argc, char* argv[])
{
    return main_test_event(argc, argv);
}
#endif
