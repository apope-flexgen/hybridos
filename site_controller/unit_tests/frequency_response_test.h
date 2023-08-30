#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Freq_Resp_Component.h>
#include <test_tools.h>

typedef struct pfr_state {
    float active_cmd;
    float trig_freq;
    float droop_freq;
    bool droop_limit;
} pfr_state;

typedef struct frrs_state {
    float trig_freq;
    bool in_trig;
    timespec trig_over;
    float recov_freq;
    bool in_recov;
    timespec recov_over;
    int slew_time;
    int slew_rate;
} frrs_state;

typedef struct ffr_state {
    float active_cmd;
    float trig_freq;
    float recov_freq;
    float instant_recov_freq;
    bool in_recov;
    timespec recov_over;
    bool in_trig;
    timespec trig_over;
    bool in_cool;
    timespec cool_over;
} ffr_state;

// create mock frequency object
class FR_Comp_Mock : public Freq_Resp_Component {
public:
    // quickies
    void reset_slews(int diff) { slew_cmd_kw.reset_slew_target(diff); };
    bool get_in_recov() { return in_recovery.value.value_bool; };
    bool get_in_cooldown() { return in_cooldown.value.value_bool; };
    // group setters
    void set_pfr_state(pfr_state);
    void set_frrs_state(frrs_state, float active_cmd, float inactive_cmd);
    void set_ffr_state(ffr_state);
};

class frequency_response_test : public testing::Test {
public:
    FR_Comp_Mock fr_comp_mock;
    virtual void SetUp() {}
    virtual void TearDown() {}
};

// PFR
TEST_F(frequency_response_test, pfr) {
    typedef struct pfr_test {
        int id;
        pfr_state state;
        float input_frequency;
        float output_power;
    } pfr_test;

    //        state name             active_cmd     trig_freq   droop_freq      droop_limit
    pfr_state uf_droop_limited = { 10.0F, 59.0F, 58.0F, true };
    pfr_state uf_droop_unlimited = { 10.0F, 59.0F, 58.0F, false };
    pfr_state of_droop_limited = { -10.0F, 61.0F, 62.0F, true };
    pfr_state of_droop_unlimited = { -10.0F, 61.0F, 62.0F, false };

    std::vector<pfr_test> tests = {
        //  ID   state name             input_freq  out_pow
        { 1, uf_droop_limited, 59.0F, 0.0F },       // uf limited @ trig freq.
        { 2, uf_droop_limited, 58.5F, 5.0F },       // uf limited between trig and droop
        { 3, uf_droop_limited, 58.0F, 10.0F },      // uf limited @ droop freq.
        { 4, uf_droop_limited, 57.0F, 10.0F },      // uf limited beyond droop freq.
        { 5, uf_droop_unlimited, 59.0F, 0.0F },     // uf unlimited @ trig freq.
        { 6, uf_droop_unlimited, 58.5F, 5.0F },     // uf unlimited between trig and droop
        { 7, uf_droop_unlimited, 58.0F, 10.0F },    // uf unlimited @ droop freq.
        { 8, uf_droop_unlimited, 57.0F, 20.0F },    // uf unlimited beyond droop freq.
        { 9, of_droop_limited, 61.0F, 0.0F },       // of limited @ trig freq.
        { 10, of_droop_limited, 61.5F, -5.0F },     // of limited between trig and droop
        { 11, of_droop_limited, 62.0F, -10.0F },    // of limited @ droop freq.
        { 12, of_droop_limited, 63.0F, -10.0F },    // of limited beyond droop freq.
        { 13, of_droop_unlimited, 61.0F, 0.0F },    // of unlimited @ trig freq.
        { 14, of_droop_unlimited, 61.5F, -5.0F },   // of unlimited between trig and droop
        { 15, of_droop_unlimited, 62.0F, -10.0F },  // of unlimited @ droop freq.
        { 16, of_droop_unlimited, 63.0F, -20.0F }   // of unlimited beyond droop freq.
    };

    for (auto& test : tests) {
        // log test identifier
        test_logger t_log("pfr", test.id, tests.size());
        // set state
        fr_comp_mock.set_pfr_state(test.state);
        // set input. ess slew limits / rated power not used for calculations so they can be constant inputs
        Frequency_Response_Inputs input = { 11.0F, 9.0F, 15.0F, test.input_frequency, { 1, 0 } };
        // run test
        Frequency_Response_Outputs actual_result = fr_comp_mock.frequency_response(input);
        // check results. ess limits should never change in pfr
        t_log.float_results.push_back({ test.output_power, actual_result.output_kw, "output_kw" });
        t_log.float_results.push_back({ 11.0F, actual_result.ess_max_potential, "ess max potential" });
        t_log.float_results.push_back({ 9.0F, actual_result.ess_min_potential, "ess min potential" });
        t_log.check_solution();
    }
}

