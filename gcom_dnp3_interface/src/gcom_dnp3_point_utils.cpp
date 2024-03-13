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
    else if (((FlexPoint *)(dbPoint->flexPointHandle))->type == Register_Types::Counter)
    {
        if ((((FlexPoint *)(dbPoint->flexPointHandle))->scale) == 0.0)
        {
            switch (dbPoint->defaultStaticVariation)
            {
            case Group20Var1:
            case Group20Var3:
            case Group20Var5:
            case Group20Var7:
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<uint32_t>(value));
                break;
            }
            case Group20Var2:
            case Group20Var4:
            case Group20Var6:
            case Group20Var8:
            {
                FORMAT_TO_BUF(send_buf, R"({})", static_cast<uint16_t>(value));
                break;
            }
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
            case Group20Var1:
            case Group20Var3:
            case Group20Var5:
            case Group20Var7:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<uint32_t>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
            case Group20Var2:
            case Group20Var4:
            case Group20Var6:
            case Group20Var8:
            {
                FORMAT_TO_BUF(send_buf, R"({:.{}g})", static_cast<uint16_t>(value) / (((FlexPoint *)(dbPoint->flexPointHandle))->scale), std::numeric_limits<double>::max_digits10 - 1);
                break;
            }
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
    }
    else
    {
        FORMAT_TO_BUF(send_buf, R"({})", 0);
    }
}

void format_bitfield(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value)
{
    uint32_t int_value = static_cast<uint32_t>(value);

    bool is_first = true;
    FORMAT_TO_BUF(send_buf, R"([)", NULL);
    for (uint32_t i = 0; i < ((FlexPoint *)dbPoint->flexPointHandle)->dbBits.size(); i++)
    {
        if ((int_value & (1 << i)) != 0 && ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first != "Unknown")
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                FORMAT_TO_BUF(send_buf, R"(,)", NULL);
            }
            FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"{}"}})", ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].second, ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first);
        }
    }
    FORMAT_TO_BUF(send_buf, R"(])", NULL);
}

void format_enum(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value)
{
    uint32_t int_value = static_cast<uint32_t>(value);
    bool found_bit = false;

    FORMAT_TO_BUF(send_buf, R"([)", NULL);

    for (uint32_t i = 0; i < ((FlexPoint *)dbPoint->flexPointHandle)->dbBits.size(); i++)
    {
        if (int_value == ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].second){
        FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"{}"}})", int_value, ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first);
        found_bit = true;
        }
    }
    if (!found_bit){
        FORMAT_TO_BUF(send_buf, R"({{"value":{},"string":"Unknown"}})", int_value);
    }
    FORMAT_TO_BUF(send_buf, R"(])", NULL);
}

void format_individual_bit(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value, TMWTYPES_UCHAR flags, TMWDTIME *tmpPtr, std::string bit)
{
    if (bit == "Unknown") {
        return;
    }
    uint32_t int_value = static_cast<uint32_t>(value);

    for (uint32_t i = 0; i < ((FlexPoint *)dbPoint->flexPointHandle)->dbBits.size(); i++)
    {
        if (((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first == bit)
        {
            if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Naked)
            {
                FORMAT_TO_BUF(send_buf, R"({})", ((int_value & (1 << i)) != 0) ? "true" : "false");
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Clothed)
            {
                FORMAT_TO_BUF(send_buf, R"({{"value":)", NULL);
                FORMAT_TO_BUF(send_buf, R"({})", ((int_value & (1 << i)) != 0) ? "true" : "false");
                FORMAT_TO_BUF(send_buf, R"(}})");
            }
            else
            {
                FORMAT_TO_BUF(send_buf, R"({{"value":)", NULL);
                FORMAT_TO_BUF(send_buf, R"({})", ((int_value & (1 << i)) != 0) ? "true" : "false");
                FORMAT_TO_BUF(send_buf, R"(,"flags":{})", DNP_FLAGS{flags});
                FORMAT_TO_BUF(send_buf, R"(,"timestamp":)");
                format_timestamp(send_buf, tmpPtr);
                FORMAT_TO_BUF(send_buf, R"(}})");
            }
            break;
        }
    }
}

void format_individual_bits(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value, TMWTYPES_UCHAR flags, TMWDTIME *tmpPtr)
{
    uint32_t int_value = static_cast<uint32_t>(value);

    bool is_first = true;
    for (uint32_t i = 0; i < ((FlexPoint *)dbPoint->flexPointHandle)->dbBits.size(); i++)
    {
        if (((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first != "Unknown")
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                FORMAT_TO_BUF(send_buf, R"(,)", NULL);
            }
            if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Naked)
            {
                FORMAT_TO_BUF(send_buf, R"("{}":{})", ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first, ((int_value & (1 << i)) != 0) ? "true" : "false");
            }
            else if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Clothed)
            {
                FORMAT_TO_BUF(send_buf, R"("{}":{{"value":)", ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first);
                FORMAT_TO_BUF(send_buf, R"({})", ((int_value & (1 << i)) != 0) ? "true" : "false");
                FORMAT_TO_BUF(send_buf, R"(}})");
            }
            else
            {
                FORMAT_TO_BUF(send_buf, R"("{}":{{"value":)", ((FlexPoint *)dbPoint->flexPointHandle)->dbBits[i].first);
                FORMAT_TO_BUF(send_buf, R"({})", ((int_value & (1 << i)) != 0) ? "true" : "false");
                FORMAT_TO_BUF(send_buf, R"(,"flags":{})", DNP_FLAGS{flags});
                FORMAT_TO_BUF(send_buf, R"(,"timestamp":)");
                format_timestamp(send_buf, tmpPtr);
                FORMAT_TO_BUF(send_buf, R"(}})");
            }
        }
    }
}

