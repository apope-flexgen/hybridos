#pragma once
#include <vector>
#include "../../../include/gcom_dnp3_system_structs.h"

std::vector<TMWSIM_POINT*> addPointsSameUri(GcomSystem& sys);
std::vector<TMWSIM_POINT*> addPoints(GcomSystem& sys);
void setIntervalSet(std::vector<TMWSIM_POINT*>& allPoints);
void setBatchSet(std::vector<TMWSIM_POINT*>& allPoints);
void setDirectPub(std::vector<TMWSIM_POINT*>& allPoints);
void setBatchPub(std::vector<TMWSIM_POINT*>& allPoints);
void setIntervalPub(std::vector<TMWSIM_POINT*>& allPoints);
void setForced(std::vector<TMWSIM_POINT*>& allPoints);
void setUnforced(std::vector<TMWSIM_POINT*>& allPoints);