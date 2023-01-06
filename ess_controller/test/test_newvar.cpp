/*
*  test_newvar.cpp
* weve added an extra layer so lets test it...
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
  this is cool stuff , all the processing maths can be configured at run time in the text config file.
                {
                    "id": "dc_volts_summary",
                    "inputs": [
                        {"var":"/components/bss_1","id":"dc_volts"},
                        {"var":"/components/bss_2","id":"dc_volts"},
                        {"var":"/components/bss_3","id":"dv_volts"}
                    ],
                    "outputs": [
                        {"op":"max","var":"/features/bss_status","id":"max_dc_volts"},
                        {"op":"min","var":"/features/bss_status","id":"min_dc_volts"}
                        {"op":"avg","var":"/features/bss_status","id":"avg_dc_volts"}
                    ],
                    "operation": "add_dbl",
                    "params": [
                      {"falseCase": 0}
                    ]
                }
*/

#include "assetTest.h"
#include "chrono_utils.hpp"


// struct funVec {
//     funVec(const char *fname) {
//         id = fname;
//         iface = new Interface;

//     }
//     ~funVec() {
//         delete iface;
//     };

//     void setFname(const char *_fname) {
//         fname = _fname;
//     }

//     Interface* iface;
//     std::string id;
//     std::string fname;

// };


//#ifdef  1 //FUNCTION_TEST
int main()
{
    varsmap vmap;
    VarMapUtils vm(vmap);
    vm.clearVmap(vmap);

    // varsmap varMaps2;

    // varmap varMap2;

    // int idx = 0;
    // assetVar *av = new assetVar("dummy", idx);
    // delete av;
    // // 184 bytes per assetVar
    // varMap2["d21"]  = new assetVar("d21",true);
    // double sval = 0.0;
    
//     vmap->clearVm(varMap2);
//     vmap->setVal(varMaps2, "/components/bss_1", "dc_volts", sval);
//     vmap->clearVmap(varMaps2);


//     //delete vmap;
//     //return 0;

//     varmap varMap;

//     //std::map<std::string,assetVar*>
//     //std::map<std::string,std::map<std::string,assetVar*>>
//     varsmap varMaps;

// // first we create a list of vars

//     varMap["d1"]  = new assetVar("d1",true);
    
    // delete funItem;
    // delete iface;
    

    // delete vmap;
    return 0;
}
//#endif