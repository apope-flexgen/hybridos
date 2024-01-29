// gcom_modbus_decode.cpp
// p. wilshire
// s .reynolds
// 11_08_2023
// self review 11_22_2023
// pr review pw 11_30_2023

// removed watchdog access , we'll do that like heartbeats

#include <iostream>
#include <iomanip>
#include <sstream>
#include <any>

#include "gcom_config.h"
#include "gcom_iothread.h"

#include "shared_utils.h"
#include "gcom_modbus_decode.h"

// To convert a `uint64_t` to `int64_t` in C++, you can simply assign the unsigned value to a signed variable. However, you should be cautious about this operation since you might face issues when the `uint64_t` value is too large to fit into an `int64_t` without overflowing.

// Here’s a simple example:

// ```cpp
// #include <iostream>
// #include <cstdint> // for std::uint64_t, std::int64_t

// int main() {
//     std::uint64_t unsignedValue = 12345678901234567890u;
//     std::int64_t signedValue;

//     if (unsignedValue <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
//         signedValue = static_cast<std::int64_t>(unsignedValue);
//         std::cout << "Converted value: " << signedValue << std::endl;
//     } else {
//         std::cerr << "Warning: Conversion would overflow. Cannot convert." << std::endl;
//     }

//     return 0;
// }
// ```

// Explanation:

// 1. The `if` condition checks if `unsignedValue` is within the valid range for `int64_t`. `std::numeric_limits<std::int64_t>::max()` provides the maximum value that `int64_t` can hold.

// 2. If `unsignedValue` is within the range, it gets cast to `int64_t` and stored in `signedValue`.

// 3. If the unsigned value is too large, a warning message is printed, and no conversion is performed to avoid overflow. You need to decide how to handle such cases in your actual application.

// Remember to include the necessary headers, and if your environment supports C++11 or later, use the `<cstdint>` header instead of `<stdint.h>`.

u64 get_io_point_forced_val(std::shared_ptr<cfg::io_point_struct> io_point);
u64 get_io_point_raw_val(std::shared_ptr<cfg::io_point_struct> io_point);
u8 get_io_point_bit_val(std::shared_ptr<cfg::io_point_struct> io_point);

std::string hex_to_str(u16 *raw16, int size);


void gcom_modbus_decode(std::shared_ptr<IO_Work> io_work, std::stringstream &ss, struct cfg &myCfg)
{
    gcom_modbus_decode_debug(io_work, ss, myCfg, true, myCfg.debug_decode);
}


void gcom_modbus_decode_debug(std::shared_ptr<IO_Work> io_work, std::stringstream &ss, struct cfg &myCfg, bool include_key, bool debug)
{
    #ifdef FPS_DEBUG_MODE
    debug = false;
    if (debug)
    {
        std::cout << " decode starting ... # io_points " << io_work->io_points.size()
                  << " start_register :" << io_work->start_register
                  << " offset :" << io_work->offset
                  << " num_registers :" << io_work->num_registers
                  << " type : " << myCfg.typeToStr(io_work->register_type)
                  << " io_points : " << io_work->io_points.size()
                  << std::endl;
        std::cout << "                    buf16_data  :"
                  << " 0x";
        for (int i = 0; i < io_work->num_registers; ++i)
        {
            std::cout << std::setfill('0') << std::setw(8) << std::hex << io_work->buf16[i] << " ";
            std::cout << " " << std::dec;
        }
        // std::cout << "  buf8_data  :"<< " 0x";
        // for (int i = 0 ; i < io_work->num_registers ; ++ i)
        // {
        //     std::cout <<  std::setfill('0') << std::setw(4) << std::hex <<  io_work->buf8[i] <<" ";
        //     std::cout <<std::dec<<  "  ";
        // }
        if ((io_work->register_type == cfg::Register_Types::Discrete_Input) || (io_work->register_type == cfg::Register_Types::Coil))
        {
            std::cout << "  buf8_data  dec :";
            for (int i = 0; i < io_work->num_registers; ++i)
            {
                std::cout << (int)io_work->buf8[i] << " ";
                std::cout << " " << std::dec;
            }
        }
        else
        {
            std::cout << "  buf16_data  dec :";
            for (int i = 0; i < io_work->num_registers; ++i)
            {
                std::cout << (int)io_work->buf16[i] << " ";
                std::cout << " " << std::dec;
            }
            std::cout << " " << std::endl;
        }
    }
    #endif
    if ((io_work->register_type == cfg::Register_Types::Discrete_Input) || (io_work->register_type == cfg::Register_Types::Coil))
    {
        bool firstItem = true;

        for (auto io_point : io_work->io_points)
        {
            u64 forced_val = get_io_point_forced_val(io_point.get());
            if (!firstItem)
            {
                ss << ",";
            }
            else
            {
                firstItem = false;
            }

            auto index = io_point->offset - io_work->offset;
            
            #ifdef FPS_DEBUG_MODE
            int buf_idx = index / 8;
            int buf_item = index % 8;
            if (debug)
            {
                std::cout << " bit offset : " << io_point->offset
                          << " index  : " << index
                          << " buf_idx  : " << buf_idx
                          << " buf_item  : " << buf_item
                          << " size : " << io_point->size
                          << std::endl;
            }
            #endif

            bool bval = (io_work->buf8[index] > 0);
            if (io_point->is_forced && !io_work->unforced)
            {
                bval = (forced_val > 0);
            }

            if (io_work->full)
            {
                if (include_key)
                {
                    ss << addQuote(io_point->id) << ":";
                }
                get_io_point_full(io_point, ss, myCfg);
            }
            else
            {
                decode_bval(bval, io_point, ss, myCfg, include_key);
            }
        }
    }
    else if ((io_work->register_type == cfg::Register_Types::Input) || (io_work->register_type == cfg::Register_Types::Holding))
    {
        bool firstItem = true;
        for (auto io_point : io_work->io_points)
        {
            u64 forced_val = get_io_point_forced_val(io_point.get());
            if (!firstItem)
            {
                ss << ",";
            }
            else
            {
                firstItem = false;
            }

            auto index = io_point->offset - io_work->offset;

#ifdef FPS_DEBUG_MODE
            if (debug)
            {
                std::cout << " reg  offset : " << io_point->offset
                          << " index  : " << index
                          //<< " buf_idx  : "      << buf_idx
                          //<< " buf_iitem  : "    << buf_item
                          << " size : " << io_point->size
                          << std::endl;
            }
            #endif

            std::any output;

            u16 *bvar = &io_work->buf16[index];
            if (io_point->is_forced && !io_work->unforced)
            {
                //bvar = (u16 *)get_forced_value(io_point);
                //bvar = (u16 *)&io_point->forced_val;
                bvar = (u16 *)&forced_val; //io_point->forced_val;
            }
            // gcom_decode_any(&io_work->buf16[index], nullptr, io_point, output, myCfg);
            // modbus_decode(&io_work->buf16[index], io_point, output, ss, myCfg);
            gcom_decode_any(bvar, nullptr, io_point, output, myCfg);
            if (io_work->full)
            {
                if (include_key)
                {
                    ss << addQuote(io_point->id) << ":";
                }
                get_io_point_full(io_point, ss, myCfg);
            }
            else
            {
                //modbus_decode(bvar, io_point, output, ss, myCfg, include_key);
                modbus_decode(io_point, output, ss, myCfg, include_key);
            }
        }
    }
    else
    {
        FPS_ERROR_LOG(" decode  unknown type ");
    }
    #ifdef FPS_DEBUG_MODE
    if (debug)
    {
        std::cout << " decode output : [" << ss.str() << "]" << std::endl;
    }
    #endif
}

