//test_single.cpp
// p. wilshire 12/20/2021
// add fims i/o to the loaded config


#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <fims/libfims.h>
#include <fims/fps_utils.h>


//#include <opendnp3/logging/LogLevels.h>
//#include <opendnp3/outstation/IUpdateHandler.h>

#include <opendnp3/DNP3Manager.h>
#include <opendnp3/outstation/UpdateBuilder.h>
#include "dnp3_utils.h"
#include <iostream>

using namespace std;

class SysCfg;
// scaled onto the wire
void addVarToBuilder (UpdateBuilder& builder, DbVar* db, int debug, bool useVindex)
{
    int idx = db->idx;
    if(useVindex)
        idx = db->vindex;

    switch (db->type) 
    {
        case Type_Analog:
        {
            double dval = db->valuedouble;
            if(debug>0)
            {
                FPS_ERROR_PRINT("*****  %s  var name [%s] db->idx [%d] db->vindex [%d] idx [%d] value [%f]\n"
                ,__FUNCTION__, db->name.c_str(), db->idx, db->vindex, idx, dval);
            }
            if((db->scale > 0.0) || (db->scale < 0.0))
            {
                dval *= db->scale;
                if(debug>0)
                {
                    FPS_ERROR_PRINT("*****  %s  var name [%s] idx [%d] vindex [%d] scaled value [%f]\n"
                    ,__FUNCTION__, db->name.c_str(), db->idx, db->vindex, dval);
                }
            }
            builder.Update(Analog(dval), idx);
            break;
        }
        case Type_AnalogOS:
        {
            builder.Update(AnalogOutputStatus(db->valuedouble), idx);
            break;
        }
        case Type_Binary:
        {
            int32_t vint = static_cast<int32_t>(db->valuedouble);
            if(db->scale < 0.0) 
            {
                if(vint > 0)
                    vint = 0;
                else
                    vint = 1;
            }

            builder.Update(Binary(vint), idx);
            break;
        }
        case Type_BinaryOS:
        {
            int32_t vint = static_cast<int32_t>(db->valuedouble);
            builder.Update(BinaryOutputStatus(vint), idx);
            break;
        }
        default:
            break;
    }
}

SysCfg * main_test( int argc, char*argv[], int id)
{
    SysCfg *sys = SysCfg::getInstance();
    sys->setId(id);
    std::cout << " \n\n test \n";
    std::cout << "..."<< argv[0] << " running \n";
    std::cout << "...  num_cfgs : "<< sys->getNum() << "\n";
    std::cout << "...  id       :"<< sys->getId() << "\n";

    return sys;
}

// std::map<uint16_t, BinaryConfig> binary_input;
//     std::map<uint16_t, DoubleBitBinaryConfig> double_binary;
//     std::map<uint16_t, AnalogConfig> analog_input;
//     std::map<uint16_t, CounterConfig> counter;
//     std::map<uint16_t, FrozenCounterConfig> frozen_counter;
//     std::map<uint16_t, BOStatusConfig> binary_output_status;
//     std::map<uint16_t, AOStatusConfig> analog_output_status;
//     std::map<uint16_t, TimeAndIntervalConfig> time_and_interval;
//     std::map<uint16_t, OctetStringConfig> octet_string;

void test_stack(OutstationStackConfig *config)
{

   cout << " map_sizes :\n";
   cout <<" binary_input         :" <<  config->database.binary_input.size() << " \n"; 
   cout <<" double_binary        :" <<  config->database.double_binary.size() << " \n"; 
   cout <<" analog_input         :" <<  config->database.analog_input.size() << " \n"; 
   cout <<" counter              :" <<  config->database.counter.size() << " \n"; 
   cout <<" frozen_counter       :" <<  config->database.frozen_counter.size() << " \n"; 
   cout <<" binary_output_status :" <<  config->database.binary_output_status.size() << " \n"; 
   cout <<" analog_output_status :" <<  config->database.analog_output_status.size() << " \n"; 
   cout <<" time_and_interval    :" <<  config->database.time_and_interval.size() << " \n"; 
   cout <<" octet_string         :" <<  config->database.octet_string.size() << " \n"; 

}

// Moved to dnp3_utils.cpp
//fill out from system config
DatabaseConfig ConfigureDatabase(sysCfg* sys);

// { 
//     DatabaseConfig config(0); // 10 of each type with default settings
  
