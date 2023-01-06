#ifndef TESTUTILS_HPP
#define TESTUTILS_HPP

// Make sure to put includes for your test functions here:
#include <iostream>
#include <limits>
#include <chrono>
#include <string>
#include <vector>

// Source: https://en.cppreference.com/w/cpp/utility/exchange
// Very useful operation that is not in C++11, so I imported it here (USED IN DUMMY CLASS! DO NOT DELETE!):
namespace std
{
    template<class T, class U = T>
    T exchange(T& obj, U&& new_value)
    {
        T old_value = std::move(obj);
        obj = std::forward<U>(new_value);
        return old_value;
    }
}

// source: Can't remember just know it's from "The Cherno" on youtube.
// Extra debug utilities:
// static uint32_t s_AllocCount = 0;

// void* operator new(size_t size)
// {
//     // s_AllocCount++;
//     printf("Allocating %zu bytes.\n", size);
//     return malloc(size);
// }

// Put this at the end of main:
// printf("%d allocation(s) total.\n", s_AllocCount);

// Namespace to define functions to help you test your c++ code
namespace testUtils
{

    // Source for idea: https://en.cppreference.com/w/cpp/language/rule_of_three
    // Class that tells you what is going on to it (for data structures, algorithms, etc.). Uses the rule-of-five
    class Dummy
    {
    public:
        std::string m_name;
        std::vector<std::string> event_history;
        bool m_debug = true; // Needed for testing data structures, do NOT remove the "= true" (bools are "= false" by default)

        // Default constructor:
        explicit Dummy(std::string name = "default", bool debug = true)
            : m_name(std::move(name)), m_debug(debug)
        {
            if (m_debug) printf("(Dummy: %s) was constructed.\n", m_name.c_str());
        }
        // Destructor:
        ~Dummy()
        {
            if (m_debug) printf("(Dummy: %s) was destroyed, event_history(begin->end): \n", m_name.c_str());
            for (const std::string& event : event_history)
            {
                if (m_debug) printf("%s,", event.c_str());
            }
            if (m_debug && !event_history.empty()) printf("\n");
        }

        // Copy constructor:
        Dummy(const Dummy& other, bool debug = true)
            : m_name(other.m_name), m_debug(debug)
        {
            if (other.m_debug) printf("(Dummy: %s) was copied by constructor!\n", other.m_name.c_str());
            const_cast<Dummy&>(other).event_history.emplace_back("(Copy Constructor)");
        }
        // Copy assignment operator:
        Dummy& operator=(const Dummy& other)
        {
            if (this != &other)
            {
                if (m_debug) printf("(Dummy: %s) is copying (Dummy: %s) by copy assignment!\n", m_name.c_str(), other.m_name.c_str());
                event_history.emplace_back("(Copy Assignment, was: " + m_name + ")");
                m_name = other.m_name;
            }
            else
            {
                if (m_debug) printf("(Dummy: %s) is copying ITSELF by copy assignment!\n", m_name.c_str());
                event_history.emplace_back("(COPIED ITSELF by assignment! Why?)");
            }
            return *this;
        }

        // Move constructor:
        Dummy(Dummy&& other, bool debug = true) noexcept
            : m_name(std::exchange(other.m_name, "null")), m_debug(debug)
        {
            if (other.m_debug) printf("(Dummy: %s) was moved by constructor!\n", m_name.c_str());
            other.event_history.emplace_back("(Move Constructor, was: " + m_name + ")");
        }
        // Move assignment operator:
        Dummy& operator=(Dummy&& other) noexcept
        {
            if (this != &other)
            {
                if (m_debug) printf("(Dummy: %s) is turning into (Dummy: %s) by move assignment!\n", m_name.c_str(), other.m_name.c_str());
                event_history.emplace_back("(Move Assignment (taker), was: " + m_name + ")");
                other.event_history.emplace_back("(Move Assignment (takee), was: " + other.m_name + ")");
                m_name = std::exchange(other.m_name, "null");
            }
            else
            {
                if (m_debug) printf("(Dummy: %s) is turning into ITSELF by move assignment!\n", m_name.c_str());
                event_history.emplace_back("(MOVED ITESLF by assignment! Why?)");
            }
            return *this;
        }
    };

    // Source: https://www.youtube.com/watch?v=oEx5vGNFrLk
    // Timer class to help you figure out how long a block of code takes to run (for algorithms, etc.)
    class Timer
    {
        std::chrono::_V2::system_clock::time_point start, end;
        std::chrono::duration<float> duration;
        std::string m_name;
        bool m_debug = true;
    public:
        explicit Timer(std::string name, bool debug = true)
            : m_name(std::move(name)), m_debug(debug)
        {
            start = std::chrono::high_resolution_clock::now();
        };
        ~Timer()
        {
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;

            float ms = duration.count() * 1000.0f;
            if (m_debug) printf("(timer %s) took: %fms\n", m_name.c_str(), ms);
        };
    };

    // use this to pause a test to check changes, then press y to continue to the next step (disable it by passing in false):
    void step(bool enabled = true)
    {
        if (enabled)
        {
            char input = '\0';
            while (!input)
            {
                std::cout << "\nPress y to continue the program, or q to quit: ";
                std::cin >> input;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignores other characters beyond the first
                if (tolower(input) == 'y')
                {
                    std::cout << "Continuing program now:\n\n";
                    return;
                }
                else if (tolower(input) == 'q')
                {
                    std::cout << "\nExiting program now!\n";
                    exit(-1);
                }
                input = '\0';
            }
        }
    }
}
#endif
