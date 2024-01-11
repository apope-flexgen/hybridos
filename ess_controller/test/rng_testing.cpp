#include "spdlog/fmt/fmt.h"
#include "spdlog/fmt/bundled/ranges.h"
#include "spdlog/fmt/bundled/format.h"

#include <map>

#include "RNG.hpp"

namespace fmt 
{
    template<>
    struct formatter<std::pair<const char* const, int>>
    {
        auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template <typename FormatContext>
        auto format(const std::pair<const char* const, int>& p, FormatContext& ctx) -> decltype(ctx.out()) 
        {
            return format_to(ctx.out(), R"("{}":{})", p.first, p.second);
        }
    };
}

int main()
{
    std::map<const char*, int> testing;
    testing["hello"] = 2;
    testing["world"] = 4;
    fmt::print("{{\n");
    for (const auto& pair : testing)
    {
        fmt::print("{}\n", pair);
    }
    fmt::print("}}\n");
    fmt::print("map = {{{}}}\n", fmt::join(testing, ","));
}
