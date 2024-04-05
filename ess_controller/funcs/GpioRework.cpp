/*
 * Gpio Controller functions , include i2c setup
 */
#include "asset.h"
#include "scheduler.h"
#include "varMapUtils.h"
#include <bitset>  // library for decoding the string from the gpio
#include <i2c-dev.h>
#include <i2cbusses.h>
#define I2C_BUS "4"
#define SLAVE_ADDRESS "0x26"

extern "C" {
int lookup_i2c_bus(const char* c);
int parse_i2c_address(const char* c);
int open_i2c_dev(int i2cbus, char* filename, size_t size, int quiet);
int set_slave_addr(int file, int address, int force);
}
extern "C++" {
void rwhelp(bool);  //) __attribute__ ((noreturn));
int GpioInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int GpioScan(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int GpioRwPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void SendPub(varsmap& vmap, asset_manager* am, const char* uri, const char* puri, assetVar* aV);
int HandleSchedLoad(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SchedRunFunc(varsmap& vmap, const char* sname, const char* func, double runAfter, double refTime, double repTime,
                 assetVar* aV);
int SetupRwGpioSched(scheduler* sched, asset_manager* am);
int GpioOut(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}
//  // this should be it....
// //                        schedId        func,     initial delay, priority ,
// repeat time, assetVar
// //SchedRunFunction(vmap, "GpioPubId",    GpioPub,  2.0,           0.01,
// 0.1,        aV);
// static
void rwhelp(bool exitflag)
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
int SetupRwGpioSched(scheduler* sched, asset_manager* am)
{
    UNUSED(sched);
    const char* aname = am->name.c_str();
    if (1)
        FPS_ERROR_PRINT("%s >> Running  [%s] am %p \n", __func__, am->name.c_str(), (void*)am);
    am->vm->setFunc(*am->vmap, aname, "GpioScan", (void*)&GpioScan);
    am->vm->setFunc(*am->vmap, aname, "GpioOut", (void*)&GpioOut);
    am->vm->setFunc(*am->vmap, aname, "GpioInit", (void*)&GpioInit);
    am->vm->setFunc(*am->vmap, aname, "GpioRwPub", (void*)&GpioRwPub);
    am->vm->setFunc(*am->vmap, aname, "HandleSchedLoad", (void*)&HandleSchedLoad);
    am->vm->setFunc(*am->vmap, aname, "SchedRunFunc", (void*)&SchedRunFunc);
    return 0;
}

// Notes Gpio outputs
// set register 3 bit n to 0 to create an output
// write value to register 1 for all bits
// b
// a set /components/gpio output will
//          a// check the config register (3) has been set correctly
//          b/ set the correct bit in the output map
//          c/ set the output.
// the Av we send here  is /components/gpios/Some_name
// it will have a dir param set to output and a bit
int GpioOut(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    essPerf ePerf(aV->am, aname, "GpioOutLog");
    int reload = 0;
    std::bitset<8> gpio_bitout{ 0xff };  // 8 bits are needed for gpio config.
    if (1)
        FPS_ERROR_PRINT("%s >> checking aV [%s] am %p \n", __func__, aV->name.c_str(), (void*)aV->am);
    if (!aV->am)
    {
        FPS_ERROR_PRINT("%s >>  aV [%s:%s] No am found %p \n", __func__, aV->comp.c_str(), aV->name.c_str(),
                        (void*)aV->am);
        return 0;
    }
    asset_manager* am = aV->am;
    // piggy back off GpioScan /config/gpio/gpios
    assetVar* gpioAv = amap["GpioScan"];
    if (!gpioAv || (reload = gpioAv->getiVal()) == 0)
    {
        assetVar* gaV = am->vm->getVar(vmap, "/config/gpio", "gpios");
        GpioScan(vmap, amap, aname, p_fims, gaV);
        gpioAv = amap["GpioScan"];
    }
    // The input config stuff would have set / cleared the bit in the  output
    // register...
    // remap uri /status/gpio/GPIOOut[<bit>]
    // func GpioOut amap gpio

    int cfgaddress = 3;
    int outaddress = 1;
    int cfg = 0;
    int i2cdev = -1;
    int out = amap["GPIOout"]->getiVal();
    gpio_bitout = out;
    bool bval = aV->getbVal();
    // watch out for inverted...

    if (bval)
    {
        gpio_bitout.set(aV->getiParam("bit"));
        // out |= 1<< (aV->getiParam("bit"));
    }
    else
    {
        // out &= ~(1<< (aV->getiParam("bit")));
        gpio_bitout.reset(aV->getiParam("bit"));
    }
    out = (int)gpio_bitout.to_ulong();
    FPS_ERROR_PRINT("%s >> GPIO #1  aV [%s] setting bval [%s] out 0x%04x \n", __func__, aV->fname,
                    bval ? "true" : "false", out);
    if (amap["gpios"]->gotParam("i2cdevint"))
    {
        FPS_ERROR_PRINT("%s >> GPIO #2  aV [%s] setting bval [%s] out 0x%04x \n", __func__, aV->fname,
                        bval ? "true" : "false", out);
        i2cdev = amap["gpios"]->getiParam("i2cdevint");
        int cfgVal = amap.at("GPIOcfg")->getiVal();  // amap["gpios"]->getiParam("i2cdevint");
        cfg = i2c_smbus_read_byte_data(i2cdev, cfgaddress);
        if ((i2cdev > -1) && (cfg != cfgVal))
        {
            i2c_smbus_write_byte_data(i2cdev, cfgaddress, cfgVal);
        }
        FPS_ERROR_PRINT(
            "%s >> GPIO #3  aV [%s] setting bval [%s] i2cdev %d  "
            "cfgVal 0x%04x cfg 0x%04x \n",
            __func__, aV->fname, bval ? "true" : "false", i2cdev, cfgVal, cfg);
        i2c_smbus_write_byte_data(i2cdev, outaddress, out);
        // if(0) std::cout <<  cfg;
    }
    return 0;
}
// uses a /config/gpio var to hold all the gpio details.
// still need components/gpio to define pins
// "/config/gpio":{
//  "gpios" {
//        "i2c_bus":"4",
//        "slave_address":"0x26"
//         }
//
//
//}
int GpioScan(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
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
    essPerf ePerf(aV->am, aname, "GpioScanLog");
    int reload = 0;
    double dval = 0.0;
    bool bval = false;
    int ival = 0;
    int cfgVal = 0xFF;
    std::bitset<8> gpio_bitfield{ 0 };   // 8 bits are needed for decoding the gpio input.
    std::bitset<8> gpio_bitres{ 0 };     // 8 bits are needed for decoding the gpio input.
    std::bitset<8> gpio_bitcfg{ 0xff };  // 8 bits are needed for gpio config.
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    int daddress = 0;
    int i2cdev = -1;
    int cfgaddress = 3;
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
        if (aV->gotParam("i2cdevint"))
        {
            i2cdev = aV->getiParam("i2cdevint");
        }
        for (auto x : vmap["/components/gpio_out"])
        {
            assetVar* av = x.second;
            if (av)
            {
                FPS_ERROR_PRINT("%s >> found gpio [%s] pin [%d]\n", __func__, av->name.c_str(), av->getiParam("pin"));
                // linkVals(*vm, vmap, amap, aname, "/components", bval,
                // av->name.c_str()); if(av->gotParam("dir"))
                // {
                //     if(strcmp(av->getcParam("dir"), "output")== 0)
                //     {
                //         if(av->gotParam("bit"))
                //         {
                int bit = av->getiParam("bit");  // clear bit in cfgVal
                gpio_bitcfg.reset(bit);
                //         }
                //     }
                // }
            }
        }
        cfgVal = (int)gpio_bitcfg.to_ulong();
        // end config to i2cdev rgister 3
        if (i2cdev > -1)
        {
            i2c_smbus_write_byte_data(i2cdev, cfgaddress, cfgVal);
        }
        linkVals(*vm, vmap, amap, aname, "/components", dval, "GPIORefTime");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "GPIObits");
        linkVals(*vm, vmap, amap, aname, "/config", ival,
                 "GPIOcfg");  // output config
        linkVals(*vm, vmap, amap, aname, "/status", ival,
                 "GPIOout");  // output values
        linkVals(*vm, vmap, amap, aname, "/config", ival, "GPIOsim");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "GPIOres");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "gpios");
        if (reload < 1)  // complete restart
        {
            GpioInit(vmap, amap, aname, p_fims, aV);
            gpioAv = amap.at(__func__);
            amap.at("GPIORefTime")->setVal(dval);
            ival = 0;
            amap.at("GPIOsim")->setVal(ival);
            amap.at("GPIOres")->setVal(ival);
            amap.at("GPIOout")->setVal(ival);
            // default value all inputs
            amap.at("GPIOcfg")->setVal(cfgVal);
            for (auto x : vmap["/components/gpio"])
            {
                assetVar* av = x.second;
                if (av)
                {
                    if (av->getbParam("enabled"))
                    {
                        av->setVal(!av->getbParam("inverted"));
                    }
                    else
                    {
                        bval = false;
                        av->setVal(bval);
                    }
                }
            }
        }
        reload = 2;
        gpioAv->setVal(reload);
    }
    assetVar* GPIORefTime = amap.at("GPIORefTime");
    // int daddress = 0;
    // int i2cdev = -1;
    uint32_t oldres = amap.at("GPIObits")->getiVal();
    uint32_t res = oldres;
    if (amap.at("GPIOsim")->getiVal())
    {
        res = amap.at("GPIOres")->getiVal();
    }
    else
    {
        if (aV->gotParam("i2cdevint"))
        {
            i2cdev = aV->getiParam("i2cdevint");
            amap["gpios"]->setParam("i2cdevint", i2cdev);
            res = i2c_smbus_read_byte_data(i2cdev, daddress);
        }
        if (0)
            FPS_ERROR_PRINT("%s >> checking aV [%s] res  %d  oldres %d i2cdev %d \n", __func__, aV->getfName(), res,
                            oldres, i2cdev);
    }
    // if(res != oldres)
    {
        gpio_bitfield = res ^ oldres;
        gpio_bitres = res;
        int bits = res ^ oldres;
        if (1 && bits != 0)
            FPS_ERROR_PRINT("%s >> checking aV [%s] res  %d  oldres %d bf 0x%0x \n", __func__, aV->getfName(), res,
                            oldres, bits);
        ival = (int)res;
        amap.at("GPIObits")->setVal(ival);
        varsmap* vlist = vm->createVlist();  // for sending assetVars out through
                                             // fims_server/modbus_client
        // runs multiple commands based on which switches are on:
        // NOTE: Remove " ~ " - from res if res is all zeroes by default, only here
        // to zero out
        GPIORefTime->setVal(vm->get_time_ref());
        vm->addVlist(vlist, GPIORefTime);
        double tNow = vm->get_time_dbl();
        // int countflag = 0;
        for (auto x : vmap["/components/gpio"])
        {
            // this scan looks for the false -> true transition
            assetVar* avg = x.second;
            if (avg)
            {
                // bool dirin = true;
                // if (avg->gotParam("dir") && (strcmp(avg->getcParam("dir"),"output")
                // ==0))
                // {
                //     dirin = false;
                // }
                if (avg->getbParam("enabled") && avg != GPIORefTime)
                {
                    bval = !avg->getbParam("Pending");
                    if (0)
                        FPS_ERROR_PRINT(
                            "%s >> tNow [%f] found gpio [%s] pin [%d] bitfield [%s] bitres "
                            "[%s] bval [%s]\n",
                            __func__, tNow, avg->getfName(), avg->getiParam("bit"),
                            bits & (1 << avg->getiParam("bit")) ? "true" : "false",
                            res & (1 << avg->getiParam("bit")) ? "true" : "false", bval ? "true" : "false");
                    if (bits & (1 << avg->getiParam("bit")))
                    {
                        if (1)
                            FPS_ERROR_PRINT(
                                "%s >> tNow [%f] toggled gpio [%s] pin [%d] "
                                "pending [%s] res [%d] bits [%d]\n",
                                __func__, tNow, avg->getfName(), avg->getiParam("bit"), bval ? "true" : "false", res,
                                bits);
                        if (((res & (1 << avg->getiParam("bit"))) ^ avg->getbParam("inverted")) != bval)
                        {
                            avg->setParam("Pending", bval);
                            if (bval == true)
                            {
                                tNow = vm->get_time_dbl();
                                avg->setParam("SeenTime", tNow);
                                if (1)
                                    FPS_ERROR_PRINT(
                                        "%s >> tNow [%f] toggled gpio [%s] pin [%d] pending "
                                        "[true] SeenTime/Threshold %2.3f/%2.3f\n",
                                        __func__, tNow, avg->getfName(), avg->getiParam("bit"),
                                        tNow - avg->getdParam("SeenTime"), avg->getdParam("SeenTimeThreshold"));
                            }
                            else
                            {
                                avg->setVal(bval);  // No timer on reset - function only measures
                                                    // high level time currently
                                if (1)
                                    FPS_ERROR_PRINT(
                                        "%s >> tNow [%f] toggled gpio [%s] pin [%d] pending "
                                        "[false] SeenTime/Threshold %2.3f/%2.3f\n",
                                        __func__, tNow, avg->getfName(), avg->getiParam("bit"),
                                        tNow - avg->getdParam("SeenTime"), avg->getdParam("SeenTimeThreshold"));
                                dval = 0.0;
                                avg->setParam("SeenTime", dval);
                            }
                        }
                    }
                    bval = avg->getbParam("Pending");
                    if (bval && (avg->getdParam("SeenTime") > 0.0))
                    {
                        tNow = vm->get_time_dbl();
                        if (tNow - avg->getdParam("SeenTime") >= avg->getdParam("SeenTimeThreshold"))
                        {
                            if (1)
                                FPS_ERROR_PRINT(
                                    "tNow [%f] SeenTime/Threshold %2.3f/%2.3f has "
                                    "been reached for pin: [%s]\n",
                                    tNow, tNow - avg->getdParam("SeenTime"), avg->getdParam("SeenTimeThreshold"),
                                    avg->getName());
                            dval = 0.0;
                            avg->setParam("SeenTime", dval);
                            avg->setVal(bval);
                            // if(dirin)
                            // {
                            //     vm->addVlist(vlist, avg);
                            //     countflag++;
                            // }
                        }
                    }
                }
            }
        }
        // if(countflag > 0)
        // {
        //     vm->sendVlist(p_fims, "pub", vlist);
        // }
        // leak fix
        vm->clearVlist(vlist);
    }
    return 0;
}
// Better
// "/config/gpio":{
//  "gpios" {
//        "i2c_bus":"4",
//        "slave_address":"0x26"
//        "fcn":"GpioSignalInit",
//        "scanTime": 0.005,
//        "pubTime": 0.100
//         }
//
//
// just run this and I think it will all happen
int GpioInit(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    int quiet = 1;
    char fname[20];
    int i2cdev;
    int force = 0;
    bool i2cOK = false;
    if (1)
        FPS_ERROR_PRINT(">>>>>>>>Init >>>>%s running for GPIO Manager\n", __func__);
    asset_manager* am = aV->am;
    am->vm->setTime();
    for (auto x : vmap["/components/gpio"])
    {
        assetVar* av = x.second;
        if (av)
        {
            if (!av->gotParam("SeenTimeThreshold"))  // no threshold exists for this
                                                     // GPIO pin, setting it to 15ms:
            {
                FPS_ERROR_PRINT(
                    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
                    "SeenTimeThreshold not setup for GPIO pin: %s, setting "
                    "it to 15ms\n",
                    av->getfName());
                av->setParam("SeenTimeThreshold", 0.015);
            }
            if (!av->gotParam("SeenTime"))
            {
                av->setParam("SeenTime", 0.0);
            }
            if (!av->gotParam("Pending"))
            {
                av->setParam("Pending", false);
            }
        }
    }

    aV->setParam("i2cOK", i2cOK);
    double pubTime = 1.0;
    if (aV->gotParam("pubTime"))
    {
        pubTime = aV->getdParam("pubTime");
    }
    am->amap["pubTime"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "pubTime", pubTime);
    double scanTime = 0.005;
    if (aV->gotParam("scanTime"))
    {
        scanTime = aV->getdParam("scanTime");
    }
    am->amap["scanTime"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "scanTime", scanTime);
    if (am->vm->argc != 2)
        system("sudo modprobe i2c_dev");  // make sure to start the program with sudo
                                          // as well.
    char* i2c_bus = (char*)I2C_BUS;
    int i2cbusint = -1;
    char* slave_address = (char*)SLAVE_ADDRESS;
    i2cOK = true;
    if (aV->gotParam("i2c_bus"))
    {
        i2c_bus = aV->getcParam("i2c_bus");
        i2cbusint = lookup_i2c_bus(i2c_bus);
        if (i2cbusint < 0)
        {
            FPS_ERROR_PRINT("%s>> failed to get i2c_bus [%s]\n", __func__, i2c_bus);
            rwhelp(false);
            i2cOK = false;
            // return 0;
        }
        aV->setParam("i2cbusint", i2cbusint);
    }
    if (aV->gotParam("slave_address"))
    {
        slave_address = aV->getcParam("slave_address");
    }
    int address = parse_i2c_address(slave_address);
    if (address < 0)
    {
        FPS_ERROR_PRINT("%s>> failed to get i2c_address [%s]\n", __func__, slave_address);
        rwhelp(false);
        i2cOK = false;
        // return 0;
    }
    aV->setParam("address", address);
    i2cdev = open_i2c_dev(i2cbusint, fname, sizeof(fname), quiet);
    if (i2cdev < 0)
    {
        FPS_ERROR_PRINT("%s>> pass one failed to get i2c_dev [%s] with a value of : [%d]\n", __func__, fname, i2cdev);
        // rwhelp(false);
        i2cOK = false;
        char* tmp;
        asprintf(&tmp, "sudo chmod a+rw %s", fname);
        if (am->vm->argc != 2)
            system(tmp);
        free(tmp);
        // return 0;
    }
    i2cdev = open_i2c_dev(i2cbusint, fname, sizeof(fname), quiet);
    if (i2cdev < 0)
    {
        FPS_ERROR_PRINT("%s>> pass 2 failed to get i2c_dev [%s] with a value of : [%d]\n", __func__, fname, i2cdev);
        rwhelp(false);
        i2cOK = false;
        // we can keep going , we may need to simulate
        // return 0;
    }
    aV->setParam("address", address);
    aV->setParam("fname", fname);
    aV->setParam("i2cdevint", i2cdev);
    int saddr = set_slave_addr(i2cdev, address, force);
    if (saddr < 0)
    {
        force = 1;
        saddr = set_slave_addr(i2cdev, address, force);
    }
    if (saddr < 0)
    {
        FPS_ERROR_PRINT("%s>> failed to set  saddr [%s] force %d\n", __func__, slave_address, force);
        rwhelp(false);
        i2cOK = false;
        // return 0;
    }
    aV->setParam("saddr", saddr);
    aV->setParam("i2cOK", i2cOK);
    if (1)
        FPS_ERROR_PRINT(" i2cbus %d address %d i2cdev %d fname [%s] saddr %d force %d\n", i2cbusint, address, i2cdev,
                        fname, saddr, force);

    // this should be it....
    //               name          func,     initial delay, priority , repeat
    //               time, assetVar
    // int SchedRunFunc(varsmap& vmap, const char* sname, const char* func,
    // double runAfter,  double refTime,  double repTime, assetVar *aV)
    SchedRunFunc(vmap, "GpioRwPub", "GpioRwPub", 2.0, 0.1, pubTime, aV);
    SchedRunFunc(vmap, "GpioScan", "GpioScan", 2.0, 0.2, scanTime, aV);
    return 0;
}
int GpioRwPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = aV->am;
    // VarMapUtils* vm = am->vm;
    std::string auri = "/components/gpio";
    // int countflag = 0;
    int debug = aV->getiParam("debug");
    if (debug)
        FPS_ERROR_PRINT("%s  sysVec -> [%s] aV [%s]\n", __func__, auri.c_str(), aV->getfName());
    // varsmap* vlist = vm->createVlist(); // for sending assetVars out through
    // fims_server/modbus_client for (auto x :vmap["/components/gpio"])
    // {
    //     // this scan looks for the false -> true transition
    //     assetVar* avg  = x.second;
    //     if(avg)
    //     {
    //         // TODO add opts
    //         bool dirin = true;
    //         if (avg->gotParam("dir") && (strcmp(avg->getcParam("dir"),"output")
    //         ==0))
    //         {
    //             dirin = false;
    //         }
    //         if(dirin)
    //         {
    //             countflag++;
    //             vm->addVlist(vlist, avg);
    //         }
    //     }
    // }
    // if(countflag > 0)
    // {
    //     vm->sendVlist(p_fims, "pub", vlist);
    // }
    // vm->clearVlist(vlist);
    SendPub(vmap, am, auri.c_str(), auri.c_str(), aV);
    return 0;
}

//#endif