// FRRS, recovery latch, and slew
TEST_F(frequency_response_test, frrs) {
    typedef struct frrs_test {
        int id;
        frrs_state state;
        float active_cmd;
        float inactive_cmd;
        float input_frequency;
        float output_power;
        bool in_recov;
    } frrs_test;

    //         state name   trig_freq   in_trig     trig_over   recov_freq  in_recov    recov_over  slew_time   slew_rate
    frrs_state uf_no_trig = { 59.0F, false, { 0, 0 }, 59.0F, false, { 0, 0 }, 1, 10 };
    frrs_state of_no_trig = { 61.0F, false, { 0, 0 }, 61.0F, false, { 0, 0 }, 1, 10 };
    frrs_state uf_in_reco = { 59.0F, true, { 2, 0 }, 59.0F, true, { 2, 0 }, 1, 10 };
    frrs_state of_in_reco = { 61.0F, true, { 2, 0 }, 61.0F, true, { 2, 0 }, 1, 10 };

    std::vector<frrs_test> tests = {
        //   ID state name      active_cmd  inactive_cmd    input_freq  out_pow     in_recov
        { 1, uf_no_trig, 10.0F, 0.0F, 58.0F, 10.0F, false },    // uf no trig to trig
        { 2, of_no_trig, -10.0F, 0.0F, 62.0F, -10.0F, false },  // of no trig to trig
        { 3, uf_no_trig, 20.0F, 0.0F, 58.0F, 10.0F, false },    // uf no trig to trig but slew cutoff
        { 4, of_no_trig, -20.0F, 0.0F, 62.0F, -10.0F, false },  // of no trig to trig but slew cutoff
        { 5, uf_in_reco, 10.0F, 0.0F, 58.0F, 10.0F, true },     // uf recovery conditions gone but it is latched
        { 6, of_in_reco, -10.0F, 0.0F, 62.0F, -10.0F, true },   // of recovery conditions gone but it is latched
        { 7, uf_no_trig, 10.0F, 1.0F, 60.0F, 1.0F, false }      // non-zero inactive cmd
    };

    for (auto& test : tests) {
        // log test identifier
        test_logger t_log("frrs", test.id, tests.size());
        // set state
        fr_comp_mock.set_frrs_state(test.state, test.active_cmd, test.inactive_cmd);
        // set input. ess slew limits / rated power not used for calculations so they can be constant inputs
        Frequency_Response_Inputs input = { 11.0F, 9.0F, 15.0F, test.input_frequency, { 1, 0 } };
        // run test
        Frequency_Response_Outputs actual_result = fr_comp_mock.frequency_response(input);
        // check results. ess limits should never change in frrs
        t_log.range_results.push_back({ test.output_power, 0.001F, actual_result.output_kw, "output_cmd" });
        t_log.float_results.push_back({ 11.0F, actual_result.ess_max_potential, "ess max potential" });
        t_log.float_results.push_back({ 9.0F, actual_result.ess_min_potential, "ess min potential" });
        t_log.bool_results.push_back({ test.in_recov, fr_comp_mock.get_in_recov(), "in_recovery" });
        t_log.check_solution();
    }
}

