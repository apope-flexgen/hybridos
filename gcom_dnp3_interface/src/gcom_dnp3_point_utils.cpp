#include "gcom_dnp3_point_utils.h"

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
#include "gcom_dnp3_system_structs.h"

void format_point_value(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value)
{
    if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Analog)
    {
        if ((((FlexPoint *)(dbPoint->flexPointHandle))->scale) == 0.0)
        {
            switch (dbPoint->defaultStaticVariation)
            {
            case Group30Var1:
            case Group30Var3:
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<int32_t>(value));
                break;
            }
            case Group30Var2:
            case Group30Var4:
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<int16_t>(value));
                break;
            }
            case Group30Var5:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<float>(value), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group30Var6:
            default:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", value, std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            }
        }
        else
        {
            switch (dbPoint->defaultStaticVariation)
            {
            case Group30Var1:
            case Group30Var3:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<int32_t>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group30Var2:
            case Group30Var4:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<int16_t>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group30Var5:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<float>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group30Var6:
            default:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", value / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            }
        }
    }
    else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnalogOS ||
             ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt16 ||
             ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPF32 ||
             ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::AnOPInt32)
    {
        if ((((FlexPoint *)(dbPoint->flexPointHandle))->scale) == 0.0)
        {
            switch (dbPoint->defaultStaticVariation)
            {
            case Group40Var1:
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<int32_t>(value));
                break;
            }
            case Group40Var2:
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<int16_t>(value));
                break;
            }
            case Group40Var3:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<float>(value), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group40Var4:
            default:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", value, std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            }
        }
        else
        {
            switch (dbPoint->defaultStaticVariation)
            {
            case Group40Var1:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<int32_t>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group40Var2:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<int16_t>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group40Var3:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<float>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group40Var4:
            default:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", value / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            }
        }
    }
    else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Binary ||
    ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::BinaryOS ||
             ((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::CROB)
    {
        if (((FlexPoint *)(dbPoint->flexPointHandle))->scale < 0)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
            {
                FORMAT_TO_BUF(send_buf, R"("{}")", static_cast<bool>(value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true));
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<bool>(value) ? 0 : 1);
            }
            else
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<bool>(value) ? "false" : "true");
            }
        }
        else
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_string)
            {
                FORMAT_TO_BUF(send_buf, R"("{}")", static_cast<bool>(value) ? (((FlexPoint *)(dbPoint->flexPointHandle))->crob_true) : (((FlexPoint *)(dbPoint->flexPointHandle))->crob_false));
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->crob_int)
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<bool>(value) ? 1 : 0);
            }
            else
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<bool>(value) ? "true" : "false");
            }
        }
    } else {
        FORMAT_TO_BUF(send_buf, R"({})", 0);
    }
}

void format_timestamp(fmt::memory_buffer &send_buf, TMWDTIME *tmpPtr) {
    FORMAT_TO_BUF(send_buf, R"("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}")", tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
}

void format_point_with_key(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value, TMWTYPES_UCHAR flags, TMWDTIME *tmpPtr){
    FORMAT_TO_BUF(send_buf, R"("{}":)", ((FlexPoint *)(dbPoint->flexPointHandle))->name);
    if(((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Naked) {
        format_point_value(send_buf, dbPoint, value);
    } else if(((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Clothed) {
        FORMAT_TO_BUF(send_buf, R"({{"value":)");
        format_point_value(send_buf, dbPoint, value);
        FORMAT_TO_BUF(send_buf, R"(}})");
    } else {
        FORMAT_TO_BUF(send_buf, R"({{"value":)");
        format_point_value(send_buf, dbPoint, value);
        FORMAT_TO_BUF(send_buf, R"(,"flags":{})", DNP_FLAGS{flags});
        FORMAT_TO_BUF(send_buf, R"(,"timestamp":)");
        format_timestamp(send_buf, tmpPtr);
        FORMAT_TO_BUF(send_buf, R"(}})");
    }
}