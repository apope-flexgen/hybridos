#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <cjson/cJSON.h>
#include <csignal>
#include <cstring>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

#define DBL_EPSILON 2.2204460492503131e-16
#include "channel.h"
#include "newAv2.h"
#include "newUtils.h"
#include <fims/libfims.h>

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#endif

extern "C" {
typedef int (*myAvfun_t)(AssetVar* vmap, AssetVar* amap, const char* aname, fims* p_fims, AssetVar* av);
int TestFunc(AssetVar* vmap, AssetVar* amap, const char* aname, fims* p_fims, AssetVar* av);
}

int TestFunc(AssetVar* vmap, AssetVar* amap, const char* aname, fims* p_fims, AssetVar* av)
{
    cout << __func__ << " Running for av [" << av->cstring << "] " << endl;
    return 0;
}

// double base_time = 0.0;
// long int get_time_us()
// {
//     long int ltime_us;
//     timespec c_time;
//     clock_gettime(CLOCK_MONOTONIC, &c_time);
//     ltime_us = (c_time.tv_sec * 1000000) + (c_time.tv_nsec / 1000);
//     return ltime_us;
// }

// double get_time_dbl()
// {
//     if(base_time == 0.0)
//     {
//         base_time = get_time_us()/ 1000000.0;
//     }

//     return  (double)get_time_us() / 1000000.0 - base_time;
// }

// int split(vector<string> &output, const string& s, char seperator)
// {
//     int i = 0;
//     string::size_type prev_pos = 0, pos = 0;

//     while((pos = s.find(seperator, pos)) != string::npos)
//     {
//         string substring( s.substr(prev_pos, pos-prev_pos) );
// 		if(substring.size()> 0)
// 		{
//         	output.push_back(substring);
// 			i++;
// 		}
//         prev_pos = ++pos;
//     }

//     output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

//     return i;
// }

// // decode options
// // '{"a":b}'   -- simple av(a) ->value = b what ever b's type is stick it in
// a value
// // '{"a":{"value":c,"p1":d}}' populate params value p1 etc
// //
// // we loose the varsmap instead root everything from a base av.

// /* securely comparison of floating-point variables */
// bool compare_double(double a, double b)
// {
//     double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
//     return (fabs(a - b) <= maxVal * DBL_EPSILON);
// }

// // now import the real cJSON Parser
// // first into a cJSON object
// // then into out AssetVar
// // typedef struct {
// //     const unsigned char *json;
// //     size_t position;
// // } error;
// //static error global_error = { NULL, 0 };
// /* get the decimal point character of the current locale */
// static unsigned char get_decimal_point(void)
// {
// #ifdef ENABLE_LOCALES
//     struct lconv *lconv = localeconv();
//     return (unsigned char) lconv->decimal_point[0];
// #else
//     return '.';
// #endif
// }

// move from linkded list with ->next and ->prev to a parent mounted vector.
// the AssetVar cstring really means nothing it is an abstract object with one
// or more identities
//  depending on where it is used.
// for example
// /status/bms:soc
// /components/sbmu_1_status:bms_soc

// we define in the incoming json object /components/sbmu_1_status:bms_soc
// then simply SetLink(vmap, "/status/bms:soc", "/components/sbmu_1:bms_soc")
// the same object is under two names in two different parent av's
// the Av will keep its original comp name (/components/sbmu_1) and its cname (
// bms_soc) to do this we have to have the aVmap <local_name,Av*> and a vector
// aVec <pair::<Av*,local_name (char *)> the ->prev ->next stuff will be handled
// by the vector. arrays   will have null pointers for local_name
//  thing1->next(thing2)->prev(null)
//  thing2->next(null)->prev(thing1)
// add thing3
// in list1
//           thing1->next(thing2)->prev(null)
//           thing2->next(thing3)->prev(thing1)
//           thing3->next(null)->prev(thing2)
// but in list2
//
//           thing1->next(thing2  no its thing3 now huh)->prev(null)
//           thing3->next(null)->prev(thing2 no its thing 1 now )

//

// list1(has its own thingslist)
//     list1_thing1->thing1
//     list1_thing2->thing2
//     list1_thing3->thing3

// list2(also has its own thingslist)
//     list2_thing1->thing1
//     list2_thing3->thing3

// need to use a singleton here
NewUtils* vmp;

