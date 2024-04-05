#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#define closesocket close

#include "asset.h"
#include <csignal>
#include <fims/libfims.h>

#include "channel.h"
#include "scheduler.h"
#include "varMapUtils.h"

#include "ESSLogger.hpp"
#include "chrono_utils.hpp"

// things planned 06052021
// find out why amaps are blown away with load pcs
//
// socket list  socket_192.168.112.3:1234
//          convert db thing to use this and just send to the fd.

// show sysvec
// fix ifchanged

// thread list
//    create thread to service socket.
/// avec aval perhaps  looking good..

// CalculateVar use sysname..

// ifchanged .. at the end of the whole action

// question ? how do we enable the checkuri bypass to be "gpio" and not "ess"
// answer set up vm.uriroot

// Use this to trigger a pub
// /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/control/gpio/getPubs
// true

// use this to enter sim mode.
// /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/config/GPIOsim 1
// trigger a sim value

// use this to change the pub rate
// /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/sched/gpio
// '{"schedGpioRwPub":{"value":"schedGpioRwPub","repTime":20}}'

asset_manager* flex_man = nullptr;
int run_secs = 0;
// volatile
int running = 1;
int debug = 0;

const char* flexPack = "flexPack";
const char* flexName = "flex";

void signal_handler(int sig)
{
    running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    if (flex_man)
    {
        flex_man->running = 0;
        if (flex_man->wakeChan)
            flex_man->wakeChan->put(-1);
    }
    signal(sig, SIG_DFL);
}

