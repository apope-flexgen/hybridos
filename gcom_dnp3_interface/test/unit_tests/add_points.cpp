#include "add_points.h"

#include <vector>

extern "C" {
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwtimer.h"
}

#include "../../include/gcom_dnp3_system_structs.h"
#include "../../include/gcom_dnp3_utils.h"

std::vector<TMWSIM_POINT*> addPoints(GcomSystem& sys)
{
    std::vector<TMWSIM_POINT*> allPoints;
    char* name = strdup("point000");
    char* uri = strdup("/0");
    char* variation = strdup("none");
    TMWSIM_POINT* dbPoint;
    for (int i = 0; i < NumTypes; i++)
    {  // number of types
        if (i == Type_AnalogOS)
        {
            continue;
        }
        for (int j = i * 10; j < (i * 10 + 10); j += 2)
        {  // number of points
            if (i == AnIn16 || i == AnIn32 || i == AnF32)
            {
                sprintf(uri, "/%d", Type_AnalogOS);
            }
            else
            {
                sprintf(uri, "/%d", i);
            }
            sprintf(name, "point%d%d", i, j);
            dbPoint = newDbVar(sys, name, i, j, uri, variation);
            ((FlexPoint*)dbPoint->flexPointHandle)->timeout = 1000;
            tmwtimer_init(&((FlexPoint*)dbPoint->flexPointHandle)->timeout_timer);
            ((FlexPoint*)dbPoint->flexPointHandle)->event_pub = false;
            ((FlexPoint*)(dbPoint->flexPointHandle))->scale = rand() % 100 + 1;
            addDbUri(sys, ((FlexPoint*)(dbPoint->flexPointHandle))->uri, dbPoint);

            tmwtimer_init(&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer);
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.send_uri.clear();
            FORMAT_TO_BUF(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.send_uri, R"({}/{})",
                          ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                          ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.dbPoint = dbPoint;
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value = 0.0;

            tmwtimer_init(&((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer);

            ((FlexPoint*)(dbPoint->flexPointHandle))->format = FimsFormat::Naked;
            allPoints.push_back(dbPoint);
        }
    }
    getSysUris(sys, sys.protocol_dependencies->who, 0);
    free(name);
    free(uri);
    free(variation);
    return allPoints;
}

std::vector<TMWSIM_POINT*> addPointsSameUri(GcomSystem& sys)
{
    std::vector<TMWSIM_POINT*> allPoints;
    char* name = strdup("point000");
    char* uri = strdup("/components/test");
    char* variation = strdup("none");
    TMWSIM_POINT* dbPoint;
    for (int i = 0; i < NumTypes; i++)
    {  // number of types
        if (i == Type_AnalogOS)
        {
            continue;
        }
        for (int j = i * 10; j < (i * 10 + 10); j += 2)
        {  // number of points
            sprintf(name, "point%d%d", i, j);
            dbPoint = newDbVar(sys, name, i, j, uri, variation);
            ((FlexPoint*)dbPoint->flexPointHandle)->timeout = 1000;
            tmwtimer_init(&((FlexPoint*)dbPoint->flexPointHandle)->timeout_timer);
            ((FlexPoint*)dbPoint->flexPointHandle)->event_pub = false;
            ((FlexPoint*)(dbPoint->flexPointHandle))->scale = rand() % 100 + 1;
            addDbUri(sys, ((FlexPoint*)(dbPoint->flexPointHandle))->uri, dbPoint);

            tmwtimer_init(&((FlexPoint*)(dbPoint->flexPointHandle))->set_timer);
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.send_uri.clear();
            FORMAT_TO_BUF(((FlexPoint*)(dbPoint->flexPointHandle))->set_work.send_uri, R"({}/{})",
                          ((FlexPoint*)(dbPoint->flexPointHandle))->uri,
                          ((FlexPoint*)(dbPoint->flexPointHandle))->name.c_str());
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.dbPoint = dbPoint;
            ((FlexPoint*)(dbPoint->flexPointHandle))->set_work.value = 0.0;

            tmwtimer_init(&((FlexPoint*)(dbPoint->flexPointHandle))->pub_timer);

            ((FlexPoint*)(dbPoint->flexPointHandle))->format = FimsFormat::Naked;
            allPoints.push_back(dbPoint);
        }
    }
    getSysUris(sys, sys.protocol_dependencies->who, 0);
    free(name);
    free(uri);
    free(variation);
    return allPoints;
}

void setIntervalSet(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_set_rate = 1000;
        ((FlexPoint*)(dbPoint->flexPointHandle))->direct_sets = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets = false;
    }
}

void setBatchSet(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_sets = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->direct_sets = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_sets = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_set_rate = 1000;
    }
}

void setDirectPub(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->direct_pubs = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs = false;
    }
}

void setBatchPub(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->direct_pubs = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pub_rate = 1000;
    }
}

void setIntervalPub(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->event_pub = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pubs = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->interval_pub_rate = 1000;
        ((FlexPoint*)(dbPoint->flexPointHandle))->direct_pubs = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->batch_pubs = false;
    }
}

void setForced(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        dbPoint->flags |= DNPDEFS_DBAS_FLAG_LOCAL_FORCED;
        ((FlexPoint*)(dbPoint->flexPointHandle))->is_forced = true;
        ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->operate_value = 0;
        ((FlexPoint*)(dbPoint->flexPointHandle))->standby_value = 0;
    }
}

void setUnforced(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        dbPoint->flags &= ~DNPDEFS_DBAS_FLAG_LOCAL_FORCED;
        ((FlexPoint*)(dbPoint->flexPointHandle))->is_forced = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->sent_operate_before_or_during_force = false;
        ((FlexPoint*)(dbPoint->flexPointHandle))->operate_value = 0;
        ((FlexPoint*)(dbPoint->flexPointHandle))->standby_value = 0;
    }
}

void setDisabled(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->is_enabled = false;
    }
}

void setEnabled(std::vector<TMWSIM_POINT*>& allPoints)
{
    for (TMWSIM_POINT* dbPoint : allPoints)
    {
        ((FlexPoint*)(dbPoint->flexPointHandle))->is_enabled = true;
    }
}