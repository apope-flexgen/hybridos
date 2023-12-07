#pragma once
#include <sstream>
#include "gcom_timer.h"

struct cfg;

void get_stats(std::stringstream &ss, struct cfg &myCfg) ;
void pubStatsCallback(TimeObject *t, void *p);
void gcom_setup_stats_pubs(struct cfg &myCfg, double init_time);