int AssetVar::av_id = 0;
int test_main2()
{
    std::vector<std::string> svec;
    // int depth  = 0;
    AssetVar avess("ess");
    cout << "creating a status object with a test child " << endl;
    cJSON_AVParse2(&avess, "{\"status\": {\"test\":123}}");
    {
        ostringstream sfoo;
        // sfoo << endl<<" hi sfoo"<<
        sfoo << avess["status"];
        cout << sfoo.str() << endl;
        // print_Avvalue(&avess, depth);
        cout << endl << endl;
    }
    // This should give test a value of 123 and a unit of 24
    cJSON_AVParse2(&avess, "{\"status\": {\"test\":{\"unit\":24}}}");
    {
        ostringstream sfoo;
        // sfoo << endl<<" hi sfoo"<<
        sfoo << avess["status"];
        cout << sfoo.str() << endl;
        // print_Avvalue(&avess, depth);
        cout << endl << endl;
    }
    return 0;
}
int test_main()
{
    std::vector<std::string> svec;
    int depth = 0;
    AssetVar avess("ess");
    cout << "creating a status object" << endl;
    avess["status"] = 123;  // todo put this into a value param
    // cout << " all ess after [] operations >>> ess" <<endl;
    {
        ostringstream sfoo;
        // sfoo << endl<<" hi sfoo"<<
        sfoo << avess["status"];
        cout << sfoo.str() << endl;
        // print_Avvalue(&avess, depth);
        cout << endl << endl;
    }

    cout << "adding other stuff as well " << endl;
    avess["status"]["ess_1"]["unit"] = 1.5;
    {
        ostringstream sfoo;
        // sfoo << endl<<" hi sfoo"<<
        sfoo << avess["status"];
        cout << sfoo.str() << endl;
        // print_Avvalue(&avess, depth);
        cout << endl << endl;
    }
    cout << "now change the original value  " << endl;
    avess["status"] = 456;
    {
        ostringstream sfoo;
        // sfoo << endl<<" hi sfoo"<<
        sfoo << avess["status"];
        cout << sfoo.str() << endl;
        // print_Avvalue(&avess, depth);
        cout << endl << endl;
    }

    // print_Avvalue(&avess, depth);
    cout << endl << endl;
    // return 0;
    ////avess["status"]["ess_1"]["voltage"] = 1300;
    ////avess["config"]["ess_1"]["max_voltage"] = 1500;
    cout << endl << endl;

    cout << " all ess after [] operations >>> ess" << endl;
    // print_Avvalue(&avess, depth);
    cout << endl << endl;

    avess["status"]["ess_1"]["unit"]["name"] = "This is my name";
    cout << " all ess after [] adding name >>> ess" << endl;
    // print_Avvalue(&avess, depth);
    cout << endl << endl;
    // close but no cigar
    // avess["array"][0] = 21;
    cout << " all ess after [] adding array >>> ess" << endl;
    // print_Avvalue(&avess, depth);
    cout << endl << endl;
    // return 0;
    // cout << "show ess/status/ess" <<endl;

    ////print_Avvalue(&avess["status"]["ess_1"], depth);
    // cout << endl << endl;

    const char* sp2 =
        "{\"foo\":{\"value2\":22},\"foo3\":{\"value3\":33}"
        ",\"foo4\":{\"myval2\":{\"extra\":345}}}";
    // const char* sp2 = "{\"foo\":22}";
    // ,\"foo3\":{\"value3\":33}"
    //         ",\"foo4\":{\"myval2\":{\"extra\":345}}}";

    cout << " test string [" << sp2 << "]" << endl << endl;
    cout << " trying << operator" << endl << endl;

    avess << sp2;  // this is broken
    cout << " Test done " << endl;
    // return 0;
    cout << " Value from print_Avvalue   ##########" << endl;
    // print_Avvalue(&avess, depth);
    cout << " Done Value from print_Avvalue   ##########" << endl;

    // return 0;
    ostringstream sfoo, sout;
    // sfoo << endl<<" hi sfoo"<<
    sfoo << avess["status"];
    cout << sfoo.str() << endl;
    cout << " Value from [status] to sout   ##########" << endl;
    // needs comma's
    stream_Avvalue(sout, &avess["status"], depth);
    cout << sout.str() << endl;

    // return 0;
    // these are not in the delete list.
    AssetVar avint("intAv");
    AssetVar avdbl("dblAv");
    AssetVar avtest("testAv");
    // AssetVar avkid("Avkid", &avtest );

    avint = 22;
    // adds to vectors (old Params)
    avint["MaxVal"] = 25;
    avint["MinVal"] = 15;

    avdbl = 22.3;
    avint.show();
    avint["MaxVal"].show();
    avdbl.show();
    // avtest["one"]["two"]["three"] =  3;   //deep param map
    // avtest["one"]["two"]["three"].show();
    // avtest.find("/one/two")  will split string and cycle through vecs
    // avtest["one"]["two"].showKids();
    // avtest.showKids();
    split(svec, "/one/two/three", '/');
    for (auto s : svec)
    {
        cout << "[" << s << "]" << endl;
    }
    svec.clear();
    const char* sp =
        "{\"foo\":{\"fvalue\":21,\"myval\":100,\"actions\":{"
        "\"onSet\":[{\"enum\":["
        "{\"eone\":1, \"two_1\":2, \"three_1\":\"sthree\"},"
        "{\"etwo\":1, \"two_2\":2, \"three_2\":\"sthree\"}"
        "]}]},\"value\":234}}";

    // as you can see, you no longer have to type "\" for escape characters
    // anymore, it is also easier to read. format is as follows: R"(everything
    // inside these paranthesis is your raw string)";
    const char* raw = R"(
        {
            "this": "is", 
            "a": "json", 
            "raw strings": "are cool"
        }
    )";
    cout << raw << endl;

    //"]}]}}}";
    double tNow = get_time_dbl();

    AssetVar avmap("avMap");
    avmap.setType("Vmap");
    cJSON* cj = cJSON_Parse(sp);
    // decodeCJ(&avmap, cj);
    double tCJ = get_time_dbl() - tNow;

    avmap.show();

    tNow = get_time_dbl();
    AssetVar avmap2("avMap2");
    avmap2.setType("Vmap");

    // decodeStr(&avmap2,sp);
    double tSTR = get_time_dbl() - tNow;
    avmap2.show();
    // cout << " looking for foo "<< avmap2.getParam(0)->gotParam("foo") << endl;
    // cout << " looking for foo :actions "<<
    // avmap2.getParam(0)->gotParam("foo")->gotParam("actions") << endl;

    cout << " Parse Results CJ :[" << tCJ * 1000000.0 << "] STR :[" << tSTR * 1000000.0 << "]" << endl;

    char* tmp = cJSON_Print(cj);
    cout << tmp << endl;
    cout << " our own CJ Tree " << endl;
    // printCJTree(cj);
    // int depth = 0;

    print_CJvalue(cj, depth);
    cout << " our own CJ Parser " << endl;
    tNow = get_time_dbl();
    cJSON* cj2 = cJSON_CJParse(sp);
    cout << " our own CJ Parser Output" << endl;
    depth = 0;
    print_CJvalue(cj2, depth);
    tCJ = get_time_dbl() - tNow;

    cout << "Now do our own AVParser " << endl << endl;

    cout << endl;
    cout << " our own Av Parser original sp" << endl;
    tNow = get_time_dbl();
    AssetVar* vmap = cJSON_AVParse(sp);
    cout << " ###########################our own Av Parser after original sp" << endl;

    // print_Avvalue(vmap, depth);
    tSTR = get_time_dbl() - tNow;
    cout << " Parse Results CJ :[" << tCJ * 1000000.0 << "] Av :[" << tSTR * 1000000.0 << "]" << endl;
    cout << " now put in the extra stuff " << endl;
    cJSON_AVParse2(vmap, sp2);
    depth = 0;
    cout << " our own Av Parser extended " << endl;
    // print_Avvalue(vmap, depth);

    setValue(vmap, "foo", "value4", " this is value 4");
    setValue(vmap, "foo", "double4", 123.4);
    setValue(vmap, "foo", "bool5f", false);
    setValue(vmap, "foo", "bool5t", true);
    setValue(vmap, "foo", "value", 1023.4);
    cout << " after setValue" << endl;
    // print_Avvalue(vmap, depth);
    cout << "AVtest" << endl;
    depth = 0;
    setValue(&avtest, "test", "value4", " this is value 4");
    setValue(&avtest, "test", "valueint", 21);
    // print_Avvalue(&avtest, depth);

    avtest["one"]["two"]["three"].show();
    cout << endl;
    cout << "AVarMap 1 at End " << endl;
    AssetVar::showMap();

    cout << "AVarMap 2 at End " << endl;
    for (auto x : AVarMap)
    {
        cout << x.first;
        if (x.second)
        {
            cout << " id : " << x.first->id;
            if (x.first->cstring)
            {
                cout << " cstring [" << x.first->cstring << "]";
            }
            else
            {
                cout << " name [" << x.first->name << "]";
            }
        }
        else
        {
            cout << " ... deleted";
        }
        cout << endl;
    }
    AssetVar* avFoo = vmap->gotParam("foo");
    cout << " AvFoo == " << avFoo << endl;
    for (auto x : avFoo->aList)
    {
        if (x.second)
        {
            cout << " ID :" << x.first->id << " myName :" << x.second << endl;
        }
        else
        {
            cout << " ID :" << x.first->id << " NULL :" << endl;
        }
    }
    cout << "########################" << endl;
    ostringstream sfoo2, sfoo3;
    AssetVar vm2("lroot");
    vm2["lbase"]["value21"] = 21;
    vm2["lbase"]["value22"] = "new value 22";
    vm2["lbase"]["value23"] = "new value 23";
    // vm2["lbase"]["value23"].fixupAlist();
    sfoo2 << endl << " hi sfoo2" << endl;
    depth = 0;
    stream_Avvalue(sfoo2, &vm2, depth);
    // sfoo2<< vm2;
    cout << sfoo2.str() << endl;
    cout << "###########create link a[xx]=b[yy] #############" << endl;

    // this is one way to create a link
    vm2["lbase"]["value21"] = vm2["lbase"]["value23"];
    depth = 0;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;

    cout << "###########vm.make_link(\"/a/b/c:d\",\"/x/y:z\"); #############" << endl;
    // this another way to create a link
    vm2.makeLink("/control/pcs/powerDemand", "/components/pcs/fast/statusPower");
    depth = 0;

    // this is how we clear an ostringstream
    sfoo3.str(string());
    sfoo3.clear();
    // ostringstream sfoo4;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;

    cout << "###########check link(\"/a/b/c:d\",\"/x/y:z\"); #############" << endl;
    // vm2["components"]["pcs"]["fast"]["statusPower"] = "a lot of power";
    vm2.setVal("/components/pcs/fast/statusPower/value", "a lot of power here");

    sfoo3.str(string());
    sfoo3.clear();
    // ostringstream sfoo4;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;
    // OK add an action for statusPower
    vm2.addAction("/components/pcs/fast/statusPower", "onSet", "enum",
                  "{\"iValue\": 21 , \"outValue\":\"The value is 21\"}");
    vm2.addAction("/components/pcs/fast/statusPower", "onSet", "enum",
                  "{\"iValue\": 22 , \"outValue\":\"The value is 22\"}");
    vm2.addAction("/components/pcs/fast/statusPower", "onSet", "remap", "{\"uri\":\"/components/bcs/outThere\"}");
    vm2.setVal("/components/pcs/fast/statusPower/value", "less is more");

    vm2.setVal("/components/pcs/test/TestValues:val1@myParam45", 45);

    sfoo3.str(string());
    sfoo3.clear();
    // ostringstream sfoo4;
    stream_Avvalue(sfoo3, &vm2, depth);
    cout << sfoo3.str() << endl;
    cout << endl;
    // depth = 0;
    // print_CJvalue(vm2, depth);
    cout << "########################" << endl;
    return 0;
}

