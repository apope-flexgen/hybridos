#include "gcom_dnp3_io_filter.h"

#include <chrono>
extern "C"
{
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/dnp/mdnpsim.h"
}

#include "logger/gcom_dnp3_logger.h"
#include "gcom_dnp3_system_structs.h"
#include "shared_utils.hpp"
#include "gcom_dnp3_flags.h"

bool isDirectPub(TMWSIM_POINT *dbPoint)
{
    return ((FlexPoint *)(dbPoint->flexPointHandle))->direct_pubs && ((FlexPoint *)(dbPoint->flexPointHandle))->event_pub;
}

bool isDirectSet(TMWSIM_POINT *dbPoint)
{
    return ((FlexPoint *)(dbPoint->flexPointHandle))->direct_sets;
}