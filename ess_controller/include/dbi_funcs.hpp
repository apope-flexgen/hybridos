#pragma once

#include <fstream>
#include <vector>

#include "asset.h"
#include "chrono_utils.hpp"
#include "formatters.hpp"  // for sending out the assetVar's if they need to be updated

struct dbi_var_ref
{
    const assetVar& dbi_var;
    std::chrono::steady_clock::time_point update_timeout_start;
};

// no sorting required, just put this inside varmap_utils as a data structure
// that or make it global
using dbi_var_list = std::vector<dbi_var_ref>;

// this function sends the dbi vars out if they have timed out then resets their
// timers
bool check_dbi_var_timeouts(dbi_var_list& var_list, fims* fims_gateway);

// this adds a var to be monitored on the list for updates
// possible TODO: Make sure that the same assetVar isn't added twice
// probably just needs to check the address
bool add_dbi_var(dbi_var_list& var_list, assetVar& to_add);

// this function will be run once and put all of our assetVar's into the list
// this inits all of their timeouts for synchronization purposes
bool init_dbi_var_list_timeouts(dbi_var_list& var_list);

// this tells dbi to send the data to /ess/dbi/comp for processing so it gets
// back into our database we "check" it there for some reason - I have no idea
bool get_dbi_var(const assetVar& to_get, fims* p_fims);

// testing 123
bool send_config_to_dbi(const std::string& file_path_and_name, fims* p_fims);

bool get_config_from_dbi(const char* filename_no_ext, fims* p_fims);