//     // just deal with analog vars and Group30Var5, this allows floating point numbers through the system
//     //auto dsize = sys->dbVec[Type_Analog].size();
//     auto dsize = sys->getTypeSize(Type_Analog);
//     for (int i = 0; i < static_cast<int32_t>(dsize); i++)
//     {
//         DbVar* db = sys->getDbVarId(Type_Analog, i);
//         if(db != NULL)
//         {
//             if(db->variation == Group30Var5)
//             {
//                 config.analog_input[i].svariation = StaticAnalogVariation::Group30Var5;
//             }
//             else if(db->variation == Group30Var2)
//             {
//                 config.analog_input[i].svariation = StaticAnalogVariation::Group30Var2;
//             }
//             if(db->evariation == Group32Var0)
//             {
//                 //TODO config.analog_input[i].evariation = EventAnalogVariation::Group32Var0;
//             }
//             else if(db->evariation == Group32Var1)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var1;
//             }
//             else if(db->evariation == Group32Var2)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var2;
//             }
//             else if(db->evariation == Group32Var3)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var3;
//             }
//             else if(db->evariation == Group32Var4)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var4;
//             }
//             else if(db->evariation == Group32Var5)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var5;
//             }
//             else if(db->evariation == Group32Var6)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var6;
//             }
//             else if(db->evariation == Group32Var7)
//             {
//                 config.analog_input[i].evariation = EventAnalogVariation::Group32Var7;
//             }
//             if (db->deadband > 0.0) {
//                 config.analog_input[i].deadband = db->deadband;
//             }
//             else
//             {
//                 config.analog_input[i].deadband = sys->deadband;
//             }
            
    
//             if(db->clazz != 0)
//             {
//                 switch (db->clazz ) 
//                 {
//                     case 1:
//                     {
//                         config.analog_input[i].clazz = PointClass::Class1;
//                         break;
//                     }
//                     case 2:
//                     {
//                         config.analog_input[i].clazz = PointClass::Class2;
//                         break;
//                     }
//                     case 3:
//                     {
//                         config.analog_input[i].clazz = PointClass::Class3;
//                         break;
//                     }
//                     default:
//                         break;
//                 }
//                 //if (sys->useVindex)
//                 //    config.analog_input[i].vIndex = db->idx;
//             }

//             // else if(db->variation == Group30Var2)
//             // {
//             //     config.analog_input[i].svariation = StaticAnalogVariation::Group30Var2;
//             // }
//         }
//     }
//     auto bsize = sys->getTypeSize(Type_Binary);
//     for (int i = 0; i < static_cast<int32_t>(bsize); i++)
//     {
//         DbVar* db = sys->getDbVarId(Type_Binary , i);
//         if(db != NULL)
//         {
//             if(db->variation == Group1Var1)
//             {
//                 config.binary_input[i].svariation = StaticBinaryVariation::Group1Var1;
//             }
//             //if (sys->useVindex)
//             //    config.binary_input[i].vIndex = db->idx;
//         }
//     }

//     // example of configuring analog index 0 for Class2 with floating point variations by default
//     //config.analog_input[0].clazz = PointClass::Class2;
//     //config.analog_input[0].svariation = StaticAnalogVariation::Group30Var5;
//     //config.analog_input[0].evariation = EventAnalogVariation::Group32Var7;
//     //config.analog_input[0].deadband = 1.0; ///EventAnalogVariation::Group32Var7;   
//     return config;
// }

void setConfigUnsol(sysCfg* sys, OutstationStackConfig *config)
{
    if(sys->unsol == 0)
        config->outstation.params.allowUnsolicited = false;
    else
        config->outstation.params.allowUnsolicited = true;
}