volatile int run;

channel<int> wakeChan;
// channel <schedItem*>reqChan; // anyone can post run_reqs
channel<AssetVar*> reqChan;       // anyone can post run_reqs
channel<fims_message*> fimsChan;  // anyone can post run_reqs
channel<char*> msgChan;           // anyone can post run_reqs

// this may go into an assetvar aList
// typedef vector<AssetVar*>schlist;

#define OPT_NAKED 1 << 1
#define OPT_FULL 1 << 2
#define OPT_MAKE 1 << 3

void signal_handler(int sig)
{
    run = 0;
    // FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    wakeChan.put(-1);
    signal(sig, SIG_DFL);
}

// add a schedItem into the list of reqs at the proper slot
int addSchedReq(alist* sreqs, AssetVar* av);

int runAvFunc(AssetVar* vmap, AssetVar* sv, AssetVar* av, fims* p_fims)
{
    int ret = 0;

    if (!av->gotParam("enabled") || av->getbParam("enabled"))
    {
        double tNow = get_time_dbl();
        const char* myfcn = av->getcParam("fcn");
        const char* aname = av->getcParam("aname");

        if (1)
            FPS_ERROR_PRINT("%s >> Running one at %2.6f name [%s] func %s \n", __func__, tNow, av->cstring,
                            myfcn ? myfcn : "No FCN");

        if (myfcn && aname)
        {
            void* res1 = getFunc(vmap, aname, myfcn);
            // extern "C" typedef int (*myAvfun_t)(varsmap &vmap, varmap &amap, const
            // char* aname, fims* p_fims, assetVar* av);

            if (res1)
            {
                myAvfun_t amFunc = reinterpret_cast<myAvfun_t>(res1);
                ret = amFunc(vmap, nullptr, aname, p_fims, av);
            }
        }
    }
    return ret;
}

