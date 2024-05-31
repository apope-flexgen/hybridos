#include "gcom_timer.h"
#include "gcom_config.h"

void load_points_from_dbi_client(cfg& myCfg);
void send_to_dbi_client(cfg& myCfg, std::string& uri, std::any value);
void load_to_dbi_client(cfg& myCfg, std::shared_ptr<cfg::io_point_struct> io_point_shared_ptr, std::any value);
void send_to_dbi_callback(std::shared_ptr<TimeObject> t, void* pDbiStruct);
void start_send_to_dbi_timer(struct cfg& myCfg, std::string point_uri, int dbi_update_frequency);