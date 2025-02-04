/* =======================================================================
 * @Component			OMAPCONF
 * @Filename			i2cbusses.c
 * @Description			Print the installed i2c busses for both 2.4
 *				and 2.6 kernels.
 * @Copyright			GPL
 *======================================================================== */
/*
    i2cbusses: Print the installed i2c busses for both 2.4 and 2.6 kernels.
               Part of user-space programs to access for I2C
               devices.
    Copyright (c) 1999-2003  Frodo Looijaard <frodol@dds.nl> and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2008-2010  Jean Delvare <khali@linux-fr.org>
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

/* For strdup and snprintf */
#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1

#include "i2cbusses.h"
#include "i2c-dev.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   /* for strcasecmp() */
#include <sys/param.h> /* for NAME_MAX */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum adt
{
    adt_dummy,
    adt_isa,
    adt_i2c,
    adt_smbus,
    adt_unknown
};

struct adap_type
{
    const char* funcs;
    const char* algo;
};

static struct adap_type adap_types[5] = {
    {
        "dummy",
        "Dummy bus",
    },
    {
        "isa",
        "ISA bus",
    },
    {
        "i2c",
        "I2C adapter",
    },
    {
        "smbus",
        "SMBus adapter",
    },
    {
        "unknown",
        "N/A",
    },
};

static enum adt i2c_get_funcs(int i2cbus)
{
    unsigned long funcs;
    int file;
    char filename[20];
    enum adt ret;

    file = open_i2c_dev(i2cbus, filename, sizeof(filename), 1);
    if (file < 0)
    {
        return adt_unknown;
    }

    if (ioctl(file, I2C_FUNCS, &funcs) < 0)
    {
        ret = adt_unknown;
    }
    else if ((funcs & I2C_FUNC_I2C) != 0u)
    {
        ret = adt_i2c;
    }
    else if ((funcs & (I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA)) != 0u)
    {
        ret = adt_smbus;
    }
    else
    {
        ret = adt_dummy;
    }

    close(file);
    return ret;
}

/* Remove trailing spaces from a string
   Return the new string length including the trailing NUL */
static int rtrim(char* s)
{
    int i;

    for (i = strlen(s) - 1; i >= 0 && (s[i] == ' ' || s[i] == '\n'); i--)
    {
        s[i] = '\0';
    }
    return i + 2;
}

void free_adapters(struct i2c_adap* adapters)
{
    int i;

    for (i = 0; adapters[i].name != nullptr; i++)
    {
        free(adapters[i].name);
    }
    free(adapters);
}

/* We allocate space for the adapters in bunches. The last item is a
   terminator, so here we start with room for 7 adapters, which should
   be enough in most cases. If not, we allocate more later as needed. */
#define BUNCH 8

/* n must match the size of adapters at calling time */
static struct i2c_adap* more_adapters(struct i2c_adap* adapters, int n)
{
    struct i2c_adap* new_adapters;

    new_adapters = static_cast<i2c_adap*>(realloc(adapters, (n + BUNCH) * sizeof(struct i2c_adap)));
    if (new_adapters == nullptr)
    {
        free_adapters(adapters);
        return nullptr;
    }
    memset(new_adapters + n, 0, BUNCH * sizeof(struct i2c_adap));

    return new_adapters;
}

struct i2c_adap* gather_i2c_busses(void)
{
    char s[120];
    struct dirent *de, *dde;
    DIR *dir, *ddir;
    FILE* f;
    char fstype[NAME_MAX], sysfs[NAME_MAX + 1], n[NAME_MAX * 4];
    int foundsysfs = 0;
    int count = 0;
    struct i2c_adap* adapters;

    adapters = static_cast<i2c_adap*>(calloc(BUNCH, sizeof(struct i2c_adap)));
    if (adapters == nullptr)
    {
        return nullptr;
    }