// std::string addQuote(const std::string &si)
// {
//     return "\"" + si + "\"";
// }


void printConstAny(const std::any &value, int indent);

template <typename T>
bool testAnyVal(const std::any &anyVal, const T &defaultValue)
{
    return anyVal.type() == typeid(T);
}

#include <cxxabi.h>
#include <memory>
#include <typeinfo>
#include <iostream>

std::string demangle(const char* name) {
    int status = -1; 
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };
    return (status==0) ? res.get() : name ;
}

template <typename T>
T convertAnyToType(const std::any &anyVal) {
    if (anyVal.type() == typeid(int)) {
        return static_cast<T>(std::any_cast<int>(anyVal));
    } else if (anyVal.type() == typeid(float)) {
        return static_cast<T>(std::any_cast<float>(anyVal));
    } else if (anyVal.type() == typeid(unsigned long)) {
        return static_cast<T>(std::any_cast<unsigned long>(anyVal));
    }
    throw std::bad_any_cast();
}

template <typename T>
T getAnyVal(const std::any &anyVal, const T &defaultValue) {
    try {
        return std::any_cast<T>(anyVal);
    } catch (const std::bad_any_cast &) {
        try {
            return convertAnyToType<T>(anyVal);
        } catch (const std::bad_any_cast &) {
            std::cout << "Could not decode value properly; needed type '" << typeid(T).name() 
                      << "', but got type '" << demangle(anyVal.type().name()) << "'" << std::endl;
            return defaultValue;
        }
    }
}

// template <typename T>
// T xxgetAnyVal(const std::any &anyVal, const T &defaultValue)
// {
//     try
//     {
//         return std::any_cast<T>(anyVal);
//     }
//     catch (const std::bad_any_cast &)
//     {
//         //std::any def = defaultValue;
//         if (typeid(T) == typeid(double))
//         {
//             if (anyVal.type() == typeid(int))
//             {
//                 auto intval = std::any_cast<int>(anyVal);
//                 return (T)intval;
//             }
//             else if (anyVal.type() == typeid(float))
//             {
//                 auto intval = std::any_cast<float>(anyVal);
//                 return (T)intval;
//             }
//             else if (anyVal.type() == typeid(unsigned long))
//             {
//                 auto intval = std::any_cast<unsigned long>(anyVal);
//                 return (T)intval;
//             }
//             std::cout << " we need a double  from "<< demangle(anyVal.type().name()) << std::endl;
//         }
//         else if (typeid(T) == typeid(int))
//         {
//             if (anyVal.type() == typeid(float))
//             {
//                 auto intval = std::any_cast<float>(anyVal);
//                 return (T)intval;
//             }
//             else if (anyVal.type() == typeid(unsigned long))
//             {
//                 auto intval = std::any_cast<unsigned long>(anyVal);
//                 return (T)intval;
//             }
//             std::cout << " we need an int  from "<< demangle(anyVal.type().name()) << std::endl;
//         }
//         else if (typeid(T) == typeid(unsigned long))
//         {
//             if (anyVal.type() == typeid(float))
//             {
//                 auto intval = std::any_cast<float>(anyVal);
//                 return (T)intval;
//             }
//             else if (anyVal.type() == typeid(int))
//             {
//                 auto intval = std::any_cast<int>(anyVal);
//                 return (T)intval;
//             }
//             std::cout << " we need an unsigned long  from "<< demangle(anyVal.type().name()) << std::endl;
//         }

//         std::cout <<" could not decode value properly; default type '" << typeid(defaultValue).name() << "' value type '" << anyVal.type().name() 
//             << "'"
//             << std::endl;
//         std::any &nonConstAnyVal = const_cast<std::any&>(anyVal);
//         printConstAny(nonConstAnyVal, 2);

//         //printAny(anyVal, 2);
//         return defaultValue;
//     }
// }

