//test_single.cpp
// p. wilshire 12/20/2021
// test the config read and load into the outstation stack


#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>

//#include <opendnp3/logging/LogLevels.h>
//#include <opendnp3/outstation/IUpdateHandler.h>

#include <opendnp3/DNP3Manager.h>
//#include <opendnp3/outstation/UpdateBuilder.h>
#include "dnp3_utils.h"
#include <iostream>

using namespace std;

class SysCfg;

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

    for (int ixs = 0 ; ixs < num_configs; ixs++ )
    {
        sysCfg *sys = sys_cfg[ixs];
        delete sys->OSconfig;
    }

    sys1->clear();
    delete sys_cfg[0];
    

    return 0;
}