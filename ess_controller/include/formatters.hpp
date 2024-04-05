#ifndef FORMATTERS_HPP
#define FORMATTERS_HPP

#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"
#include <map>

#include "asset.h"

// "FPS" stands for "Flexgen Power Systems"
// some useful helper macros for logging to console (this is the new standard I
// want, for safety and extensibility reasons)

// two below output to stdout:
#define FPS_PRINT_INFO(fmt, ...) SPDLOG_INFO(FMT_STRING(fmt), ##__VA_ARGS__)
#define FPS_PRINT_WARN(fmt, ...) SPDLOG_WARN(FMT_STRING(fmt), ##__VA_ARGS__)

// two below output to stderr:
#define FPS_PRINT_CRIT(fmt, ...) SPDLOG_LOGGER_CRITICAL(spdlog::get("stderr"), FMT_STRING(fmt), ##__VA_ARGS__)
#define FPS_PRINT_ERROR(fmt, ...) SPDLOG_LOGGER_ERROR(spdlog::get("stderr"), FMT_STRING(fmt), ##__VA_ARGS__)

// the one below is to stdout:
#ifdef FPS_DEBUG_MODE
// NOTE: Please change "tweakme.h" inside of /usr/local/include/spdlog/ to get
// this macro to expand out uncomment and change line 114 to -> #define
// SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG also don't forget to call
// "spdlog::set_level(spdlog::level::debug);" once at the start of main of your
// program
#define FPS_PRINT_DEBUG(fmt, ...) SPDLOG_DEBUG(FMT_STRING(fmt), ##__VA_ARGS__)
#elif FPS_TEST_MODE
#define FPS_PRINT_DEBUG(fmt, ...) SPDLOG_DEBUG(FMT_STRING(fmt), ##__VA_ARGS__)
#else
#define FPS_PRINT_DEBUG(fmt, ...)
#endif

// used for printing out null for our const char*'s and char*'s
// might look into a different fix later
struct cstr
{
    const char* str;
};

// Potential flags (I want to make this a thing in the future):
// this will probably be a good standard going forward
// c = clothed (with object {} braces and value = x) -> default
// n - naked (without object {} braces and value = x)

// these two below are extra flags and probably need to come last.
// m = minified (for fims communication) -> default
// p = prettified (spaces and indenting - might be tricky to implement, probably
// the last thing todo)

// i = interface/UI (for publishing to the UI -> this is ESS specific)
// f = full (the full assetVar -> this is ESS specific)

namespace fmt
{
// formatter for cstr (for char* == nullptr cases):
template <>
struct formatter<cstr> : formatter<const char*>
{
    using formatter<const char*>::parse;

    template <typename FormatContext>
    auto format(const cstr& c, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return formatter<const char*>::format(c.str ? c.str : "null", ctx);
    }
};

// formatter for assetVal:
template <>
struct formatter<assetVal>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    // when we get around to it, get an array working for this:
    // for options
    template <typename FormatContext>
    auto format(const assetVal& aV, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (aV.type)
        {
            case assetVal::ATypes::AINT:
                return format_to(ctx.out(), "{}", aV.valueint);
            case assetVar::ATypes::ABOOL:
                return format_to(ctx.out(), "{}", aV.valuebool);
            case assetVar::ATypes::AFLOAT:
                return format_to(ctx.out(), "{}", aV.valuedouble);
            case assetVar::ATypes::ASTRING:
                return format_to(ctx.out(), R"("{}")", cstr{ aV.valuestring });
            default:  // is not a common json value, is null, or is another assetVar
                      // somehow (type = AAVAR)
                return format_to(ctx.out(), R"("error")");
        }
    }
};

// formatter for assetVar:
// todo: get some formatting flags going for this:
// unclothed would be nice, this is currently MVP for formatting
template <>
struct formatter<assetVar>
{
    char presentation = 'c';  // clothed by default
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        auto end = ctx.end();