/// @brief gcom_decode_any
/// correctly decode the raw16 buffer received by the modbus into an std::any structure
/// that can then be used to process the strigstream output.
/// @param raw16
/// @param raw8
/// @param io_point
/// @param output
/// @param myCfg
/// @return
u64 gcom_decode_any(u16 *raw16, u8 *raw8, std::shared_ptr<cfg::io_point_struct> io_point, std::any &output, struct cfg &myCfg)
{
    #ifdef FPS_DEBUG_MODE
    bool debug = false;
    #endif
    u64 raw_data = 0UL;

    u64 current_unsigned_val = 0UL;
    s64 current_signed_val = 0;
    f64 current_float_val = 0.0;

    if (io_point->size == 1)
    {
        raw_data = *(u16 *)raw16;

        current_unsigned_val = raw_data;
        #ifdef FPS_DEBUG_MODE
        if(debug)
            std::cout << " io_point raw_data " << raw_data <<" current_unsigned_val "<<  current_unsigned_val<< std::endl;
        #endif
        if (io_point->uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point->invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point->care_mask;
        }
        // do signed stuff (size 1 cannot be float):
        if (io_point->is_signed)
        {
            s16 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
            #ifdef FPS_DEBUG_MODE
            if(debug)
                std::cout << " io_point to_reinterpret " << to_reinterpret <<" current_signed_val "<<  current_signed_val<< std::endl;
            #endif
            output = static_cast<double>(current_signed_val);
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " current_signed_val " << current_signed_val << std::endl;
            #endif
        }
        else
        {

            output = static_cast<double>(current_unsigned_val);
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " current_unsigned_val " << current_unsigned_val << std::endl;
                          #endif
        }
    }
    else if (io_point->size == 2)
    {
        raw_data = (static_cast<u64>(raw16[0]) << 16) +
                   (static_cast<u64>(raw16[1]) << 0);

        current_unsigned_val = raw_data;
        #ifdef FPS_DEBUG_MODE
        if(debug)
            std::cout << " io_point raw_data " << raw_data <<" current_unsigned_val "<<  current_unsigned_val<< std::endl;
        #endif
        if (!io_point->is_byte_swap) // normal:
        {
            current_unsigned_val = raw_data;
        }
        else // swapped:
        {
            current_unsigned_val = (static_cast<u64>(raw16[0]) << 0) +
                                   (static_cast<u64>(raw16[1]) << 16);
        }
        if (io_point->uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point->invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point->care_mask;
        }
        // do signed/float stuff:
        if (io_point->is_signed)
        {
            s32 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
            //output = current_signed_val;
            double dval = static_cast<double>(current_signed_val); 
            output = dval;
            #ifdef FPS_DEBUG_MODE
            if (debug)
            {
                std::cout << " io_point signed to_reinterpret " << to_reinterpret 
                        << " name " << io_point->id 
                        <<" current_signed_val "<<  current_signed_val
                        <<" current_output_val "<<  static_cast<double>(current_signed_val)
                        <<" dval "<<  dval
                        << std::endl;
                printAny(output, 2);
                std::cout << " output  "
                          << " current_signed_val " << current_signed_val << std::endl;

            }
            #endif
        }
        else if (io_point->is_float)
        {
            f32 to_reinterpret = 0.0f;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_float_val = to_reinterpret;
            output = current_float_val;
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " current_float_val " << current_float_val << std::endl;
                          #endif
        }
        else
        {
            output = static_cast<double>(current_unsigned_val);
            #ifdef FPS_DEBUG_MODE
            if(debug)
                std::cout 
                        << " io_point name " << io_point->id 
                        <<" current_unsigned_val "<<  current_unsigned_val
                        << std::endl;
            #endif
        }
    }
    else // size of 4:
    {
        raw_data = (static_cast<u64>(raw16[0]) << 48) +
                   (static_cast<u64>(raw16[1]) << 32) +
                   (static_cast<u64>(raw16[2]) << 16) +
                   (static_cast<u64>(raw16[3]) << 0);

        if (!io_point->is_byte_swap) // normal:
        {
            current_unsigned_val = raw_data;
        }
        else // swapped:
        {
            current_unsigned_val = (static_cast<u64>(raw16[0]) << 0) +
                                   (static_cast<u64>(raw16[1]) << 16) +
                                   (static_cast<u64>(raw16[2]) << 32) +
                                   (static_cast<u64>(raw16[3]) << 48);
        }

        if (io_point->word_order > 0)
        {
            current_unsigned_val = (static_cast<u64>(raw16[io_point->byte_index[0]]) << 0) +
                                   (static_cast<u64>(raw16[io_point->byte_index[1]]) << 16) +
                                   (static_cast<u64>(raw16[io_point->byte_index[2]]) << 32) +
                                   (static_cast<u64>(raw16[io_point->byte_index[3]]) << 48);
        }

        if (io_point->uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point->invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point->care_mask;
        }
        // do signed/float stuff:
        if (io_point->is_signed) // this is only for whole numbers really (signed but not really float):
        {
            s64 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " current_signed_val " << current_signed_val << std::endl;
                          #endif
            output = static_cast<double>(current_signed_val);
        }
        else if (io_point->is_float)
        {
            f64 to_reinterpret = 0.0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_float_val = to_reinterpret;
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " current_float_val " << current_float_val << std::endl;
            #endif
            output = current_float_val;
        }
        else
        {
            output = static_cast<double>(current_unsigned_val);
        }
    } // end of size 4

    // do scaling/shift stuff at the end:
    if (io_point->is_float)
    {
        if (io_point->scale) // scale != 0
        {
            current_float_val /= io_point->scale;
        }
        current_float_val += static_cast<f64>(io_point->shift); // add shift
        #ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << " output  "
                      << " current_float_val #2 " << current_float_val << std::endl;
                      #endif
        output = current_float_val;
    }
    else if (io_point->is_signed) // unsigned whole numbers:
    {
        if (io_point->scale) // scale == 0
        {
            double scaled = static_cast<double>(current_signed_val) / io_point->scale;
            scaled += static_cast<double>(io_point->shift); // add shift
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " >>>>>>>>>>> output #1 "
                          << " scaled " << scaled << std::endl;
            #endif
            output = scaled;
        }
        else
        {
            current_signed_val += io_point->shift; // add shift
            current_signed_val >>= io_point->starting_bit_pos;

            output = static_cast<double>(current_signed_val);
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " >>>>>>>>>output #2 "
                          << " current_signed_val " << current_signed_val 
                          << " current_float_val " << current_float_val 
                          << " scale " << io_point->scale 
                          << std::endl;
                          #endif
        }
    }
    else // unsigned whole numbers:
    {
        if (io_point->scale) // scale == 0
        {
            auto scaled = static_cast<f64>(current_unsigned_val) / io_point->scale;
            scaled += static_cast<f64>(io_point->shift); // add shift
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " scaled #2" << scaled << std::endl;
                          #endif
            output = scaled;
        }
        else
        {
            current_unsigned_val += io_point->shift; // add shift
            current_unsigned_val >>= io_point->starting_bit_pos;
            output = current_unsigned_val;
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " output  "
                          << " current_unsigned_val #3 " << current_unsigned_val << std::endl;
                          #endif
        }
    }
    // TODO remove float64
    return raw_data;
}

void decode_bval(bool bval, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg)
{
    decode_bval(bval, io_point, ss, cfg, true);
}

void decode_bval(bool bval, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg, bool include_key)
{
    if (include_key)
    {
        ss << addQuote(io_point->id) << ":";
    }
    if (io_point->scale < 0)
    {
        //printf(" %s is scaled \n", io_point->id.c_str());
        bval = !bval;
    }

    if (bval)
    {
        if (io_point->format == Fims_Format::Naked)
        {
            if (io_point->use_bool)
            {
                ss << "true";
            }
            else
            {
                ss << 1;
            }
        }
        else
        {
            if (io_point->use_bool)
            {
                ss << "{\"value\": true }";
            }
            else
            {
                ss << "{\"value\": 1 }";
            }
        }
    }
    else
    {
        if (io_point->format == Fims_Format::Naked)
        {
            if (io_point->use_bool)
            {
                ss << "false";
            }
            else
            {
                ss << 0;
            }
        }
        else
        {
            if (io_point->use_bool)
            {
                ss << "{\"value\": false }";
            }
            else
            {
                ss << "{\"value\": 0 }";
            }
        }
    }
}

// decode_individual_bits
void decode_individual_bits(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg)
{
    decode_individual_bits(value, io_point, ss, cfg, true);
}
/// @brief
/// @param value
/// @param io_point
/// @param ss
/// @param cfg
void decode_individual_bits(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg, bool include_key)
{
    u64 val = getAnyVal(value, (u64)0);
    bool firstOne = true;
    if (!include_key)
    {
        ss << "{";
    }
    for (u8 idx = 0; idx < io_point->bit_str.size(); ++idx)
    {
        if (!(io_point->bits_unknown & (1 << idx)))
        {
            if (firstOne)
            {
                firstOne = false;
            }
            else
            {
                ss << ",";
            }
            bool bval = (bool)((val >> idx) & 1UL) == 1UL; // std::any_cast<bool>(input);
            std::string bstr("false");
            if (bval)
                bstr = "true";

            if (io_point->format == Fims_Format::Clothed)
            {
                ss << addQuote(io_point->bit_str[idx]) << ":"
                   << "{\"value\":" << bstr << "}";
            }
            else
            {
                ss << addQuote(io_point->bit_str[idx]) << ":" << bstr;
            }
        }
    }
    if (!include_key)
    {
        ss << "}";
    }
}

void decode_bval_from_value(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg)
{
    decode_bval_from_value(value, io_point, ss, cfg, true);
}

void decode_bval_from_value(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &cfg, bool include_key)
{
    u64 val = getAnyVal(value, (u64)0);
    auto bval = (val != 0);
    decode_bval(bval, io_point, ss, cfg, include_key);
}

