#ifndef SYSTEM_ENUMS
#define SYSTEM_ENUMS

namespace Power_Electronics
{
enum Inverter_Status_Messages
{
    POWERUP = 0,        // PUP
    INIT = 1,           // initialization
    OFF = 2,            // Off
    PRECHARGE = 3,      // PCHG
    READY = 4,          // REA
    WAIT = 5,           // Wait
    ON = 6,             // On
    STOP = 7,           // Stop
    DISCHARGE = 8,      // DISC
    FAULT = 9,          // FLT
    LVRT = 10,          // Low Voltage Ride Through (algorithm is running)
    OVRT = 11,          // Over Voltage Ride Through (algorithm is running)
    NIGHT_MODE = 12,    // NGHT
    NIGHT_DC_OFF = 13,  // NDCO
    STANDBY = 14,       // STB
    HVPL = 15,          // high voltage phase lost
    // What happened to 16?!
    PRE_ON = 17,            // PRON
    SELF_DIAGNOSIS = 18,    // DIAG
    LCON = 19,              // LC filter contactors on/activated
    PREMAGENTIZATION = 20,  // PRMG
    BANK_BALANCING = 21,    // BBAL
    CVSB = 22               // cv standby algorithm running
};
}  // namespace Power_Electronics

namespace CATL
{
enum BMS_Status_Messages
{
    INIT = 0,
    NORMAL = 1,
    FULLCHARGE = 2,
    FULLDISCHARGE = 3,
    WARNING = 4,
    FAULT = 5
};
enum BMS_Commands
{
    INITIAL = 0,
    STAYSTATUS = 1,
    POWERON = 2,
    POWEROFF = 3
};
enum BMS_Poweron
{
    POWEROFFREADY = 0,
    POWERONREADY = 1,
    POWERONFAULT = 2,
    POWEROFFUALT = 3
};
}  // namespace CATL

#endif