        if (it != end && (*it == 'c' || *it == 'n' || *it == 'i' || *it == 'f' || *it == 'u'))
        {
            presentation = *it++;
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}')
        {
            throw format_error("invalid format 1");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // I don't think this requires .name, might just be aval really
    // minor change, but shouldn't be too bad.
    // also 'f' for full probably needs to be implemented when doing /ess/full/
    // might be tricky to get everything (flags will need to cascade)
    template <typename FormatContext>
    auto format(const assetVar& aVr, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (presentation)
        {
            case 'c':
                return format_to(ctx.out(), R"({{"value":{}}})", aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal);
            case 'n':
                return format_to(ctx.out(), "{}", aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal);
            case 'u':
                if (aVr.extras && (aVr.extras->alarmVec.size() > 0))
                {
                    // return format_to(ctx.out(), R"("{}":{{"value":{},"options":{}}})",
                    //     aVr.name,
                    //     aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal,
                    //     aVr.extras->alarmVec
                    // );
                    return format_to(ctx.out(), R"({{"value":{},"options":{}}})",
                                     // aVr.name,
                                     aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal, aVr.extras->alarmVec);
                }
                else if (aVr.extras && (aVr.extras->optDict))
                {
                    if (0)
                        FPS_PRINT_INFO("FOUND EXTRAS for [{}]", aVr.name);
                    if (aVr.extras->optDict)
                    {
                        if (0)
                            FPS_PRINT_INFO("FOUND OPTS for [{}]", aVr.name);
                        return format_to(ctx.out(),
                                         R"({{"value":{},"options":[{}],"enabled":{}}})",
                                         aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal, *aVr.extras->optDict,
                                         aVr.getbParam("enabled"));
                    }
                }
                else if (aVr.extras && (aVr.extras->optVec))  // here for optVec
                {
                    const char* options = nullptr;
                    if (aVr.extras->optVec)
                    {
                        options = cJSON_PrintUnformatted(aVr.extras->optVec->cjopts);
                    }
                    else
                    {
                        options = strdup("[]");
                    }
                    if (0)
                        FPS_PRINT_INFO("FOUND OPTS for [{}]", aVr.name);
                    auto fmtOutput = format_to(ctx.out(), R"({{"value":{},"options":{},"enabled":{}}})",
                                               aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal, options,
                                               aVr.getbParam("enabled"));
                    if (options)
                        free((void*)options);
                    return fmtOutput;
                }
                else
                {
                    return format_to(ctx.out(), R"({{"value":{},"enabled":true}})",
                                     aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal);
                    // return format_to(ctx.out(), "{}", aVr.linkVar ? *aVr.linkVar->aVal :
                    // *aVr.aVal);
                }
                [[fallthrough]];
            case 'i':
                if (aVr.extras)
                {
                    // I assume extras exists here
                    if (aVr.extras->optDict)
                    {
                        return format_to(
                            ctx.out(),
                            R"("{}":{{"value":{},"options":[{}],"enabled":{},"name":"{}","scaler":{},"type":"{}","ui_type":"{}","unit":"{}"}})",
                            aVr.name, aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal, *aVr.extras->optDict,
                            aVr.getbParam("enabled"), cstr{ aVr.getcParam("name") },  // might need to check this
                            aVr.getiParam("scaler"), cstr{ aVr.getcParam("type") }, cstr{ aVr.getcParam("ui_type") },
                            cstr{ aVr.getcParam("unit") });
                    }
                    else if (aVr.extras->alarmVec.size() > 0)  // we have an alarmVec
                    {
                        return format_to(
                            ctx.out(),
                            R"("{}":{{"value":{},"options":{},"enabled":{},"name":"{}","scaler":{},"type":"{}","ui_type":"{}","unit":"{}"}})",
                            aVr.name, aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal, aVr.extras->alarmVec,
                            aVr.getbParam("enabled"), cstr{ aVr.getcParam("name") },  // might need to check this
                            aVr.getiParam("scaler"), cstr{ aVr.getcParam("type") }, cstr{ aVr.getcParam("ui_type") },
                            cstr{ aVr.getcParam("unit") });
                    }
                    else  // here for optVec
                    {
                        const char* options = nullptr;
                        if (aVr.extras->optVec)
                        {
                            options = cJSON_PrintUnformatted(aVr.extras->optVec->cjopts);
                        }
                        else
                        {
                            options = strdup("[]");
                        }
                        // const char* options = aVr.extras->optVec ?
                        // cJSON_PrintUnformatted(aVr.extras->optVec->cjopts) : "[]";
                        auto xx = format_to(
                            ctx.out(),
                            R"("{}":{{"value":{},"options":{},"enabled":{},"name":"{}","scaler":{},"type":"{}","ui_type":"{}","unit":"{}"}})",
                            aVr.name, aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal, options, aVr.getbParam("enabled"),
                            cstr{ aVr.getcParam("name") },  // might need to check this
                            aVr.getiParam("scaler"), cstr{ aVr.getcParam("type") }, cstr{ aVr.getcParam("ui_type") },
                            cstr{ aVr.getcParam("unit") });
                        if (options)
                            free((void*)options);  // here for safety, just in case cJSON
                                                   // allocates (NEVER trust a C API)
                        return xx;
                    }
                }
                [[fallthrough]];
            case 'f':  // TODO(WALKER): format cjopts here as well then make sure to
                       // delete the char* to prevent a memory leak
                // format value first:
                format_to(ctx.out(), R"({{"value":{})", aVr.linkVar ? *aVr.linkVar->aVal : *aVr.aVal);
                // extras does not exist, so finish formatting:
                if (!aVr.extras)
                {
                    return format_to(ctx.out(), "}}");
                }
                // extras exists - starting formatting (baseDict first):
                if (aVr.extras->baseDict && !aVr.extras->baseDict->featMap.empty())
                {
                    format_to(ctx.out(), R"(,{})", *aVr.extras->baseDict);
                }
                // format cjopts if we have it:
                if (aVr.extras->optVec && aVr.extras->optVec->cjopts)
                {
                    auto cjstr = cJSON_PrintUnformatted(aVr.extras->optVec->cjopts);
                    if (cjstr)
                    {
                        format_to(ctx.out(), R"(,"options":{})", cjstr);
                        free(cjstr);
                    }
                }
                // format actVec if we have it:
                if (!aVr.extras->actVec.empty())
                {
                    format_to(ctx.out(), R"(,"actions":{{{}}})", aVr.extras->actVec);
                }
                // end brace and finish formatting:
                return format_to(ctx.out(), "}}");
            default:
                return format_to(ctx.out(), R"("error")");
        }
    }
};

template <>
struct formatter<asset_log*>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const asset_log* const aLog, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), R"({{"name":"{}","return_value":{}}})", aLog->almsg,
                         *static_cast<assetVal*>(aLog->aVal));
    }
};

