//test_single.cpp
// p. wilshire 12/20/2021
// test the singleton operation for storing sysCfg structures

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

    SysCfg* sys1 = main_test(argc, argv, 21);
    std::cout << "...  sys1 id       :"<< sys1->getId() << "\n";
    SysCfg* sys2 = main_test(argc, argv, 22);
    std::cout << "...  sys1 id       :"<< sys1->getId() << "\n";
    std::cout << "...  sys2 id       :"<< sys2->getId() << "\n";
    SysCfg* sys3 = main_test(argc, argv, 23);
    std::cout << "...  sys1 id       :"<< sys1->getId() << "\n";
    std::cout << "...  sys2 id       :"<< sys2->getId() << "\n";
    std::cout << "...  sys3 id       :"<< sys3->getId() << "\n";
    sys1->clear();

    return 0;
}