// TODO add ignored bits
void decode_bit_field(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &bss, struct cfg &cfg)
{
    u64 val = getAnyVal(value, (u64)0);
    bool firstOne = true;
    int last_idx = (int)(io_point->bit_str.size());
    bss << "[";
    for (int idx = 0; idx < (int)io_point->bit_str.size(); ++idx)
    {
        // const auto& enum_str = io_point.bit_str[enum_str_idx];
        bool bval = (bool)((val >> idx) & 1UL) == 1UL; // std::any_cast<bool>(input);
        if (io_point->bit_str[idx] == "IGNORE")
            bval = false;
        if (!(io_point->bits_unknown & (1 << idx)))
        // if (!io_point->bit_str_known[idx]) // "known" bits that are not ignored:
        {
            if (bval)
            {
                // if (((val >> idx) & 1UL) == 1UL) {// only format bits that are high:
                if (firstOne)
                {
                    firstOne = false;
                }
                else
                {
                    bss << ",";
                }
                bss << "{\"value\":" << idx << ",\"string\":" << addQuote(io_point->bit_str[idx]) << "}";
            }
        }
        else // "unknown" bits that are "inbetween" bits (and not ignored):
        {
            if (bval)
            { //((val >> idx) & 1UL) == 1UL) {// only format bits that are high:
                if (firstOne)
                {
                    firstOne = false;
                }
                else
                {
                    bss << ",";
                }
                bss << "{\"value\":  " << idx << ",\"string\":" << addQuote("unknown") << "}";
            }
        }
    }
    for (; last_idx < (int)(io_point->size * 16); ++last_idx)
    {
        bool bval = (bool)((val >> last_idx) & 1UL) == 1UL; // std::any_cast<bool>(input);
        if (bval)
        { // only format bits that are high:
            if (firstOne)
            {
                firstOne = false;
            }
            else
            {
                bss << ",";
            }
            bss << "{\"value\":" << last_idx << ",  \"string\":" << addQuote("unknown") << "}";
        }
    }
    bss << "]";
}

/// @brief
/// @param value
/// @param io_point
/// @param bss
/// @param cfg
void decode_enum(std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &bss, struct cfg &cfg)
{
    #ifdef FPS_DEBUG_MODE
    bool debug = cfg.debug_decode;
    #endif
    bool enum_found = false;
    u64 val = getAnyVal(value, (u64)0);
    #ifdef FPS_DEBUG_MODE
    if (debug)
        std::cout << __func__ << " val " << val << " bitstr size " << io_point->bit_str.size() << " bitstr num size " << io_point->bit_str_num.size() << std::endl;
    #endif
    if (io_point->bit_str.size() == io_point->bit_str_num.size())
    {
        for (int idx = 0; idx < (int)(io_point->bit_str.size()); ++idx)
        {
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << __func__ << " idx " << (int)idx << " bit_str_num " << io_point->bit_str_num[idx] << std::endl;
            #endif
            if (val == (u64)io_point->bit_str_num[idx] && io_point->bit_str[idx].compare("") != 0)
            {
                enum_found = true;
                bss << "[{\"value\":" << val << ", \"string\":" << addQuote(io_point->bit_str[idx]) << "}]";
                break;
            }
        }
    }
    if (!enum_found)
    { // config did not account for this value:
        bss << "[{\"value\":" << val << ", \"string\":" << addQuote("unknown") << "}]";
    }
    #ifdef FPS_DEBUG_MODE
    if (debug)
        std::cout << __func__ << " bss " << bss.str() << std::endl;
        #endif
}

/* @brief decode the packed register option
/// @param raw16
/// @param value
/// @param io_point
/// @param bss
/// @param cfg
*/
void decode_packed( std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &bss, struct cfg &cfg)
{
    decode_packed( value, io_point, bss, cfg, true);
}
void decode_packed( std::any &value, std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &bss, struct cfg &cfg, bool include_key)
{
    if (!include_key)
    {
        bss << "{";
    }
    u64 val = getAnyVal(value, (u64)0);
    bool firstOne = true;
    for (auto bitem : io_point->bit_ranges)
    {
        if (firstOne)
        {
            firstOne = false;
        }
        else
        {
            bss << ",";
        }
        std::any bvalue = (val >> bitem->starting_bit_pos & bitem->bit_mask);


        modbus_decode( bitem, bvalue, bss, cfg, true);
    }
    if (!include_key)
    {
        bss << "}";
    }
}

/// @brief modbus decode
////  take the std::any strucure from gcom_decode_any and produce a string from it.

bool modbus_decode(std::shared_ptr<cfg::io_point_struct> io_point, std::any &value, std::stringstream &ss, struct cfg &cfg)
{
    return modbus_decode(io_point, value, ss, cfg, true);
}

/// @param raw16
/// @param io_point
/// @param value
/// @param ss
/// @param cfg
/// @return
bool modbus_decode(std::shared_ptr<cfg::io_point_struct> io_point, std::any &value, std::stringstream &ss, struct cfg &cfg, bool include_key = true)
{
    //std::cout << __func__ << " Point id :" <<io_point->id << std::endl;

    if (io_point->is_bit)
    {
        decode_bval_from_value(value, io_point, ss, cfg, include_key);
    }
    else if (io_point->use_raw)
    {
        u64 val = getAnyVal(value, (u64)0);
        if (include_key)
        {
            ss << addQuote(io_point->id) << ":";
        }
        if (io_point->format == Fims_Format::Clothed)
        {
            ss << "{\"value\":";
            if (io_point->use_hex)
            {
                ss << std::hex << val << std::dec;
            }
            else
            {
                ss << val;
            }
            ss << "}";
        }
        else
        {
            if (io_point->use_hex)
            {
                ss << std::hex << val << std::dec;
            }
            else
            {
                ss << val;
            }
        }
    }
    else if (io_point->is_signed || io_point->scale)
    {
        double val = getAnyVal(value, (double)0);
        if (include_key)
        {
            ss << addQuote(io_point->id) << ":";
        }
        if (io_point->format == Fims_Format::Clothed)
        {
            ss  << "{\"value\":" 
                << std::fixed << std::setprecision(4.7) << val << std::defaultfloat
                << "}";
        }
        else
        {
            ss << std::fixed << std::setprecision(4.7) << val << std::defaultfloat;
        }
    }
    else if (io_point->is_individual_bits)
    {
        std::stringstream bss;
        decode_individual_bits(value, io_point, bss, cfg, include_key);

        ss << bss.str();
    }
    else if (io_point->is_bit_field)
    {
        std::stringstream bss;
        decode_bit_field(value, io_point, bss, cfg);
        if (include_key)
        {
            ss << addQuote(io_point->id) << ":";
        }
        if (io_point->format == Fims_Format::Clothed)
        {
            ss << "{\"value\":" << bss.str() << "}";
        }
        else
        {
            ss << bss.str();
        }
    }
    else if (io_point->packed_register)
    {
        std::stringstream bss;
        decode_packed( value, io_point, bss, cfg, include_key);
        ss << bss.str();
    }
    else if (io_point->is_enum)
    {
        std::stringstream bss;
        decode_enum(value, io_point, bss, cfg);
        if (include_key)
        {
            ss << addQuote(io_point->id) << ":";
        }
        if (io_point->format == Fims_Format::Clothed)
        {
            ss << "{\"value\":" << bss.str() << "}";
        }
        else
        {
            ss << bss.str();
        }
    }
    else if (io_point->is_float)
    {
        double dval = -12.3;
        float fval = -12.3;
        double val;
        if (testAnyVal(value, dval))
        {
            val = getAnyVal(value, dval);
        }
        else if (testAnyVal(value, fval))
        {
            val = getAnyVal(value, fval);
        }
        if (include_key)
        {
            ss << addQuote(io_point->id) << ":";
        }
        if (io_point->format == Fims_Format::Clothed)
        {
            ss << "{\"value\":" << std::fixed << std::setprecision(4.7) << val << std::defaultfloat << "}";
        }
        else
        {
            ss << std::fixed << std::setprecision(4.7) << val << std::defaultfloat;
        }
    }
    else
    {
        if (include_key)
        {
            ss << addQuote(io_point->id) << ":";
        }
        if (io_point->format == Fims_Format::Clothed)
        {
            ss << "{\"value\":" << getAnyVal(value, (u64)4) << "}";
        }
        else
        {
            ss << getAnyVal(value, (u64)4);
        }
    }
    return true;
}

