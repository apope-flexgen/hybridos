/*
 * here is the basc functions lib.
 *  g++ -std=c++11 -g -o ./test_ess -I ./include test/test_ess.cpp -lpthread
 * -lcjson -lfims
 */
#ifndef TEST_FUNC_LIB
#define TEST_FUNC_LIB

#include "asset.h"
#include "assetVar.h"

#include "../funcs/CheckAmHeartbeat.cpp"
#include "../funcs/CheckAmTimestamp.cpp"
#include "../test/release_fcns.cpp"
#include "../test/test_phil_fcns.cpp"
#include "../test/test_release_fcns.cpp"
//#include "../funcs/CheckMonitorVar.cpp"
#include "../funcs/HandleAssetHeartbeat.cpp"
#include "../funcs/SimHandleHeartbeat.cpp"
//#include "../funcs/UpdateSysTime.cpp"
#include "../funcs/CheckEssStatus.cpp"
#include "../funcs/module_faultFuncs.cpp"
#include "../funcs/module_runFuncs.cpp"

#endif