template <>
struct formatter<std::vector<asset_log*>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::vector<asset_log*>& alarmVec, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), "[{}]", fmt::join(alarmVec, ","));
    }
};

// formatter for Abitmap pairs:
// for some reason we don't use the "key" which is an int
template <>
struct formatter<std::pair<const int, assetBitField*>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::pair<const int, assetBitField*>& bitMap, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), "{{{}}}", *bitMap.second->featDict);
    }
};

// formatter for Abitmap:
template <>
struct formatter<std::map<int, assetBitField*>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::map<int, assetBitField*>& bitMap, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), "{}", fmt::join(bitMap, ","));
    }
};

// formatter for assetAction:
template <>
struct formatter<assetAction*>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const assetAction* const assAct, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), R"({{"{}":[{}]}})", assAct->name, assAct->Abitmap);
    }
};

// formatter for pair inside of actVec (inside of extras):
template <>
struct formatter<std::pair<const std::string, std::vector<assetAction*>>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::pair<const std::string, std::vector<assetAction*>>& actVp, FormatContext& ctx)
        -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), R"("{}":[{}])", actVp.first, fmt::join(actVp.second, ","));
    }
};

// formatter for actVec (inside extras):
template <>
struct formatter<std::map<std::string, std::vector<assetAction*>>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::map<std::string, std::vector<assetAction*>>& actV, FormatContext& ctx)
        -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), "{}", fmt::join(actV, ","));
    }
};