// FFR and recovery/cooldown
TEST_F(frequency_response_test, ffr_recovery) {
    typedef struct ffr_test {
        int id;
        ffr_state state;
        float input_frequency;
        float output_power;
        bool output_in_recov;
        bool output_in_cool;
    } ffr_test;
    //        state name                active_cmd  trig_freq  recov_freq  instant_freq  in_recov  recov_over  in_trig  trig_over  in_cool  cool_over
    ffr_state uf_no_trig_no_cool = { 10.0F, 59.0F, 59.0F, 60.0F, false, { 0, 0 }, false, { 0, 0 }, false, { 0, 0 } };
    ffr_state of_no_trig_no_cool = { -10.0F, 61.0F, 61.0F, 60.0F, false, { 0, 0 }, false, { 0, 0 }, false, { 0, 0 } };
    ffr_state uf_no_trig_yes_cool = { 10.0F, 59.0F, 59.0F, 60.0F, false, { 0, 0 }, false, { 0, 0 }, true, { 2, 0 } };
    ffr_state of_no_trig_yes_cool = { -10.0F, 61.0F, 61.0F, 60.0F, false, { 0, 0 }, false, { 0, 0 }, true, { 2, 0 } };
    ffr_state uf_leaving_cool = { 10.0F, 59.0F, 59.0F, 60.0F, false, { 0, 0 }, false, { 0, 0 }, true, { 0, 0 } };
    ffr_state uf_yes_trig_no_recov = { 10.0F, 59.0F, 59.0F, 60.0F, false, { 0, 0 }, true, { 2, 0 }, false, { 0, 0 } };
    ffr_state of_yes_trig_no_recov = { -10.0F, 61.0F, 61.0F, 60.0F, false, { 0, 0 }, true, { 2, 0 }, false, { 0, 0 } };
    ffr_state uf_yes_trig_yes_recov = { 10.0F, 59.0F, 59.0F, 60.0F, true, { 0, 0 }, true, { 2, 0 }, false, { 0, 0 } };
    ffr_state of_yes_trig_yes_recov = { -10.0F, 61.0F, 61.0F, 60.0F, true, { 0, 0 }, true, { 2, 0 }, false, { 0, 0 } };

    std::vector<ffr_test> tests = {
        //  ID   state name             input_freq  out_pow     out_in_recov    out_in_cool
        { 1, uf_no_trig_no_cool, 58.0F, 10.0F, false, false },    // uf no trig to trig
        { 2, of_no_trig_no_cool, 62.0F, -10.0F, false, false },   // of no trig to trig
        { 3, uf_no_trig_no_cool, 60.0F, 0.0F, false, false },     // uf no trig to no trig (no freq dev)
        { 4, of_no_trig_no_cool, 60.0F, 0.0F, false, false },     // of no trig to no trig (no req dev)
        { 5, uf_yes_trig_no_recov, 59.5F, 10.0F, true, false },   // uf trig to recov
        { 6, of_yes_trig_no_recov, 60.5F, -10.0F, true, false },  // of trig to recov
        { 7, uf_yes_trig_no_recov, 60.1F, 0.0F, false, true },    // uf trig to instant no trig
        { 8, of_yes_trig_no_recov, 59.9F, 0.0F, false, true },    // of trig to instant no trig
        { 9, uf_yes_trig_yes_recov, 59.5F, 0.0F, false, true },   // uf recov to no trig
        { 10, of_yes_trig_yes_recov, 60.5F, 0.0F, false, true },  // of recov to no trig
        { 11, uf_no_trig_yes_cool, 58.0F, 0.0F, false, true },    // uf no trig to no trig (in cooldown)
        { 12, of_no_trig_yes_cool, 62.0F, 0.0F, false, true },    // of no trig to no trig (in cooldown)
        { 13, uf_leaving_cool, 60.0F, 0.0F, false, false },       // uf cooldown no trig to no cooldown no trig
        { 14, uf_leaving_cool, 58.0F, 10.0F, false, false }       // uf cooldown to trig
    };

    for (auto& test : tests) {
        // log test identifier
        test_logger t_log("ffr and recovery/cooldown", test.id, tests.size());
        // set state
        fr_comp_mock.set_ffr_state(test.state);
        // set input. ess slew limits / rated power not used for calculations so they can be constant inputs
        Frequency_Response_Inputs input = { 11.0F, 9.0F, 15.0F, test.input_frequency, { 1, 0 } };
        // run test
        Frequency_Response_Outputs actual_result = fr_comp_mock.frequency_response(input);
        // check results. ess limits should always be +/- rated power in ffr
        t_log.float_results.push_back({ test.output_power, actual_result.output_kw, "output_kw" });
        t_log.float_results.push_back({ (test.output_power == 0.0F) ? 11.0F : 15.0F, actual_result.ess_max_potential, "ess max potential" });
        t_log.float_results.push_back({ (test.output_power == 0.0F) ? 9.0F : -15.0F, actual_result.ess_min_potential, "ess min potential" });
        t_log.bool_results.push_back({ test.output_in_recov, fr_comp_mock.get_in_recov(), "in_recovery" });
        t_log.bool_results.push_back({ test.output_in_cool, fr_comp_mock.get_in_cooldown(), "in_cooldown" });
        t_log.check_solution();
    }
}

