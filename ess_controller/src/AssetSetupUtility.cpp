#ifndef ASSETSETUPUTILITY_CPP
#define ASSETSETUPUTILITY_CPP

#include <string>
#include <unordered_set>

// /home/docker/hybridos/ess_controller/configs/functional_ess_controller/stdcfg
extern const std::string stdcfgPath = "stdcfg";
extern const std::string stdcfgEssDir = "/ess";
extern const std::string stdcfgPcsDir = "/pcs";
extern const std::string stdcfgBmsDir = "/bms";

extern const std::unordered_set<std::string> stdcfgEssDirFileNames = {
    "std_ess_controller_manager", "std_ess_controller_controls", "std_ess_controller_faults", "std_site_reporter"
};

extern const std::unordered_set<std::string> stdcfgPcsDirFileNames = {
    "std_v_pcs_manager",        "std_v_pcs_manager_controls", "std_v_pcs_manager_faults", "std_pcs_manager",
    "std_pcs_manager_controls", "std_pcs_manager_faults",     "std_pcs_module",           "std_pcs_module_faultsjson"
};
extern const std::unordered_set<std::string> stdcfgBmsDirFileNames = {
    "std_v_bms_manager", "std_v_bms_manager_controls", "std_v_bms_manager_faults",
    "std_bms_manager",   "std_bms_manager_controls",   "std_bms_manager_faults",
    "std_bms_rack"
};

#endif