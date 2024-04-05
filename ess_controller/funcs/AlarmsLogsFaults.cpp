#ifndef ALARMSLOGSFAULTS_CPP
#define ALARMSLOGSFAULTS_CPP

#include "asset.h"

// Each assetvar can process logs.. Just like alarms but they are sent or a file
// object.
// a short buffer of say 32 entries are kept im memory
// amap["log"]->setLog(const char *fname, int memSize);
// amap["log"]->sendLog(vmap, av, varargs ...);
// amap["log"]->flushLog()
// amap["log"]->openLog(vm->runLogDir, fname)
// amap["log"]->closePerf(fname)

#include <stdarg.h>

// not like this at all...
extern "C++" {
int process_log(varsmap& vmap, assetVar* av);
int process_fault(varsmap& vmap, assetVar* av);

int process_alarm(varsmap& vmap, assetVar* av);
}

#endif
