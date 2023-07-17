#ifndef TIMEGEN_HH
#define TIMEGEN_HH
#include "asset.h"

class timegen : public asset {
public:
    timegen () {
        name = "Dummy";
        std::cout << " timegen instance ["<<name<< "] created\n";
        pthread_create (&time_th, nullptr, this->timer_loop, (void*)this) ;
    };
    
    timegen (const char *_name) {
        name = _name;
        std::cout << " timegen instance ["<<name<< "] created\n";
        pthread_create (&time_th, nullptr, this->timer_loop, (void*)this) ;
        ticks = 0;
        secs = 0;
    };
    ~timegen() {
         std::cout << " timegen instance ["<<name<< "] deleted\n";
         stop = 1;
         // pthread_join(&time_th, nullptr);
    };

    static void* timer_loop(void *data)
    {
        VarMapUtils vm;
        timegen * tg = (timegen*)data; 
        while (!tg->stop)
        {
            tg->secs++;
            
            cJSON*cjr = cJSON_CreateObject();
            char * tmp;
            asprintf(&tmp,"{\"sec\":{\"value\":%d}}", tg->secs);
            vm.processRawMsg(*tg->vmap, "set", "/system/time", nullptr, tmp, &cjr);
            cJSON_Delete(cjr);
            free((void*)tmp);
            poll(nullptr,0,1000);

        }
        pthread_exit(nullptr);
    }
    // todo get rid of this
    virtual double area() const {
        return 3.5;
    }
    void set_comp(const char*compname)
    {
        comp=compname;

    };
    void tick() {
        VarMaps vm;
        ticks++;
        
        cJSON*cjr = cJSON_CreateObject();
        char * tmp;
        asprintf(&tmp,"{\"hb\":{\"value\":%d}}", ticks);
        vm.processRawMsg(*vmap, "set", "/system/time", nullptr, tmp, &cjr);
        cJSON_Delete(cjr);
        free((void*)tmp);

    }
    // add a list of comps and vars
    // todo add thread in here that ticks every second
    // and updates sec/min/day etc
    // recieves a command
    virtual const char* get_command(const char*dest, const char* cmd)
    {
        std::string rstr = "{\"bms_name\":";
        std::cout << "dest :" << dest << " got  command:\n" << cmd << "\n";
        rstr.append(name);
        rstr.append(",\"cmd\":\"");
        rstr.append(cmd);
        rstr.append("\"}");

        return strdup(rstr.c_str());
    }
    int ticks;
    int secs;
    std::string comp;
    pthread_t time_th;
    int stop = 0;
};

#endif
