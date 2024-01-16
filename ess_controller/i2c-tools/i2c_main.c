/* ======================================================================= 
 * @Component			OMAPCONF
 * @Filename			i2cmain.c
 * @Description			A user-space program to read an I2C register.
 * @Copyright			GPL
 *======================================================================== */
/*
    i2cget.c - A user-space program to read an I2C register.
    Copyright (C) 2005-2010  Jean Delvare <khali@linux-fr.org>
    Based on i2cset.c:
    Copyright (C) 2001-2003  Frodo Looijaard <frodol@dds.nl>, and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2004-2005  Jean Delvare <khali@linux-fr.org>
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <i2c-dev.h>
#include <i2cbusses.h>
#include <util.h>
#include <poll.h>
#include <version.h>

#define I2C_BUS "4"
#define SLAVE_ADDRESS "0x26"
#define RUN_FREQ   1
#define PUB_FREQ    500


static void help(void) __attribute__ ((noreturn));
int lookup_i2c_bus(const char * c);
int parse_i2c_address(const char *c);
int open_i2c_dev(int i2cbus, char *filename, size_t size, int quiet);
int set_slave_addr(int file, int address, int force);

static void help(void)
{
	fprintf(stderr,
		"Usage: i2cget [-f] [-y] I2CBUS CHIP-ADDRESS [DATA-ADDRESS [MODE]]\n"
		"  I2CBUS is an integer or an I2C bus name\n"
		"  ADDRESS is an integer (0x03 - 0x77)\n"
		"  MODE is one of:\n"
		"    b (read byte data, default)\n"
		"    w (read word data)\n"
		"    c (write byte/read byte)\n"
		"    Append p for SMBus PEC\n");
	exit(1);
}

int running = 1;
// just runs forever reading gpio registers
int main(int argc, char *argv[])
{
    int big_tick = 0;
    int tick = 0;
    int i2cbus = lookup_i2c_bus(I2C_BUS);
	if (i2cbus < 0)
    {
        printf("%s>> failed to get i2c_bus [%s]\n",__func__, I2C_BUS);
		help();
    }
    int address = parse_i2c_address(SLAVE_ADDRESS);
	if (address < 0)
    {
        printf("%s>> failed to get i2c_address [%s]\n",__func__, SLAVE_ADDRESS);
		help();
    }
    int quiet = 1;
    char fname[20];
    int i2cdev = open_i2c_dev(i2cbus, fname, sizeof(fname), quiet);
	if (i2cdev < 0)
    {
        printf("%s>> failed to get i2c_dev [%s]\n",__func__, fname);
		help();
    }
    int force = 0;
    int saddr = set_slave_addr(i2cdev, address, force);
	if (saddr < 0)
    {
        force = 1;
        saddr = set_slave_addr(i2cdev, address, force);
    }
	if (saddr < 0)
    {
        printf("%s>> failed to set  saddr [%s] force %d\n",__func__, SLAVE_ADDRESS, force);
		help();
    }

    printf(" i2cbus %d address %d i2cdev %d fname [%s] saddr %d force %d\n"
                , i2cbus, address, i2cdev, fname
                , saddr
                , force
                );
    int daddress = 0;
    unsigned int res = 0;
    unsigned int lastres = 0;
    int pub_freq = 0;
    while (running)
    {
        res = i2c_smbus_read_byte_data(i2cdev, daddress);
        pub_freq++;
        if((res!=lastres)|| (pub_freq >PUB_FREQ)) 
        {
            if(pub_freq >PUB_FREQ)
            {
                pub_freq = 0;
            }

            printf(" Tick %d res ox%02x\n", tick++, res);
            char *tmp;
            asprintf(&tmp,"/usr/local/bin/fims/fims_send -m pub -u /assets/gpio '{\"input_data\":%d}'",res);
            system(tmp);
            free(tmp);
            lastres = res;
        }
        poll(nullptr,0,RUN_FREQ);
    }

    return 0;
}