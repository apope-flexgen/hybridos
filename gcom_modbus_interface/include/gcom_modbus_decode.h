#include <iostream>
#include <iomanip>
#include <sstream>
#include <any>

#include "gcom_config.h"
#include "gcom_iothread.h"

#include "shared_utils.h"


/**
 * @brief Output the full details of an io_point to a std::stringstream.
 * 
 * Point is output to the stringstream in the form of a json object (enclosed in curly braces).
 * 
 * @param io_point the io_point_struct to "print" to the stringstream
 * @param ss the std::stringstream to append the output to
 * @param myCfg the config structure that the io_point belongs to
*/
void get_io_point_full(std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &myCfg);


/**
 * @brief Add escaped quotes around either side of the string.
 * 
 * For example, a string "some_string" would become "\"some_string\"".
 * 
 * @param si the string to add quotes around
 * @return the string with escaped quotes on either side
*/
std::string addQuote(const std::string &si);
bool gcom_modbus_decode(std::shared_ptr<IO_Work> io_work, std::stringstream &ss, struct cfg &myCfg);
bool gcom_modbus_decode_debug(std::shared_ptr<IO_Work> io_work, std::stringstream &ss, struct cfg &myCfg, bool include_key, bool debug);
bool decode_bval(bool bval, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg& cfg);
bool decode_bval(bool bval, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg, bool include_key);
u64 gcom_decode_any(u16* raw16, u8*raw8, std::shared_ptr<cfg::io_point_struct>io_point, std::any& output, struct cfg& myCfg);
bool modbus_decode(std::shared_ptr<cfg::io_point_struct> io_point, std::any& value, std::stringstream &ss, struct cfg& cfg);
bool modbus_decode( std::shared_ptr<cfg::io_point_struct> io_point, std::any& value, std::stringstream &ss, struct cfg& cfg, bool include_key);
void store_raw_data(std::shared_ptr<IO_Work> io_work, bool debug);
void extract_raw_data(std::shared_ptr<IO_Work> io_work, bool debug);
bool decode_bval_from_value(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg, bool include_key);
void decode_packed( std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &bss, struct cfg &cfg, bool include_key);
void decode_packed( std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &bss, struct cfg &cfg);
void decode_individual_bits(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg, bool include_key);
void decode_individual_bits(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg);
void set_io_point_float_val(cfg::io_point_struct* io_point, double val);
double get_io_point_float_val(cfg::io_point_struct* io_point);
u8 get_io_point_bit_val(cfg::io_point_struct* io_point);
u64 get_io_point_raw_val(cfg::io_point_struct* io_point);
u64 get_io_point_forced_val(cfg::io_point_struct* io_point);

