//test_single.cpp
// p. wilshire 12/20/2021
// test the config read

#include "dnp3_utils.h"
#include <iostream>

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

int main( int argc, char*argv[])
{
    //bool running = true;
    sysCfg *sys_cfg[16];

    int num_configs = getConfigs(argc, argv, (sysCfg**)&sys_cfg, DNP3_OUTSTATION, nullptr);
    std::cout << "...  num_configs    :"<< num_configs << "\n";
    SysCfg* sys1 = main_test(argc, argv, 21);
    std::cout << "...  sys1 id       :"<< sys1->getId() << "\n";
    sys1->clear();
    delete sys_cfg[0];
    

    return 0;
}