template <>
struct formatter<assFeat>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const assFeat& af, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (af.type)
        {
            case assFeat::AFTypes::AINT:
                return format_to(ctx.out(), "{}", af.valueint);
            case assetVar::ATypes::ABOOL:
                return format_to(ctx.out(), "{}", af.valuebool);
            case assetVar::ATypes::AFLOAT:
                return format_to(ctx.out(), "{}", af.valuedouble);
            case assetVar::ATypes::ASTRING:
                return format_to(ctx.out(), R"("{}")", cstr{ af.valuestring });
            default:  // is not a common json value, is null, or is another assetVar
                      // somehow (type = AAVAR)
                return format_to(ctx.out(), R"("error")");
        }
        // return format_to(ctx.out(), "{}", "hello");
    }
};

template <>
struct formatter<std::pair<const std::string, assFeat*>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::pair<const std::string, assFeat*>& fdp, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), R"("{}":{})", fdp.first, *fdp.second);
    }
};

template <>
struct formatter<assetFeatDict>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const assetFeatDict& fd, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), "{}", fmt::join(fd.featMap, ","));
    }
};

template <>
struct formatter<assetVar*> : formatter<assetVar>
{
    using formatter<assetVar>::parse;

    template <typename FormatContext>
    auto format(const assetVar* const aVrp, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return formatter<assetVar>::format(*aVrp, ctx);
    }
};

// NOTE: These have to be const for the key due to how std::map works

// formatter for varmap key/value pairs:
template <>
struct formatter<std::pair<const std::string, assetVar*>>
{
    char presentation = 'c';  // clothed by default
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        auto end = ctx.end();

        if (it != end && (*it == 'c' || *it == 'n' || *it == 'f' || *it == 'u'))
        {
            presentation = *it++;
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}')
        {
            throw format_error("invalid format 2");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const std::pair<const std::string, assetVar*>& p, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (presentation)
        {
            case 'c':
                return format_to(ctx.out(), R"("{}":{})", p.first, *p.second);
            case 'n':
                return format_to(ctx.out(), R"("{}":{:n})", p.first, *p.second);
            case 'f':  // todo: working on this
                return format_to(ctx.out(), R"("{}":{:f})", p.first, *p.second);
            case 'u':  // todo: working on this
                return format_to(ctx.out(), R"("{}":{:u})", p.first, *p.second);
            default:
                return format_to(ctx.out(), R"("error")");
        }
    }
};

// formatter for varmap:
template <>
struct formatter<varmap>
{
    char presentation = 'c';  // clothed by default
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        auto end = ctx.end();

        if (it != end && (*it == 'c' || *it == 'n' || *it == 'f' || *it == 'u'))
        {
            presentation = *it++;
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}')
        {
            throw format_error("invalid format 3");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const varmap& vm, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (presentation)
        {
            case 'c':
                return format_to(ctx.out(), "{{{}}}", fmt::join(vm, ","));
            case 'n':
                return format_to(ctx.out(), "{{{:n}}}", fmt::join(vm, ","));
            case 'f':
                return format_to(ctx.out(), "{{{:f}}}", fmt::join(vm, ","));
            case 'u':
                return format_to(ctx.out(), "{{{:u}}}", fmt::join(vm, ","));
            default:
                return format_to(ctx.out(), R"("error")");
        }
    }
};

// formatter for varsmap key/value pairs:
template <>
struct formatter<std::pair<const std::string, varmap>>
{
    char presentation = 'c';  // clothed by default
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        auto end = ctx.end();

        if (it != end && (*it == 'c' || *it == 'n' || *it == 'f' || *it == 'u'))
        {
            presentation = *it++;
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}')
        {
            throw format_error("invalid format 4");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const std::pair<const std::string, varmap>& p, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (presentation)
        {
            case 'c':
                return format_to(ctx.out(), R"("{}":{})", p.first, p.second);
            case 'n':
                return format_to(ctx.out(), R"("{}":{:n})", p.first, p.second);
            case 'f':  // todo: working on this
                return format_to(ctx.out(), R"("{}":{:f})", p.first, p.second);
            case 'u':
                return format_to(ctx.out(), R"("{}":{:u})", p.first, p.second);
            default:
                return format_to(ctx.out(), R"("error")");
        }
    }
};

