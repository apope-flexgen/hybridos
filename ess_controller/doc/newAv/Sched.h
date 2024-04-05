#ifndef __SCHED_H
#define __SCHED_H

int addSchedReq(alist* sreqs, AssetVar* av);
double getSchedDelay(AssetVar* vmap, alist* rreqs, fims* p_fims);
void schedThread(AssetVar* av, alist* sreqs, const char* aname, fims* p_fims);

#endif
