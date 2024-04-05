#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP
/*
*  functions.h
 * these are going to be fun functions
  * void func(vector<assVars*> &outputs, vector<assVars *>&inputs)
  * keep em in a map by name
  * define at config time
  *
   function group (name)
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))
           func name ((op1 ,op2, op3), (ip1,ip2,ip3))

     example sum, max, min, avg
     if (i0) {
         for each i1 ...in)
             if(0=o0)o0 += ix
            if(o1)o1 = max (o1,ix)
            if(o2)o2 = min(o2,ix)
            if(o0 && o3) o3 = o0/n

     }
  this is cool stuff , all the processing maths can be configured at run time in
the text config file.

   functions are organised in groups for specific operations
   start at the init group and head of to the standby group
start with some simple ones
   collect the average voltage from n bms units
*/

#include "assetVar.h"
#include <cstring>
#include <fims/libfims.h>
#include <iostream>
#include <malloc.h>
#include <map>

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif

// ths interface is a vector of ins , outs , ( alerts and warning etc to be
// added)
struct Interface
{
    Interface(){};
    ~Interface()
    {
        ins.clear();
        // outs.clear();
    };
    int addIn(assetVar* av)
    {
        int rc = ins.size();
        ins.push_back(av);
        return rc;
    }

    int addOut(const char* op, assetVar* av)
    {
        printf(" adding interface out var for [%s] \n", op);
        int rc = 0;
        outs[op] = av;
        return rc;
    }

    assetVar* inAt(int idx)
    {
        if (idx < (int)ins.size())
            return ins[idx];
        return nullptr;
    }
    assetVar* outAt(const char* op)
    {
        auto it = outs.find(op);
        if (it != outs.end())
        {
            printf(" ### found out var for [%s]\n", op);
            return outs[op];
        }
        printf(" ###>>>found no out var for [%s]\n", op);
        return nullptr;
    }

    std::vector<assetVar*> ins;
    std::map<std::string, assetVar*> outs;
};

struct funVec
{
    funVec(const char* fname)
    {
        id = fname;
        iface = new Interface;
    }
    ~funVec() { delete iface; };

    void setFname(const char* _fname) { fname = _fname; }

    Interface* iface;
    std::string id;
    std::string fname;
};
// possibly not going to use this
template <typename T>
class make_vector
{
public:
    typedef make_vector<T> my_type;
    my_type& operator<<(const T& val)
    {
        data_.push_back(val);
        return *this;
    }
    operator std::vector<T>() const { return data_; }

private:
    std::vector<T> data_;
};

// here is the function type
typedef bool (*boolFunType)(Interface*);

funVec* parse_funItem(cJSON* cj, varsmap& varMaps);

#endif