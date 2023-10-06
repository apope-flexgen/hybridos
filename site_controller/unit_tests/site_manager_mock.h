#include <Site_Manager.h>

class Site_Manager_Mock : Site_Manager {
public:
    Site_Manager_Mock() : Site_Manager(NULL){};

    void call_parse_default_vals(cJSON* JSON_defaults, Fims_Object& default_vals) { default_vals = parse_field_defaults(JSON_defaults).first; }
};