// formatter for varsmap:
template <>
struct formatter<varsmap>
{
    char presentation = 'c';  // clothed by default
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin();
        auto end = ctx.end();

        if (it != end && (*it == 'c' || *it == 'n' || *it == 'f' || *it == 'u'))
        {
            presentation = *it++;
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}')
        {
            throw format_error("invalid format 5");
        }

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const varsmap& vmap, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        switch (presentation)
        {
            case 'c':
                return format_to(ctx.out(), "{{{}}}", fmt::join(vmap, ","));
            case 'n':
                return format_to(ctx.out(), "{{{:n}}}", fmt::join(vmap, ","));
            case 'f':
                return format_to(ctx.out(), "{{{:f}}}", fmt::join(vmap, ","));
            case 'u':
                return format_to(ctx.out(), "{{{:u}}}", fmt::join(vmap, ","));
            default:
                return format_to(ctx.out(), R"("error")");
        }
    }
};

// formatter for nvarsmap key/value pairs:
template <>
struct formatter<std::pair<const std::string, varmap*>>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const std::pair<const std::string, varmap*>& p, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), R"("{}":{})", p.first, *p.second);
    }
};

// formatter for asset:
// template<>
// struct formatter<asset>
// {
//     auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
//     {
//         return ctx.end();
//     }

//     // this might not be correct
//     template <typename FormatContext>
//     auto format(const asset& as, FormatContext& ctx) -> decltype(ctx.out())
//     {
//         return format_to(ctx.out(), R"("{}":{})", as.name, as.amap);
//     }
// };

// // formatter for aimap key/value pairs:
// template<>
// struct formatter<std::pair<const std::string, asset*>>
// {
//     auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
//     {
//         return ctx.end();
//     }

//     template <typename FormatContext>
//     auto format(const std::pair<const std::string, asset*>& p, FormatContext&
//     ctx) -> decltype(ctx.out()) const
//     {
//         return format_to(ctx.out(), R"("{}":{})", p.first, *p.second);
//     }
// };

// // formatter for aimap:
// template<>
// struct formatter<aimap>
// {
//     auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
//     {
//         return ctx.end();
//     }

//     template <typename FormatContext>
//     auto format(const aimap& aiM, FormatContext& ctx) -> decltype(ctx.out())
//     {
//         return format_to(ctx.out(), "{{{}}}", fmt::join(aiM, ","));
//     }
// };

// // formatter for asset_manager:
// template<>
// struct formatter<asset_manager>
// {
//     auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
//     {
//         return ctx.end();
//     }

//     template <typename FormatContext>
//     auto format(const asset_manager& am, FormatContext& ctx) ->
//     decltype(ctx.out()) const
//     {
//         return format_to(ctx.out(), R"("{}":{})", am.name, am.assetMap);
//     }
// };

// // formatter for ammap key/value pairs:
// template<>
// struct formatter<std::pair<const std::string, asset_manager*>>
// {
//     auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
//     {
//         return ctx.end();
//     }

//     template <typename FormatContext>
//     auto format(const std::pair<const std::string, asset_manager*>& p,
//     FormatContext& ctx) -> decltype(ctx.out()) const
//     {
//         return format_to(ctx.out(), R"("{}":{})", p.first, *p.second);
//     }
// };

// formatter for ammap:
template <>
struct formatter<ammap>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const ammap& amM, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), "{{{}}}", fmt::join(amM, ","));
    }
};

// formatter for assetlist:
template <>
struct formatter<assetlist>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    template <typename FormatContext>
    auto format(const assetlist& asl, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        for (std::size_t i = 1; i < asl.size(); ++i)
        {
            format_to(ctx.out(), (i != asl.size() - 1) ? "{:i}," : "{:i}", *asl[i]);
        }
        return format_to(ctx.out(), "");
    }
};

template <>
struct formatter<assetList>
{
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.end(); }

    // todo: fix this for the first assetVar in the assetList
    template <typename FormatContext>
    auto format(const assetList& asL, FormatContext& ctx) -> decltype(ctx.out()) const
    {
        return format_to(ctx.out(), R"({{"name":{:n},{}}})", *asL.aList[0], asL.aList);
    }
};
}  // namespace fmt

#endif
