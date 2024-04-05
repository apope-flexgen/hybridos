/*
 * check uri
 * get /ess/comp/pattern ... will show all components matching a pattern
 * get /ess/vars/pattern ... will show all components and vars matching a
 * pattern set /ess/  bypasses blocks set /components ( or any allowed Sets will
 * create havoc .. not really will create components for us) set/pub /status (
 * or any Blocked sets are disabled) set/pub /ess/status ( or or any Blocked
 * sets are allowed) uri /x/y/z   may be comp /x/y/z if body is cJSON and does
 * not have a "value" key     body can have {"vname":val,"vname":val...} or
 * {"vname":{"value":val}} uri /x/y/z  may be comp /x/y if it's body is NOT
 * cJSON or does have  a "value" first level key , in this case "z" is the
 * variable and we are denoted as a "SINGLE"
 *
 * wild cards only allowed at end of a word and at the last word
 * get /ess/comp/components will all tables that match "/components..."
 * get /ess/comp/components/bss will all tables that match "/components/bss/..."
 * if /ess moves past blocks
 *  sets check for blocks
 * When we get in a message :
 *  1/ check for blocks adjust uri if needed
 *  2/ check for single and split out comp and var if so
 *  3/ run the rest/
 *
 * Handling pubs.
 * pubs come out as /assets/bms/bms_1
 * so collect a list of assetvars and give em a URI to pub.
 *
 */

#include "asset.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

void printVars(VarMapUtils& vm, varsmap& vmap)
{
    cJSON* cj = vm.getMapsCj(vmap);
    char* res = cJSON_Print(cj);
    printf("vmap at end \n%s\n", res);
    free((void*)res);
    cJSON_Delete(cj);
}

void printVarVM(VarMapUtils& vm, varsmap& vmap)
{
    varsmap vmr;
    vm.getMapsVm(vmap, vmr);
    printVars(vm, vmr);
}