double getSchedDelay(AssetVar* vmap, alist* rreqs, fims* p_fims);

const char* CheckUri(AssetVar* av, int& options, const char* uri)
{
    const char* sp = uri;
    if (strncmp(uri, "/ess", strlen("/ess")) == 0)
    {
        sp += strlen("/ess");
    }
    if (strncmp(uri, "/naked", strlen("/named")) == 0)
    {
        sp += strlen("/naked");
        options |= OPT_NAKED;
    }
    if (strncmp(uri, "/make", strlen("/make")) == 0)
    {
        sp += strlen("/make");
        options |= OPT_MAKE;
    }
    if (strncmp(uri, "/full", strlen("/full")) == 0)
    {
        sp += strlen("/full");
        options |= OPT_FULL;
    }

    return sp;
}

void runFimsMsg(AssetVar* av, fims_message* msg, fims* p_fims)
{
    cout << __func__ << " method [" << msg->method << "]" << endl;
    cout << __func__ << " orig uri [" << msg->uri << "]" << endl;
    int options = 0;
    const char* uri = CheckUri(av, options, msg->uri);
    cout << __func__ << " used uri [" << uri << "]" << endl;
    if ((strcmp(msg->method, "set") == 0) && msg->body)
    {
        cout << __func__ << " body [" << msg->body << "]" << endl;
        // av << msg->body;
        AssetVar* av2 = av->getAv(uri, options);
        cJSON_AVParse2(av2, msg->body);
        if (msg->replyto)
            p_fims->Send("set", msg->replyto, NULL, msg->body);
    }
    else if ((strcmp(msg->method, "get") == 0) && msg->replyto)
    {
        int depth = 0;
        AssetVar* av2 = av->getAv(uri, options);  // todo add options make:full:naked
        cout << __func__ << " recovered av [" << av2 << "]" << endl;
        ostringstream oss;
        stream_Avvalue(oss, av2, depth);
        cout << __func__ << oss.str();
        p_fims->Send("set", msg->replyto, NULL, oss.str().c_str());
    }
    else if ((strcmp(msg->method, "run") == 0) && msg->replyto)
    {
        int depth = 0;
        AssetVar* av2 = av->getAv(uri, options);  // todo add options make:full:naked
        cout << __func__ << " recovered av [" << av2 << "]" << endl;

        // reqChan.put(av2);
        // wakeChan.put(21/*tick++*/);   // but this did not get serviced immediatey
        AssetVar* sv = nullptr;
        runAvFunc(av, sv, av2, p_fims);
        ostringstream oss;
        stream_Avvalue(oss, av2, depth);
        cout << __func__ << oss.str();
        p_fims->Send("set", msg->replyto, NULL, oss.str().c_str());
    }
    else if ((strcmp(msg->method, "sched") == 0) && msg->replyto)
    {
        int depth = 0;
        AssetVar* av2 = av->getAv(uri, options);  // todo add options make:full:naked
        cout << __func__ << " recovered av [" << av2 << "]" << endl;

        reqChan.put(av2);
        wakeChan.put(21 /*tick++*/);  // but this did not get serviced immediatey
        // AssetVar* sv = nullptr;
        // runAvFunc(av, sv, av2, p_fims);
        // reqChan.put(av);
        // wakeChan.put(2);
        ostringstream oss;
        stream_Avvalue(oss, av2, depth);
        cout << __func__ << oss.str();
        p_fims->Send("set", msg->replyto, NULL, oss.str().c_str());
    }
}