// older stuff possible  deprecated
// functions for decoding and encoding (only for Holding and Input registers):
u64 decode_raw(const u16 *raw_registers, cfg::io_point_struct &io_point, std::any &decode_output)
{
    u64 raw_data = 0UL;

    u64 current_unsigned_val = 0UL;
    s64 current_signed_val = 0;
    f64 current_float_val = 0.0;

    if (io_point.size == 1)
    {
        raw_data = raw_registers[0];

        current_unsigned_val = raw_data;
        if (io_point.uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point.invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point.care_mask;
        }
        // do signed stuff (size 1 cannot be float):
        if (io_point.is_signed)
        {
            s16 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
        }
    }
    else if (io_point.size == 2)
    {
        raw_data = (static_cast<u64>(raw_registers[0]) << 16) +
                   (static_cast<u64>(raw_registers[1]) << 0);

        if (!io_point.is_byte_swap) // normal:
        {
            current_unsigned_val = raw_data;
        }
        else // swap:
        {
            current_unsigned_val = (static_cast<u64>(raw_registers[0]) << 0) +
                                   (static_cast<u64>(raw_registers[1]) << 16);
        }
        if (io_point.uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point.invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point.care_mask;
        }
        // do signed/float stuff:
        if (io_point.is_signed)
        {
            s32 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
        }
        else if (io_point.is_float)
        {
            std::cout << " float size 2 Curr unsigned val " << current_unsigned_val << std::endl;
            f32 to_reinterpret = 0.0f;
            // f32 test_val = 1230.0f;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            // memcpy(&raw_data, &test_val, sizeof(to_reinterpret));
            current_float_val = to_reinterpret;
        }
    }
    else // size of 4:
    {
        raw_data = (static_cast<u64>(raw_registers[0]) << 48) +
                   (static_cast<u64>(raw_registers[1]) << 32) +
                   (static_cast<u64>(raw_registers[2]) << 16) +
                   (static_cast<u64>(raw_registers[3]) << 0);

        if (!io_point.is_byte_swap) // normal:
        {
            current_unsigned_val = raw_data;
        }
        else // swap:
        {
            current_unsigned_val = (static_cast<u64>(raw_registers[0]) << 0) +
                                   (static_cast<u64>(raw_registers[1]) << 16) +
                                   (static_cast<u64>(raw_registers[2]) << 32) +
                                   (static_cast<u64>(raw_registers[3]) << 48);
        }
        if (io_point.word_order > 0)
        {
            current_unsigned_val = (static_cast<u64>(raw_registers[io_point.byte_index[0]]) << 0) +
                                   (static_cast<u64>(raw_registers[io_point.byte_index[1]]) << 16) +
                                   (static_cast<u64>(raw_registers[io_point.byte_index[2]]) << 32) +
                                   (static_cast<u64>(raw_registers[io_point.byte_index[3]]) << 48);
        }

        if (io_point.uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point.invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point.care_mask;
        }
        // do signed/float stuff:
        if (io_point.is_signed) // this is only for whole numbers really (signed but not really float):
        {
            s64 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
        }
        else if (io_point.is_float)
        {
            f64 to_reinterpret = 0.0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_float_val = to_reinterpret;
        }
    }

    // do scaling/shift stuff at the end:
    if (io_point.is_float)
    {
        if (io_point.scale) // scale != 0
        {
            current_float_val /= io_point.scale;
        }
        current_float_val += static_cast<f64>(io_point.shift); // add shift
        decode_output = current_float_val;
    }
    else if (!io_point.is_signed) // unsigned whole numbers:
    {
        if (!io_point.scale) // scale == 0
        {
            current_unsigned_val += io_point.shift; // add shift
            current_unsigned_val >>= io_point.starting_bit_pos;
            decode_output = current_unsigned_val;
        }
        else
        {
            auto scaled = static_cast<f64>(current_unsigned_val) / io_point.scale;
            scaled += static_cast<f64>(io_point.shift); // add shift
            decode_output = scaled;

        }
    }
    else // signed whole numbers:
    {
        if (!io_point.scale) // scale == 0
        {
            current_signed_val += io_point.shift; // add shift
            current_signed_val >>= io_point.starting_bit_pos;
            decode_output = current_signed_val;
        }
        else
        {
            auto scaled = static_cast<f64>(current_signed_val) / io_point.scale;
            scaled += static_cast<f64>(io_point.shift); // add shift
            decode_output = scaled;
        }
    }
    return raw_data;
}