void format_timestamp(fmt::memory_buffer &send_buf, TMWDTIME *tmpPtr)
{
    if (tmpPtr){
        FORMAT_TO_BUF(send_buf, R"("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}")", tmpPtr->year, tmpPtr->month, tmpPtr->dayOfMonth, tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs / 1000, tmpPtr->mSecsAndSecs % 1000);
    } else {
        FORMAT_TO_BUF(send_buf, R"("null")");
    }

}

void format_point(fmt::memory_buffer &send_buf, TMWSIM_POINT *dbPoint, double &value, TMWTYPES_UCHAR flags, TMWDTIME *tmpPtr, bool include_key)
{
    if (((FlexPoint *)(dbPoint->flexPointHandle))->is_individual_bits)
    {
        if(!include_key) { // basically implies that this is a raw message body for the individual bits ONLY
            FORMAT_TO_BUF(send_buf, R"({{)", NULL);
        }
        format_individual_bits(send_buf, dbPoint, value, flags, tmpPtr);
        if(!include_key) {
            FORMAT_TO_BUF(send_buf, R"(}})", NULL);
        }
    }
    else
    {
        if(include_key) {
            FORMAT_TO_BUF(send_buf, R"("{}":)", ((FlexPoint *)(dbPoint->flexPointHandle))->name);
        }
        if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Naked)
        {
            if(((FlexPoint *)(dbPoint->flexPointHandle))->is_bitfield) {
                format_bitfield(send_buf, dbPoint, value);
            } else if (((FlexPoint *)(dbPoint->flexPointHandle))->is_enum){
                format_enum(send_buf, dbPoint, value);
            } else {
                format_point_value(send_buf, dbPoint, value);
            }
        }
        else if (((FlexPoint *)(dbPoint->flexPointHandle))->format == FimsFormat::Clothed)
        {
            FORMAT_TO_BUF(send_buf, R"({{"value":)");
            if(((FlexPoint *)(dbPoint->flexPointHandle))->is_bitfield) {
                format_bitfield(send_buf, dbPoint, value);
            } else if (((FlexPoint *)(dbPoint->flexPointHandle))->is_enum){
                format_enum(send_buf, dbPoint, value);
            } else {
                format_point_value(send_buf, dbPoint, value);
            }
            FORMAT_TO_BUF(send_buf, R"(}})");
        }
        else
        {
            FORMAT_TO_BUF(send_buf, R"({{"value":)");
            if(((FlexPoint *)(dbPoint->flexPointHandle))->is_bitfield) {
                format_bitfield(send_buf, dbPoint, value);
            } else if (((FlexPoint *)(dbPoint->flexPointHandle))->is_enum){
                format_enum(send_buf, dbPoint, value);
            } else {
                format_point_value(send_buf, dbPoint, value);
            }
            FORMAT_TO_BUF(send_buf, R"(,"flags":{})", DNP_FLAGS{flags});
            FORMAT_TO_BUF(send_buf, R"(,"timestamp":)");
            format_timestamp(send_buf, tmpPtr);
            FORMAT_TO_BUF(send_buf, R"(}})");
        }
    }
}