void FR_Comp_Mock::set_pfr_state(pfr_state state) {
    // variables
    active_cmd_kw.value.set(state.active_cmd);
    trigger_freq_hz.value.set(state.trig_freq);
    droop_freq_hz.value.set(state.droop_freq);
    droop_limit_flag.value.set(state.droop_limit);
    recovery_freq_hz.value.set(state.trig_freq);
    instant_recovery_freq_hz.value.set(state.trig_freq);
    // constants
    inactive_cmd_kw.value.set(0.0F);
    ess_slew_override.value.set(false);
    active_response_status.value.set(false);
    trigger_duration_sec.value.set(10);
    in_cooldown.value.set(false);
    cooldown_duration_sec.value.set(0);
    recovery_duration_sec.value.set(0);
    in_recovery.value.set(false);
    recovery_latch.value.set(false);
    droop_bypass_flag.value.set(false);
    cooldown_over_time = { 0, 0 };
    slew_rate_kw.value.set(INT_MAX);
    output_kw.value.set(0.0F);
    // init
    ASSERT_EQ(initialize_state(60.0F), true);
    reset_slews(1);  // called after initialize_state to override the slew reset that happened there
}

void FR_Comp_Mock::set_frrs_state(frrs_state state, float active_cmd, float inactive_cmd) {
    // constants
    ess_slew_override.value.set(false);
    droop_bypass_flag.value.set(true);
    instant_recovery_freq_hz.value.set(60.0F);
    recovery_latch.value.set(true);
    trigger_duration_sec.value.set(10);
    recovery_duration_sec.value.set(10);
    cooldown_duration_sec.value.set(10);
    in_cooldown.value.set(false);
    cooldown_over_time = { 0, 0 };
    // variables
    active_cmd_kw.value.set(active_cmd);
    inactive_cmd_kw.value.set(inactive_cmd);
    output_kw.value.set(state.in_trig ? active_cmd : inactive_cmd);
    slew_rate_kw.value.set(state.slew_rate);
    trigger_freq_hz.value.set(state.trig_freq);
    droop_freq_hz.value.set(state.trig_freq);
    recovery_freq_hz.value.set(state.recov_freq);
    in_recovery.value.set(state.in_recov);
    recovery_over_time = state.recov_over;
    active_response_status.value.set(state.in_trig);
    trigger_over_time = state.trig_over;
    // init
    ASSERT_EQ(initialize_state(60.0F), true);
    reset_slews(state.slew_time);  // called after initialize_state to override the slew reset that happened there
}

void FR_Comp_Mock::set_ffr_state(ffr_state state) {
    // constants
    ess_slew_override.value.set(true);
    droop_bypass_flag.value.set(true);
    recovery_latch.value.set(false);
    trigger_duration_sec.value.set(10);
    recovery_duration_sec.value.set(10);
    cooldown_duration_sec.value.set(10);
    inactive_cmd_kw.value.set(0.0F);
    slew_rate_kw.value.set(INT_MAX);
    // variables
    output_kw.value.set(state.in_trig ? state.active_cmd : 0.0F);
    active_cmd_kw.value.set(state.active_cmd);
    trigger_freq_hz.value.set(state.trig_freq);
    droop_freq_hz.value.set(state.trig_freq);
    recovery_freq_hz.value.set(state.recov_freq);
    instant_recovery_freq_hz.value.set(state.instant_recov_freq);
    in_recovery.value.set(state.in_recov);
    recovery_over_time = state.recov_over;
    active_response_status.value.set(state.in_trig);
    trigger_over_time = state.trig_over;
    in_cooldown.value.set(state.in_cool);
    cooldown_over_time = state.cool_over;
    // init
    ASSERT_EQ(initialize_state(60.0F), true);
    reset_slews(1);  // called after initialize_state to override the slew reset that happened there
}
