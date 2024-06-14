#ifndef _MOCK_MODBUS_H
#define _MOCK_MODBUS_H

#include <cstdint>

#define MODBUS_ENOBASE 112345678
/* Protocol exceptions */
enum
{
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
    MODBUS_EXCEPTION_ACKNOWLEDGE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
    MODBUS_EXCEPTION_MEMORY_PARITY,
    MODBUS_EXCEPTION_NOT_DEFINED,
    MODBUS_EXCEPTION_GATEWAY_PATH,
    MODBUS_EXCEPTION_GATEWAY_TARGET,
    MODBUS_EXCEPTION_MAX
};

#define EMBXILFUN (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_FUNCTION)
#define EMBXILADD (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS)
#define EMBXILVAL (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE)
#define EMBXSFAIL (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE)
#define EMBXACK (MODBUS_ENOBASE + MODBUS_EXCEPTION_ACKNOWLEDGE)
#define EMBXSBUSY (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY)
#define EMBXNACK (MODBUS_ENOBASE + MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE)
#define EMBXMEMPAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_MEMORY_PARITY)
#define EMBXGPATH (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_PATH)
#define EMBXGTAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_TARGET)

/* Native libmodbus error codes */
#define EMBBADCRC (EMBXGTAR + 1)
#define EMBBADDATA (EMBXGTAR + 2)
#define EMBBADEXC (EMBXGTAR + 3)
#define EMBUNKEXC (EMBXGTAR + 4)
#define EMBMDATA (EMBXGTAR + 5)
#define EMBBADSLAVE (EMBXGTAR + 6)

typedef enum
{
    MODBUS_ERROR_RECOVERY_NONE = 0,
    MODBUS_ERROR_RECOVERY_LINK = (1 << 1),
    MODBUS_ERROR_RECOVERY_PROTOCOL = (1 << 2)
} modbus_error_recovery_mode;

struct _modbus_t
{
    bool connected = false;
};

typedef struct _modbus_t modbus_t;

void modbus_flush(modbus_t* ctx);
char* modbus_strerror(int errno_code);
int modbus_read_registers(modbus_t* ctx, int addr, int nb, uint16_t* dest);
int modbus_write_register(modbus_t* ctx, int addr, int value);
int modbus_write_registers(modbus_t* ctx, int addr, int nb, const uint16_t* src);
int modbus_read_input_registers(modbus_t* ctx, int addr, int nb, uint16_t* dest);
int modbus_read_bits(modbus_t* ctx, int addr, int nb, uint8_t* dest);
int modbus_write_bit(modbus_t* ctx, int coil_addr, int status);
int modbus_write_bits(modbus_t* ctx, int addr, int nb, const uint8_t* src);
int modbus_connect(modbus_t* ctx);
int modbus_close(modbus_t* ctx);
int modbus_set_slave(modbus_t* ctx, int slave);
int modbus_set_response_timeout(modbus_t* ctx, int sec, int usec);
int modbus_set_byte_timeout(modbus_t* ctx, int sec, int usec);
int modbus_set_bits_from_byte(uint8_t* dest, int idx, const uint8_t value);
int modbus_get_byte_from_bits(const uint8_t* src, int idx);
int modbus_set_bits_from_bytes(uint8_t* dest, int nb_bits, const uint8_t* tab_byte);
int modbus_read_input_bits(modbus_t* ctx, int addr, int nb, uint8_t* dest);
int modbus_write_and_read_registers(modbus_t* ctx, int write_addr, int nb_write, const uint16_t* src, int read_addr,
                                    int nb_read, uint16_t* dest);
int modbus_set_error_recovery(modbus_t* ctx, int error_recovery);
int modbus_receive(modbus_t* ctx, uint8_t* rsp);
int modbus_get_socket(modbus_t* ctx);
int modbus_send_raw_request(modbus_t* ctx, uint8_t* req, int req_length);
modbus_t* modbus_new_rtu(const char* device, int baud, char parity, int data_bit, int stop_bit);
modbus_t* modbus_new_tcp(const char* ip, int port);
int modbus_free(modbus_t* ctx);
#endif