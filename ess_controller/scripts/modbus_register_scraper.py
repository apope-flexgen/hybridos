from pymodbus.client.sync import ModbusTcpClient
import pymodbus
from pymodbus.constants import Endian
from pymodbus.payload import BinaryPayloadDecoder
from pymodbus.client.sync import ModbusTcpClient

from optparse import OptionParser

# --------------------------------------------------------------------------- #
# Define some constants
# --------------------------------------------------------------------------- #
COUNT = 1    # The number of bits/registers to read at once
DELAY = 0    # The delay between subsequent reads
SLAVE = 0x01  # The slave unit id to read from


# function based off of the validator found in the below post:
# source: https://stackoverflow.com/questions/53010239/pymodbus-read-holding-registers
def decode_single_register(result):
    if not result.isError():
        '''.isError() implemented in pymodbus 1.4.0 and above.'''
        decoder = BinaryPayloadDecoder.fromRegisters(
            result.registers,
            byteorder=Endian.Big, wordorder=Endian.Little
        )
        print("u_int", decoder.decode_16bit_uint())
        # these functions flush the buffer once it decodes it (cannot call decode more than once)
        # return (("Binary:", bin(decoder.decode_16bit_uint())), ("u_int:", decoder.decode_16bit_uint()),
        #         ("int:", decoder.decode_16bit_int()), ("float:", decoder.decode_16bit_float()))
    else:
        # Error handling.
        print("This is NOT a valid register, Try again.")
        return None


def determine_if_coil(client, register_id):
    try:
        result = client.read_coils(
            address=register_id, count=1, unit=SLAVE)
        return (type(result) != pymodbus.pdu.ExceptionResponse)
    except Exception as ex:
        print("error in determine_if_coil:", ex)
        return False


def determine_if_input_register(client, register_id):
    try:
        result = client.read_input_registers(
            address=register_id, count=1, unit=SLAVE)
        return (type(result) != pymodbus.pdu.ExceptionResponse)
    except Exception as ex:
        print("error in determine_if_input_register:", ex)
        return False


def determine_if_holding_register(client, register_id):
    try:
        result = client.read_holding_registers(
            address=register_id, count=1, unit=SLAVE)
        return (type(result) != pymodbus.pdu.ExceptionResponse)
    except Exception as ex:
        print("error in determine_if_holding_register:", ex)
        return False


def determine_if_discrete_input(client, register_id):
    try:
        result = client.read_discrete_inputs(
            address=register_id, count=1, unit=SLAVE)
        return (type(result) != pymodbus.pdu.ExceptionResponse)
    except Exception as ex:
        print("error in determine_if_discrete_input:", ex)
        return False


def get_single_register_types(client, register_id):
    try:
        return [
            ("Register_id = ", register_id, hex(register_id)),
            ("Is coil?",
             determine_if_coil(client, register_id)),
            ("Is holding register?",
             determine_if_holding_register(client, register_id)),
            ("Is input register?",
             determine_if_input_register(client, register_id)),
            ("Is discerete input?",
             determine_if_discrete_input(client, register_id))
        ]
    except Exception as ex:
        print("error in get_single_register_types:", ex)


def make_server_register_map(options, client):
    try:
        query = [int(p) for p in options.range.split(':')]
        with open(options.output, 'w') as f:
            for x in range(query[0], query[1] + 1): # + 1 makes range inclusive.
                print("Reading register:", x, "hex =", hex(x))
                register_types = get_single_register_types(client, x)
                print(register_types, file=f)
    except Exception as ex:
        print("error in make_server_register_map:", ex)


def get_options():
    """ A helper method to parse the command line options

    :returns: The options manager
    """
    parser = OptionParser()

    parser.add_option("-o", "--output",
                      help="The resulting output file for the scrape",
                      dest="output", default="modbus_server_register_map.txt")

    parser.add_option("-p", "--port",
                      help="The port to connect to", type='int',
                      dest="port", default=502)

    parser.add_option("-s", "--server",
                      help="The server to scrape",
                      dest="server", default="127.0.0.1")

    parser.add_option("-r", "--range",
                      help="The register range to scan. Default = 0:65535",
                      dest="range", default="0:65535")  # 65535 (or 2^16 possible addresses) is the maximum register address that can be read on modbus.

    opt = parser.parse_args()[0]
    return opt


def main():
    try:
        options = get_options()
        print(options)

        client = ModbusTcpClient(options.server, options.port)
        client.connect()

        # for reading a particular register (outputs if it has changed since last read only)
        # previousValue = 0
        # currentValue = 0
        # decoder = BinaryPayloadDecoder
        # while (True):
        #     result = client.read_holding_registers( # change this read type for your register type that you want read.
        #     address=768, count=1, unit=SLAVE)
        #     currentValue = decoder.fromRegisters(result.registers,
        #         byteorder=Endian.Big, wordorder=Endian.Little).decode_16bit_uint()
        #     if currentValue != previousValue:
        #         print (currentValue)
        #         previousValue = currentValue

        # for reading register types into an output file
        make_server_register_map(options, client)

        client.close()
    except Exception as ex:
        print("error in main:", ex)


if __name__ == "__main__":
    main()