int main( int argc, char*argv[])
{
    //bool running = true;
    sysCfg *sys_cfg[16];

    int num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_OUTSTATION, nullptr);
    std::cout << "...  num_configs    :"<< num_configs << "\n";
    SysCfg* sys1 = main_test(argc, argv, 21);
    std::cout << "...  sys1 id       :"<< sys1->getId() << "\n";

  // put returned outstation into the config context
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        if(1||sys->debug)
        {
            cout<<"*** Binaries: dbVec: "<<sys->dbVec[Type_Binary].size()
                << " indx :" << sys->getTypeSize(Type_Binary) << endl;

            cout<<"*** Analogs: dbVec: "<<sys->dbVec[Type_Analog].size()
                <<" indx :" << sys->getTypeSize(Type_Analog)+1 << endl;
        }
        sys->OSconfig = new OutstationStackConfig(ConfigureDatabase(sys));
    }
    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        test_stack(sys->OSconfig);
    }



    fims* p_fims = new fims();
    int fims_connect = 0;
    int rc = 0;

    if (p_fims == NULL)
    {
        rc = 1;
        FPS_ERROR_PRINT("Failed to allocate connection to FIMS server rc = %d.\n", rc);
        return 1;
        //goto cleanup;
    }
    
    FPS_ERROR_PRINT("Connected to FIMS server.\n");
    // next we get the uris to subscribe to
    const char **subs = NULL;
    bool *bpubs = NULL;
    int num = getSysUris((sysCfg**)&sys_cfg, DNP3_OUTSTATION, subs, bpubs, num_configs);
    if(num < 0)
    {
        FPS_ERROR_PRINT("Failed to create subs array.\n");
        return 1;
    }

    FPS_ERROR_PRINT(">>>>>> Num Uris found %d .\n", num);
    for (int ix = 0; ix < num; ix++ )
    {
        FPS_ERROR_PRINT(">>>>>>>>>> Uri [%d] [%s] \n", ix, subs[ix]);    
    }
    // use the id for fims connect but also add outstation designation 
    {
        char tmp[1024];
        snprintf(tmp, sizeof(tmp),"DNP3_O_%s", sys_cfg[0]->id);
        while(fims_connect < MAX_FIMS_CONNECT && p_fims->Connect(tmp) == false)
        {
            fims_connect++;
            sleep(1);
        }
    }

    if(fims_connect >= MAX_FIMS_CONNECT)
    {
        FPS_ERROR_PRINT("Failed to establish connection to FIMS server.\n");
        rc = 1;
        return 1;
        //goto cleanup;
    } 

    if(p_fims->Subscribe(subs, num, bpubs) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }

    free((void *)bpubs);
    free((void *)subs);
    int running = 1;
    int ttick = 0;
    /// here we run fims and listen for messages.
    FPS_ERROR_PRINT("Start sending ....\n");
    while(running && p_fims->Connected())
    {
        // use a time out to detect init failure 
        fims_message* msg = p_fims->Receive_Timeout(1000000);
        if(msg == NULL)
        { 
            // TODO check for all the getURI resposes
            FPS_DEBUG_PRINT("Timeout tick %d\n", ttick);
            ttick++;
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];

                bool ok = sys->checkUris(DNP3_OUTSTATION);
                if(ok == false)
                {
                    if (ttick > MAX_SETUP_TICKS)
                    {
                        // just quit here
                        FPS_DEBUG_PRINT("QUITTING TIME Timeout tick %d\n", ttick);
                    }
                }
            }
        }
        else
        {
            for (int ixs = 0 ; ixs < num_configs; ixs++ )
            {
                sysCfg *sys = sys_cfg[ixs];
                if (sys->debug)
                    FPS_ERROR_PRINT("****** Hey %s got a message uri [%s] \n", __FUNCTION__, msg->uri);
                dbs_type dbs; // collect all the parsed vars here

                cJSON* cjb = parseBody(dbs, sys, msg, DNP3_OUTSTATION);
                if(dbs.size() > 0)
                {
                    cJSON* cj = NULL;                
                    if((msg->replyto != NULL) && (strcmp(msg->method, "pub") != 0))
                        cj = cJSON_CreateObject();
                    
                    UpdateBuilder builder;
                    int varcount = 0;
                    //int format = 0;
                    while (!dbs.empty())
                    {
                        std::pair<DbVar*,int>dbp = dbs.back();
                        DbVar* db = dbp.first;
                        // only do this on sets pubs or  posts
                        if (
                            (strcmp(msg->method, "set") == 0) || 
                            (strcmp(msg->method, "post") == 0) || 
                            (strcmp(msg->method, "pub") == 0)
                            )
                            {
                                varcount++;
                                addVarToBuilder(builder, db, sys->debug, sys->useVindex);
                            }
                        addVarToCj(cj, dbp);  // include flag
                        addCjTimestamp(cj, "Timestamp");
                        dbs.pop_back();
                    }

                    //finalize set of updates
                    if(varcount > 0) 
                    {
                        auto updates = builder.Build();
                        if (sys->outstation)
                        {
                            sys->outstation->Apply(updates);
                        }
                        else
                        {
                            std::cout << " NO outstation set \n";
                        }
                    }
                    if(cj)
                    {
                        if(msg->replyto)
                        {
                            const char* reply = cJSON_PrintUnformatted(cj);
                            p_fims->Send("set", msg->replyto, NULL, reply);
                            free((void* )reply);
                        }
                        cJSON_Delete(cj);
                        cj = NULL;
                    }
                }
            
                if (sys->scanreq > 0)
                {
                    FPS_ERROR_PRINT("****** outstation scanreq %d ignored \n", sys->scanreq);
                    sys->scanreq = 0;
                }

                if (sys->unsolUpdate)
                {
                    FPS_ERROR_PRINT("****** outstation unsol %d handled \n", sys->unsol);
                    setConfigUnsol(sys, sys->OSconfig);
                    sys->unsolUpdate = false;
                }

                if (sys->cjclass != NULL)
                {
                    const char*tmp = cJSON_PrintUnformatted(sys->cjclass);
                    if(tmp != NULL)
                    {                
                        FPS_ERROR_PRINT("****** outstation class change [%s] handled \n", tmp);
                        free((void*)tmp);
                        sys->cjclass = NULL;
                    }
                }

                if (cjb != NULL)
                {
                    cJSON_Delete(cjb);
                    cjb = NULL;
                }
            }
            p_fims->free_message(msg);
        }
    }

    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        delete sys->OSconfig;
    }

    sys1->clear();
    delete sys_cfg[0];

    if (p_fims) 
    {
        p_fims->Close();
        delete p_fims;    
    }

    return 0;
}