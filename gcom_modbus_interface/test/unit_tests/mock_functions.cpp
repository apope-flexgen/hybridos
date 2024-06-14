#include <cstdint>
#include "mock_modbus.h"

// Global variables
int modbus_flush_ret;
char* modbus_strerror_ret;
int modbus_read_registers_ret;
int modbus_write_register_ret;
int modbus_write_registers_ret;
int modbus_read_input_registers_ret;
int modbus_read_bits_ret;
int modbus_write_bit_ret;
int modbus_write_bits_ret;
int modbus_connect_ret;
int modbus_close_ret;
int modbus_set_slave_ret;
int modbus_set_response_timeout_ret;
int modbus_set_byte_timeout_ret;
int modbus_set_bits_from_byte_ret;
int modbus_get_byte_from_bits_ret;
int modbus_set_bits_from_bytes_ret;
int modbus_read_input_bits_ret;
int modbus_write_and_read_registers_ret;
int modbus_set_error_recovery_ret;
int modbus_receive_ret;
int modbus_get_socket_ret;
int modbus_send_raw_request_ret;
modbus_t* modbus_new_rtu_ret;
modbus_t* modbus_new_tcp_ret;
int modbus_free_ret;
// Function definitions
void modbus_flush(modbus_t* ctx)
{
    return;
}
char* modbus_strerror(int errno_code)
{
    return modbus_strerror_ret;
}
int modbus_read_registers(modbus_t* ctx, int addr, int nb, uint16_t* dest)
{
    return modbus_read_registers_ret;
}
int modbus_write_register(modbus_t* ctx, int addr, int value)
{
    return modbus_write_register_ret;
}
int modbus_write_registers(modbus_t* ctx, int addr, int nb, const uint16_t* src)
{
    return modbus_write_registers_ret;
}
int modbus_read_input_registers(modbus_t* ctx, int addr, int nb, uint16_t* dest)
{
    return modbus_read_input_registers_ret;
}
int modbus_read_bits(modbus_t* ctx, int addr, int nb, uint8_t* dest)
{
    return modbus_read_bits_ret;
}
int modbus_write_bit(modbus_t* ctx, int coil_addr, int status)
{
    return modbus_write_bit_ret;
}
int modbus_write_bits(modbus_t* ctx, int addr, int nb, const uint8_t* src)
{
    return modbus_write_bits_ret;
}
int modbus_connect(modbus_t* ctx)
{
    return modbus_connect_ret;
}
int modbus_close(modbus_t* ctx)
{
    return modbus_close_ret;
}
int modbus_set_slave(modbus_t* ctx, int slave)
{
    return modbus_set_slave_ret;
}
int modbus_set_response_timeout(modbus_t* ctx, int sec, int usec)
{
    return modbus_set_response_timeout_ret;
}
int modbus_set_byte_timeout(modbus_t* ctx, int sec, int usec)
{
    return modbus_set_byte_timeout_ret;
}
int modbus_set_bits_from_byte(uint8_t* dest, int idx, const uint8_t value)
{
    return modbus_set_bits_from_byte_ret;
}
int modbus_get_byte_from_bits(const uint8_t* src, int idx)
{
    return modbus_get_byte_from_bits_ret;
}
int modbus_set_bits_from_bytes(uint8_t* dest, int nb_bits, const uint8_t* tab_byte)
{
    return modbus_set_bits_from_bytes_ret;
}
int modbus_read_input_bits(modbus_t* ctx, int addr, int nb, uint8_t* dest)
{
    return modbus_read_input_bits_ret;
}
int modbus_write_and_read_registers(modbus_t* ctx, int write_addr, int nb_write, const uint16_t* src, int read_addr,
                                    int nb_read, uint16_t* dest)
{
    return modbus_write_and_read_registers_ret;
}
int modbus_set_error_recovery(modbus_t* ctx, int error_recovery)
{
    return modbus_set_error_recovery_ret;
}
int modbus_receive(modbus_t* ctx, uint8_t* rsp)
{
    return modbus_receive_ret;
}
int modbus_get_socket(modbus_t* ctx)
{
    return modbus_get_socket_ret;
}
int modbus_send_raw_request(modbus_t* ctx, uint8_t* req, int req_length)
{
    return modbus_send_raw_request_ret;
}
modbus_t* modbus_new_rtu(const char* device, int baud, char parity, int data_bit, int stop_bit)
{
    return modbus_new_rtu_ret;
}
modbus_t* modbus_new_tcp(const char* ip, int port)
{
    return modbus_new_tcp_ret;
}
int modbus_free(modbus_t* ctx)
{
    if (ctx != nullptr)
        delete ctx;
    return modbus_free_ret;
}