//
// this little chunk of code will decode the registers we get in from the server read into a raw value
// and then into std::any for pub output
//  we may have to do something different for the bits.  Not sure yet.
// TODO pick register
u64 gcom_decode_any(u16 *raw16, u8 *raw8, struct cfg::io_point_struct *io_point, std::any &output, struct cfg &myCfg)
{
    //    static constexpr u64 decode(const u16* raw_registers, const Decode_Info& current_decode, Jval_buif& current_jval) noexcept
    //{
    u64 raw_data = 0UL;

    u64 current_unsigned_val = 0UL;
    s64 current_signed_val = 0;
    f64 current_float_val = 0.0;

    if (io_point->size == 1)
    {
        raw_data = *(u64 *)raw16;

        current_unsigned_val = raw_data;
        if (io_point->uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point->invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point->care_mask;
        }
        // do signed stuff (size 1 cannot be float):
        if (io_point->is_signed)
        {
            s16 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
        }
    }
    else if (io_point->size == 2)
    {
        raw_data = (static_cast<u64>(raw16[0]) << 16) +
                   (static_cast<u64>(raw16[1]) << 0);

        if (!io_point->is_byte_swap) // normal:
        {
            current_unsigned_val = raw_data;
        }
        else // swap:
        {
            current_unsigned_val = (static_cast<u64>(raw16[0]) << 0) +
                                   (static_cast<u64>(raw16[1]) << 16);
        }
        if (io_point->uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point->invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point->care_mask;
        }
        // do signed/float stuff:
        if (io_point->is_signed)
        {
            s32 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
        }
        else if (io_point->is_float)
        {
            f32 to_reinterpret = 0.0f;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_float_val = to_reinterpret;
        }
    }
    else // size of 4:
    {
        raw_data = (static_cast<u64>(raw16[0]) << 48) +
                   (static_cast<u64>(raw16[1]) << 32) +
                   (static_cast<u64>(raw16[2]) << 16) +
                   (static_cast<u64>(raw16[3]) << 0);

        if (!io_point->is_byte_swap) // normal:
        {
            current_unsigned_val = raw_data;
        }
        else // swap:
        {
            current_unsigned_val = (static_cast<u64>(raw16[0]) << 0) +
                                   (static_cast<u64>(raw16[1]) << 16) +
                                   (static_cast<u64>(raw16[2]) << 32) +
                                   (static_cast<u64>(raw16[3]) << 48);
        }
        if (io_point->word_order > 0)
        {
            current_unsigned_val = (static_cast<u64>(raw16[io_point->byte_index[0]]) << 0) +
                                   (static_cast<u64>(raw16[io_point->byte_index[1]]) << 16) +
                                   (static_cast<u64>(raw16[io_point->byte_index[2]]) << 32) +
                                   (static_cast<u64>(raw16[io_point->byte_index[3]]) << 48);
        }

        if (io_point->uses_masks)
        {
            // invert mask stuff:
            current_unsigned_val ^= io_point->invert_mask;
            // care mask stuff:
            current_unsigned_val &= io_point->care_mask;
        }
        // do signed/float stuff:
        if (io_point->is_signed) // this is only for whole numbers really (signed but not really float):
        {
            s64 to_reinterpret = 0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_signed_val = to_reinterpret;
        }
        else if (io_point->is_float)
        {
            f64 to_reinterpret = 0.0;
            memcpy(&to_reinterpret, &current_unsigned_val, sizeof(to_reinterpret));
            current_float_val = to_reinterpret;
        }
    }

    // do scaling/shift stuff at the end:
    if (io_point->is_float)
    {
        if (io_point->scale) // scale != 0
        {
            current_float_val /= io_point->scale;
        }
        current_float_val += static_cast<f64>(io_point->shift); // add shift
        output = current_float_val;
    }
    else if (!io_point->is_signed) // unsigned whole numbers:
    {
        if (!io_point->scale) // scale == 0
        {
            current_unsigned_val += io_point->shift; // add shift
            current_unsigned_val >>= io_point->starting_bit_pos;
            output = current_unsigned_val;
        }
        else
        {
            auto scaled = static_cast<f64>(current_unsigned_val) / io_point->scale;
            scaled += static_cast<f64>(io_point->shift); // add shift
            output = scaled;
        }
    }
    else // signed whole numbers:
    {
        if (!io_point->scale) // scale == 0
        {
            current_signed_val += io_point->shift; // add shift
            current_signed_val >>= io_point->starting_bit_pos;
            output = current_signed_val;

        }
        else
        {
            auto scaled = static_cast<f64>(current_signed_val) / io_point->scale;
            scaled += static_cast<f64>(io_point->shift); // add shift
            output = scaled;
        }
    }
    // TODO float64
    return raw_data;
}

// deprecated we use string stream now
// u64 gcom_decode_any(u16* raw16, u8*raw8, struct cfg::io_point_struct* io_point, std::any& output, struct cfg& myCfg)
// void decode_to_string(u16* regs16, u8* regs8, struct cfg::io_point_struct& io_point, std::any input, fmt::memory_buffer& buf, struct cfg& myCfg)
void decode_to_string(u16 *regs16, u8 *regs8, struct cfg::io_point_struct &io_point, fmt::memory_buffer &buf, struct cfg &myCfg)
//  const Jval_buif& to_stringify,
//  fmt::memory_buffer& buf, u8 bit_idx = Bit_All_Idx,
//  Bit_Strings_Info_Array* bit_str_array = nullptr,
//  String_Storage* str_storage = nullptr)
{
    //    int key = 1234;
    //    int bit_str_idx = 21;
    //    std::string bit_str = "hello kitty";
    //    fmt::format_to(std::back_inserter(buf), "{}: ", key);
    //    fmt::format_to(std::back_inserter(buf),
    //    R"({{"value":{},"string":"{}"}},)",
    //                                                bit_str_idx,
    //                                               bit_str);

    // questions how do the bits arrive
    // if (io_point->reg->u8_buf
    // poll_work->errors = modbus_read_bits(thread->ctx, poll_work->offset, poll_work->num_registers, poll_work->u8_buff);

    // first we have to add the flags and work out how to decode the bit_strings and the enum strings
    if (io_point.register_type == cfg::Register_Types::Coil || io_point.register_type == cfg::Register_Types::Discrete_Input)
    {
        bool val = (regs8[0] != 0);
        // what about io_point inverted
        if (val)
        {

            fmt::format_to(std::back_inserter(buf), "{}: ", "true");
            // FORMAT_TO_BUF(buf, "{}", val);
        }
        else
        {
            fmt::format_to(std::back_inserter(buf), "{}: ", "false");
        }
    }
    else // Holding and Input:
    {
        if (!io_point.is_bit_string_type) // normal formatting:
        {
            // we may get these values from the register
            // auto val = std::any_cast<bool>(input);
            bool val = (regs16[0] != 0);
            if (val)
            {

                fmt::format_to(std::back_inserter(buf), "{}: ", "true");
            }
            else
            {
                fmt::format_to(std::back_inserter(buf), "{}: ", "false");
            }
        }
        else // format using "bit_strings":
        {
            u8 last_idx = 0;
            // to do allow this to span multiple registers as per size
            auto curr_unsigned_val = regs16[0];
            if (io_point.is_individual_bits) // resolve to multiple uris (true/false per bit):
            {
                // if (bit_idx == Bit_All_Idx) // they want the whole thing
                // {
                //     // POSSIBLE TODO(WALKER): The raw id pub might or might not need to include the binary string as a part of its pub (maybe wrap it in an object?)
                //     FORMAT_TO_BUF(buf, "{}", curr_unsigned_val); // this is the whole individual_bits as a raw number (after basic decoding)
                // }
                // else // they want the individual true/false bit (that belongs to this bit_idx)
                // {
                auto val = ((curr_unsigned_val >> io_point.bit_index) & 1UL) == 1UL; // std::any_cast<bool>(input);
                fmt::format_to(std::back_inserter(buf), "{}: ", val);
                //    FORMAT_TO_BUF(buf, "{}", ); // this is the individual bit itself (true/false)
                // }
            }
            else if (io_point.is_bit_field) // resolve to array of strings for each bit/bits that is/are == the expected value (1UL for individual bits):
            {
                buf.push_back('[');
                for (u8 bit_str_idx = 0; bit_str_idx < io_point.bit_str.size(); ++bit_str_idx)
                {
                    const auto &bit_str = io_point.bit_str[bit_str_idx];
                    last_idx = io_point.bit_str.size() + 1;
                    // I think we'll preload the unknown bit strings
                    if (!io_point.bit_str_known[bit_str_idx]) // "known" bits that are not ignored:
                    {
                        if (((curr_unsigned_val >> bit_str_idx) & 1UL) == 1UL) // only format bits that are high:
                        {
                            // FORMAT_TO_BUF(buf,
                            fmt::format_to(std::back_inserter(buf),
                                           R"({{"value":{},"string":"{}"}},)",
                                           bit_str_idx,
                                           bit_str);
                        }
                    }
                    else // "unknown" bits that are "inbetween" bits (and not ignored):
                    {
                        if (((curr_unsigned_val >> bit_str_idx) & 1UL) == 1UL) // only format bits that are high:
                        {
                            // FORMAT_TO_BUF(buf,
                            fmt::format_to(std::back_inserter(buf),
                                           R"({{"value":{},"string":"Unknown"}},)",
                                           bit_str_idx);
                        }
                    }
                }
                //     // all the other bits that we haven't masked out during the initial decode step that are high we print out as "Unknown" (all at the end of the array):
                for (; last_idx < (io_point.size * 16); ++last_idx)
                {
                    if (((curr_unsigned_val >> last_idx) & 1UL) == 1UL) // only format bits that are high:
                    {
                        // FORMAT_TO_BUF(buf,
                        fmt::format_to(std::back_inserter(buf),
                                       R"({{"value":{},"string":"Unknown"}},)", last_idx);
                    }
                }
                buf.resize(buf.size() - 1); // this gets rid of the last excessive ',' character
                buf.push_back(']');         // finish the array
            }
            else if (io_point.is_enum) // resolve to single string based on current input value (unsigned whole numbers):
            {
                bool enum_found = false;
                for (u8 enum_str_idx = 0; enum_str_idx < io_point.bit_str.size(); ++enum_str_idx)
                {
                    // const auto& enum_str = io_point.bit_str[enum_str_idx];
                    if (curr_unsigned_val == io_point.bit_str_num[enum_str_idx])
                    {
                        enum_found = true;
                        // FORMAT_TO_BUF(buf,
                        fmt::format_to(std::back_inserter(buf),
                                       R"([{{"value":{},"string":"{}"}}])", curr_unsigned_val, io_point.bit_str[enum_str_idx]);
                        break;
                    }
                }
                if (!enum_found) // config did not account for this value:
                {
                    // FORMAT_TO_BUF(buf,
                    fmt::format_to(std::back_inserter(buf),
                                   R"([{{"value":{},"string":"Unknown"}}])", curr_unsigned_val);
                }
            }
        }
    }
}


