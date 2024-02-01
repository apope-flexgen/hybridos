#include "gcom_dnp3_system_structs.h"
extern "C"
{
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwdtime.h"
}
#include <cstdint>
#include "spdlog/fmt/compile.h"
#include "spdlog/details/fmt_helper.h"
#include "spdlog/fmt/chrono.h"
#include "shared_utils.hpp"
#include "gcom_dnp3_utils.h"

void format_point_value(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value);
void format_timestamp(fmt::memory_buffer &send_buf, TMWDTIME *tmpPtr);
void format_point_with_key(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value, TMWTYPES_UCHAR flags, TMWDTIME *tmpPtr);
