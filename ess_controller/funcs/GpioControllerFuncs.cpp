/*
 * Gpio Controller functions , include i2c setup
 */

#include <bitset>  // library for decoding the string from the gpio
#include <i2c-dev.h>
#include <i2cbusses.h>

#include "asset.h"
#include "scheduler.h"
#include "varMapUtils.h"

// TODO after MVP put config details into an assetVar

#define I2C_BUS "4"
#define SLAVE_ADDRESS "0x26"

extern "C" {
int lookup_i2c_bus(const char* c);
int parse_i2c_address(const char* c);
int open_i2c_dev(int i2cbus, char* filename, size_t size, int quiet);
int set_slave_addr(int file, int address, int force);
}
extern "C++" {
void help(bool);  //) __attribute__ ((noreturn));
int CheckGPIOForEmergencySignal(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int GpioSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int GpioEvery5mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int Gpio100mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int GpioPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void SendPub(varsmap& vmap, asset_manager* am, const char* uri, const char* puri, assetVar* aV);
int SetupGpioSched(scheduler* sched, asset_manager* am);
int HandleSchedLoad(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}
// static
void help(bool exitflag)
{
    FPS_ERROR_PRINT(
        "Usage: i2cget [-f] [-y] I2CBUS CHIP-ADDRESS [DATA-ADDRESS [MODE]]\n"
        "  I2CBUS is an integer or an I2C bus name\n"
        "  ADDRESS is an integer (0x03 - 0x77)\n"
        "  MODE is one of:\n"
        "    b (read byte data, default)\n"
        "    w (read word data)\n"
        "    c (write byte/read byte)\n"
        "    Append p for SMBus PEC\n");
    if (exitflag)
        exit(1);
}

// i2c globals:
int i2cbus;
int address;
int quiet = 1;
char fname[20];
int i2cdev;
int force = 0;
int saddr;
std::bitset<8> gpio_bitfield{ 0 };  // 8 bits are needed for decoding the gpio input.

int SetupGpioSched(scheduler* sched, asset_manager* am)
{
    UNUSED(sched);
    const char* aname = am->name.c_str();

    am->vm->setFunc(*am->vmap, aname, "GpioEvery5mS", (void*)&GpioEvery5mS);
    am->vm->setFunc(*am->vmap, aname, "GpioSystemInit", (void*)&GpioSystemInit);
    am->vm->setFunc(*am->vmap, aname, "GpioPub", (void*)&GpioPub);
    am->vm->setFunc(*am->vmap, aname, "HandleSchedLoad", (void*)&HandleSchedLoad);

    return 0;
}

int GpioEvery5mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    return CheckGPIOForEmergencySignal(vmap, amap, aname, p_fims, aV);
}

int CheckGPIOForEmergencySignal(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if (0)
        FPS_ERROR_PRINT("%s >> checking aV [%s] am %p \n", __func__, aV->name.c_str(), (void*)aV->am);
    // FPS_ERROR_PRINT("%s >> checking aV [%s] am %p \n",
    // __func__, aV->am->name.c_str() , aV->am);
    if (!aV->am)
    {
        FPS_ERROR_PRINT("%s >>  aV [%s:%s] No am found %p \n", __func__, aV->comp.c_str(), aV->name.c_str(),
                        (void*)aV->am);
        return 0;
    }
    // essPerf ePerf(aV->am, aname, "checkGPIOForEmergencySignalLog");

    int reload = 0;
    double dval = 0.0;
    bool bval = false;
    int ival = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

    assetVar* gpioAv = amap[__func__];
    if (!gpioAv || (reload = gpioAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }
    if (0)
        FPS_ERROR_PRINT("%s >> checking aV [%s] am %p reload %d \n", __func__, aV->am->name.c_str(), (void*)aV->am,
                        reload);

    if (reload < 2)
    {
        FPS_ERROR_PRINT("%s >> checking aV [%s] am %p reload %d \n", __func__, aV->am->name.c_str(), (void*)aV->am,
                        reload);
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);

        // TODO after MVP get these GPIO names from config file

        for (auto x : vmap["/components/gpio"])
        {
            assetVar* av = x.second;
            if (av)
            {
                FPS_ERROR_PRINT("%s >> found gpio [%s] pin [%d]\n", __func__, av->name.c_str(), av->getiParam("pin"));
            }
        }

        linkVals(*vm, vmap, amap, aname, "/components", bval, "EStop", "Disconnect Switch", "Door Latch",
                 "Surge Arrester", "Fire Alarm", "Fuse Monitoring");

        linkVals(*vm, vmap, amap, aname, "/components", dval, "GPIORefTime");

        linkVals(*vm, vmap, amap, aname, "/status", ival, "GPIObits");

        linkVals(*vm, vmap, amap, aname, "/config", ival, "GPIOsim");

        if (reload < 1)  // complete restart
        {
            gpioAv = amap.at(__func__);
            amap.at("EStop")->setVal(false);
            amap.at("Disconnect Switch")->setVal(false);
            amap.at("Door Latch")->setVal(false);
            amap.at("Surge Arrester")->setVal(false);
            amap.at("Fire Alarm")->setVal(false);
            amap.at("Fuse Monitoring")->setVal(false);
            amap.at("GPIORefTime")->setVal(dval);
            ival = 0;
            amap.at("GPIOsim")->setVal(ival);
        }
        reload = 2;
        gpioAv->setVal(reload);
    }

    // These are here so we don't "log(n)" search through the map again over and
    // over when using the assetVars. Silly, but it works.
    assetVar* EStop = amap.at("EStop");
    assetVar* Disconnect_Switch = amap.at("Disconnect Switch");
    assetVar* Door_Latch = amap.at("Door Latch");
    assetVar* Surge_Arrester = amap.at("Surge Arrester");
    assetVar* Fire_Alarm = amap.at("Fire Alarm");
    assetVar* Fuse_Monitoring = amap.at("Fuse Monitoring");
    assetVar* GPIORefTime = amap.at("GPIORefTime");

    int daddress = 0;

    unsigned int res = 0;
    if (amap.at("GPIOsim")->getiVal())
    {
        res = amap.at("GPIObits")->getiVal();
    }
    else
    {
        res = i2c_smbus_read_byte_data(i2cdev, daddress);
    }

    varsmap* vlist = vm->createVlist();  // for sending assetVars out through
                                         // fims_server/modbus_client

    // runs multiple commands based on which switches are on:
    // NOTE: Remove " ~ " - from res if res is all zeroes by default, only here to
    // zero out
    gpio_bitfield = ~res;

    GPIORefTime->setVal(vm->get_time_ref());
    vm->addVlist(vlist, GPIORefTime);
    int countflag = 0;

    // TODO after MVP navigate through al the config GPIO items and get pin
    // numbers from them

    // exclusive ors between the current gpio bit and the "inverted" status, just
    // change the config
    for (std::size_t i = 0; i < gpio_bitfield.size(); ++i)
    {
        // EStop:

        if (i == 0 && EStop->getbParam("enabled"))
        {
            if ((gpio_bitfield[i] ^ EStop->getbParam("inverted")))
            {
                EStop->setVal(true);
                vm->addVlist(vlist, EStop);
                countflag++;
            }  // continuously sends when on.
            else if (!(gpio_bitfield[i] ^ EStop->getbParam("inverted")) && EStop->valueChangedReset())
            {
                EStop->setVal(false);
                vm->addVlist(vlist, EStop);
                countflag++;
            }  // Reset EStop
        }
        if (i == 1 && Disconnect_Switch->getbParam("enabled"))
        {
            // Disconnect Switch:
            if ((gpio_bitfield[i] ^ Disconnect_Switch->getbParam("inverted")))
            {
                Disconnect_Switch->setVal(true);
                vm->addVlist(vlist, Disconnect_Switch);
                countflag++;
            }  // Continuously sends when on.
            else if (!(gpio_bitfield[i] ^ Disconnect_Switch->getbParam("inverted")) &&
                     Disconnect_Switch->valueChangedReset())
            {
                Disconnect_Switch->setVal(false);
                vm->addVlist(vlist, Disconnect_Switch);
                countflag++;
            }  // Reset Disconnect Switch
        }
        if (i == 2 && Door_Latch->getbParam("enabled"))
        {
            // Door Latch:
            if ((gpio_bitfield[i] ^ Door_Latch->getbParam("inverted")))
            {
                Door_Latch->setVal(true);
                vm->addVlist(vlist, Door_Latch);
                countflag++;
            }  // Continuously sends when on.
            else if (!(gpio_bitfield[i] ^ Door_Latch->getbParam("inverted")) && Door_Latch->valueChangedReset())
            {
                Door_Latch->setVal(false);
                vm->addVlist(vlist, Door_Latch);
                countflag++;
            }  // Reset Door Latch
        }
        if (i == 3 && Surge_Arrester->getbParam("enabled"))
        {
            // Surge Arrrester:
            if ((gpio_bitfield[i] ^ Surge_Arrester->getbParam("inverted")))
            {
                Surge_Arrester->setVal(true);
                vm->addVlist(vlist, Surge_Arrester);
                countflag++;
            }  // Continuously sends when on.
            else if (!(gpio_bitfield[i] ^ Surge_Arrester->getbParam("inverted")) && Surge_Arrester->valueChangedReset())
            {
                Surge_Arrester->setVal(false);
                vm->addVlist(vlist, Surge_Arrester);
                countflag++;
            }  // Reset Surge Arrester
        }
        // Fire Alarm:
        if (i == 4 && Fire_Alarm->getbParam("enabled"))
        {
            if ((gpio_bitfield[i] ^ Fire_Alarm->getbParam("inverted")))
            {
                Fire_Alarm->setVal(true);
                vm->addVlist(vlist, Fire_Alarm);
                countflag++;
            }  // Continuously sends when on.
            else if (!(gpio_bitfield[i] ^ Fire_Alarm->getbParam("inverted")) && Fire_Alarm->valueChangedReset())
            {
                Fire_Alarm->setVal(false);
                vm->addVlist(vlist, Fire_Alarm);
                countflag++;
            }  // Reset Fire Alarm
        }
        // Fuse Monitoring:
        if (i == 5 && Fuse_Monitoring->getbParam("enabled"))
        {
            if ((gpio_bitfield[i] ^ Fuse_Monitoring->getbParam("inverted")))
            {
                Fuse_Monitoring->setVal(true);
                vm->addVlist(vlist, Fuse_Monitoring);
                countflag++;
            }  // Continuously sends when on.
            else if (!(gpio_bitfield[i] ^ Fuse_Monitoring->getbParam("inverted")) &&
                     Fuse_Monitoring->valueChangedReset())
            {
                Fuse_Monitoring->setVal(false);
                vm->addVlist(vlist, Fuse_Monitoring);
                countflag++;
            }  // Reset Fuse Monitoring
        }
        // else if (i == 6 && gpio_bitfield[i]) {  } // Unused
        // else if (i == 7 && gpio_bitfield[i]) {  } // Unused
    }

    if (countflag > 0)
    {
        vm->sendVlist(p_fims, "pub", vlist);
    }
    // else
    // {
    //     FPS_ERROR_PRINT("%s >> No messages enabled \n",
    //     __func__);
    // }
    vm->clearVlist(vlist);

    return 0;
}

// HMM
int GpioSystemInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    if (1)
        FPS_ERROR_PRINT(">>>>>>>>SCHED >>>>%s running for GPIO Manager\n", __func__);
    asset_manager* am = aV->am;

    am->vm->setTime();

    bool bval = false;
    int ival = 0;

    am->amap["RunPub"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunPub", bval);
    am->amap["RunWake5"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunWake5", bval);
    am->amap["RunWake1000"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunWake1000", bval);
    am->amap["RunPub"]->setVal(true);
    am->amap["RunWake5"]->setVal(true);
    am->amap["RunWake1000"]->setVal(true);
    ival = 100;
    am->amap["PubTime"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "PubTime", ival);

    if (am->vm->argc != 2)
        system("sudo modprobe i2c_dev");  // make sure to start the program with sudo
                                          // as well.
    i2cbus = lookup_i2c_bus(I2C_BUS);
    bool cont = true;
    if (i2cbus < 0)
    {
        FPS_ERROR_PRINT("%s>> failed to get i2c_bus [%s]\n", __func__, I2C_BUS);
        cont = false;
        help(false);
    }
    if (cont)
    {
        address = parse_i2c_address(SLAVE_ADDRESS);
        if (address < 0)
        {
            FPS_ERROR_PRINT("%s>> failed to get i2c_address [%s]\n", __func__, SLAVE_ADDRESS);
            help(false);
            cont = false;
        }
    }
    if (cont)
    {
        i2cdev = open_i2c_dev(i2cbus, fname, sizeof(fname), quiet);
        if (i2cdev < 0)
        {
            FPS_ERROR_PRINT("%s>> failed to get i2c_dev [%s] with a value of : [%d]\n", __func__, fname, i2cdev);
            help(false);
            cont = false;
        }
    }
    if (cont)
    {
        saddr = set_slave_addr(i2cdev, address, force);
        if (saddr < 0)
        {
            force = 1;
            saddr = set_slave_addr(i2cdev, address, force);
        }
        if (saddr < 0)
        {
            FPS_ERROR_PRINT("%s>> failed to set  saddr [%s] force %d\n", __func__, SLAVE_ADDRESS, force);
            help(false);
            cont = false;
        }
    }
    if (cont)
    {
        if (1)
            FPS_ERROR_PRINT(" i2cbus %d address %d i2cdev %d fname [%s] saddr %d force %d\n", i2cbus, address, i2cdev,
                            fname, saddr, force);
    }
    return 0;
}

int Gpio100mS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(aV);
    return 0;
}

int GpioPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = aV->am;
    // if( am->amap["RunPub"]->getbVal())
    // {
    // for (auto sv : *am->syscVec)
    // {
    std::string auri = "/components/gpio";
    if (0)
        FPS_ERROR_PRINT("%s  sysVec -> [%s]\n", __func__, auri.c_str());
    SendPub(vmap, am, auri.c_str(), auri.c_str(), aV);

    // }
    // }
    return 0;
}

//#endif
