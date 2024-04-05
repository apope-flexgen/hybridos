
#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "asset.h"

class monCheck
{
public:
    monCheck(){};
    ~monCheck()
    {
        FPS_ERROR_PRINT("              start del check\n");
        for (auto& x : checksMap)
        {
            cJSON* cj = x.second;
            checksMap.erase(x.first);
            char* tmp = cJSON_PrintUnformatted(cj);
            FPS_ERROR_PRINT(" del check name [%s] [%s]\n", x.first.c_str(), tmp);
            free((void*)tmp);
            cJSON_Delete(cj);
        }
        checksMap.clear();
        FPS_ERROR_PRINT("               done  del check\n");
    };

    void addCheck(const char* _name, cJSON* cj)
    {
        char* tmp = cJSON_PrintUnformatted(cj);
        FPS_ERROR_PRINT(" add check [%s]\n", tmp);
        free((void*)tmp);

        checksMap[_name] = cJSON_Duplicate(cj, true);
    };

    std::map<std::string, cJSON*> checksMap;
};

class monItem
{
public:
    monItem(const char* _name) { name = _name; }

    ~monItem()
    {
        FPS_ERROR_PRINT(" monItem name [%s]\n", name.c_str());
        while (!mcVec.empty())
        {
            monCheck* mc = mcVec[0];
            mcVec.erase(mcVec.begin());
            delete mc;
        }
        mcVec.clear();
    };

    void addCheck(cJSON* cj)
    {
        monCheck* mc = new monCheck;
        mcVec.push_back(mc);
        cJSON* cji = cj->child;
        while (cji)
        {
            FPS_ERROR_PRINT(" monitem >>>>>>>>check component [%s] \n", cji->string);
            char* tmp = cJSON_PrintUnformatted(cji);
            FPS_ERROR_PRINT(" monitem >>>>>>>>before add check [%s]\n", tmp);
            free((void*)tmp);

            mc->addCheck(cji->string, cji);

            cji = cji->next;
        }
        cji = cj;

        cJSON* cjcomp = cJSON_GetObjectItem(cji, "comp");
        cJSON* cjvar = cJSON_GetObjectItem(cji, "var");
        // cJSON* cjmode = cJSON_GetObjectItem (cji,"mode");
        // cJSON* cjwrap = cJSON_GetObjectItem (cji,"wrap");
        // cJSON* cjtime = cJSON_GetObjectItem (cji,"time");
        // cJSON* cjonError = cJSON_GetObjectItem (cji,"onError");
        // cJSON* cjonPass = cJSON_GetObjectItem (cji,"onPass");
        // cJSON* cjmax = cJSON_GetObjectItem (cji,"max");
        // cJSON* cjmin = cJSON_GetObjectItem (cji,"min");
        // cJSON* cjmaxvar = cJSON_GetObjectItem (cji,"maxvar");
        // cJSON* cjminvar = cJSON_GetObjectItem (cji,"minvar");
        if (cjcomp && cjvar)
            FPS_ERROR_PRINT(">>> CHECK comp [%s] var [%s] \n", cjcomp->valuestring, cjvar->valuestring);
    }

    std::string name;
    std::vector<monCheck*> mcVec;
};

class monitor
{
public:
    monitor(){};
    ~monitor()
    {
        for (auto& x : monItems)
        {
            FPS_ERROR_PRINT(" freeing monItem [%s]\n", x.first.c_str());
            delete x.second;
        }
        monItems.clear();
    };

    // todo clean up the fims message thrashing
    // configure the asset
    void configure(const char* fname)
    {
        VarMapUtils vm;
        cJSON* cj = vm.get_cjson(fname);
        // cJSON* cj = cjbase;
        // const char* vname;
        cJSON* cjm = cJSON_GetObjectItem(cj, "monitor");
        cjm = cjm->child;
        while (cjm)
        {
            monItem* mi = monItems[cjm->string] = new monItem(cjm->string);

            // cJSON * cjm = cJSON_GetObjectItem (cj,"monitor");
            // uri - cj->string
            // uri->body - cj->child
            // FPS_ERROR_PRINT(" cj->string [%s] child [%p]\n", cj->string, (void *)
            // cj->child);  char* body = cJSON_PrintUnformatted(cj);
            // cJSON_Array
            FPS_ERROR_PRINT(" Monitor State  [%s] child [%p] \n", cjm->string, (void*)cjm->child);
            cJSON* cjf = cJSON_GetObjectItem(cjm, "freqmS");
            cJSON* cjck = cJSON_GetObjectItem(cjm, "checks");
            FPS_ERROR_PRINT(" processing a checks freq [%f] \n", cjf->valuedouble);

            FPS_ERROR_PRINT(">>> processing checks type %d child type %d array %d\n", cjck->type, cjck->child->type,
                            cJSON_Array);
            cJSON* cji;
            cJSON_ArrayForEach(cji, cjck)
            {
                FPS_ERROR_PRINT(
                    " main config >>> processing a check  type %d child "
                    "type %d array %d\n",
                    cji->type, cji->child->type, cJSON_Array);
                char* tmp = cJSON_PrintUnformatted(cji);
                FPS_ERROR_PRINT(" main config >>>> add check [%s]\n", tmp);
                free((void*)tmp);
                mi->addCheck(cji);
            }

            cjm = cjm->next;
        }
    }

    std::map<std::string, monItem*> monItems;
};

#endif