// int scheduler::runChans(varsmap &vmap, schlist &rreqs, asset_manager* am)
// void schedThread(scheduler*sc, varsmap *vmap,  schlist *rreqs, asset_manager*
// am, fims* p_fims)
void schedThread(AssetVar* av, alist* sreqs, const char* aname, fims* p_fims);

void fimsThread(/*scheduler *sc,*/ fims* p_fims)
{
    double tNow = get_time_dbl();
    int tick = 0;
    fims_message* msg = p_fims->Receive_Timeout(100);
    while (run)
    {
        msg = p_fims->Receive_Timeout(1000000);
        if (1)
        {
            // just for testing
            tNow = get_time_dbl();
            if (0)
                FPS_ERROR_PRINT("%s >>Fims Tick %d msg %p p_fims %p time %2.3f\n", __func__, tick, msg, p_fims, tNow);
            tick++;
        }
        if (msg)
        {
            cout << __func__ << " we got a message " << endl;
            // if (strcmp(msg->method, "get") == 0)
            // {
            //     if (1)FPS_ERROR_PRINT("%s >> %2.3f GET uri  [%s]\n"
            //         , __func__, sc->vm->get_time_dbl(),
            //         msg->uri);
            // }
            if (run)
            {
                fimsChan.put(msg);
                wakeChan.put(tick);  // but this did not get serviced immediatey
            }
            else
            {
                if (p_fims)
                    p_fims->free_message(msg);
                // p_fims->delete
            }
        }
    }
    // AssetVar aV;
    tNow = get_time_dbl();
    // TODO
    // aV.sendEvent("GPIO_CONTROLLER", p_fims,  Severity::Info, "Gpio controller
    // shutting down at %2.3f", tNow);
    FPS_ERROR_PRINT("%s >> fims shutting down\n", __func__);
    // if(p_fims)delete p_fims;
    // p_fims = NULL;
}

