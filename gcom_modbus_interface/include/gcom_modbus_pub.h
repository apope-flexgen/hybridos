#pragma once
#include <any>
#include <map>
#include <string>
struct cfg;
struct TimeObject;
void pubCallback(TimeObject* t, void* p);
int  gcom_setup_pubs(std::map<std::string, std::any>& gcom_map, struct cfg& myCfg, double init_time, bool debug);