// this is the full ess controller under new management
// functions
extern "C++" {
int TestTriggerFunc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandleSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// int AddSchedItem(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, assetVar* av);
int EssSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every100mSP1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every100mSP2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every100mSP3(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int Every1000mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SlowPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
int HandlePowerLimit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandleSchedLoad(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SetupGpioSched(scheduler* sched, asset_manager* am);
int RunSched(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunMonitor(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int StopSched(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunVLinks(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunLinks(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int MakeLink(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunTpl(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int RunPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int DumpConfig(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int LoadConfig(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int LoadServer(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int LoadClient(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int LoadFuncs(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
void SendPub(asset_manager* am, const char* uri, const char* puri);
int checkAv(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
VarMapUtils* getVm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int RunSystemCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int process_sys_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int CalculateVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int RunCell(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SetupRwFlexSched(scheduler* sched, asset_manager* am);
int FlexInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int CheckTableVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int MathMovAvg(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SimHandlePcs(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SimHandleBms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int FastPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SendDb(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

// int checkAv(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     int rc = 0;
//     asset_manager* am = aV->am;
//     //int debug = 1;

//     if(!am)
//     {
//         FPS_ERROR_PRINT("%s >> Error no am for  [%s] \n"
//         , __func__
//         , aV->getfName()
//         );
//         return rc;
//     }
//     if(!am->vm)
//     {
//         FPS_ERROR_PRINT("%s >> Error no am->vm for  [%s] \n"
//         , __func__
//         , aV->getfName()
//         );
//         return rc;

//     }

//     if(!am->vmap)
//     {
//         FPS_ERROR_PRINT("%s >> Setting am->vmap for  [%s] value [%s]\n"
//         , __func__
//         , aV->getfName()
//         , aV->getcVal()
//         );
//         am->vmap = &vmap;
//     }
//     rc = 1;
//     return rc;
// }

// int LoadConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     if(!checkAv(vmap,amap, aname,p_fims,aV))
//     {
//         return -1;
//     }

//     asset_manager* am = aV->am;
//     if(aV->gotParam("config"))
//     {
//         char* cfgname = am->vm->getFname(aV->getcParam("config"));

//         FPS_ERROR_PRINT("%s >> run config for  [%s]  file [%s]\n"
//             , __func__
//             , aV->getfName()
//             , cfgname
//             );
//         // no reps here
//         am->vm->configure_vmap(vmap, cfgname, nullptr, am);
//         free(cfgname); // LEAK
//     }
//     return 0;
// }

// int DumpConfig(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     if(!checkAv(vmap, amap, aname,p_fims,aV))
//     {
//         return -1;
//     }
//     char* outFile = nullptr;
//     char* table = nullptr;

//     int vmlen  = 0;
//     asset_manager* am = aV->am;
//     if(aV->gotParam("table"))
//     {
//         table = am->vm->getFname(aV->getcParam("table"));
//     }
//     if(aV->gotParam("outFile"))
//     {
//         outFile = aV->getcParam("outFile");

//         char* fname = nullptr;
//         vmlen = asprintf(&fname,"%s%s_%s_dump.json"
//             , am->vm->runCfgDir?am->vm->runCfgDir:"run_configs"
//             , am->vm->uriroot?am->vm->uriroot:"flex"
//             , outFile
//             );
//         // this a test for our config with links
//         cJSON* cjbm = am->vm->getMapsCj(vmap, table, nullptr, 0x10000);
//         if(cjbm)
//         {
//             am->vm->write_cjson(fname, cjbm);
//             cJSON_Delete(cjbm);
//         }
//         free(fname);

//         FPS_ERROR_PRINT("%s >> dump config  av [%s] table [%s]  file [%s] len
//         [%d]\n"
//             , __func__
//             , aV->getfName()
//             , table?table:"none"
//             , outFile
//             , vmlen
//             );
//     }
//     return 0;
// }

// int LoadClient(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {

//     FPS_ERROR_PRINT("%s >> run client for  [%s]\n"
//         , __func__
//         , aV->getfName()
//     );

//     if(!checkAv(vmap,amap, aname,p_fims,aV))
//     {
//         return -1;
//     }

//     asset_manager* am = aV->am;
//     if(aV->gotParam("client"))
//     {
//         char* cfgname = am->vm->getFname(aV->getcParam("client"));
//         //cJSON* cj = am->vm->get_cjson(cfgname);

//         FPS_ERROR_PRINT("%s >> run site config for  [%s]  file [%s] cfgname
//         [%s]\n"
//             , __func__
//             , aV->getfName()
//             , aV->getcParam("client")
//             , cfgname
//             );

//         // this is for the site interface
//         am->vm->loadClientMap(vmap, cfgname);
//         free(cfgname); // LEAK
//     }
//     return 0;
// }

// int LoadServer(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     if(!checkAv(vmap,amap, aname,p_fims,aV))
//     {
//         return -1;
//     }

//     asset_manager* am = aV->am;
//     if(aV->gotParam("server"))
//     {
//         char* cfgname = am->vm->getFname(aV->getcParam("server"));
//         cJSON* cj = am->vm->get_cjson(cfgname);

//         FPS_ERROR_PRINT("%s >> run site config for  [%s]  file [%s] cj %p\n"
//             , __func__
//             , aV->getfName()
//             , cfgname
//             , cj
//             );
//         // this is for the site interface
//         if(cj)am->vm->loadSiteMap(vmap, cj);
//         free(cfgname); // LEAK
//         if(cj)cJSON_Delete(cj);
//     }
//     return 0;
// }
// Very rough , needs to use channels
// Get a time stamp in nanoseconds.
#include <time.h>
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec)*1000000000)

// uint64_t nanos()
// {
//     struct timespec ts;
//     timespec_get(&ts, TIME_UTC);
//     uint64_t ns = SEC_TO_NS((uint64_t)ts.tv_sec) + (uint64_t)ts.tv_nsec;
//     return ns;
// }

// int PostHtml(varsmap &vmap, VarMapUtils*vm, char*host, int port,  std::string
// hdr, std::string msg)
// {
//     std::string response;
//     struct iovec iv[2];
//     struct sockaddr_in addr;
//     int sock, len = 0 ,ret_code = 0;//, content_length = 0, len = 0;
//     sock = vm->findSocket(vmap,host,port);
//     if(sock < 0)
//     {
//         //char ch;
//         //unsigned char chunked = 0;

//         addr.sin_family = AF_INET;
//         addr.sin_port = htons(port);
//         if((addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
//         {
//             return -1;
//         }
//         if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2;

//         if(connect(sock, (struct sockaddr*)(&addr), sizeof(addr)) < 0) {
//             closesocket(sock);
//             return -3;
//         }
//         vm->setSocket(vmap,host,port, sock);
//     }

//     iv[0].iov_base = (void *)hdr.c_str();
//     iv[0].iov_len = hdr.size();
//     iv[1].iov_base = (void *)msg.c_str();
//     iv[1].iov_len = msg.size();

//     if(writev(sock, iv, 2) < (int)(iv[0].iov_len + iv[1].iov_len))
//     {
//         ret_code = -6;
//     }
//     else
//     {
//         response.resize(len = 0x100);

//         iv[0].iov_len = len;

//         // TODO get a proper response
//         iv[0].iov_len = recv(sock, &response[0], response.length(), len = 0);
//         //     == size_t(-1))
//         FPS_ERROR_PRINT(" %s >> received %d chars [%s] \n",
//         __func__, (int)iv[0].iov_len, response.c_str());
//     }
//     //closesocket(sock);

//     return ret_code;
// }
// //
// // TODO look for a DBthread struct basd on av name
// // if we dont have one then create one and start the thread (if it is not
// running)
// // this will have two channels wake and message
// // the thread will also be able to punt to the incoming fims channel
// // the thread will open the connection  and listen for incoming messages.
// // the PostHtml will write to the out channel and wake up the thread
// // incoming messages should just be OK
// // we may need a create db
// // the global "run" flag will be used to terminate the thread.

// //
// //
// int RunDb(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     asset_manager* am = aV->am;
//     VarMapUtils* vm = am->vm;
//     int debug = 1;

//     if(!checkAv(vmap, amap, aname,p_fims,aV))
//     {
//         return -1;
//     }
//     char* host = (char*) "127.0.0.1";
//     int port = 8086;
//     if(aV->gotParam("host"))
//     {
//         host = aV->getcParam("host");
//     }

//     if(aV->gotParam("port"))
//     {
//         port = aV->getiParam("port");
//     }
//     if(!aV->gotParam("host"))
//     {
//         FPS_ERROR_PRINT("%s >> Please provide table param for [%s]\n"
//         , __func__
//         , aV->getfName()
//         );
//         return 0;
//     }
//     if(!aV->gotParam("table"))
//     {
//         FPS_ERROR_PRINT("%s >> Please provide table param for [%s]\n"
//         , __func__
//         , aV->getfName()
//         );
//         return 0;
//     }
//     if(!aV->gotParam("db"))
//     {
//         FPS_ERROR_PRINT("%s >> Please provide db param for [%s]\n"
//         , __func__
//         , aV->getfName()
//         );
//         return 0;
//     }
//     if(!aV->gotParam("measure"))
//     {
//         FPS_ERROR_PRINT("%s >> Please provide measure for [%s]\n"
//         , __func__
//         , aV->getfName()
//         );
//         return 0;
//     }

//     char *db = aV->getcParam("db");
//     char *table = aV->getcParam("table");
//     char *measure = aV->getcParam("measure");
//     essPerf ePerf(am, "flex_influx", table, nullptr);

//     if(debug)FPS_ERROR_PRINT("%s >> Processing [%s] publishing db [%s] table
//     [%s] as [%s]\n"
//         , __func__
//         , aV->getfName()
//         , db
//         , table
//         , measure
//         );
//     //create the string
//     std::stringstream msg;
//     if(vmap.find(table)!= vmap.end())
//     {
//         msg << measure;
//         msg << ",mytag=1";
//         bool first = true;
//         for (auto x: vmap[table])
//         {
//             assetVar *av = x.second;
//             if(first)
//             {
//                 first = false;
//                 msg << " ";
//             }
//             else
//             {
//                msg << ",";
//             }
//             msg << av->name;
//             msg << "=";

//             switch (av->type)
//             {
//                 case assetVar::ASTRING:
//                     msg << "\"" <<av->getcVal()<<"\"" ;
//                     break;
//                 case  assetVar::AINT:
//                 case  assetVar::AFLOAT:
//                     msg << av->getiVal();
//                     break;

//                 case  assetVar::ABOOL:
//                     if (av->getbVal())
//                     {
//                         msg << "true";
//                     }
//                     else
//                     {
//                         msg << "false";
//                     }
//                     break;
//                 default:
//                     msg << "Undef";
//                     break;

//             }

//         }
//         msg << " " <<nanos();

//     }
//     std::stringstream hdr;
//     hdr << "POST http://localhost:8086/write?";
//     hdr << "db="<<db;
//     hdr << "&p=s";
//     hdr << " HTTP/1.1\r\n";
//     hdr << "Host: localhost:8086 \r\n";
//     hdr << "Content-Type: application/x-www-form-urlencoded\r\n";
//     hdr << "Accept: */*\r\n";
//     hdr << "Content-Length: "<< msg.str().size()<< "\r\n\r\n";

//     //POST "http://localhost:8086/write?db=mydb&precision=s"
//     //%s /%s?db=%s&u=%s&p=%s&epoch=%s%s HTTP/1.1\r\nHost:
//     %s\r\nContent-Length: %d\r\n\r\n",
//     //               method, uri, si.db_.c_str(), si.usr_.c_str(),
//     si.pwd_.c_str(), si.precision_.c_str(), FPS_ERROR_PRINT("%s >> created
//     hdr [%s]\n", __func__, hdr.str().c_str());
//     FPS_ERROR_PRINT("%s >> created message [%s]\n",
//     __func__, msg.str().c_str()); PostHtml(vmap, vm,
//     host, port, hdr.str(), msg.str());

//     //SendPub(am, table, sendas);
//     return 0;
// }
// //
// int RunPub(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     asset_manager* am = aV->am;
//     int debug = 1;
//     if(!checkAv(vmap, amap, aname,p_fims,aV))
//     {
//         return -1;
//     }

//     if(!aV->gotParam("table"))
//     {
//         FPS_ERROR_PRINT("%s >> Please provide table param for   [%s]\n"
//         , __func__
//         , aV->getfName()
//         );
//         return 0;
//     }

//     char *table = aV->getcParam("table");
//     essPerf ePerf(am, "flex_pub", table, nullptr);
//     char *sendas = table;
//     if(aV->gotParam("sendas"))
//     {
//         sendas = aV->getcParam("sendas");
//     }
//     if(debug)FPS_ERROR_PRINT("%s >> Processing [%s] publishing table [%s] as
//     [%s]\n"
//         , __func__
//         , aV->getfName()
//         , table
//         , sendas
//         );
//     SendPub(am, table, sendas);
//     return 0;
// }

// we can run vlinks for  a specific aname or for the whole thing.
// with FlexPack the vlinks are run on demand.
// we trigger the demand atfer loading any configs.
//  how do we specfy where .. worry about that later for now do the whole lot
// int RunVLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     essPerf ePerf(aV->am, "flex_system", "vlinks", nullptr);
//     bool mkfrom = false;
//     bool mkto = false;

//     if(aV->gotParam("from"))
//     {
//         mkfrom = aV->getbParam("from");
//         aV->setParam("from", false);
//     }

//     if(aV->gotParam("to"))
//     {
//         mkto = aV->getbParam("to");
//         aV->setParam("to", false);
//     }
//     char* flexName = (char *) aV->am->vm->sysName;
//     FPS_ERROR_PRINT("%s >> Flex >>Setting vlinks\n",
//     __func__); aV->am->vm->setVLinks(vmap, flexName,
//     mkfrom, mkto); FPS_ERROR_PRINT("%s >> Flex >>Done Setting
//     vlinks\n",__func__); return 0;
//}
// "/links/rack_6":{
//     "Year": {
//           "value": "/components/catl_rack_6_ems_bms_rw:ems_rtc_year",
//           "linkvar": "/status/rack_6:year",
//           "defval": 0,
//           "pname": "bms",
//         }
//    }
// int RunLinks(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     essPerf ePerf(aV->am, "flex_system", "links", nullptr);
//     char* flexName = (char *) aV->am->vm->getSysName(vmap);
//     if(aV->gotParam("amap"))
//     {
//         flexName = aV->getcParam("amap");
//     }

//     FPS_ERROR_PRINT("%s >> Flex >>Setting links for [%s] \n",
//     __func__, flexName); aV->am->vm->setLinks(vmap,
//     flexName); FPS_ERROR_PRINT("%s >> Flex >>Done Setting
//     links\n",__func__); if(aV->gotParam("amap"))
//     {
//         aV->setParam("amap", (char*)" ");
//     }
//     return 0;
// }
// simple function to run a moving avg and plot it somewhere
// needs a (new) value vec in an assetVar
// Params (vecAv, outav, depth, amap)
// /system/flex/movavg
// int MathMovAvg(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     essPerf ePerf(aV->am, "math_system", "MovAvg", nullptr);
//     char* flexName = (char *) aV->am->vm->getSysName(vmap);
//     asset_manager* am = aV->am;
//     VarMapUtils* vm = am->vm;
//     int depth = 16;

//     if(aV->gotParam("amap"))
//     {
//         flexName = aV->getcParam("amap");
//     }
//     if(aV->gotParam("depth"))
//     {
//         depth = aV->getiParam("depth");
//     }
//     assetVar* vecAv = vm->getaVParam(vmap, aV, "vecAv");
//     double dval = aV->getdVal();
//     if(!vecAv)
//     {
//         if(aV->gotParam("vecAv"))
//         {
//             vecAv = vm->setVal(vmap, aV->getcParam("vecAv"), nullptr, dval);
//         }
//         if(vecAv)
//         {
//             if(!vecAv->am)
//                 vecAv->am = aV->am;
//             vecAv->setParam("depth", depth);
//         }
//         else
//         {
//             FPS_ERROR_PRINT("%s >> no vecAv Param for  [%s]\n"
//                 , __func__, aV->getfName());
//         }
//     }

//     vecAv->setVecVal(dval, depth);
//     dval = vecAv->getVecMean(depth);

//     if(aV->gotParam("outAv"))
//     {
//         char* spv = aV->getcParam("outAv");
//         assetVar* outAv = vm->setVal(vmap, spv, nullptr, dval);
//         if(!outAv->am)
//                 outAv->am = aV->am;
//         FPS_ERROR_PRINT("%s >> set outAv  [%s] to [%2.3f] \n"
//             , __func__, spv, dval);
//     }
//     // vecAv contains a vector of assetVals (aValVec) in extras
//     // push aV->getdVal(depth) onto vecAv() upto depth
//     // get sum of vecAv // divide by number of entries.
//     // put result into outAv
//     // pus
//     FPS_ERROR_PRINT("%s >> name [%s] input [%s] depth %d\n"
//         , __func__, flexName, aV->getfName(), depth);
//     // if(aV->gotParam("amap"))
//     // {
//     //     aV->setParam("amap", (char*)" ");
//     // }
//     return 0;
// }

// run the template config load

// int RunTpl(varsmap &vmap, varmap &amap, const char* xaname, fims* p_fims,
// assetVar* aV)
// {
//     essPerf ePerf(aV->am, "flex_system", "tpl", nullptr);

//     char* aname = nullptr;
//     char* pname = nullptr;
//     char* fname = nullptr;
//     if(aV->gotParam("aname"))
//     {
//         aname = aV->getcParam("aname");
//     }
//     if(aV->gotParam("pname"))
//     {
//         pname = aV->getcParam("pname");
//     }
//     if(aV->gotParam("fname"))
//     {
//         fname = aV->getcParam("fname");
//     }
//     FPS_ERROR_PRINT("%s >> setting up manager [%s->%s] filename
//     [%s]\n",__func__, pname, aname, fname);

//     // first find parent
//     if (pname == nullptr)
//     {
//         pname = (char*)"flex";
//     }
//     if(!pname | !aname|!fname)
//     {
//         FPS_ERROR_PRINT(" %s >> pname aname or fname missing\n",
//         __func__); return 0;
//     }
//     asset_manager* pam = aV->am->vm->getaM(vmap, pname);
//     if(!pam)
//     {
//         pam = new asset_manager(pname);
//     }
//     if(!pam )
//     {
//         FPS_ERROR_PRINT(" %s >> unable to create pam [%p]\n",
//         __func__, pam); return 0;
//     }

//     asset_manager* am = aV->am->vm->getaM(vmap, aname);
//     if(!am)
//     {
//         am = new asset_manager(aname);
//     }

//     am->p_fims = p_fims;
//     am->wakeChan = pam->wakeChan;
//     am->reqChan = (void*)pam->reqChan;

//     am->setFrom(pam);
//     //TODO if (1)FPS_ERROR_PRINT(" %s >> syscVec size [%d]\n",
//     __func__, (int)syscpVec->size());
//                     // now get the asset_manager to configure itsself
//                     // syscpVec needs to be solved
//     //am->vm->syscVec = &am->vm->syscharVec;
//     if(am->configure(&vmap, fname, aname,
//     am->vm->syscVec/*nullptr*//*syscpVec*/, nullptr, am) < 0)
//     {
//         FPS_ERROR_PRINT(" %s >> error in [%s] config file [%s]\n",
//         __func__, aname, fname); exit(0);
//     }
//     //int idx = 0;
//     // for ( auto &x: am->vm->sysVec)
//     // {
//     //     FPS_ERROR_PRINT("%s >> syscpVec [%d] [%s]\n",
//     __func__, idx++, x);
//     // }

//     //if (iy.first == "pcs") PCSInit(vmap, Av->am->amap, aname, p_fims, Av);

//     // TODO add Init func
//     //int ccntam = 0;
//     //FPS_ERROR_PRINT("%s >> setting up a %s manager pubs found %d \n",
//     __func__, aname, ccntam);

//     // add it to the assetList
//     pam->addManAsset(am, aname);
//     FPS_ERROR_PRINT("%s >> done setting up a %s manager
//     \n",__func__, aname); am->running = 1; return 0;
// }

// int StopSched(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
// assetVar* aV)
// {
//     asset_manager* am = aV->am;
//     //int debug = 1;

//     if(!checkAv(vmap,amap, aname,p_fims,aV))
//     {
//         return -1;
//     }
//     if (!aV->gotParam("uri")|| (strcmp(aV->getcParam("uri"),"none") ==0))
//     {
//         FPS_ERROR_PRINT("%s >>  [%s] missing uri\n"
//         , __func__
//         , aV->getfName()
//         );
//         return 0;
//     }
//     char* uri = aV->getcParam("uri");

//     double tNow = am->vm->get_time_dbl();

//     assetVar* av = am->vm->setVal(*am->vmap, uri, nullptr, tNow);
//     if(!av)
//     {
//         FPS_ERROR_PRINT("%s >> error setVal aV [%s] uri[%s]\n"
//         , __func__
//         , aV->getfName()
//         , uri
//         );
//         return 0;
//     }

//     av->am = am;

//     FPS_ERROR_PRINT("%s >> running with av [%s] uri [%s]\n"
//     , __func__
//     , av->getfName()
//     , uri
//     );
//     av->setVal(av->name.c_str());   //schedId
//     av->setParam("uri",uri);
//     av->setParam("fcn",(char*)"RunTarg");
//     av->setParam("targ",uri);
//     av->setParam("amap",aname);

//     av->setParam("reftime",aV->getdParam("offset"));
//     double runTime = aV->getdParam("in")+ tNow;

//     av->setParam("endTime", runTime);
//     av->setParam("debug", aV->getiParam("debug"));
// //    double runFor   = av->getdParam("runFor");
// //    double endTime  = av->getdParam("endTime");
// //    int debug       = av->getiParam("debug");
//     return 0;

// }

// dummy function to run a cell in charge or discharge mode
// /status/flex/chargeCurrent = system charge current -ve to charge +ve to
// discharge /status/flex/chargeTime = simulated time between cycles
// /status/flex/chargeCurrent = system charge current
//  cell params
//     soc state of charge
//     volts volts available
//     current used or provided
//     temp

double getCellVoltsFromSoc(double soc)
{
    double dval = 0.0;
    if (soc < 80)
    {
        dval = 1.0 + (2 * soc / 80.0);
    }
    else
    {
        dval = 3 + (0.4 * ((soc - 80.0) / 20.0));
    }
    return dval;
}

double getCellChargeCurrentFromSoc(double soc, double cap)
{
    double dval = 0.0;
    double volts;
    if (soc < 80)
    {
        dval = cap;
    }
    else
    {
        volts = getCellVoltsFromSoc(soc);
        if (std::abs(volts - 3.0) < 0.2)
        {
            dval = cap;
        }
        else
        {
            // volts = 3 return cap/1.0
            double div = (volts - 3.0) * (50 / 0.4);
            dval = cap / div;
        }
    }
    return dval;
}

double getCellDischargeCurrentFromSoc(double soc, double cap)
{
    double dval = 0.0;
    // double volts;
    if (soc > 20)
    {
        dval = cap;
    }
    else if (soc > 5)
    {
        //
        double div(20.0 - soc);
        if (std::abs(div) < 0.2)
        {
            div = 1.1;
        }
        dval = cap / div;
    }
    else
    {
        dval = 0.0;
    }
    return dval;
}

double getCellChargeAlloc(double cellCharge, double totalCharge, int numCells, int soc, int avg_soc)
{
    double dval = totalCharge / numCells;
    // we have totalCharge/numCells available for each cell
    // this cell needs callCharge we take what we need or what we are allowed
    // just take what we need
    if (cellCharge < dval)
    {
        dval = cellCharge;
    }
    // if soc is below avg_soc we can ask for more if above we ask for less
    else
    {
        double socadjust = (avg_soc - soc) * 0.1 / 100.0;
        if (0)
            dval = dval + (1.0 - socadjust) * dval;
    }
    return dval;
}

double getCellDischargeAlloc(double cellCharge, double totalCharge, int numCells, int soc, int avg_soc)
{
    double dval = totalCharge / numCells;
    // we have totalCharge/numCells required  for each cell
    // this cell can give callCharge we give what we can or what we are required
    if (cellCharge < dval)
    {
        dval = cellCharge;
    }
    // if soc is above avg_soc we can privide for more.
    else
    {
        double socadjust = (avg_soc - soc) * 0.1 / 100.0;
        if (0)
            dval = dval + (1.0 - socadjust) * dval;
    }
    return dval;
}

int RunCell(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(p_fims);
    double dval = 0.0;
    int numRacks = 9;
    int numModules = 8;
    int numCellsPerModule = 50;

    FPS_ERROR_PRINT(
        "%s >> Running for assetVar [%s] with aname [%s] and asset "
        "manager [%s]\n",
        __func__, aV->getfName(), aname, aV->am->name.c_str());

    asset_manager* fixed_am = aV->am->vm->getaM(vmap, aname);
    // Make sure we are using the right asset manager for the assetVar based on
    // the given aname
    if (fixed_am)
        aV->am = fixed_am;
    FPS_ERROR_PRINT(
        "%s >> Running for assetVar [%s] with aname [%s] and asset "
        "manager (fixed)  [%s]\n",
        __func__, aV->name.c_str(), aname, fixed_am->name.c_str());

    VarMapUtils* vm = aV->am->vm;
    // reload is pitched at the base level we'll call this flex shouls probably be
    // bms any cell being calculated after that time will use the same base
    // options. if we have different cabinets with different characteristics then
    // the reload can be rooted at bms_1 / bms_2  etc the cell itself will have
    // its nesting specified /bms_x/rack_y/module_z that will map out the
    // asset_master layout the cell itself will have a defined flat table based on
    // this /bms_x{delim}rack_y{delim}module_z:{"cell_xx":{},...} why ... well it
    // helps make the system work. the interface (modbus) maps require the flat
    // structure although some are getting more modular
    //  we need the asset_manager->asset structure  am->am->am->ai to manage the
    //  recursive detail collecting data
    // from all the units up the tree using the amap structure.

    std::string reloadStr = __func__;
    reloadStr += "_flex";
    int reload = vm->CheckReload(vmap, aV->am->amap, aname, reloadStr.c_str());
    if (1)
        FPS_ERROR_PRINT("%s >> Reload val for asset manager %s reload name %s is %d.\n", __func__, aname,
                        reloadStr.c_str(), reload);
    if (reload < 2)
    {
        FPS_ERROR_PRINT(
            "%s >> Reload val for asset manager %s reload name %s is "
            "%d. Reloading...\n",
            __func__, aname, reloadStr.c_str(), reload);

        amap["chargePower"] = vm->setLinkVal(vmap, "flex", "/status", "chargePower", dval);
        amap["chargeCurrent"] = vm->setLinkVal(vmap, "flex", "/controls", "chargeCurrent", dval);
        amap["runTime"] = vm->setLinkVal(vmap, "flex", "/status", "runTime", dval);
        dval = 280.0;
        amap["cellCapacity"] = vm->setLinkVal(vmap, "flex", "/param", "cellCapacity", dval);
        dval = numRacks * numModules * numCellsPerModule;
        amap["numCells"] = vm->setLinkVal(vmap, "flex", "/param", "numCells", dval);
        amap["numCellsPerModule"] = vm->setLinkVal(vmap, "flex", "/param", "numCellsPerModule", numCellsPerModule);
        amap["numModules"] = vm->setLinkVal(vmap, "flex", "/param", "numModules", numModules);
        amap["numRacks"] = vm->setLinkVal(vmap, "flex", "/param", "numRacks", numRacks);

        if (reload < 1)
        {
            amap["chargeCurrent"]->setVal(dval);
            amap["chargePower"]->setVal(dval);
            dval = 0.100;
            amap["runTime"]->setVal(dval);
            dval = 280.0;
            amap["cellCapacity"]->setVal(dval);
            amap["numCellsPerModule"]->setVal(numCellsPerModule);
            amap["numModules"]->setVal(numModules);
            amap["numRacks"]->setVal(numRacks);
        }
        {
            double volts;
            int numCells = 9 * 8 * 50;
            double timeSlice = 0.1;      // for power calcs
            double powerDerating = 0.5;  // limit on charge / discharge power
            double chargeCurrent;
            double dischargeCurrent;
            double chargeAlloc;
            double dischargeAlloc;

            double cellCapacity = 280;

            double totalCharge = cellCapacity * timeSlice * numCells * powerDerating;
            double totalDischarge = cellCapacity * timeSlice * numCells * powerDerating;

            FPS_ERROR_PRINT(
                "%s >>\tsoc,\tvolts,\tcharge,(charge power), "
                "(alloc),\t\tdischarge,(dischargepower),(alloc)\n",
                __func__);
            for (int soc = 0; soc < 100; soc += 5)
            {
                volts = getCellVoltsFromSoc(soc);
                chargeCurrent = getCellChargeCurrentFromSoc(soc, cellCapacity);
                dischargeCurrent = getCellDischargeCurrentFromSoc(soc, cellCapacity);
                dischargeAlloc = getCellDischargeAlloc(dischargeCurrent * volts * timeSlice, totalDischarge, numCells,
                                                       soc, soc);
                chargeAlloc = getCellChargeAlloc(chargeCurrent * volts * timeSlice, totalCharge, numCells, soc, soc);
                FPS_ERROR_PRINT(
                    "%s "
                    ">>\t%02d,\t%2.3f,\t%2.3f,\t%2.3f,\t%2.3f,\t%2.3f,\t%2."
                    "3f,\t%2.3f\n",
                    __func__, soc, volts, chargeCurrent, (volts * chargeCurrent * 0.1), chargeAlloc, dischargeCurrent,
                    (volts * dischargeCurrent * 0.1), dischargeAlloc);
            }
        }
        int ival = 2;
        // av->am->amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload",
        // reloadStr.c_str(), ival);
        amap[reloadStr]->setVal(ival);
    }
    assetVar* av = aV;

    if (!av->gotParam("soc"))
    {
        av->setParam("soc", 35.0);
    }
    if (!av->gotParam("volts"))
    {
        av->setParam("volts", 3.1);
    }
    if (!av->gotParam("current"))
    {
        av->setParam("current", 0);
    }
    if (!av->gotParam("temp"))
    {
        av->setParam("temp", 25.0);
    }
    if (!av->gotParam("rand"))
    {
        av->setParam("rand", 0.0);
    }

    numCellsPerModule = amap["numCellsPerModule"]->getiVal();
    numModules = amap["numModules"]->getiVal();
    numRacks = amap["numRacks"]->getiVal();
    double runTime = amap["runTime"]->getdVal();
    double soc = av->getdParam("soc");
    double volts = av->getdParam("volts");
    // double current         = av->getdParam("current");
    // double temp            = av->getdParam("temp");
    double chargePower = amap["chargePower"]->getdVal();
    double cellCapacity = amap["cellCapacity"]->getdVal();
    volts = getCellVoltsFromSoc(soc);
    double chargeCurrent = getCellChargeCurrentFromSoc(soc, cellCapacity);
    double dischargeCurrent = getCellDischargeCurrentFromSoc(soc, cellCapacity);
    double cellPowerChange = 0.0;
    FPS_ERROR_PRINT("%s >> av [%s] soc %f volts %f discharge %f \n", __func__, av->getfName(), soc, volts,
                    dischargeCurrent);
    // are we discharging
    if (chargePower > 0.0)
    {
        cellPowerChange = volts * dischargeCurrent * runTime;
        soc -= cellPowerChange;
        // TODO limit this based on power requested
        av->setParam("soc", soc);
        av->setParam("cellPowerChange", cellPowerChange);
    }
    else
    {
        cellPowerChange = volts * chargeCurrent * runTime;
        soc += cellPowerChange;
        // TODO limit this based on power requested
        av->setParam("soc", soc);
        av->setParam("cellPowerChange", cellPowerChange);
        FPS_ERROR_PRINT("%s >> av [%s] discharge  soc %f volts %f discharge %f \n", __func__, av->getfName(), soc,
                        volts, dischargeCurrent);
    }

    return 0;
}
// move to scheduler
void scFimsThread(scheduler* sc, fims* p_fims);
void fimsThread(scheduler* sc, fims* p_fims)
{
    double tNow = sc->vm->get_time_dbl();
    int tick = 0;
    fims_message* msg = p_fims->Receive_Timeout(100);
    while (*sc->run)
    {
        msg = p_fims->Receive_Timeout(1000000);
        if (debug)
        {
            // just for testing
            tNow = sc->vm->get_time_dbl();
            if (1)
                FPS_ERROR_PRINT("%s >>Fims Tick %d msg %p p_fims %p time %2.3f\n", __func__, tick, (void*)msg,
                                (void*)p_fims, tNow);
            tick++;
        }
        if (msg)
        {
            // TODO check for blocks here
            // if (strcmp(msg->method, "get") == 0)
            // {
            //     if (1)FPS_ERROR_PRINT("%s >> %2.3f GET uri  [%s]\n"
            //         , __func__, sc->vm->get_time_dbl(),
            //         msg->uri);
            // }
            if (*sc->run)
            {
                sc->fimsChan.put(msg);
                sc->wakeChan.put(tick);  // but this did not get serviced immediatey
            }
            else
            {
                if (p_fims)
                    p_fims->free_message(msg);
                // p_fims->delete
            }
        }
    }
    assetVar aV;
    tNow = sc->vm->get_time_dbl();
    aV.sendEvent(flexPack, p_fims, Severity::Info, "FlexPack shutting down at %2.3f", tNow);
    FPS_ERROR_PRINT("%s >> fims shutting down\n", __func__);
    // if(p_fims)delete p_fims;
    sc->p_fims = nullptr;
}

// these allow us to stop the program /control/flex:runTime
// /control/flex:stopTime

void setupControls(scheduler* sc, varsmap* vmap, schlist* rreqs, asset_manager* am, fims* p_fims)
{
    UNUSED(rreqs);
    UNUSED(am);
    UNUSED(p_fims);
    char* fimsname = (char*)"/sched/fims:dummy";
    assetVar* fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);
    double dval = 0.0;

    if (!fimsav)
    {
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_PRINT("%s >> Created Fims var [%s] \n ", __func__, av->getfName());
        fimsav = av;
    }

    fimsname = (char*)"/control/flex:runTime";
    assetVar* runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    // double dval = 0.0;

    if (!runav)
    {
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_PRINT("%s >> Created Runvar var [%s:%s] \n ", __func__, av->comp.c_str(), av->name.c_str());
        runav = av;
    }
    fimsname = (char*)"/control/flex:stopTime";
    assetVar* stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    // double dval = 0.0;

    if (!stopav)
    {
        assetUri my(fimsname, nullptr);

        assetVar* av = new assetVar(my.Var, my.Uri, dval);
        (*vmap)[av->comp][av->name] = av;
        FPS_ERROR_PRINT("%s >> Created Stop var [%s:%s] \n ", __func__, av->comp.c_str(), av->name.c_str());
        stopav = av;
    }
}

// int scheduler::runChans(varsmap &vmap, schlist &rreqs, asset_manager* am)
void schedThread(scheduler* sc, varsmap* vmap, schlist* rreqs, asset_manager* am, fims* p_fims)
{
    // scheduler *sc = (scheduler *) data;

    // int running = 1;
    double delay = 1.0;  // Sec
    int wakeup = 0;
    schedItem* si = nullptr;
    double tNow = 0.0;
    fims_message* msg = nullptr;
    // double tStart = sc->vm->get_time_dbl();
    char* cmsg;
    bool triggered = false;
    bool stopped = false;
    bool stopSeen = false;

    setupControls(sc, vmap, rreqs, am, p_fims);

    char* fimsname = (char*)"/sched/flex:dummy";
    // assetVar*fimsav = sc->vm->getVar(*vmap, fimsname, nullptr);

    fimsname = (char*)"/control/flex:runTime";
    assetVar* runav = sc->vm->getVar(*vmap, fimsname, nullptr);
    // double dval = 0.0;

    double runTime = runav->getdVal();
    fimsname = (char*)"/control/flex:stopTime";
    assetVar* stopav = sc->vm->getVar(*vmap, fimsname, nullptr);
    runTime = runav->getdVal();
    if (runTime < 15)
    {
        runTime = 15.0;
        runav->setVal(runTime);
    }
    double stopTime = stopav->getdVal();

    while (*sc->run)
    {
        // this will wake up after delay regardless
        // the delay is nominally 200mS but could be less , it depends on the
        // timeout specified
        // bflag =
        sc->wakeChan.timedGet(wakeup, delay);
        essPerf* essLog = new essPerf(am, "flex_test", "WakePerf", nullptr);
        tNow = sc->vm->get_time_dbl();
        if (0)
            FPS_ERROR_PRINT("%s >> Sched Tick   at %2.6f\n", __func__, tNow);
        if (0)
            sc->showReqs(*rreqs);
        stopTime = stopav->getdVal();
        runTime = runav->getdVal();
        if (stopTime > 0 && !stopSeen)
        {
            stopSeen = true;
            FPS_ERROR_PRINT("%s >> Sched Tick   at %2.6f stopTime set %2.3f \n", __func__, tNow, stopTime);
        }
        if ((runTime > 0) && (tNow > runTime) && !triggered)
        {
            // triggered = true;
            runav->setVal(0.0);
        }
        if ((stopTime > 0) && (tNow > stopTime) && !stopped)
        {
            stopped = true;
            *sc->run = 0;
            //     sc->wakeChan.put(-1);   // but this did not get serviced immediatey
        }

        if (sc->msgChan.get(cmsg, false))
        {
            FPS_ERROR_PRINT("%s >> message -> data  [%s] \n", __func__, cmsg);
            // use the message to set the next delay
            if (strcmp(cmsg, "quit") == 0)
            {
                FPS_ERROR_PRINT("%s >> wakeup value  %2.3f time to stop \n", __func__, tNow);
                *sc->run = 0;
            }
            free((void*)cmsg);
        }

        // handle an incoming avar run request .. avoids too many locks
        if (sc->reqChan.get(si, false))
        {
            if (si)
            {
                // look for id allready allocated
                FPS_ERROR_PRINT(
                    "%s >> Servicing  Sched Request %p id [%s] (%p)  "
                    "uri [%s] repTime %2.3f at %2.3f\n",
                    __func__, (void*)si, si->id, (void*)si->id, si->uri, si->repTime, tNow);
                if (!si->id || (strcmp(si->id, "None") == 0))
                {
                    FPS_ERROR_PRINT("%s >> this is not a schedItem \n", __func__);
                    delete si;
                    si = nullptr;
                }

                if (si)
                {
                    schedItem* oldsi = sc->find(*rreqs, si);
                    if (oldsi)
                    {
                        FPS_ERROR_PRINT("%s >> this is a replacement schedItem %p \n", __func__, (void*)oldsi);
                        oldsi->repTime = 0;
                        oldsi->endTime = 0.1;

                        FPS_ERROR_PRINT("%s >>  schedItem deleted, seting repTime to 0.0\n", __func__);
                    }
                    // else
                    {
                        FPS_ERROR_PRINT("%s >>  schedItem added  %p\n", __func__, (void*)si);
                        sc->addSchedReq(*rreqs, si);
                    }
                }
            }
            else
            {
                FPS_ERROR_PRINT("%s >> got nullptr si request !!\n", __func__);
            }
        }

        // fims
        // this gets incoming  fims messages
        // needs a fims loop setup
        //
        if (sc->fimsChan.get(msg, false))
        {
            if (msg)
            {
                essPerf ePerf(am, "flex_fims", msg->uri, nullptr);
                if (sc->vm->schedActions > 0)
                {
                    if (0)
                        FPS_ERROR_PRINT("%s >> Setting up for a fims message old actions %d  \n", __func__,
                                        sc->vm->schedActions);
                }
                sc->vm->schedaV = nullptr;
                sc->vm->schedTime = tNow;
                // the first action will pick up the schedAv
                sc->vm->schedActions = 0;
                if (0)
                    sc->fimsDebug1(*vmap, msg, am);
                // mvmap=getvmap(msg) // here is our chance to switch vmaps

                sc->vm->runFimsMsgAm(*vmap, msg, am, p_fims);
                if (0)
                    sc->fimsDebug2(*vmap, msg, am);
                // sc->p_fims->free_message(msg);
            }
        }

        // int xdelay = sc->getSchedDelay(*vmap, *rreqs);
        // this also runs the scheduled tasks
        double ddelay = sc->getSchedDelay(*vmap, *rreqs);
        delete essLog;

        if (debug)
            FPS_ERROR_PRINT("%s>> new delay = %2.6f\n", __func__, ddelay);
        delay = ddelay;
        // if ((sc->vm->get_time_dbl() - tStart)  > 15.0) running = 0;
    }

    tNow = sc->vm->get_time_dbl();
    FPS_ERROR_PRINT("%s >> shutting down at %2.6f\n", __func__, tNow);
    return;
}

#define LOGDIR "/var/log/flex_controller"

// Here is a connection of nasty globals , --to be removed.

VarMapUtils vm;

varsmap vmap;
// deprecated
// std::vector<char *> syscVec;

// typedef std::vector<schedItem*>schlist;
schlist schreqs;

// typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
varsmap funMap;

// std::vector<char*>* syscpVec;

// typedef std::map<std::string, std::vector<std::string>*>vecmap;
vecmap vecs;

cJSON* getSchListcJ(schlist& schreqs);

cJSON* getSchList()
{
    return getSchListcJ(schreqs);
}

int main_test_new_flex(int argc, char* argv[])
{
    // syscpVec = new std::vector<char*>;
    FPS_ERROR_PRINT(" hello its me 1 %s ...\n", __func__);

    // get this from arg 2
    vm.argc = argc;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // double maxTnow = 15.0;
    // reserver some scheduler slots , it will add more if needed.
    scheduler sched("flexSched", &running);
    sched.vm = &vm;
    vm.setSysName(vmap, "flex");
    // int vmlen = asprintf(&vm.uriroot, "/%s", vm.sysName);
    // sched.vm->syscVec = &vm.syscharVec;
    vm.syscVec = new std::vector<char*>;  //&vm.syscharVec;
    // vm.funMapp = new varsmap;
    schreqs.reserve(64);
    FPS_ERROR_PRINT(" hello its me 2 %s ...\n", __func__);

    vm.vmapp = &vmap;
    vm.funMapp = &funMap;
    int rc = 0;

    // read config dir
    if (argc > 1)
    {
        vm.setFname(argv[1]);
    }
    else
    {
        char* sp;
        vm.vmlen = asprintf(&sp, "configs/%s_controller", vm.getSysName(vmap));
        vm.setFname(sp);
        free(sp);
    }

    // vm.setRunLog(LOGDIR "/run_logs");
    vm.setRunCfg(LOGDIR "/run_configs");

    // asset_manager*
    flex_man = setupEssManager(&vm, vmap, vecs, nullptr /*syscpVec*/, vm.sysName, &sched);
    FPS_ERROR_PRINT(" hello its me 3 %s ...\n", __func__);
    // should alreary be done
    vm.setaM(vmap, vm.sysName, flex_man);
    flexName = (char*)vm.sysName;

    vm.setFunc(*flex_man->vmap, flexName, "RunVLinks", (void*)&RunVLinks);
    vm.setFunc(*flex_man->vmap, flexName, "RunLinks", (void*)&RunLinks);
    vm.setFunc(*flex_man->vmap, flexName, "MakeLink", (void*)&MakeLink);
    vm.setFunc(*flex_man->vmap, flexName, "RunTpl", (void*)&RunTpl);
    vm.setFunc(*flex_man->vmap, flexName, "RunSched", (void*)&RunSched);
    vm.setFunc(*flex_man->vmap, flexName, "RunMonitor", (void*)&RunMonitor);
    vm.setFunc(*flex_man->vmap, flexName, "StopSched", (void*)&StopSched);
    vm.setFunc(*flex_man->vmap, flexName, "RunPub", (void*)&RunPub);
    vm.setFunc(*flex_man->vmap, flexName, "SendDb", (void*)&SendDb);
    vm.setFunc(*flex_man->vmap, flexName, "LoadServer", (void*)&LoadServer);
    vm.setFunc(*flex_man->vmap, flexName, "LoadClient", (void*)&LoadClient);
    // vm.setFunc(*flex_man->vmap, flexName, "LoadClient",   (void*)&LoadClient);
    vm.setFunc(*flex_man->vmap, flexName, "LoadConfig", (void*)&LoadConfig);
    vm.setFunc(*flex_man->vmap, flexName, "DumpConfig", (void*)&DumpConfig);
    vm.setFunc(*flex_man->vmap, flexName, "RunSystemCmd", (void*)&RunSystemCmd);
    vm.setFunc(*flex_man->vmap, flexName, "process_sys_alarm", (void*)&process_sys_alarm);
    vm.setFunc(*flex_man->vmap, flexName, "CalculateVar", (void*)&CalculateVar);
    vm.setFunc(*flex_man->vmap, flexName, "RunSystemCmd", (void*)&RunSystemCmd);
    vm.setFunc(*flex_man->vmap, flexName, "CheckTableVar", (void*)&CheckTableVar);
    vm.setFunc(*flex_man->vmap, flexName, "CheckMonitorVar", (void*)&CheckMonitorVar);
    vm.setFunc(*flex_man->vmap, flexName, "RunCell", (void*)&RunCell);
    vm.setFunc(*flex_man->vmap, flexName, "MathMovAvg", (void*)&MathMovAvg);
    vm.setFunc(*flex_man->vmap, flexName, "HandleSchedItem", (void*)&HandleSchedItem);
    vm.setFunc(*flex_man->vmap, flexName, "SimHandlePcs", (void*)&SimHandlePcs);
    vm.setFunc(*flex_man->vmap, flexName, "SimHandleBms", (void*)&SimHandleBms);
    vm.setFunc(*flex_man->vmap, flexName, "FastPub", (void*)&FastPub);

    vm.setActions(*flex_man->vmap, flexName);
    // vm.setSequence(*flex_man->vmap, flexName, "help",     (void*)nullptr,"used
    // to save sequences of operations ");

    // varmaputils cannot access scheduler.h yet
    flex_man->wakeChan = &sched.wakeChan;
    flex_man->reqChan = (void*)&sched.reqChan;
    sched.am = flex_man;
    // vm.setaM(vmap,flexName, flex_man);
    // set up functions
    // SetupRwGpioSched(&sched, flex_man);

    if (debug)
        FPS_ERROR_PRINT(" %s >> %s start load controller \n", __func__, flexName);
    {
        char* sp;
        vm.vmlen = asprintf(&sp, "%s_controller.json", flexName);
        rc = loadEssConfig(&vm, vmap, sp, flex_man, &sched);
        free(sp);
    }
    if (debug)
        FPS_ERROR_PRINT(" %s >> %s done load controller rc %d \n", __func__, flexName, rc);

    int ccntam = 0;
    vm.getVList(*flex_man->vecs, vmap, flex_man->amap, flexName, "Subs", ccntam);
    FPS_ERROR_PRINT("%s >> setting up [%s] manager Subs found %d rc %d \n", __func__, flexName, ccntam, rc);
    // vm.showvecMap(*ess_man->vecs, "Subs");
    int mysublen;
    char** mysubs = vm.getVecListbyName(*flex_man->vecs, "Subs", mysublen);
    FPS_ERROR_PRINT("%s >> recovered  [%s]  Subs %p found %d \n", __func__, flexName, (void*)mysubs, mysublen);
    fims* p_fims = nullptr;

    if (mysublen > 0)
    {
        if (1)
            FPS_ERROR_PRINT(" %s >> flex_man >> Subs found in flex config \n", __func__);

        p_fims = sched.fimsSetup((const char**)mysubs, mysublen, "FlexSched", vm.argc);
        vm.clearList(mysubs, flexName, mysublen);
        if (1)
            FPS_ERROR_PRINT(" %s >> flex_man >> p_fims %p  sched %p\n", __func__, (void*)p_fims, (void*)sched.p_fims);
    }
    else
    {
        if (1)
            FPS_ERROR_PRINT(" %s >> flex_man >> No Subs found in flex config \n", __func__);
        return 0;
    }

    // p_fims = sched.p_fims;
    flex_man->p_fims = p_fims;
    if (debug)
        FPS_ERROR_PRINT(" %s >> %s >> p_fims %p\n", __func__, flexName, (void*)p_fims);
    // return 0;
    // next we load the asset managers
    // int rc =
    // may not need these
    //     loadAssetManagers(vmap, ess_man, syscpVec, p_fims);
    // show subs vecs and dump config to an initial file.
    debugSystemLoad(&vm, vmap, vecs, nullptr /*syscpVec*/, flexName, LOGDIR);

    FPS_ERROR_PRINT("Flex >>Setting vlinks\n");
    vm.setVLinks(vmap, flexName);
    FPS_ERROR_PRINT("Flex >>Done Setting vlinks\n");

    // no need to load schedule thie init does it all for us.

    // if (1)FPS_ERROR_PRINT(" %s >> gpio start load schedule \n",
    // __func__); rc = loadEssConfig(&vm, vmap,
    // "gpio_schedule.json", flex_man, &sched); if (1)FPS_ERROR_PRINT(" %s >> gpio
    // done  load schedule rc %d \n", __func__, rc);

    double tNow = vm.get_time_dbl();
    assetVar* aV = vmap["/config/flex"]["flex"];
    if (1)
        FPS_ERROR_PRINT(" %s >> flex aV %p \n", __func__, (void*)aV);
    if (aV)
    {
        aV->sendEvent(flexPack, p_fims, Severity::Info, "%s starting  at %2.3f", flexName, tNow);
        SetupGit(vmap, &vm, GITBRANCH, GITCOMMIT, GITTAG, GITVERSION);

        // GpioInit(vmap, flex_man->amap, "flex", p_fims, aV);
    }
    else
    {
        if (1)
            FPS_ERROR_PRINT(" %s >> %s aV error %p \n", __func__, flexName, (void*)aV);
        return 0;
    }
    //    int GpioInit(varsmap& vmap, varmap& amap, const char* aname, fims*
    //    p_fims, assetVar* aV)

    // std::thread fThread(fimsThread, &sched, p_fims);
    std::thread fThread(scFimsThread, &sched, p_fims);
    std::thread sThread(schedThread, &sched, &vmap, &schreqs, flex_man, p_fims);

    if (fThread.joinable())
        fThread.join();
    FPS_ERROR_PRINT("%s >> fims thread done .... \n", __func__);
    if (sThread.joinable())
        sThread.join();
    FPS_ERROR_PRINT("%s >> sched thread done  .... \n", __func__);

    vm.clearVmap(vmap);

    vm.amMap.clear();

    // delete ess_man->p_fims;
    flex_man->p_fims = nullptr;
    // vecm[name] = (new std::vector<T>);
    for (auto xx : vecs)
    {
        xx.second->clear();
        delete xx.second;
    }
    vecs.clear();
    // delete flex_man;
    int idx = 0;
    FPS_ERROR_PRINT("%s >> things found in syscVec  .... [%p]\n", __func__, (void*)vm.syscVec);
    if (vm.syscVec)
    {
        for (auto x : *vm.syscVec)
        {
            FPS_ERROR_PRINT(" syscVec[%d] %p [%s]\n", idx++, (void*)x, x);
            if (x)
                free(x);
        }
        vm.syscVec->erase(vm.syscVec->begin(), vm.syscVec->begin() + vm.syscVec->size());
        vm.syscVec->clear();
        delete vm.syscVec;
        vm.syscVec = nullptr;
    }

    FPS_ERROR_PRINT("%s >>  deleting  remaining sched items .... \n", __func__);
    for (auto sr : schreqs)
    {
        // FPS_ERROR_PRINT("%s >>  we got a schedItem %p not deleting it for now
        // .... \n", __func__, sr);
        sr->show();
        delete sr;
    }
    schreqs.clear();

    vm.amMap.clear();
    vm.aiMap.clear();
    if (p_fims)
        delete p_fims;

    return 0;
}

// Please do NOT remove these, they are global extern variables for the GPIO
// controller.
namespace flex
{
const std::chrono::system_clock::time_point epoch = please::dont::use::this_func::setEpoch();
const std::chrono::steady_clock::time_point base_time = std::chrono::steady_clock::now();
}  // namespace flex

int main(int argc, char* argv[])
{
    // setting up stdout console sink:
    auto std_out = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    std_out->set_level(spdlog::level::debug);

    // setting up stderr console sink:
    auto std_err = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
    std_err->set_level(spdlog::level::err);

    // setting the default logger to log to both:
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(
        "", std::initializer_list<spdlog::sink_ptr>{ std::move(std_out), std::move(std_err) }));

    // setting up the default console logger for the FlexPack Controller from now
    // on offers safety and extensibilty, even more safety once we move to C++17
    // and up
    spdlog::set_level(spdlog::level::debug);  // please change "tweakme.h" to get this

    // setting up the elapsed time formatter for the global logger (similar to the
    // way we have it for ESS Controller): NOTE: please refer to this for help ->
    // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<elapsed_time_formatter>('*').set_pattern("[%-8*] [%^%-8l%$] [%-15!!] %v");
    spdlog::set_formatter(std::move(formatter));

    return main_test_new_flex(argc, argv);
}