#ifndef UPDATESYSTIME_CPP
#define UPDATESYSTIME_CPP

#include <chrono>
#include <ctime>

#include "asset.h"
#include "formatters.hpp"

extern "C++" {
int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

/**
 * @brief Converts the tm struct to string without new line. Referenced from
 * http://www.cplusplus.com/reference/ctime/asctime/
 *
 * @param timeptr the pointer to time struct
 * @return char* the string representation of date and time
 */
// char* mystrtime(const struct tm *timeptr);

// {
//     static const char wday_name[][4] = {
//         "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
//     };
//     static const char mon_name[][4] = {
//         "Jan", "Feb", "Mar", "Apr", "May", "Jun",
//         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
//     };

//     static char result[64];
//     snprintf(result, sizeof(result), "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
//         wday_name[timeptr->tm_wday],
//         mon_name[timeptr->tm_mon],
//         timeptr->tm_mday, timeptr->tm_hour,
//         timeptr->tm_min, timeptr->tm_sec,
//         1900 + timeptr->tm_year);

//     return result;
// }

/**
 * @brief Updates the system's TOD (year, month, day, hour, min, sec) every
 * second
 *
 * @param vmap the global data map
 * @param amap the local data map
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object to send out data to
 * @param am the asset manager
 */

#define TBUFFER_SIZE 80
int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(p_fims);
    using namespace std::chrono;
    asset_manager* am = av->am;
    VarMapUtils* vm = am->vm;

    // essLogger eLog(am, essName, "UpdateSysTime");

    tm* local_tm = vm->get_local_time_now();

    int year = local_tm->tm_year + 1900;
    int month = 1 + local_tm->tm_mon;
    int day = local_tm->tm_mday;
    int hour = local_tm->tm_hour;
    int min = local_tm->tm_min;
    int sec = local_tm->tm_sec;
    double tNow = vm->get_time_dbl();
    char tbuffer[TBUFFER_SIZE];
    bool bval = false;
    if (0)
    {
        FPS_PRINT_INFO("Year: {}", year);
        FPS_PRINT_INFO("Month: {}", month);
        FPS_PRINT_INFO("Day: {}", day);
        FPS_PRINT_INFO("Time: {}:{}:{}", hour, min, sec);
    }

    int ival = 0;
    int reload = vm->CheckReload(vmap, amap, aname, "UpdateSysTime");
    if (reload < 2)
    {
        ival = 0;
        // dval = 1.0;
        // bool bval = false;
        // Link This to an incoming component
        if (1)
            FPS_PRINT_INFO("{} reload first run {}", aname, reload);
        amap["Year"] = vm->setLinkVal(vmap, aname, "/status", "Year", year);
        amap["Month"] = vm->setLinkVal(vmap, aname, "/status", "Month", month);
        amap["Day"] = vm->setLinkVal(vmap, aname, "/status", "Day", day);
        amap["Hour"] = vm->setLinkVal(vmap, aname, "/status", "Hour", hour);
        amap["Min"] = vm->setLinkVal(vmap, aname, "/status", "Min", min);
        amap["Sec"] = vm->setLinkVal(vmap, aname, "/status", "Sec", sec);
        amap["Heartbeat"] = vm->setLinkVal(vmap, aname, "/status", "Heartbeat", ival);
        if (!amap["Heartbeat"]->gotParam("MaxHeartbeat"))
            amap["Heartbeat"]->setParam("MaxHeartbeat", 60);

        amap["Debug"] = vm->setLinkVal(vmap, aname, "/config", "UpdateSysTimeDebug", ival);
        amap["Send"] = vm->setLinkVal(vmap, aname, "/config", "UpdateSysTimeSend", ival);
        bval = true;
        amap["SendAll"] = vm->setLinkVal(vmap, aname, "/config", "UpdateSysTimeSendAll", bval);
        amap["SendAllOK"] = vm->setLinkVal(vmap, aname, "/config", "UpdateSysTimeSendAllOK", bval);
        amap["SendHB"] = vm->setLinkVal(vmap, aname, "/config", "UpdateSysTimeSendHB", bval);
        amap["tNow"] = vm->setLinkVal(vmap, aname, "/status", "tNow", tNow);

        // If we are the ess controller
        if (!am->am)
        {
            strftime(tbuffer, TBUFFER_SIZE, "%c.", local_tm);
            amap["timeString"] = vm->setLinkVal(vmap, aname, "/status", "timeString", tbuffer);
        }

        if (reload == 0)  // complete restart
        {
            amap["tNow"]->setVal(tNow);
            amap["tNow"]->setParam("tLast", tNow);
            amap["SendAll"]->setVal(true);
            amap["SendHB"]->setVal(true);
        }
        // reset reload
        ival = 2;
        amap["UpdateSysTime"]->setVal(ival);
    }

    double tLast = amap["tNow"]->getdParam("tLast");
    double tDiff = tNow - tLast;
    int debug = amap["Debug"]->getiVal();
    // debug = 1;
    int send = amap["Send"]->getiVal();
    bool sendAll = amap["SendAll"]->getbVal();
    bool sendHB = amap["SendHB"]->getbVal();
    if (debug)
        FPS_PRINT_INFO("Time before {} / Time diff {}", tLast, tDiff);
    if (tNow - tLast >= 1.0)
    {
        if (!am->am)
        {
            strftime(tbuffer, TBUFFER_SIZE, "%c.", local_tm);
            amap["timeString"]->setVal(tbuffer);
            if (debug)
                FPS_PRINT_INFO("aname [{}] Timestring [{}]", aname, tbuffer);
        }
        else
        {
            if (debug)
                FPS_PRINT_INFO("aname [{}] Year {}   Month {}   Day {}   Time {}:{}:{} [{}:{}]", aname, year, month,
                               day, hour, min, sec, amap["Sec"]->comp, amap["Sec"]->name);
        }

        amap["tNow"]->setVal(tNow);
        amap["tNow"]->setParam("tLast", tNow);
        // Update the ess sys time first, if possible
        if (!am->am)
        {
            amap["Year"]->setVal(year);
            amap["Month"]->setVal(month);
            amap["Day"]->setVal(day);
            amap["Hour"]->setVal(hour);
            amap["Min"]->setVal(min);
            amap["Sec"]->setVal(sec);
            int hb = amap["Heartbeat"]->getiVal();
            if (amap["Heartbeat"]->gotParam("MaxHeartbeat"))
            {
                if (hb >= amap["Heartbeat"]->getiParam("MaxHeartbeat"))
                    amap["Heartbeat"]->setVal(0);
                else
                    amap["Heartbeat"]->setVal(hb + 1);
            }
            else
            {
                FPS_PRINT_INFO("no MaxHeartbeat parameter found for {}", aname);
                amap["Heartbeat"]->setVal(0);
            }

            // Cascade down to other asset managers and update their system time
            assetVar Avc;

            for (auto& ass_man : am->assetManMap)
            {
                Avc.am = ass_man.second;
                if (0)
                    FPS_PRINT_INFO("I am the {} asset manager", ass_man.second->name);
                UpdateSysTime(vmap, ass_man.second->amap, ass_man.second->name.c_str(), ass_man.second->p_fims, &Avc);
            }
        }
        // Otherwise, update the sys time for the bms/pcs/other asset managers
        else
        {
            if (0)
                FPS_ERROR_PRINT("%s >> I am the %s asset manager\n", __func__, aname);
            amap["Year"]->setVal(year);
            amap["Month"]->setVal(month);
            amap["Day"]->setVal(day);
            amap["Hour"]->setVal(hour);
            amap["Min"]->setVal(min);
            amap["Sec"]->setVal(sec);
            // allow a sync 01:01:01
            if ((hour == 1) && (min == 1) && (sec == 1))
            {
                amap["SendAll"]->setVal(true);
            }

            int hb = amap["Heartbeat"]->getiVal();
            if (amap["Heartbeat"]->gotParam("MaxHeartbeat"))
            {
                if (hb >= amap["Heartbeat"]->getiParam("MaxHeartbeat"))
                    amap["Heartbeat"]->setVal(1);
                else
                    amap["Heartbeat"]->setVal(hb + 1);
            }
            else
            {
                FPS_PRINT_INFO("no MaxHeartbeat parameter found for {}", aname);
                amap["Heartbeat"]->setVal(0);
            }
            if (0)
                FPS_ERROR_PRINT("%s >> I am the %s sending heartbeats send %d  [%s] \n", __func__, aname, send,
                                am->sendOK ? "true" : "false");

            // if(send > 0 && am->sendOK)
            if (am->sendOK)
            {
                if (sendAll || sendHB)
                {
                    if (0)
                        FPS_ERROR_PRINT("%s >> I am the %s sending heartbeats\n", __func__, aname);

                    varsmap* vlist = vm->createVlist();
                    if (sendAll)
                    {
                        if (amap["SendAll"])
                            amap["SendAll"]->setVal(false);

                        if (amap["SendAllOK"]->getbVal())
                        {
                            vm->addVlist(vlist, amap["Sec"]);
                            vm->addVlist(vlist, amap["Min"]);
                            vm->addVlist(vlist, amap["Hour"]);
                            vm->addVlist(vlist, amap["Year"]);
                            vm->addVlist(vlist, amap["Month"]);
                            vm->addVlist(vlist, amap["Day"]);
                        }
                    }
                    if (sendHB)
                        vm->addVlist(vlist, amap["Heartbeat"]);
                    vm->sendVlist(vm->p_fims, "set", vlist);
                    vm->clearVlist(vlist);
                }
            }
        }
    }
    return 0;
}

#endif