    /* look in /proc/bus/i2c */
    if ((f = fopen("/proc/bus/i2c", "r")) != nullptr)
    {
        while (fgets(s, 120, f) != nullptr)
        {
            char *algo, *name, *type, *all;
            int len_algo, len_name, len_type;
            int i2cbus;

            algo = strrchr(s, '\t');
            *(algo++) = '\0';
            len_algo = rtrim(algo);

            name = strrchr(s, '\t');
            *(name++) = '\0';
            len_name = rtrim(name);

            type = strrchr(s, '\t');
            *(type++) = '\0';
            len_type = rtrim(type);

            sscanf(s, "i2c-%d", &i2cbus);

            if ((count + 1) % BUNCH == 0)
            {
                /* We need more space */
                adapters = more_adapters(adapters, count + 1);
                if (adapters == nullptr)
                {
                    return nullptr;
                }
            }

            all = static_cast<char*>(malloc(len_name + len_type + len_algo));
            if (all == nullptr)
            {
                free_adapters(adapters);
                return nullptr;
            }
            adapters[count].nr = i2cbus;
            adapters[count].name = strcpy(all, name);
            adapters[count].funcs = strcpy(all + len_name, type);
            adapters[count].algo = strcpy(all + len_name + len_type, algo);
            count++;
        }
        fclose(f);
        goto done;
    }

    /* look in sysfs */
    /* First figure out where sysfs was mounted */
    if ((f = fopen("/proc/mounts", "r")) == nullptr)
    {
        goto done;
    }
    while (fgets(n, NAME_MAX, f) != nullptr)
    {
        sscanf(n, "%*[^ ] %[^ ] %[^ ] %*s\n", sysfs, fstype);
        if (strcasecmp(fstype, "sysfs") == 0)
        {
            foundsysfs++;
            break;
        }
    }
    fclose(f);
    if (foundsysfs == 0)
    {
        goto done;
    }

