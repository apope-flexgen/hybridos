/***
 *
 *  G. Briggs 5-7-2024
 *  LowPassFilter.cpp
 *  Implements a first-order LowPass filter module
 *  See https://flexgen.atlassian.net/browse/ACO-373 for details
 * 
 *  
***/

#ifndef LOWPASSFILTER_CPP
#define LOWPASSFILTER_CPP

#include "LowPassFilter.hpp"

void LowPassFilter::step() 
{
    if (!Dw.initialized) initialize();
    if (InputRef->debug)
    {
        FPS_PRINT_INFO("My critical data:", nullptr);
        FPS_PRINT_INFO("Input: {}, fs: {}, fc: {}",InputRef->input, Configs.fs, Configs.fc);
        FPS_PRINT_INFO("a1: {}, b0: {}, b1: {}, x1: {}, y1: {}", a1, b0, b1, x1, y1);
    }

    OutputRef->output = b0 * InputRef->input + b1 * x1 + a1 * y1;
    x1 = InputRef->input;
    y1 = OutputRef->output;
}

void LowPassFilter::initialize()
{
    if (Configs.fs == 0)
    {
        Configs.fs = 1;
        // TODO error
    }

    double omega_c = 2 * M_PI * Configs.fc;
    double T = 1/Configs.fs;
    double divisor = 2 + T * omega_c;
    if (divisor == 0)
    {
        divisor = 0.00001;
        // TODO error
    }
    a1 = (2 - T * omega_c) / (divisor);
    b1 = T * omega_c / (divisor);
    b0 = b1;
    // Initialize the filter to the current value. Prevents having to wait for 1/fc seconds for filter to converge at startup. 
    x1 = InputRef->input;
    y1 = InputRef->input;
    Dw.initialized = true;
}

// constructor
LowPassFilter::LowPassFilter(ExtU* inputs_i, ExtY* outputs_o): InputRef(), OutputRef() 
{
    InputRef = inputs_i;
    OutputRef = outputs_o;
    Dw.initialized = false;
}

#endif