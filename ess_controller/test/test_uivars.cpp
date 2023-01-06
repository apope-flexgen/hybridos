/*
 * test_uivars
 * * load a varmap from a Ui spec sampleui.json
 * phil wilshire 10/27/2020
*
 */
#include "asset.h"
#include "varMapUtils.h"
#include "alarm.h"
#include "assetFunc.cpp"
#include "chrono_utils.hpp"

int main ()
{

    varsmap vmap;
    VarMapUtils vm;
//    system("mkdir -p run_configs");

 //

    //void (*tf)(void *) = (void (*tf)(void *))
    
    
    const char* cfgname = "configs/sample_ui.json";

    FPS_ERROR_PRINT("%s >> Getting initial configuration   from [%s]\n", __func__, cfgname);

    vm.configure_vmap(vmap, cfgname);
    // this a test for our config.
    cJSON *cjbm = nullptr;
    cjbm = vm.getMapsCj(vmap);
    const char* fname =  "run_configs/sample_ui_out.json";
    vm.write_cjson(fname, cjbm);
    //cJSON* cjbm = ess_man->getConfig();
    cJSON_Delete(cjbm);

    return 0;

}
