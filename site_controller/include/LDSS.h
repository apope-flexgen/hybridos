/**
 * LDSS.h
 * 
 * Created October 2021
 */

#ifndef LDSS_H_
#define LDSS_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <list>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <macros.h>
#include <Asset_Generator.h>
#include <ESS_Manager.h>

/**
 * @param enabled LDSS feature enable flag.
 * @param priority_setting Enumerated integer representing static or dynamic priority.
 * @param max_load_threshold Max load before a new generator should be started.
 * @param min_load_threshold_percent Min load before a running generator should be stopped.
 * @param warmup_time Amount of time a gen should stay running before it is allowed to stop.
 * @param cooldown_time Amount of time a gen should stay stopped before it is allowed to start.
 * @param start_gen_time How long to wait after max load threshold is hit to start a gen.
 * @param stop_gen_time How long to wait after min load threshold is hit to stop a gen.
 * @param enable_soc_thresholds_flag Whether to manage SoC thresholds as part of this feature
 * @param min_soc_percent Min SoC until a generator should be started iff enable_soc_thresholds_flag
 * @param max_soc_percent Max SoC until a generator should be stopped iff enable_soc_thresholds_flag
 */
struct LDSS_Settings {
    bool enabled;
    LDSS_Priority_Setting priority_setting;
    float max_load_threshold_percent;
    float min_load_threshold_percent;
    int warmup_time;
    int cooldown_time;
    int start_gen_time;
    int stop_gen_time;
    bool enable_soc_thresholds_flag;
    float min_soc_percent;
    float max_soc_percent;
    ESS_Manager* pEss;
};

class Asset_Generator;

class LDSS {
private:
    // check() helper functions
    void check_start_generator(int num_controllable, float max_load_threshold_kw, float target_kw);
    void check_stop_generator(int num_controllable, float min_load_threshold_kw, float target_kw);
    void update_cooldown_and_warmup();
public:
    std::vector<Asset_Generator*> generators;
    LDSS_Priority_Setting priority_setting;
    int stop_gen_time;
    int stop_gen_countdown;
    int start_gen_time;
    int start_gen_countdown;
    int warmup_time;
    int cooldown_time;
    int min_generators_active;
    float max_load_threshold_percent;
    float min_load_threshold_percent;
    bool enabled;
    bool enabled_rising_edge;
    bool start_first_gen;
    bool first_gen_is_starting;

    bool enable_soc_thresholds_flag;
    float min_soc_percent;
    float max_soc_percent;
    ESS_Manager* pEss;

    LDSS();
    bool configure_priorities(std::vector<Asset_Generator*> const &pg, cJSON* static_run_priorities);
    void update_settings(LDSS_Settings &settings);
    void enable(bool flag);
    void check(float targetActivePowerkW);
    void reset_start_gen_countdown();
    void reset_stop_gen_countdown();
    void adjust_dynamic_start_priorities(Asset_Generator*);
    void adjust_dynamic_stop_priorities(Asset_Generator*);
    void move_gen_to_front_of_both_start_priority_lists(Asset_Generator*);
    void move_gen_to_front_of_both_stop_priority_lists(Asset_Generator*);
    void move_gen_to_back_of_dynamic_start_priority_list(Asset_Generator*);
    void move_gen_to_back_of_dynamic_stop_priority_list(Asset_Generator*);
    bool start_generator();
    void stop_generator();
    Asset_Generator* get_gen_to_start();
    Asset_Generator* get_gen_to_stop();
    bool is_time_for_check();
    int get_num_controllable();
};

#endif /* LDSS_H_ */