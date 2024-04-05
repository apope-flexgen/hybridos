#ifndef RNG_HPP
#define RNG_HPP

#include "pcg_extras.hpp"
#include "pcg_random.hpp"
#include <random>

// installation, super easy
// git clone git@github.com:imneme/pcg-cpp.git
// cd pcg-cpp
// make
// make test
// sudo make install
// done

class RNG
{
    pcg64 rng;

    // singleton, this is expensive to construct and reseed
    // so we do it only once up front
    RNG()
    {
        pcg_extras::seed_seq_from<std::random_device> seed_source;
        rng.seed(seed_source);
    }

public:
    static RNG& get()
    {
        static RNG instance;
        return instance;
    }

    // return a random whole number (Int = integral, NOT integer)
    // this is inclusive to the upper bound [lower, upper]
    template <typename T = int>
    T randInt(T lower, T upper)
    {
        std::uniform_int_distribution<T> dist{ lower, upper };
        return dist(rng);
    }

    // return a random floating point number (double, long double, or float)
    // this is exclusive to the upper bound [lower, upper)
    template <typename T = double>
    T randReal(T lower, T upper)
    {
        std::uniform_real_distribution<T> dist{ lower, upper };
        return dist(rng);
    }

    // generates a random floating point number around the mean by some std
    // deviation (double, long double, float) NOTE: If std deviation is < 0
    // produced undefined behaviour.
    template <typename T = double>
    T randNormal(T mean, T stddev = 1.0)
    {
        std::normal_distribution<T> dist{ mean, stddev };
        return dist(rng);
    }

    // call this if you want to reseed the device. Very rare.
    // NOTE: EXPENSIVE!!! do NOT spam this
    void reSeed()
    {
        pcg_extras::seed_seq_from<std::random_device> seed_source;
        rng.seed(seed_source);
    }
};

#endif