u64 get_io_point_forced_val(cfg::io_point_struct* io_point)
{
    std::unique_lock<std::mutex> lock(io_point->mtx); 
    return io_point->forced_val;
}
u64 get_io_point_raw_val(cfg::io_point_struct* io_point)
{
    std::unique_lock<std::mutex> lock(io_point->mtx); 
    return io_point->raw_val;
}

u8 get_io_point_bit_val(cfg::io_point_struct* io_point)
{
    std::unique_lock<std::mutex> lock(io_point->mtx); 
    return io_point->reg8[0];
}

double get_io_point_float_val(cfg::io_point_struct* io_point, u64 raw_val)
{
    double float_val;
    if (io_point->scale != 0)
        float_val= float(raw_val) / io_point->scale;
    else
        float_val= float(raw_val);
    float_val -= io_point->shift;        

    return float_val;
}

void set_io_point_float_val(cfg::io_point_struct* io_point, double val)
{
    std::unique_lock<std::mutex> lock(io_point->mtx); 
    io_point->float_val = val;
}

void get_io_point_full(std::shared_ptr<cfg::io_point_struct> io_point, std::stringstream &ss, struct cfg &myCfg)
{

    u8 reg8 ;//= get_io_point_bit_val(io_point.get());
    u64 forced_val; //= get_io_point_forced_val(io_point.get());
    u64 raw_val; //= get_io_point_raw_val(io_point.get());
    std::string username;
    std::string process_name;
    double tUpdate;
    double tlastUpdate;
    {
        std::unique_lock<std::mutex> lock(io_point->mtx); 
        forced_val = io_point->forced_val;
        raw_val = io_point->raw_val;
        reg8 = io_point->reg8[0];
        username = io_point->username;
        process_name = io_point->process_name;
        tUpdate = io_point->tUpdate;
        tlastUpdate = io_point->tlastUpdate;

    }

    double float_val = get_io_point_float_val(io_point.get(), raw_val);
    ss << "{";
    ss << "\"name\": \"" << io_point->name << "\", ";
    ss << "\"id\": \"" << io_point->id << "\", ";
    ss << "\"raw_val\": " << raw_val << ", ";
    ss << "\"device_id\": \"" << io_point->device_id << "\", ";
    ss << "\"offset\": " << io_point->offset << ", ";
    ss << "\"size\": " << io_point->size << ", ";
    ss << "\"enabled\": " << (io_point->is_enabled ? "true" : "false") << ", ";
    ss << "\"process_name\": \"" << process_name << "\", ";
    ss << "\"username\": \"" << username << "\", ";

    //ss << "\"forced\": " << (io_point->is_forced ? "true" : "false") << ", ";
    ss << "\"is_forced\": " << (io_point->is_forced ? "true" : "false") << ", ";
    ss << "\"forced_val\": " << forced_val << ", ";
    ss << "\"float_val\": " << float_val << ", ";

    ss << "\"update_time\": " << tUpdate << ", ";
    ss << "\"last_update_time\": " << tlastUpdate << ", ";

    //ss << "\"enabled\": " << (io_point->is_enabled ? "true" : "false") << ", ";
    ss << "\"register_type\": \"" << io_point->register_type_str << "\", ";
    if ((io_point->register_type == cfg::Register_Types::Input) || (io_point->register_type == cfg::Register_Types::Holding))
    {
        ss << "\"value\": ";
        modbus_decode(io_point, io_point->value, ss, myCfg, false);
        ss << ", ";
        ss << "\"raw_hex_val\": \"" << hex_to_str((u16*)(&raw_val), io_point->size) << "\", ";
        ss << "\"last_value\": ";
        modbus_decode(io_point, io_point->last_value, ss, myCfg, false);
        ss << ", ";
        ss << "\"set_value\": ";
        modbus_decode(io_point, io_point->set_value, ss, myCfg, false);
        ss << ", ";
        // if (io_point)
        //     ss << "\"raw_hex\": \"" << hex_to_str(io_point->reg16) << "\", ";
    }
    else if ((io_point->register_type == cfg::Register_Types::Discrete_Input) || (io_point->register_type == cfg::Register_Types::Coil))
    {
        ss << "\"value\": ";
        decode_bval(reg8, io_point, ss, myCfg, false);
        ss << ", ";
        // ss << "\"last_value\": ";
        // decode_bval(reg8, io_point, ss, myCfg, false);
        // ss << ", ";
        // ss << "\"set_value\": ";
        // decode_bval(reg8, io_point, ss, myCfg, false);
        // ss << ", ";
        // if (io_point)
        ss << "\"raw_hex\": \"" << hex_to_str(reg8) << "\", ";
    }

    ss << "\"starting_bit_pos\": " << io_point->starting_bit_pos << ", ";
    ss << "\"number_of_bits\": " << io_point->number_of_bits << ", ";
    ss << "\"bit_mask\": \"" << hex_to_str(io_point->bit_mask) << "\", ";
    ss << "\"shift\": " << io_point->shift << ", ";
    ss << "\"scale\": " << io_point->scale << ", ";
    ss << "\"normal_set\": " << (io_point->normal_set ? "true" : "false") << ", ";
    ss << "\"invert_mask\": \"" << hex_to_str(io_point->invert_mask) << "\", ";
    ss << "\"care_mask\": \"" << hex_to_str(io_point->care_mask) << "\", ";
    ss << "\"uses_mask\": " << (io_point->uses_masks ? "true" : "false") << ", ";

    ss << "\"is_float\": " << (io_point->is_float ? "true" : "false") << ", ";
    ss << "\"is_signed\": " << (io_point->is_signed ? "true" : "false") << ", ";
    ss << "\"is_byte_swap\": " << (io_point->is_byte_swap ? "true" : "false") << ", ";
    ss << "\"word_order\": " << io_point->word_order << ", ";
    ss << "\"is_bit_string_type\": " << (io_point->is_bit_string_type ? "true" : "false") << ", ";
    ss << "\"is_individual_bits\": " << (io_point->is_individual_bits ? "true" : "false") << ", ";
    ss << "\"is_bit_field\": " << (io_point->is_bit_field ? "true" : "false") << ", ";
    ss << "\"packed_register\": " << (io_point->packed_register ? "true" : "false") << ", ";
    if (io_point->is_bit)
    {
        ss << "\"bit_index\": " << io_point->bit_index << ", ";
    }
    ss << "\"bits_known\": " << io_point->bits_known << ", ";
    ss << "\"bits_unknown\": " << io_point->bits_unknown << ", ";
    ss << "\"bit_strings\": [";
    bool is_first = true;
    for (const std::string &bit_str : io_point->bit_str)
    {
        if (is_first)
        {
            ss << "\"" << bit_str << "\"";
            is_first = false;
        }
        else
        {
            ss << ", \"" << bit_str << "\"";
        }
    }

    ss << "], ";

    ss << "\"bit_str_known\": [";
    is_first = true;
    for (bool known : io_point->bit_str_known)
    {
        if (is_first)
        {
            ss << known;
            is_first = false;
        }
        else
        {
            ss << ", " << known;
        }
    }
    ss << "], ";

    ss << "\"bit_str_num\": [";
    is_first = true;
    for (int num : io_point->bit_str_num)
    {
        if (is_first)
        {
            ss << num;
            is_first = false;
        }
        else
        {
            ss << ", " << num;
        }
    }
    ss << "], ";

    // ss << "\"bit_ranges\": [";
    // for (const std::shared_ptr<cfg::io_point_struct> &range : io_point->bit_ranges)
    // {
    //     ss << "\"<pointer to bit_range>\", "; // You can replace this with the actual content if needed
    // }

    // ss << "], ";

    ss << "\"is_enum\": " << (io_point->is_enum ? "true" : "false") << ", ";
    ss << "\"is_random_enum\": " << (io_point->is_random_enum ? "true" : "false") << ", ";
    ss << "\"use_debounce\": " << (io_point->use_debounce ? "true" : "false") << ", ";
    ss << "\"use_deadband\": " << (io_point->use_deadband ? "true" : "false") << ", ";
    //ss << "\"last_float_val\": " << io_point->last_float_val << ", ";
    //ss << "\"last_raw_val\": " << io_point->last_raw_val << ", ";
    ss << "\"use_bool\": " << (io_point->use_bool ? "true" : "false") << ", ";
    ss << "\"use_hex\": " << (io_point->use_hex ? "true" : "false") << ", ";
    ss << "\"use_raw\": " << (io_point->use_raw ? "true" : "false") << ", ";
    ss << "\"is_bit\": " << (io_point->is_bit ? "true" : "false") << ", ";
    ss << "\"is_watchdog\": " << (io_point->is_watchdog ? "true" : "false") << ", ";
    // ss << "\"watchdog_name\": \"" << io_point->watchdog_name << "\", ";
    ss << "\"multi_write_op_code\": " << (io_point->multi_write_op_code ? "true" : "false");
    ss << "}";
}

