#ifndef BMS_HPP
#define BMS_HPP
#include "asset.h"

#include <cmath>
// g++ -shared -fpic -o bms.so  bms.cpp

class bms : public asset
{
public:
    bms()
    {
        name = "Dummy";
        std::cout << " bms instance [" << name << "] created\n";
    };

    bms(const char* _name)
    {
        name = _name;
        std::cout << " bms instance [" << name << "] created\n";
    };
    ~bms() { std::cout << " bms instance [" << name << "] deleted\n"; };

    void cfgwrite(const char* fname, const char* aname = nullptr)
    {
        VarMapUtils vm;

        cJSON* cj = getConfig();
        vm.write_cjson(fname, cj);
        //         char* res = cJSON_Print(cjbm);
        // //      printf("Maps at beginning \n%s\n", res);
        //         free((void *)res) ;
        cJSON_Delete(cj);
    }

    // recieves a command
    virtual const char* get_command(const char* dest, const char* cmd)
    {
        std::string rstr = "{\"bms_name\":";
        std::cout << "dest :" << dest << " got  command:\n" << cmd << "\n";
        rstr.append(name);
        rstr.append(",\"cmd\":\"");
        rstr.append(cmd);
        rstr.append("\"}");

        return strdup(rstr.c_str());
    }
};

class bms_manager : public asset_manager
{
public:
    bms_manager(const char* _name)
    {
        name = _name;
        std::cout << " bms manager created\n";
    }
    ~bms_manager() { std::cout << " bms manager deleted\n"; }
    // configure the asset  varsmap vmap;
    //     void cfgwrite(const char* fname, const char *aname=nullptr)
    //     {
    //         VarMapUtils vm;

    //         cJSON* cj = getConfig();
    //         vm.write_cjson(fname, cj);
    // //         char* res = cJSON_Print(cjbm);
    // // //      printf("Maps at beginning \n%s\n", res);
    // //         free((void *)res) ;
    //         cJSON_Delete(cj);
    //     }

    void configure(varsmap* vmap, const char* fname, const char* aname)
    {
        VarMapUtils vm;

        // vm.getReps(const char *fname, const char *aname,)
        cJSON* cjbase = vm.get_cjson(fname, nullptr);

        char* assname = nullptr;
        asprintf(&assname, "/assets/%s", aname);

        cJSON* cj = cjbase->child;
        cJSON* cja = cJSON_GetObjectItem(cjbase, assname);
        cja = cja->child;

        while (cja)
        {
            cJSON* cjsi;
            cJSON* cjt = cJSON_GetObjectItem(cja, "template");
            if (0)
                FPS_ERROR_PRINT(" %s >> found asset [%s]\n", __func__, cja->string);
            if (0)
                FPS_ERROR_PRINT(" %s >> found asset [%s] template [%s]\n", __func__, cja->string,
                                cjt->valuestring ? cjt->valuestring : "No template");

            cJSON* cjs = cJSON_GetObjectItem(cja, "subs");
            if (0)
                FPS_ERROR_PRINT(" %s   >> found asset [%s] subs %p isArray %s\n", __func__, cja->string, cjs,
                                cJSON_IsArray(cjs) ? "true" : "false");
            if (cJSON_IsArray(cjs))
            {
                std::vector<std::pair<std::string, std::string>> reps;

                cJSON_ArrayForEach(cjsi, cjs)
                {
                    cJSON* cjsr = cJSON_GetObjectItem(cjsi, "replace");
                    cJSON* cjsw = cJSON_GetObjectItem(cjsi, "with");

                    if (cjsr && cjsw && cjsr->valuestring && cjsw->valuestring)
                    {
                        if (1)
                            FPS_ERROR_PRINT(" %s   >> found asset [%s] sub from  [%s] to [%s]\n", __func__, cja->string,
                                            cjsr->valuestring, cjsw->valuestring);
                        reps.push_back(std::make_pair(cjsr->valuestring, cjsw->valuestring));
                    }
                }
                bms* bms = addInstance(cja->string);
                bms->setVmap(vmap);
                bms->configure(cjt->valuestring, &reps);
            }
            cja = cja->next;
        }
    }

    virtual void mapInstance(bms* item, const char* _name = nullptr)
    {
        if (_name)
            item->setName(_name);
        auto it = assetMap.find(item->name);
        if (it == assetMap.end())
        {
            FPS_ERROR_PRINT("%s >> mapped instance %s OK\n", __func__, item->name.c_str());
            assetMap[item->name] = item;
        }
        else
        {
            FPS_ERROR_PRINT("%s >> ERROR mapped instance %s FAILED\n", __func__, item->name.c_str());
        }
    }

    virtual bms* addInstance(const char* _name)
    {
        bms* item = new bms();
        std::cout << " added bms instance " << _name << " OK\n";
        mapInstance(item, _name);
        return item;
    };

    virtual bms* getInstance(const char* _name)
    {
        auto it = assetMap.find(_name);
        if (it != assetMap.end())
        {
            return assetMap[_name];
        }
        return nullptr;
    }

    virtual int getNumAssets() { return assetMap.size(); }

    std::map<std::string, bms*> assetMap;
    // context for asset manager
    // virtual bms* getInstance(const char *_name)
    // {

    // }

    // virtual double area() const {
    //     return side_length_ * side_length_ * sqrt(3) / 2;
    // }
};

#endif