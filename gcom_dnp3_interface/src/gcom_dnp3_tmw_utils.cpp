#include "gcom_dnp3_tmw_utils.h"

extern "C"
{
#include "tmwscl/utils/tmwappl.h"
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/utils/tmwtargp.h"
#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpsim.h"
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwtargio.h"
}

#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_system_structs.h"
#include "gcom_dnp3_stats.h"
#include "gcom_dnp3_client.h"
#include "gcom_dnp3_server.h"

bool init_tmw(GcomSystem &sys)
{
    DNP3Dependencies *dnp3_sys = &(sys.protocol_dependencies->dnp3);
    if (dnp3_sys->openTMWChannel == nullptr)
    {
        FPS_ERROR_LOG("Missing declaration of TMW open channel function.");
        return false;
    }
    if (dnp3_sys->openTMWSession == nullptr)
    {
        FPS_ERROR_LOG("Missing declaration of TMW open session function.");
        return false;
    }

    bool ok = tmwappl_initSCL();
    if (!ok)
    {
        FPS_ERROR_LOG("Could not initialize TMW source code library. Try killing the application and restarting it.");
        return false;
    }
    if (Logging::console.get() != nullptr)
    {
        tmwtargp_registerPutDiagStringFunc(Logging::log_TMW_message);
    }
#ifdef DNP3_TEST_MODE
    tmwtargp_registerPutDiagStringFunc([](const TMWDIAG_ANLZ_ID *pAnlzId, const TMWTYPES_CHAR *pString) {});
#endif
    tmwtimer_initialize();
    if (sys.protocol_dependencies->who == DNP3_MASTER)
    {
        tmwdb_init(10000);
    }
    dnp3_sys->pApplContext = tmwappl_initApplication();
    if (dnp3_sys->pApplContext == TMWDEFS_NULL)
    {
        FPS_ERROR_LOG("Could not initialize TMW application context. Try killing the application and restarting it.");
        return false;
    }
    tmwtarg_initConfig(&dnp3_sys->targConfig); // Initialize channel configuration to defaults
    dnpchnl_initConfig(&dnp3_sys->channelConfig, &dnp3_sys->tprtConfig, &dnp3_sys->linkConfig, &dnp3_sys->physConfig);
    tmwtargio_initConfig(&dnp3_sys->IOConfig);

    setupTMWIOConfig(sys);

    ok = sys.protocol_dependencies->dnp3.openTMWChannel(sys);
    if (!ok)
    {
        FPS_ERROR_LOG("Could not open channel");
        return false;
    }
    ok = sys.protocol_dependencies->dnp3.openTMWSession(sys);
    if (!ok)
    {
        FPS_ERROR_LOG("Could not open session");
        return false;
    }
    return true;
}

void setupTMWIOConfig(GcomSystem &sys)
{
    TMWTARGIO_CONFIG *IOConfig = &(sys.protocol_dependencies->dnp3.IOConfig);
    DNPLINK_CONFIG *linkConfig = &(sys.protocol_dependencies->dnp3.linkConfig);

    // all of this will need to change based on config settings...
    linkConfig->networkType = DNPLINK_NETWORK_TCP_UDP;

    IOConfig->type = TMWTARGIO_TYPE_TCP;
    strcpy(IOConfig->targTCP.chnlName, sys.id);
    strcpy(IOConfig->targTCP.ipAddress, sys.protocol_dependencies->ip_address);
    IOConfig->targTCP.ipAddress[63] = '\0';
    IOConfig->targTCP.ipPort = sys.protocol_dependencies->port;
    if (!sys.protocol_dependencies->dnp3.unsolUpdate)
    {
        IOConfig->targTCP.polledMode = true;
    }
    else
    {
        IOConfig->targTCP.polledMode = false;
    }
    IOConfig->targTCP.disconnectOnNewSyn = TMWDEFS_TRUE;
    IOConfig->targTCP.initUnsolUDPPort = sys.protocol_dependencies->port;

    if (sys.protocol_dependencies->who == DNP3_MASTER)
    {
        IOConfig->targTCP.mode = TMWTARGTCP_MODE_CLIENT;
        IOConfig->targTCP.role = TMWTARGTCP_ROLE_MASTER;
    }
    else
    {
        IOConfig->targTCP.mode = TMWTARGTCP_MODE_SERVER;
        IOConfig->targTCP.role = TMWTARGTCP_ROLE_OUTSTATION;
    }
}

void shutdown_tmw(DNP3Dependencies *dnp3_sys)
{
    bool client = false;
    if (((TMWSESN *)dnp3_sys->pSession)->type == TMWTYPES_SESSION_TYPE_MASTER)
    {
        if (dnp3_sys->point_status_info)
        {
            delete dnp3_sys->point_status_info;
        }
        if (dnp3_sys->analogOutputVar1Values)
        {
            delete[] dnp3_sys->analogOutputVar1Values;
        }
        if (dnp3_sys->analogOutputVar2Values)
        {
            delete[] dnp3_sys->analogOutputVar2Values;
        }
        if (dnp3_sys->analogOutputVar3Values)
        {
            delete[] dnp3_sys->analogOutputVar3Values;
        }
        if (dnp3_sys->analogOutputVar4Values)
        {
            delete[] dnp3_sys->analogOutputVar4Values;
        }
        if (dnp3_sys->CROBInfo)
        {
            delete[] dnp3_sys->CROBInfo;
        }
        mdnpsesn_closeSession(dnp3_sys->pSession);
        client = true;
    }
    else
    {
        sdnpsesn_closeSession(dnp3_sys->pSession);
    }
    dnpchnl_closeChannel(dnp3_sys->pChannel);
    tmwappl_closeApplication(dnp3_sys->pApplContext, TMWDEFS_TRUE);
    if (client)
    {
        tmwdb_destroy();
    }
    tmwtimer_close();
}