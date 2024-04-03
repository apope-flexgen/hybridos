extern "C" {
#include "tmwscl/utils/tmwsim.h"
}
void send_analog_command_callback(void* pSetWork);
void send_binary_command_callback(void* pSetWork);
void send_interval_analog_command_callback(void* pSetWork);
void send_interval_binary_command_callback(void* pSetWork);
void handle_batch_sets(TMWSIM_POINT* dbPoint, double value);