#include <iostream>
#include "spdlog/fmt/fmt.h"
#include "load_metrics_config.hpp"
using namespace std;
int main() {
    config_loader::MetricsConfig config;

    config.load_config("metrics.json");

    for(auto [key,input] : config.inputs) {
        fmt::print("{}:{}\n", key, input.uri);
    }

    for(auto [key,input] : config.filters) {
        fmt::print("{}\n", key);
    }

    for(auto [key,input] : config.outputs) {
        fmt::print("{}: {}\n", key, input.uri);
    }
}