int printComp(VarMapUtils& vm, varsmap& vmap, const char* key, const char* var)
{
    cJSON* cj = vm.getCompsCj(vmap, key, var);
    if (cj)
    {
        char* res = cJSON_Print(cj);
        printf("comp [%s] var [%s] \n%s\n", key, var, res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    return 0;
}

// bool checkedBlockedUri(VarMapUtils &vm, varsmap &vmap, std::string &cName,
// const char* method, const char* uri)
// {
//     bool blocked = false;
//     const char* unblock = "/ess";
//     char* sp = (char *)uri;
//     if(strncmp(uri, unblock, strlen(unblock))==0)
//     {
//         blocked = false;
//         sp += strlen(unblock);
//         cName = sp;
//     }
//     else
//     {
//         cName = sp;
//         const char* blockeduri = nullptr;
//         if(strcmp(method,"set") ==0)
//         {
//             blockeduri ="/sets/blocked";
//         }
//         else if(strcmp(method,"pub") ==0)
//         {
//             blockeduri = "/pubs/blocked";
//         }
//         else if(strcmp(method,"get") ==0)
//         {
//             blockeduri = "/gets/blocked";
//         }
//         if(blockeduri)
//         {
//             printf(" %s checking blocked uri [%s] \n",
//             __func__, blockeduri);

//             auto ix = vmap.find(blockeduri);
//             if(ix != vmap.end())
//             {
//                 for (auto ix : vmap[blockeduri])
//                 {
//                     // skip leading '/'
//                     printf(" %s checking uri [%s] sets/blocked [%s]\n",
//                     __func__, uri,ix.first.c_str() );

//                     if (strncmp(&uri[1], ix.first.c_str(),
//                     strlen(ix.first.c_str())-1)==0)
//                     {
//                         blocked = true;
//                         break;
//                     }
//                 }
//             }
//         //     if(!blocked)
//         //     {
//         //         cName = *sp;
//         //     }
//         }
//     }
//     if(blocked)
//     {
//         printf(" blocked method [%s] uri [%s] ->[%s]\n"
//                 ,method, uri , cName.c_str());
//     }
//     else
//     {
//         printf(" NOT blocked method [%s] uri [%s] ->[%s]\n"
//                 ,method, uri, cName.c_str());
//     }

//     return blocked;
// }

// // gets single and value response
// //const char* uri = "/variables/bms_1/ac_contactor";
// //    const char* body1 = "true";
// cJSON *checkSingleUri(VarMapUtils &vm, varsmap &vmap, int& single,
// std::string &vName, std::string &cName, const char* uri=nullptr,const char*
// body=nullptr)
// {
//     single = 0;
//     auto ix = vmap.find(uri);
//     if (ix == vmap.end())
//     {
//        printf("possible single uri  [%s]\n", uri);
//        //single |= 1;
//     }
//     cJSON *cj = nullptr;
//     if(body) cj = cJSON_Parse(body);
//     if (!cj)
//     {
//         printf(" cj failed def single\n");
//         single |= 1;
//     }
//     else
//     {
//         printf("cj OK may not be single body [%s] type %d\n", body,
//         cj->type); if(cj->type == cJSON_Object)
//         {
//            cJSON * cji = cJSON_GetObjectItem(cj, "value");
//            if(cji)
//            {
//                 printf("cj OK  found value confirmed SINGLE  body [%s] type
//                 %d\n", body, cj->type); single |= 1; single |= 2;

//            }
//         }
//         else if(cj->type == cJSON_True)
//         {
//             printf("cj OK  found true confirmed SINGLE  body [%s] type %d\n",
//             body, cj->type); single |= 1;
//         }
//         else if(cj->type == cJSON_False)
//         {
//             printf("cj OK  found false confirmed SINGLE  body [%s] type
//             %d\n", body, cj->type); single |= 1;

//         }
//         else if(cj->type == cJSON_Number)
//         {
//             printf("cj OK  found number confirmed SINGLE  body [%s] type
//             %d\n", body, cj->type); single |= 1;

//         }
//         else if(cj->type == cJSON_String)
//         {
//             printf("cj OK  found string confirmed SINGLE  body [%s] type
//             %d\n", body, cj->type); single |= 1;
//         }
//     }

//     // if we are a single get a new URI
//     // get num parts
//     // get the last part
//     // remove the last part and we have a comp plus a var name
//     if(single & 1)
//     {
//         int parts = vm.get_nfrags(uri);
//         char* varName = vm.pull_pfrag(uri,parts);
//         char* uriName = vm.pull_uri(uri,parts);

//         printf(" got single varName [%s] uriName [%s]\n", varName, uriName);
//         vName = varName;
//         cName = uriName;
//         free((void *)varName);
//         free((void *)uriName);
//     }
//     else
//     {
//         cName = uri;
//     }

//     //if(cj)cJSON_Delete(cj);

//     return cj;
// }

void printCompVM(VarMapUtils& vm, varsmap& vmap, const char* key = nullptr, const char* var = nullptr)
{
    varsmap vmr;
    vm.getCompsVm(vmap, vmr, key, var);
    printVars(vm, vmr);
}

cJSON* getVmap(VarMapUtils& vm, varsmap& vmap, int single, const char* key = nullptr, const char* var = nullptr)
{
    varsmap vmr;
    vm.getCompsVm(vmap, vmr, key, var);
    cJSON* cj = vm.getMapsCj(vmr);
    return cj;
}

// // do this for singles
// cJSON* loadVmap(VarMapUtils &vm, varsmap& vmap, int single, const char* comp,
// const char* var, const char* body)
// {
//     char* xsp = nullptr;
//     cJSON* cjr = nullptr;
//     if(single & 1)
//     {
//         if ((single && 2) == 0)
//         {
//             asprintf(&xsp,"{\"%s\":{\"value\":%s}}",  var, body);
//         } else {
//             asprintf(&xsp,"{\"%s\":%s}", var, body);
//         }
//         vm.processMsgSetPub(vmap, "set", comp, xsp,  &cjr);

//     }
//     else
//     {
//         vm.processMsgSetPub(vmap, "set", comp, body,  &cjr);
//     }

//     if(xsp)free((void *)xsp);

//     return cjr;
// }

// cJSON* runFimsMsg(VarMapUtils &vm, varsmap &vmap, fims_message* msg , fims*
// p_fims)
// {
//     cJSON* cjr = nullptr;
//     std::string vName;  // new var name
//     std::string newUri;   // new comp (uri) name
//     std::string cName2;   // new comp (uri) name
//     std::string newBody;   // new comp (uri) name

//     bool reject = checkedBlockedUri(vm, vmap, newUri, msg->method, msg->uri);
//     printf(" after checkBlocked  newUri [%s]\n", newUri.c_str());

//     if (!reject)
//     {
//         int single;
//         // send in the new uri
//         cJSON* cj = checkSingleUri(vm, vmap, single, vName, cName2,
//         newUri.c_str(), msg->body);

//         printf(" after checkSingle single %d vName [%s] cName2 [%s] newUri
//         [%s] newBod [%s] cj %p\n"
//                 , single, vName.c_str(), cName2.c_str(), newUri.c_str()
//                 , msg->body, (void *) cj);

//         char* newBod = (char *)msg->body;  // for single adjust    /x/y/x 123
//         => /x/y '{z:{value"123}}' if (strcmp(msg->method,"get") == 0)
//             cjr = getVmap(vm, vmap, single, cName2.c_str(), vName.c_str());
//         else
//             cjr = loadVmap(vm, vmap, single, cName2.c_str(), vName.c_str(),
//             newBod);

//         //vm.processRawMsg(vmap, msg->method, cName2.c_str(), msg->replyto,
//         newBod, &cjr); if (cjr)
//         {
//             if(msg->replyto)
//             {
//                 char* tmp = cJSON_PrintUnformatted(cjr);
//                 if(tmp)
//                 {
//                     if(p_fims)p_fims->Send("set", msg->replyto, nullptr,
//                     tmp); free((void*)tmp);
//                 }
//             }
//             //cJSON_Delete(cjr);
//         }
//         if(cj)cJSON_Delete(cj);
//         if(p_fims)p_fims->free_message(msg);
//     }
//     return cjr;
// }

// new code
// // this gets in fims messages
//     if(am->fimschan.get(msg, false)) {
//         if(0)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  %s  FIMS MESSAGE
//         method [%s] uri [%s]\n"
//                 ,__func__
//                 , vm->get_time_dbl()
//                 , am->name.c_str()
//                 , msg->method
//                 , msg->uri
//                 );
//         // we need to collect responses
//         cJSON *cj = nullptr;
//         // either here of the bms instance
//         //bms_man->vmap
//        std::string vName;  // new var name
//        std::string cName1;   // new comp (uri) name
//        std::string cName2;   // new comp (uri) name
//        bool reject = checkedBlockedUri(vm, vmap, cName, msg->method,
//        msg->uri)

//         if (!reject)
//         {
//         int ret = checkSingleUri(vm, vmap, vName, cName2, cNmae1.c_str(),
//         msg->body);
//
//             am->vm->processFims(*am->vmap,  msg, &cj);
//          char * new_body = (char *)msg->body;
//          cJSON * cjr = nullptr;
//             am->vm->processRawMsg(*am->vmap, msg->method, cName2.c_str(),
//             msg->replyto, newbody &cjr) am->vm->processFimsMsg(*am->vmap,
//             msg, &cj);
//         }
//         if (cj)
//         {
//             if(msg->replyto)
//             {
//                 char* tmp = cJSON_PrintUnformatted(cj);
//                 if(tmp)
//                 {
//                     am->p_fims->Send("set",msg->replyto, nullptr, tmp);
//                     free((void*)tmp);
//                 }
//             }
//             cJSON_Delete(cj);
//         }
//         am->p_fims->free_message(msg);

// old ode in the main path
// // this gets in fims messages
//     if(am->fimschan.get(msg, false)) {
//         if(0)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  %s  FIMS MESSAGE
//         method [%s] uri [%s]\n"
//                 ,__func__
//                 , vm->get_time_dbl()
//                 , am->name.c_str()
//                 , msg->method
//                 , msg->uri
//                 );
//         // we need to collect responses
//         cJSON *cj = nullptr;
//         // either here of the bms instance
//         //bms_man->vmap
//         bool reject = false;
//         {
//             if(strcmp(msg->method,"get") == 0)
//             {
//                 if(1)FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  %s  FIMS
//                 MESSAGE  method [%s] uri [%s]\n"
//                 ,__func__
//                 , vm->get_time_dbl()
//                 , am->name.c_str()
//                 , msg->method
//                 , msg->uri
//                 );
//             }
//             else if(strcmp(msg->method,"set") == 0)
//             {
//                 if(strcmp(msg->uri,"/status/ess") == 0)
//                 {
//                     if(0)std::cout << am->name << "  >> fims_msg uri "<<
//                     msg->uri  << " REJECTED\n"; reject = true;
//                 }
//                 if(strcmp(msg->uri,"/status/bms") == 0)
//                 {
//                     if(0)std::cout << am->name << "  >> fims_msg uri "<<
//                     msg->uri  << " REJECTED\n"; reject = true;
//                 }
//                 if(strcmp(msg->uri,"/status/bms_1") == 0)
//                 {
//                     if(0)std::cout << am->name << "  >> fims_msg uri "<<
//                     msg->uri  << " REJECTED\n"; reject = true;
//                 }
//                 if(strcmp(msg->uri,"/status/bms_2") == 0)
//                 {
//                     if(0)std::cout << am->name << "  >> fims_msg uri "<<
//                     msg->uri  << " REJECTED\n"; reject = true;
//                 }

//             }
//         }
//         if (!reject)
//         {
//             am->vm->processFims(*am->vmap,  msg, &cj);
//         }
//         if (cj)
//         {
//             if(msg->replyto)
//             {
//                 char* tmp = cJSON_PrintUnformatted(cj);
//                 if(tmp)
//                 {
//                     am->p_fims->Send("set",msg->replyto, nullptr, tmp);
//                     free((void*)tmp);
//                 }
//             }
//             cJSON_Delete(cj);
//         }
//         am->p_fims->free_message(msg);

//         //free((void *) item3);
//     }
int main(int argc, char* argv[])
{
    // this is our main data map
    varsmap vmap;

    // this is our map utils factory

    VarMapUtils vm;

    // now load up a config file
    const char* cfgname = "configs/test_check_uri.json";
    vm.configure_vmap(vmap, cfgname);

    // const char *xsp = "{\"On\":
    // false,\"Standby\":true,\"Idleloss\":0.1,\"Idlestr\":{\"value\":\"soc0.1\"},"
    //                       "\"Idlenum\":{\"value\":0.134},"
    //                       "\"pesr\":34.5,\"Pcharge\":23,\"Pdischarge\":300.3,\"Pd_str\":\"300.3\","
    //                       "\"Ploss\":45,\"Phigh\":100.0,\"Plow\":0.01,\"Soc\":0.1}";
    // vm.processMsgSetPub(vmap, "set", "/big/pub", xsp,  nullptr);//cJSON **cjr)

    // cJSON* cjr = nullptr;
    // vm.processMsgSetPub(vmap, "set", "/big/pub", xsp,  &cjr);

    printVars(vm, vmap);

    printComp(vm, vmap, nullptr, nullptr);

    printComp(vm, vmap, "/controls*", nullptr);

    printComp(vm, vmap, "/variables/bms*", nullptr);

    printComp(vm, vmap, "/variables/bms_1", "*");

    printComp(vm, vmap, "/variables/bms_2", "bms");

    printCompVM(vm, vmap, "/controls*");

    printCompVM(vm, vmap, "/variables/bms*");

    printCompVM(vm, vmap, "/variables/bms_1", "*");

    printCompVM(vm, vmap, "/variables/bms_2", "bms*");

    // now moving on to detecting Single
    // set /variables/bms_1/ac_contactor true
    // set /variables/bms_1/ac_contactor '{"value":true)'
    // are both singles

    const char* uri = "/variables/bms_1/acContactor";
    const char* body1 = "true";
    const char* body2 = "{\"value\":true}";
    char* vName;
    char* cName;
    std::string newBody;
    int single;
    // cJSON *cj = nullptr;
    vm.checkSingleUri(vmap, single, &vName, &cName, uri, body1);
    printf(" after checkuri uri [%s] body [%s] ret 0x%x\n", uri, body1, single);
    if ((single && 1) == 1)
    {
        printf(" Found single vName [%s] cName [%s]\n", vName, cName);
        vm.loadVmap(vmap, single, cName, vName, body1);
    }
    // cj =
    vm.checkSingleUri(vmap, single, &vName, &cName, uri, body1);

    printf(" after checkuri uri [%s] body [%s] ret 0x%x\n", uri, body2, single);
    if ((single && 1) == 1)
    {
        printf(" Found single vName [%s] cName [%s]\n", vName, cName);
        vm.loadVmap(vmap, single, cName, vName, body2);
    }

    printCompVM(vm, vmap, "/variables/bms_1", "*");

    // did not work
    const char* buffer1 =
        "{"
        "\"method\":\"set\","
        "\"uri\":\"/sets/blocked\","
        "\"replyto\":\"/mee\" ,"
        "\"body\": {\"variables\":{\"value\":true}}"
        "}";
    // did not work
    // should set up two variables
    const char* buffer2 =
        "{"
        "\"method\":\"set\","
        "\"uri\":\"/sets/blocked\","
        "\"replyto\":\"/mee\" ,"
        "\"body\": {\"variables\":true,\"stuff\":true}"
        "}";

    // YES this works
    const char* buffer3 =
        "{"
        "\"method\":\"set\","
        "\"uri\":\"/sets/blocked/variables\","
        "\"replyto\":\"/mee\" ,"
        "\"body\": true}"
        "}";

    const char* buffer;
    buffer = buffer1;
    buffer = buffer3;  // works
    buffer = buffer2;  // WOrks now
    // buffer = buffer3;   // works
    buffer = buffer1;

    fims_message* msg = vm.bufferToFims(buffer);
    // if(cj)
    // {
    //     char* tmp = cJSON_PrintUnformatted(cj);
    //     if(tmp)
    //     {
    //         //if(p_fims)p_fims->Send("set", msg->replyto, nullptr, tmp);
    //         printf (" message [%s] \n reply [%s]\n", buffer, tmp);
    //         free((void*)tmp);
    //     }
    //     cJSON_Delete(cj);
    // }

    vm.runFimsMsg(vmap, msg, nullptr);  // fims* p_fims)
    // if(cj)
    // {
    //     char* tmp = cJSON_PrintUnformatted(cj);
    //     if(tmp)
    //     {
    //         //if(p_fims)p_fims->Send("set", msg->replyto, nullptr, tmp);
    //         printf (" sets/blocked message [%s] \n reply [%s]\n\n\n", buffer,
    //         tmp); free((void*)tmp);
    //     }
    //     cJSON_Delete(cj);
    // }

    buffer =
        "{"
        "\"method\":\"set\","
        "\"uri\":\"/ess/variables/bms_extra/var1\","
        "\"replyto\":\"/mee\" ,"
        "\"body\": 12345"
        "}";

    msg = vm.bufferToFims(buffer);

    vm.runFimsMsg(vmap, msg, nullptr);  // fims* p_fims)

    // if(cj)
    // {
    //     char* tmp = cJSON_PrintUnformatted(cj);
    //     if(tmp)
    //     {
    //         //if(p_fims)p_fims->Send("set", msg->replyto, nullptr, tmp);
    //         printf (" /variables/bms ( should not be blocked) message [%s] \n
    //         reply [%s]\n\n\n", buffer, tmp); free((void*)tmp);
    //     }
    //     cJSON_Delete(cj);
    // }

    buffer =
        "{"
        "\"method\":\"set\","
        "\"uri\":\"/ess/variables/bms_plus/asciivar1\","
        "\"replyto\":\"/mee\" ,"
        "\"body\": {\"textval\":{\"value\":\"this is some test\"}}"
        "}";

    msg = vm.bufferToFims(buffer);
    if (!msg)
    {
        printf("%s >> nullptr mesage ,buffer [%s]\n", __func__, buffer);
    }

    vm.runFimsMsg(vmap, msg, nullptr);  // fims* p_fims)

    // if(cj)
    // {
    //     char* tmp = cJSON_PrintUnformatted(cj);
    //     if(tmp)
    //     {
    //         //if(p_fims)p_fims->Send("set", msg->replyto, nullptr, tmp);
    //         printf (" /variables/bms ( should not be blocked) message [%s] \n
    //         reply [%s]\n\n\n", buffer, tmp); free((void*)tmp);
    //     }
    //     cJSON_Delete(cj);
    // }

    buffer =
        "{"
        "\"method\":\"get\""
        ",\"uri\":\"/ess/variables/bms_extra/var1\""
        ",\"replyto\":\"/mee\" "
        //",\"body\": 12345"
        "}";

    msg = vm.bufferToFims(buffer);

    // cj =
    vm.runFimsMsg(vmap, msg, nullptr);  // fims* p_fims)

    // if(cj)
    // {
    //     char* tmp = cJSON_PrintUnformatted(cj);
    //     if(tmp)
    //     {
    //         //if(p_fims)p_fims->Send("set", msg->replyto, nullptr, tmp);
    //         printf (" /variables/bms ( should be blocked) message [%s] \n reply
    //         [%s]\n\n\n", buffer, tmp); free((void*)tmp);
    //     }
    //     cJSON_Delete(cj);
    // }

    printCompVM(vm, vmap, "/variables/bms_extra", "*");
    printCompVM(vm, vmap, "/sets/*", "*");

    // printVars(vm, vmap);

    // delete bms_man;
    vm.clearVmap(vmap);

    return 0;
}
