#ifndef LOGFUNCS_CPP
#define LOGFUNCS_CPP

#include "asset.h"
#include "formatters.hpp"

extern "C++" {
    int LogInfo(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int LogDebug(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int LogWarn(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int LogError(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int LogIt(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);    
    // int SetLoggingSize(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
}

int LogInfo(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    // enum ATypes { AINT, AFLOAT, ASTRING, ABOOL, AAVAR, AEND };
    switch (Av->aVal->type)
    {
    case assetVal::ATypes::AINT :
        FPS_PRINT_INFO("[{}] received a value of [{}]",
            Av->getfName(), Av->getiVal());

        ESSLogger::get().info("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getiVal());
        break;
    case assetVar::ATypes::ABOOL :
        FPS_PRINT_INFO("[{}] received a value of [{}]",
            Av->getfName(), Av->getbVal());

        ESSLogger::get().info("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getbVal());
        break;
    case assetVar::ATypes::AFLOAT :
        FPS_PRINT_INFO("[{}] received a value of [{}]",
            Av->getfName(), Av->getdVal());

        ESSLogger::get().info("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getdVal());
        break;
    case assetVar::ATypes::ASTRING :
        FPS_PRINT_INFO("[{}] received a value of [{}]",
            Av->getfName(), Av->getcVal());

        ESSLogger::get().info("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getcVal());
        break;
    default: // is not a common json value, is null, or is another assetVar somehow (type = AAVAR)
        FPS_PRINT_INFO("[{}] received a value of [error]",
            Av->getfName());

        ESSLogger::get().info("[{}] called [{}] with a value of [error]",
            Av->getfName(), __func__);
        break;
    }
    return 0;
}

int LogDebug(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    // enum ATypes { AINT, AFLOAT, ASTRING, ABOOL, AAVAR, AEND };
    switch (Av->aVal->type)
    {
    case assetVal::ATypes::AINT :
        FPS_PRINT_DEBUG("[{}] received a value of [{}]",
            Av->getfName(), Av->getiVal());

        ESSLogger::get().debug("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getiVal());
        break;
    case assetVar::ATypes::ABOOL :
        FPS_PRINT_DEBUG("[{}] received a value of [{}]",
            Av->getfName(), Av->getbVal());

        ESSLogger::get().debug("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getbVal());
        break;
    case assetVar::ATypes::AFLOAT :
        FPS_PRINT_DEBUG("[{}] received a value of [{}]",
            Av->getfName(), Av->getdVal());

        ESSLogger::get().debug("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getdVal());
        break;
    case assetVar::ATypes::ASTRING :
        FPS_PRINT_DEBUG("[{}] received a value of [{}]",
            Av->getfName(), Av->getcVal());

        ESSLogger::get().debug("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getcVal());
        break;
    default: // is not a common json value, is null, or is another assetVar somehow (type = AAVAR)
        FPS_PRINT_DEBUG("[{}] received a value of [error]",
            Av->getfName());

        ESSLogger::get().debug("[{}] called [{}] with a value of [error]",
            Av->getfName(), __func__);
        break;
    }
    return 0;
}

int LogWarn(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    // enum ATypes { AINT, AFLOAT, ASTRING, ABOOL, AAVAR, AEND };
    switch (Av->aVal->type)
    {
    case assetVal::ATypes::AINT :
        FPS_PRINT_WARN("[{}] received a value of [{}]",
            Av->getfName(), Av->getiVal());

        ESSLogger::get().warn("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getiVal());
        break;
    case assetVar::ATypes::ABOOL :
        FPS_PRINT_WARN("[{}] received a value of [{}]",
            Av->getfName(), Av->getbVal());

        ESSLogger::get().warn("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getbVal());
        break;
    case assetVar::ATypes::AFLOAT :
        FPS_PRINT_WARN("[{}] received a value of [{}]",
            Av->getfName(), Av->getdVal());

        ESSLogger::get().warn("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getdVal());
        break;
    case assetVar::ATypes::ASTRING :
        FPS_PRINT_WARN("[{}] received a value of [{}]",
            Av->getfName(), Av->getcVal());

        ESSLogger::get().warn("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getcVal());
        break;
    default: // is not a common json value, is null, or is another assetVar somehow (type = AAVAR)
        FPS_PRINT_WARN("[{}] received a value of [error]",
            Av->getfName());

        ESSLogger::get().warn("[{}] called [{}] with a value of [error]",
            Av->getfName(), __func__);
        break;
    }
    return 0;
}

int LogError(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    // enum ATypes { AINT, AFLOAT, ASTRING, ABOOL, AAVAR, AEND };
    switch (Av->aVal->type)
    {
    case assetVal::ATypes::AINT :
        FPS_PRINT_ERROR("[{}] received a value of [{}]",
            Av->getfName(), Av->getiVal());

        ESSLogger::get().error("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getiVal());
        break;
    case assetVar::ATypes::ABOOL :
        FPS_PRINT_ERROR("[{}] received a value of [{}]",
            Av->getfName(), Av->getbVal());

        ESSLogger::get().error("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getbVal());
        break;
    case assetVar::ATypes::AFLOAT :
        FPS_PRINT_ERROR("[{}] received a value of [{}]",
            Av->getfName(), Av->getdVal());

        ESSLogger::get().error("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getdVal());
        break;
    case assetVar::ATypes::ASTRING :
        FPS_PRINT_ERROR("[{}] received a value of [{}]",
            Av->getfName(), Av->getcVal());

        ESSLogger::get().error("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getcVal());
        break;
    default: // is not a common json value, is null, or is another assetVar somehow (type = AAVAR)
        FPS_PRINT_ERROR("[{}] received a value of [error]",
            Av->getfName());

        ESSLogger::get().error("[{}] called [{}] with a value of [error]",
            Av->getfName(), __func__);
        break;
    }
    return 0;
}

// also known as: LogCritical (calls critical before calling LogIt)
int LogIt(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    if(!checkAv(vmap, amap, aname, p_fims, Av))
    {
        FPS_PRINT_ERROR(">> ERROR unable to continue aname [{}]", aname);
        return -1;
    }

    char* LogDir = getLogDir(vmap, *Av->am->vm);
    // enum ATypes { AINT, AFLOAT, ASTRING, ABOOL, AAVAR, AEND };
    switch (Av->aVal->type)
    {
    case assetVal::ATypes::AINT :
        ESSLogger::get().critical("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getiVal());
        break;
    case assetVar::ATypes::ABOOL :
        ESSLogger::get().critical("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getbVal());
        break;
    case assetVar::ATypes::AFLOAT :
        ESSLogger::get().critical("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getdVal());
        break;
    case assetVar::ATypes::ASTRING :
        ESSLogger::get().critical("[{}] called [{}] with a value of [{}]",
            Av->getfName(), __func__, Av->getcVal());
        break;
    default: // is not a common json value, is null, or is another assetVar somehow (type = AAVAR)
        ESSLogger::get().critical("[{}] called [{}] with a value of [error]",
            Av->getfName(), __func__);
        break;
    }

    // NOTE(WALKER): if logging isn't enabled then no log file will be produced
    // however, the logs will still accumulate in the log circular buffer
    // this means logging can be anbled and disabled at runtime at will
    // by default no log files are produced when LogIt is called (logging_enabled == false).
    if (!getLoggingEnabled(vmap, *Av->am->vm)) return 0;

    std::string dirAndFile = fmt::format("{}/{}_{}.{}", LogDir, Av->name, "logItCall", "txt");
    ESSLogger::get().logIt(dirAndFile, getLoggingTimestamp(vmap, *Av->am->vm));
    return 0;
}

// int SetLoggingSize(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
// {
//     if(!checkAv(vmap, amap, aname, p_fims, Av))
//     {
//         FPS_PRINT_ERROR(">> ERROR unable to continue aname [{}]", aname);
//         return -1;
//     }

//     setLoggingSize(vmap, *Av->am->vm);
//     return 0;
// }

#endif