#include <poll.h>
// we may need to do this in the fims thread !!otherwise gdb fails
fims* /*scheduler::*/ fimsSetup(const char* subs[], int sublen, const char* name, int argc)
{
    // int sublen = sizeof subs / sizeof subs[0];
    FPS_ERROR_PRINT(" %s >> FIMSSETUP  size of subs = %d sub1 [%s] sub2[%s]\n", __func__, sublen, subs[0], subs[1]);
    int attempt = 0;
    fims* p_fims = new fims();

    while (!p_fims->Connect((char*)name))
    {
        if (attempt == 0)
        {
            if (argc != 2)
                system("/usr/local/bin/fims/fims_server&");
        }
        else
        {
            poll(NULL, 0, 1000);
        }
        FPS_ERROR_PRINT("%s >> name %s waiting to connect to FIMS,attempt %d\n", __func__, name, ++attempt);
    }
    FPS_ERROR_PRINT("%s >> name %s connected to FIMS at attempt %d\n", __func__, name, attempt);

    bool subok = p_fims->Subscribe(subs, sublen);
    bool conok = p_fims->Connected();
    FPS_ERROR_PRINT("%s >> name %s subscribed to FIMS [%s] connected [%s]\n", __func__, name, subok ? "true" : "false",
                    conok ? "true" : "false");

    return p_fims;
}

alist schreqs;

int main(int argc, char* argv[])
{
    AvarMap::get().Map;
    // AvarMap::get().av_id;
    cout << " av_id is :" << AssetVar::av_id << endl;
    //    av_id = 21;
    run = 1;
    NewUtils vm;
    vmp = &vm;
    if (argc > 1)
    {
        if (argc > 2)
        {
            test_main2();
        }
        else
        {
            test_main();
        }
        return 0;
    }
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    fims* p_fims = nullptr;
    const char* subs[] = { "/ess", "/components", "/site", "/assets" };
    int sublen = sizeof(subs) / sizeof(subs[0]);

    p_fims = fimsSetup(subs, sublen, "Av2", 1);
    AssetVar* aV = new AssetVar("vmap");

    // void* getFunc(AssetVar* av, const char* aname, const char* fname);
    // AssetVar* setFunc(AssetVar* av, const char* aname, const char* fname, void*
    // func);
    setFunc(aV, "ess", "TestFunc", (void*)&TestFunc);

    thread fThread(fimsThread, /*&sched,*/ p_fims);
    thread sThread(schedThread, aV, &schreqs, "ess", p_fims);

    if (fThread.joinable())
        fThread.join();
    FPS_ERROR_PRINT("%s >> fims thread done .... \n", __func__);
    if (sThread.joinable())
        sThread.join();
    FPS_ERROR_PRINT("%s >> sched thread done  .... \n", __func__);

    return 0;
}