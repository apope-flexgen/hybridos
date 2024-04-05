#include "asset.h"
#include "formatters.hpp"

#ifndef FPS_ERROR_FMT
#define FPS_ERROR_FMT(...) fmt::print(stderr, __VA_ARGS__)
#endif

#ifndef MAX_VEC_DEPTH
#define MAX_VEC_DEPTH 256
#endif

// simple function to run a moving avg and plot it somewhere
// needs a (new) value vec in an assetVar
// Params (vecAv, outav, depth, amap)
// /system/flex/movavg
// add max / min / range /3sig ??
// added enable / enabled param
// added reset  param
// added savedMax / SavedMin (params for now)
extern "C++" {
int MathMovAvg(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SetVecDepth(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int initVec(varsmap& vmap, VarMapUtils* vm, varmap& amap, const char* mname, double initVal, double defdepth);
}

int initVec(varsmap& vmap, VarMapUtils* vm, varmap& amap,
            const char* mname
            /* "MaxBMSDischargePowerEstFiltIn"*/,
            double initVal, double defdepth)
{
    assetVar* av = amap[mname];
    double dval = initVal;
    double depth = defdepth;
    if (!av->gotParam("vecAv"))
    {
        FPS_ERROR_FMT("[{}] >> av [{}] has no [vecAv] param ", __func__, av->getfName());
        return -1;
    }
    if (av->gotParam("initVal"))
    {
        dval = av->getdParam("initVal");
    }
    if (av->gotParam("depth"))
    {
        depth = av->getdParam("depth");
    }

    // dval = amap["MaxBMSDischargePowerEstFiltIn"]->getdParam("initVal");
    vm->setVecDepth(vmap, amap, mname, "vecAv", depth, &dval);
    return 0;
}

// just write a value to an assetVar with "depth" and "mathVec" params set up
// with this function triggered onSet. this function will reset the depth vector
// and restart the movingAvg process you can also do this directly in appliction
// code using the  mathAv->setVecDepth(depth) function.

int SetVecDepth(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    essPerf ePerf(aV->am, "math_system", "SetVecDepth", NULL);
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    bool debug = false;
    int depth = 1;
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    if (aV->gotParam("depth"))
    {
        depth = aV->getiParam("depth");
    }

    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    char* mathVec = nullptr;
    assetVar* mathAv = nullptr;

    if (aV->gotParam("mathVec"))
    {
        mathVec = aV->getcParam("mathVec");
        mathAv = vm->getVar(vmap, (const char*)mathVec, nullptr);
    }
    if (aV->gotParam("vecAv"))
    {
        mathVec = aV->getcParam("vecAv");
        mathAv = vm->getVar(vmap, (const char*)mathVec, nullptr);
    }
    // lets have an arbitrary Max depth
    if (mathAv && depth > 0 && depth < MAX_VEC_DEPTH)
    {
        if (depth != 1)
        {
            mathAv->setVecDepth(1);
        }
        mathAv->setVecDepth(depth);
        if (debug)
            FPS_ERROR_FMT("{} >> setVecDepth for  [{}] to {}\n", __func__, mathAv->getfName(), depth);
    }

    return 0;
}

int MathMovAvg(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(p_fims);
    essPerf ePerf(aV->am, "math_system", "MovAvg", NULL);
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    int depth = 16;
    bool debug = false;
    double tNow = vm->get_time_dbl();
    double wind = 0.0;
    double filtFac = 0.0;
    double lval = 0.0;
    bool enable = true;
    bool reset = false;
    assetVar* vecAv = nullptr;
    double dval = aV->getdVal();

    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }

    // if we have a window % it will limit changes to within the windod
    if (aV->gotParam("enabled"))
    {
        enable = aV->getbParam("enabled");
    }
    if (aV->gotParam("enable"))
    {
        char* en = aV->getcParam("enable");
        assetVar* enAv = vm->getVar(vmap, en, nullptr);
        enable = enAv->getbVal();
    }
    if (!enable)
    {
        if (debug)
        {
            FPS_ERROR_FMT("{} >> var [{}] not enabled\n", __func__, aV->getfName());
        }
        return 0;
    }
    if (aV->gotParam("depth"))
    {
        depth = aV->getiParam("depth");
    }

    if (aV->gotParam("vecAv"))
    {
        char* mathVec = aV->getcParam("vecAv");
        vecAv = vm->getVar(vmap, (const char*)mathVec, nullptr);
    }

    if (vecAv && aV->gotParam("reset"))
    {
        reset = aV->getbParam("reset");
        if (reset)
        {
            // lets have an arbitrary Max depth
            if (vecAv && depth > 0 && depth < MAX_VEC_DEPTH)
            {
                if (depth != 1)
                {
                    vecAv->setVecDepth(1);
                }
                vecAv->setVecDepth(depth);
                if (debug)
                    FPS_ERROR_FMT("{} >> setVecDepth for  [{}] to {}\n", __func__, vecAv->getfName(), depth);

                aV->setParam("reset", false);
                aV->setParam("savedMin", dval);
                aV->setParam("savedMax", dval);
                aV->setParam("savedMinTime", tNow);
                aV->setParam("savedMaxTime", tNow);
            }
        }
    }
    if (aV->gotParam("window"))
    {
        wind = aV->getdParam("window");
        if (debug)
            FPS_ERROR_FMT("{} >> found window param for [{}] value {}\n", __func__, aV->getfName(), wind);
        if ((wind < 0.0) || (wind > 100.0))
        {
            FPS_ERROR_FMT("{} >> removing bad window param for [{}] value {}\n", __func__, aV->getfName(), wind);
            wind = 0.0;
        }
    }

    // get the filter factor , should be something like 0.75
    if (aV->gotParam("filtFac"))
    {
        filtFac = aV->getdParam("filtFac");
    }

    vecAv = vm->getaVParam(vmap, aV, "vecAv");

    lval = dval;
    if (aV->gotParam("lastVal"))
    {
        lval = aV->getdParam("lastVal");
    }

    if (wind > 0.0)
    {
        if (debug)
            FPS_ERROR_FMT(
                "{} >> [{}] wind {} lval {} dval {} diff {} lim {} corrected {}\n", __func__, aV->getfName(), wind,
                lval, dval, std::abs(lval - dval), lval * wind,
                (std::abs(lval - dval) > lval * wind) ? (lval > dval) ? lval * (1 - wind) : lval * (1 + wind) : dval);
    }
    // optional modify dval by window
    if ((wind > 0.0) && (std::abs(lval - dval) > lval * wind))
    {
        if (lval > dval)
        {
            dval = lval * (1 - wind);
        }
        else
        {
            dval = lval * (1 + wind);
        }
    }

    aV->setParam("lastVal", dval);

    if (!vecAv)
    {
        // create the vecAv if needed.
        // this is where we store the vector values
        if (aV->gotParam("vecAv"))
        {
            vecAv = vm->setVal(vmap, aV->getcParam("vecAv"), nullptr, dval);
        }
        if (vecAv)
        {
            if (!vecAv->am)
                vecAv->am = aV->am;
            vecAv->setParam("depth", depth);
        }
        else
        {
            if (debug)
                FPS_ERROR_FMT("{} >> no vecAv Param for  [{}]\n", __func__, aV->getfName());
            // return 0;
        }
    }

    if (vecAv)
    {
        vecAv->setVecVal(dval, depth);
        double avval, maxval, minval, spval;

        double num = (double)vecAv->getVecVals(depth, avval, minval, maxval, spval);
        if (debug)
            FPS_ERROR_FMT(
                "{} depth {} avg {:2.3f}, max {:2.3f} , min {:2.3f} spread "
                "{:2.3f} num {:2.3f}   \n",
                __func__, depth, avval, maxval, minval, spval, num);

        aV->setParam("numVals", num);
        if (aV->gotParam("outAvg"))
        {
            char* spv = aV->getcParam("outAvg");
            assetVar* outAv = vm->setVal(vmap, spv, NULL, avval);
            if (!outAv->am)
                outAv->am = aV->am;
            if (debug)
                FPS_ERROR_FMT("{} >> set outAvg  [{}] to [{:2.3f}] \n", __func__, spv, avval);
        }
        if (aV->gotParam("outMax"))
        {
            char* spv = aV->getcParam("outMax");
            assetVar* outAv = vm->setVal(vmap, spv, NULL, maxval);
            if (!outAv->am)
                outAv->am = aV->am;
            if (debug)
                FPS_ERROR_FMT("{} >> set outMax  [{}] to [{:2.3f}] \n", __func__, spv, maxval);
        }
        if (aV->gotParam("outMin"))
        {
            char* spv = aV->getcParam("outMin");
            assetVar* outAv = vm->setVal(vmap, spv, NULL, minval);
            if (!outAv->am)
                outAv->am = aV->am;
            if (debug)
                FPS_ERROR_FMT("{} >> set outMin  [{}] to [{:2.3f}] \n", __func__, spv, minval);
        }
        // span
        if (aV->gotParam("outSp"))
        {
            char* spv = aV->getcParam("outSp");
            assetVar* outAv = vm->setVal(vmap, spv, NULL, spval);
            if (!outAv->am)
                outAv->am = aV->am;
            if (debug)
                FPS_ERROR_FMT("{} >> set outSp  [{}] to [{:2.3f}] \n", __func__, spv, spval);
        }
        if (aV->gotParam("outSum"))
        {
            char* spv = aV->getcParam("outSum");
            double sum = avval * num;
            assetVar* outAv = vm->setVal(vmap, spv, NULL, sum);
            if (!outAv->am)
                outAv->am = aV->am;
            if (debug)
                FPS_ERROR_FMT("{} >> set outSum  [{}] to [{:2.3f}] \n", __func__, spv, avval * num);
        }

        double savedMax = dval;
        double savedMin = dval;
        if (aV->gotParam("savedMax"))
        {
            savedMax = aV->getdParam("savedMax");
        }
        if (maxval > savedMax)
        {
            savedMax = maxval;
            aV->setParam("savedMaxTime", tNow);
        }
        aV->setParam("savedMax", savedMax);

        if (aV->gotParam("savedMin"))
        {
            savedMin = aV->getdParam("savedMin");
        }
        if (minval < savedMin)
        {
            savedMin = minval;
            aV->setParam("savedMinTime", tNow);
        }
        aV->setParam("savedMin", savedMin);
    }
    // The formula I have is y[n] = x*filtFac - (y[n-1]*(filtFac - 1))
    // where x is the input, y is the output, filtFac is sample_rate/time_constant
    if (aV->gotParam("outFilt"))
    {
        char* spv = aV->getcParam("outFilt");
        // double sum = avval*num;
        // sum -= dval;
        double oldVal = dval * filtFac;

        assetVar* outAv = vm->getVar(vmap, spv, NULL);
        if (!outAv)
        {
            outAv = vm->setVal(vmap, spv, NULL, oldVal);
        }
        if (outAv)
        {
            if (!outAv->am)
            {
                outAv->am = aV->am;
            }
            oldVal = outAv->getdVal();
            double filt = dval * filtFac - (oldVal * (filtFac - 1));
            vm->setVal(vmap, spv, NULL, filt);
            if (debug)
                FPS_ERROR_FMT(
                    "{} >> old/dval [{:2.3f}]/[{:2.3f}] set outFilt  [{}] to "
                    "[{:2.3f}] \n",
                    __func__, oldVal, dval, spv, filt);
        }
    }
    if (aV->gotParam("outTimeFilt"))
    {
        if (!aV->gotParam("lastTimeFilt"))
        {
            aV->setParam("lastTimeFilt", tNow);
        }
        else
        {
            double tFilt = tNow - aV->getdParam("lastTimeFilt");
            aV->setParam("lastTimeFilt", tNow);
            if (tFilt > 1.0)
            {
                tFilt = 1.0;
            }

            char* spv = aV->getcParam("outTimeFilt");
            // double sum = avval*num;
            // sum -= dval;
            double oldVal = dval;

            assetVar* outAv = vm->getVar(vmap, spv, NULL);
            if (!outAv)
            {
                outAv = vm->setVal(vmap, spv, NULL, oldVal);
            }
            if (outAv)
            {
                if (!outAv->am)
                {
                    outAv->am = aV->am;
                }
                // y[n] = x*filtFac - y[n-1]*(filtFac)  +y[n-1]
                oldVal = outAv->getdVal();
                double filt = oldVal + (dval - oldVal) * ((1 - filtFac) * tFilt);
                vm->setVal(vmap, spv, NULL, filt);
                if (1 || debug)
                    FPS_ERROR_FMT("{} >> set outTimeFilt  [{}] to [{:2.3f}] \n", __func__, spv, filt);
            }
        }
    }
    // vecAv contains a vector of assetVals (aValVec) in extras
    // push aV->getdVal(depth) onto vecAv() upto depth
    // get sum of vecAv // divide by number of entries.
    // put result into outAv
    // pus
    if (debug)
        FPS_ERROR_FMT("{} >> name [{}] input [{}] depth {}\n", __func__, aname, aV->getfName(), depth);
    return 0;
}