// duplicate of write_work in io_threads but that's OK
//io_work->tRun = tRun;
void store_raw_data(std::shared_ptr<IO_Work> io_work, bool debug)
{
    if ((io_work->register_type == cfg::Register_Types::Discrete_Input) || (io_work->register_type == cfg::Register_Types::Coil))
    {
        for (auto io_point : io_work->io_points)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx); 

                io_point->data_error = io_work->data_error;
                io_point->error = io_work->errors;
                io_point->errno_code = io_work->errno_code;
                if(! io_work->data_error)
                {
                    io_point->last_value = io_point->raw_val;                 
                    io_point->tlastUpdate = io_point->tUpdate;                 
                    io_point->tUpdate = io_work->tDone;
                }
            }
            auto index = io_point->offset - io_work->offset;
            {
                std::unique_lock<std::mutex> lock(io_point->mtx); 
                // TODO lock it
                memcpy(&io_point->reg8, &io_work->buf8[index], 1);
                io_point->raw_val = (u64)io_work->buf8[index];
            }
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " io_point [" << io_point->id << "] raw value [" << io_point->raw_val << "]" << std::endl;
        #endif
        }
    }
    else if ((io_work->register_type == cfg::Register_Types::Input) || (io_work->register_type == cfg::Register_Types::Holding))
    {
        for (auto io_point : io_work->io_points)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx); 

                io_point->data_error = io_work->data_error;
                io_point->error = io_work->errors;
                io_point->errno_code = io_work->errno_code;
                if(! io_work->data_error)
                {
                    io_point->last_value = io_point->raw_val;                 
                    io_point->tlastUpdate = io_point->tUpdate;                 
                    io_point->tUpdate = io_work->tDone;
                }
            }

            auto index = io_point->offset - io_work->offset;
            {
                std::unique_lock<std::mutex> lock(io_point->mtx); 
                // TODO lock it
                memcpy(&io_point->reg16, &io_work->buf16[index], io_point->size * 2);
                if (io_point->size == 1)
                {
                    io_point->raw_val = (u64)io_work->buf16[index];
                }
                else if (io_point->size == 2)
                {
                    u16 uval[2];
                    if (io_point->is_byte_swap)
                    {
                        uval[0] = io_work->buf16[index];
                        uval[1] = io_work->buf16[index + 1];
                    }
                    else
                    {
                        uval[1] = io_work->buf16[index];
                        uval[0] = io_work->buf16[index + 1];
                    }
                    u32 *uuval = (u32 *)uval;

                    io_point->raw_val = (u64)(*uuval);
                }
                else if (io_point->size == 4)
                {
                    // u64 uval;
                    u16 uval[4];
                    // ->bval is decoded in config_any

                    uval[io_point->byte_index[0]] = io_work->buf16[index + 3];
                    uval[io_point->byte_index[1]] = io_work->buf16[index + 2];
                    uval[io_point->byte_index[2]] = io_work->buf16[index + 1];
                    uval[io_point->byte_index[3]] = io_work->buf16[index + 0];

                    u64 *uuval = (u64 *)uval;

                    io_point->raw_val = (u64)(*uuval);
                }
            }
            // done with locks
            #ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << " io_point [" << io_point->id << "] raw value [" << io_point->raw_val << "]" << std::endl;
                #endif
        }
    }
}

void extract_raw_data(std::shared_ptr<IO_Work> io_work, bool debug)
{
    if ((io_work->register_type == cfg::Register_Types::Discrete_Input) || (io_work->register_type == cfg::Register_Types::Coil))
    {
        for (auto io_point : io_work->io_points)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx);
                auto index = io_point->offset - io_work->offset;
                memcpy(&io_work->buf8[index], &io_point->reg8, 1);
            }
        }
    }
    else if ((io_work->register_type == cfg::Register_Types::Input) || (io_work->register_type == cfg::Register_Types::Holding))
    {
        for (auto io_point : io_work->io_points)
        {
            {
                std::unique_lock<std::mutex> lock(io_point->mtx); 
                auto index = io_point->offset - io_work->offset;
                memcpy(&io_work->buf16[index], &io_point->reg16, io_point->size * 2);
            }
        }
    }
}
