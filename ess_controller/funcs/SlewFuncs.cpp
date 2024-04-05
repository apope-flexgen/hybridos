#ifndef SLEWFUNCS_CPP
#define SLEWFUNCS_CPP

#include "asset.h"
#include "formatters.hpp"

extern "C++" {
int SlewVal(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
}

int SlewVal(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    if (!Av)
    {
        FPS_PRINT_ERROR("{}",
                        "cannot get Av to run slew function on, please check "
                        "that an Av is provided for this function.");
        return -1;
    }
    if (!Av->am)
    {
        //      FPS_PRINT_ERROR("cannot get Av->am for Av: {}, please make sure it
        //      has one.", Av->getfName());
        return -1;
    }
    if (!Av->am->vm)
    {
        //    FPS_PRINT_ERROR("cannot get Av->am->vm for Av: {}, please make sure it
        //    has one.", Av->getfName());
        return -1;
    }
    // check for correct type:
    if (Av->aVal->type != assetVal::ATypes::AFLOAT)
    {
        //  FPS_PRINT_ERROR("non floating point types are not-supported for SlewVal.
        //  Av name is: {}", Av->getfName());
        return -1;
    }

    // get slew val (and check for correct type):
    if (!Av->gotParam("SlewedVal"))
    {
        // FPS_PRINT_ERROR("could not get SlewedVal for Av {}, please provide one.",
        // Av->getfName());
        return -1;
    }

    auto var_to_slew_uri = Av->getcParam("SlewedVal");

    auto var_to_slew = Av->am->vm->getVar(vmap, var_to_slew_uri, nullptr);
    if (!var_to_slew)
    {
        // FPS_PRINT_ERROR("could not get the var to slew using uri: {} for Av: {}",
        // var_to_slew_uri, Av->getfName());
        return -1;
    }
    if (var_to_slew->type != assetVar::ATypes::AFLOAT)  // might need to check for linkVal?
    {
        // FPS_PRINT_ERROR("the value to be slewed -> {}, does not contain a
        // float.", var_to_slew->getfName());
        return -1;
    }

    // get the rest of the params we need (and check for types as well):
    if (!Av->gotParam("RatedValue") || !Av->gotParam("SlewRate"))
    {
        // FPS_PRINT_ERROR("can't find one of RatedValue or SlewRate for Av: {},
        // please provide them", Av->name);
        return -1;
    }

    bool debug = false;
    if (Av->gotParam("debug"))
    {
        debug = Av->getbParam("debug");
    }
    // enable by default
    bool enabled = true;

    if (Av->gotParam("enabled"))
    {
        enabled = Av->getbParam("enabled");
    }
    if (Av->gotParam("enable"))
    {
        auto enable_var = Av->am->vm->getVar(vmap, Av->getcParam("enable"), nullptr);
        if (enable_var)
        {
            enabled = enable_var->getbVal();
        }
    }

    // timing stuff:
    const auto current_time = Av->am->vm->get_time_dbl();

    // setup last time we slewed if it doesn't have it
    if (!Av->gotParam("LastSlewedTime"))
    {
        // FPS_PRINT_ERROR("can't find LastSlewedTime for Av: {}", Av->getfName());
        Av->setParam("LastSlewedTime", current_time);

        return 0;
    }

    // get necessary information:
    const auto RatedValue = Av->getdParam("RatedValue");
    // const auto SlewRate = Av->getdParam("SlewRate");
    // const auto LastslewedTime = Av->getdParam("LastSlewedTime");

    // check for correct value ranges:
    // negative is a test mode
    auto SlewRate = Av->getdParam("SlewRate");
    const auto LastslewedTime = Av->getdParam("LastSlewedTime");
    const auto SlewRateVar = Av->getcParam("SlewRate");
    if (SlewRateVar)
    {
        auto srAv = Av->am->vm->getVar(vmap, SlewRateVar, nullptr);
        if (!srAv)
        {
            // FPS_PRINT_ERROR(" unable to find var [{}]", SlewRateVar);
            return -1;
        }
        SlewRate = srAv->getdVal();
    }

    if (SlewRate < 0.0)
    {
        // FPS_PRINT_ERROR("SlewRate cannot be negative, for Av: {}",
        // Av->getfName());
        return -1;
    }
    double elapsedTime;
    // LastSlewedTime < 0 is used for testing
    if (LastslewedTime < 0)
    {
        elapsedTime = -LastslewedTime;
    }
    else
    {
        elapsedTime = current_time - LastslewedTime;
    }

    if (elapsedTime < 0.0)  // can't have negative time (someone changed it)
    {
        // FPS_PRINT_ERROR("elapsed time since LastSlewTime is negative for some
        // reason, for Av: {}", Av->getfName());
        if (LastslewedTime > 0)
            Av->setParam("LastSlewedTime", current_time);
        return -1;
    }

    if (elapsedTime == 0.0)  // can't have negative time (someone changed it)
    {
        // FPS_PRINT_ERROR("elapsed time since LastSlewTime is 0 for some reason,
        // for Av: {}", Av->getfName());
        if (LastslewedTime > 0)
            Av->setParam("LastSlewedTime", current_time);
        return -1;
    }
    const auto current_val = var_to_slew->getdVal();  // where we are currently
    const auto desired_val = Av->getdVal();           // where we want to go

    const auto to_add_or_sub = std::abs(RatedValue * SlewRate / 100 *
                                        elapsedTime);  // make sure this is always positive
    const auto new_val_pos = current_val + to_add_or_sub;
    const auto new_val_neg = current_val - to_add_or_sub;

    if (debug)
    {
        // FPS_PRINT_INFO("Av: {} rated {:2.3f} slewrate  {:2.3f} elapsed {:2.3f} "
        // , Av->getfName()
        // , RatedValue
        // , SlewRate
        // , elapsedTime
        // );
        // FPS_PRINT_INFO("Av: {} current {:2.3f} desired {:2.3f} adjust {:2.3f} "
        // , Av->getfName()
        // , current_val
        // , desired_val
        // , to_add_or_sub
        //);
    }
    // NOTE(WALKER): this determines whether or not we are slewing in the right
    // direction positive value means slew in the positive direction negative
    // value means slew in the negative direction
    const auto dif_in_desired = desired_val - current_val;

    if (!enabled)
    {
        var_to_slew->setVal(desired_val);
        if (LastslewedTime > 0)
            Av->setParam("LastSlewedTime", current_time);  // make sure to reset
                                                           // LastSlewedTime even if
                                                           // not enabled (so we don't
                                                           // get buildup when we
                                                           // re-enable it)
        return 0;
    }

    // start slewing:
    double dsval = 0;
    if (dif_in_desired < 0.0)  // we want to slew downward in a negative direction
    {
        if (new_val_neg <= desired_val)
        {
            dsval = desired_val;
        }
        else
        {
            dsval = new_val_neg;
        }
    }
    else  // we want to slew upward in a positive direction
    {
        if (new_val_pos >= desired_val)
        {
            dsval = desired_val;
        }
        else
        {
            dsval = new_val_pos;
        }
    }
    Av->am->vm->setVal(vmap, var_to_slew->getfName(), nullptr, dsval);
    if (LastslewedTime > 0)
        Av->setParam("LastSlewedTime", current_time);
    return 0;
}

#endif
