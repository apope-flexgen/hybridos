#ifndef LOWPASSFILTER_HPP
#define LOWPASSFILTER_HPP

#include "asset.h"
#include "calculator.hpp"
#include "ess_utils.hpp"
#include "formatters.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class LowPassFilter
{
private:
    double a1;
    double b0;
    double b1;
    double x1;
    double y1;

public:
    struct ExtU
    {
        double input;
        bool debug;
    };

    struct ExtY
    {
        double output;

        // TODO constructor.
    };

    struct LowPass_DW
    {
        bool initialized;
        // TODO constructor.
    };

    struct Constants
    {
        double fs;
        double fc;
    };

    ExtU* InputRef;
    ExtY* OutputRef;
    Constants Configs;
    LowPass_DW Dw;

    // constructor
    LowPassFilter(ExtU* inputs_i, ExtY* outputs_o);

    void step();
    void initialize();
};

// function headers to pass information between LowPassFilter_Interface.cpp and LowPassFilterBRB_Interface.cpp
extern std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<LowPassFilter>>> LowPassFilterObjects;
extern std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<LowPassFilter::ExtU>>>
    LowPassFilterInputs;
extern std::unordered_map<std::string, std::unordered_map<int, std::unique_ptr<LowPassFilter::ExtY>>>
    LowPassFilterOutputs;

uint8_t* getLowPassFilterInputs(std::string uri, int instance);
uint8_t* getLowPassFilterOutputs(std::string uri, int instance);
void LowPassFilterRun(std::string uri, int instance);

void setupLowPassFilterAmap(VarMapUtils* vm, varsmap& vmap, asset_manager* am, int instance, std::string uri);

#endif