    /* Bus numbers in i2c-adapter don't necessarily match those in
       i2c-dev and what we really care about are the i2c-dev numbers.
       Unfortunately the names are harder to get in i2c-dev */
    strcat(sysfs, "/class/i2c-dev");
    if ((dir = opendir(sysfs)) == nullptr)
    {
        goto done;
    }
    /* go through the busses */
    while ((de = readdir(dir)) != nullptr)
    {
        if (strcmp(de->d_name, ".") == 0)
        {
            continue;
        }
        if (strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        /* this should work for kernels 2.6.5 or higher and */
        /* is preferred because is unambiguous */
        sprintf(n, "%s/%s/name", sysfs, de->d_name);
        f = fopen(n, "r");
        /* this seems to work for ISA */
        if (f == nullptr)
        {
            sprintf(n, "%s/%s/device/name", sysfs, de->d_name);
            f = fopen(n, "r");
        }
        /* non-ISA is much harder */
        /* and this won't find the correct bus name if a driver
           has more than one bus */
        if (f == nullptr)
        {
            sprintf(n, "%s/%s/device", sysfs, de->d_name);
            if ((ddir = opendir(n)) == nullptr)
            {
                continue;
            }
            while ((dde = readdir(ddir)) != nullptr)
            {
                if (strcmp(dde->d_name, ".") == 0)
                {
                    continue;
                }
                if (strcmp(dde->d_name, "..") == 0)
                {
                    continue;
                }
                if ((strncmp(dde->d_name, "i2c-", 4) == 0))
                {
                    sprintf(n, "%s/%s/device/%s/name", sysfs, de->d_name, dde->d_name);
                    if ((f = fopen(n, "r")) != nullptr)
                    {
                        goto found;
                    }
                }
            }
        }

    found:
        if (f != nullptr)
        {
            int i2cbus;
            enum adt type;
            char* px;

            px = fgets(s, 120, f);
            fclose(f);
            if (px == nullptr)
            {
                fprintf(stderr, "%s: read error\n", n);
                continue;
            }
            if ((px = strchr(s, '\n')) != nullptr)
            {
                *px = 0;
            }
            if (sscanf(de->d_name, "i2c-%d", &i2cbus) == 0)
            {
                continue;
            }
            if (strncmp(s, "ISA ", 4) == 0)
            {
                type = adt_isa;
            }
            else
            {
                /* Attempt to probe for adapter capabilities */
                type = i2c_get_funcs(i2cbus);
            }

            if ((count + 1) % BUNCH == 0)
            {
                /* We need more space */
                adapters = more_adapters(adapters, count + 1);
                if (adapters == nullptr)
                {
                    return nullptr;
                }
            }

            adapters[count].nr = i2cbus;
            adapters[count].name = strdup(s);
            if (adapters[count].name == nullptr)
            {
                free_adapters(adapters);
                return nullptr;
            }
            adapters[count].funcs = adap_types[type].funcs;
            adapters[count].algo = adap_types[type].algo;
            count++;
        }
    }
    closedir(dir);

done:
    return adapters;
}

static int lookup_i2c_bus_by_name(const char* bus_name)
{
    struct i2c_adap* adapters;
    int i, i2cbus = -1;

    adapters = gather_i2c_busses();
    if (adapters == nullptr)
    {
        fprintf(stderr, "Error: Out of memory!\n");
        return -3;
    }

    /* Walk the list of i2c busses, looking for the one with the
       right name */
    for (i = 0; adapters[i].name != nullptr; i++)
    {
        if (strcmp(adapters[i].name, bus_name) == 0)
        {
            if (i2cbus >= 0)
            {
                fprintf(stderr, "Error: I2C bus name is not unique!\n");
                i2cbus = -4;
                goto done;
            }
            i2cbus = adapters[i].nr;
        }
    }

    if (i2cbus == -1)
    {
        fprintf(stderr,
                "Error: I2C bus name doesn't match any "
                "bus present!\n");
    }

done:
    free_adapters(adapters);
    return i2cbus;
}

/*
 * Parse an I2CBUS command line argument and return the corresponding
 * bus number, or a negative value if the bus is invalid.
 */
int lookup_i2c_bus(const char* i2cbus_arg)
{
    unsigned long i2cbus;
    char* end;

    i2cbus = strtoul(i2cbus_arg, &end, 0);
    if ((*end != 0) || (*i2cbus_arg == 0))
    {
        /* Not a number, maybe a name? */
        return lookup_i2c_bus_by_name(i2cbus_arg);
    }
    if (i2cbus > 0xFFFFF)
    {
        fprintf(stderr, "Error: I2C bus out of range!\n");
        return -2;
    }

    return i2cbus;
}

/*
 * Parse a CHIP-ADDRESS command line argument and return the corresponding
 * chip address, or a negative value if the address is invalid.
 */
int parse_i2c_address(const char* address_arg)
{
    long address;
    char* end;

    address = strtol(address_arg, &end, 0);
    if ((*end != 0) || (*address_arg == 0))
    {
        fprintf(stderr, "Error: Chip address is not a number!\n");
        return -1;
    }
    if (address < 0x03 || address > 0x77)
    {
        fprintf(stderr,
                "Error: Chip address out of range "
                "(0x03-0x77)!\n");
        return -2;
    }

    return address;
}

int open_i2c_dev(int i2cbus, char* filename, size_t size, int quiet)
{
    int file;

    snprintf(filename, size, "/dev/i2c/%d", i2cbus);
    filename[size - 1] = '\0';
    file = open(filename, O_RDWR);

    if (file < 0 && (errno == ENOENT || errno == ENOTDIR))
    {
        sprintf(filename, "/dev/i2c-%d", i2cbus);
        file = open(filename, O_RDWR);
    }

    if (file < 0 && (quiet == 0))
    {
        if (errno == ENOENT)
        {
            fprintf(stderr,
                    "Error: Could not open file "
                    "`/dev/i2c-%d' or `/dev/i2c/%d': %s\n",
                    i2cbus, i2cbus, strerror(ENOENT));
        }
        else
        {
            fprintf(stderr,
                    "Error: Could not open file "
                    "`%s': %s\n",
                    filename, strerror(errno));
            if (errno == EACCES)
            {
                fprintf(stderr, "Run as root?\n");
            }
        }
    }

    return file;
}

int set_slave_addr(int file, int address, int force)
{
    /* With force, let the user read from/write to the registers
       even when a driver is also running */
    if (ioctl(file, force != 0 ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0)
    {
        fprintf(stderr, "Error: Could not set address to 0x%02x: %s\n", address, strerror(errno));
        return -errno;
    }

    return 0;
}