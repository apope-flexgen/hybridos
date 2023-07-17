#ifndef ASSET_HPP
#define ASSET_HPP
/*
* asset and asset manager
*/


#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <malloc.h>
#include <cjson/cJSON.h>
#include <poll.h>
#include <signal.h>
#include <cstring>
#include <pthread.h>
#include <fims/libfims.h>

#include "assetVarTest.h"

#ifndef FPS_ERROR_PRINT
#define FPS_ERROR_PRINT printf
#define FPS_DEBUG_PRINT printf
#endif

// an asset will have variables, states and parameters
// an asset can also have alarms and warnings


class asset {

protected:
    double side_length_;

public:
    asset()
        : side_length_(0)
    {
        // // somehow these stay around
        // std::map<std::string,assetVar*> m1;
        // std::map<std::string,assetVar*> m2;
        // std::map<std::string,assetVar*> m3;
        // //std::map<std::string, std::map<std::string,assetVar*>>allVars;
        // allVars.insert(std::make_pair("conifg", m1));
        // allVars.insert(std::make_pair("status", m2));
        // allVars.insert(std::make_pair("cmd", m3));
    }

    virtual ~asset() 
    {
        std::cout << "asset_delete :" << name << "\n"; 
    }

    void setName(const char* _name) {
        name = _name;
    }

    virtual assetVar* getMVar(std::map<std::string,assetVar*>&map, const char* name)
    {
        auto it = map.find(name);
        return (it != map.end())?it->second:nullptr;
    }
    template <class T>
    assetVar* setMVar(std::map<std::string,assetVar*>&map, const char* name, T val)
    {
        auto it = map.find(name);
        if (it == map.end())
            map[name] = new assetVar(name, val);
        return map[name];
    }

    // virtual assetVar* setMVar(std::map<std::string,assetVar*>&map, const char* name, int val)
    // {
    //     std::map<std::string,assetVar*>::iterator it=map.find(name);
    //     if (it == map.end())
    //         map[name] = new assetVar(name, val);
    //     return map[name];
//   }
    // template < class T>
    // assetVar* addStatusVar(const char *vname, T val)
    // {
    //     return setMVar(allVars["status"], vname, val);
    // }
    // template < class T>
    // assetVar* addConfigVar(const char *vname, T val)
    // {
    //     return setMVar(allVars["config"], vname, val);
    // }
    // template < class T>
    // assetVar* addCmdVar(const char *vname, T val)
    // {
    //     return setMVar(allVars["cmd"], vname, val);
    // }
    // virtual assetVar* getCmdVar(const char *vname)
    // {
    //     return getMVar(allVars["cmd"],vname);
    // }
    // virtual assetVar* getConfigVar(const char *vname)
    // {
    //     return getMVar(allVars["conifg"],vname);
    // }
    // virtual assetVar* getStatusVar(const char *vname)
    // {
    //     return getMVar(allVars["status"],vname);
    // }
    // template<class T>
    // void setStatusVar(const char *vname, T val)
    // {
    //     assetVar* av = getStatusVar(vname);
    //     if(av) av->setVal(val);
    // }
    // template<class T>
    // void setCmdVar(const char *vname, T val)
    // {
    //     assetVar* av = getCmdVar(vname);
    //     if(av) av->setVal(val);
    // }
    // template<class T>
    // void setConfigVar(const char *vname, T val)
    // {
    //     assetVar* av = getConfigVar(vname);
    //     if(av) av->setVal(val);
    // }

    //void configure(const char* fname);
    //void configure2(const char* fname);

    virtual const char* get_command(const char*dest, const char* cmd)
    {
        std::string rstr = "{\"asset_name\":";
        std::cout << "asset_dest :" << dest << " got  command:\n" << cmd << "\n";
        rstr.append(name);
        rstr.append(",\"cmd\":\"");
        rstr.append(cmd);
        rstr.append("\"}");

        return strdup(rstr.c_str());
    }

// cJSON* cjbm = bm->getConfig();
//     char* res = cJSON_Print(cjbm);
//     printf("Maps at beginning \n%s\n", res);
//     free((void *)res) ;
//     cJSON_Delete(cjbm);

#if 0
    // todo clean up the fims message thrashing
    // configure the asset
    void configure(const char* fname,std::vector<std::pair<std::string, std::string>> *reps = nullptr)
    {
        VarMapUtils vm;
        cJSON* cjbase = vm.get_cjson(fname, reps);
        cJSON* cj = cjbase->child;
        const char* vname;
        while(cj)
        {
            // uri - cj->string
            // uri->body - cj->child
            //FPS_ERROR_PRINT(" cj->string [%s] child [%p]\n", cj->string, (void *) cj->child);
            char* body = cJSON_Print(cj);
            FPS_ERROR_PRINT(" %S >> cj->string [%s] child [%p] body \n[%s]\n"
                , __func__, cj->string, (void *) cj->child, body);

            char* buf = vm.fimsToBuffer("set", cj->string, nullptr , body);
            free((void *)body);
            fims_message* msg = vm.bufferToFims(buf);
            free((void *)buf);
            cJSON *cjb = nullptr;
            vm.processFims(*vmap, msg,  &cjb);
            vm.free_fims_message(msg);
            // //processFims(nullptr,msg, &cjb);
            // buf = vm.fimsToBuffer("get","/system" , "/mee" , nullptr);
            // msg = vm.bufferToFims(buf);
            // vm.processFims(*vmap, msg, &cjb);
            // if(buf) free((void *)buf);

            buf = cJSON_Print(cjb);
            if(cjb) cJSON_Delete(cjb);
            // vm.free_fims_message(msg);


            FPS_ERROR_PRINT("%s >>  configured [%s]\n",__func__, buf);
            free((void *)buf);
            //free((void *)msg);

            cj = cj->next;
        }
        cJSON_Delete(cjbase);

    }
    cJSON* getConfig()
    {
        VarMapUtils vm;
        if(vmap)
            return  vm.getMapsCj(*vmap);
        return nullptr;
    }
#endif
    

    bool free_message(fims_message* message)
    {
        // TODO manage memory better
        if(message == nullptr)
            return false;
        delete(message);
        return true;
    }

    // component. var name, idname , assetvar
    //virtual bool setRegisterId(const char* cname, const char *rname, const char*idname, assetVar* avar);
    //virtual assetVar* getRegisterId(const char* cname, const char*idname);


    // //virtual void showCompMap(void);
    // virtual bool setRegisterId(const char* cname, const char *vname,const char*idname, assetVar* avar)
    // {
    //     bool ok = true;
    //     FPS_ERROR_PRINT( "     >>>   component_id [%s] var name [%s] , id name [%s] \n", cname, vname, idname);
    //     //add this idname to the asset component map
    //     //               component        reg_id
    //     // std::map<std::string,std::map<std:string, assetVar *>>
    //     compMap[cname][idname] = avar;; // std::make_pair(idname ,avar);

    //     return ok;
    // }

    // virtual assetVar* getRegisterId(const char* cname,const char*idname)
    // {
    //     assetVar* avar = nullptr;
    //     auto it = compMap.find(cname);
    //     if(it == compMap.end())
    //     {
    //         return avar;
    //     }
    //     auto iv = it->second.find(idname);
    //     if(iv == it->second.end())
    //     {
    //         return avar;
    //     }
    //     return iv->second;
    // }


    // void showCompMap(void)
    // {
    //     for (auto &m : compMap)
    //     {
    //         FPS_ERROR_PRINT(" comp_name [%s] \n", m.first.c_str());
    //         for (auto &r : m.second)
    //         {
    //             FPS_ERROR_PRINT("    >>>var_reg_id [%s] \tavar [%s]\n", r.first.c_str(), r.second->getName());
    //         }
    //     }
    // }


    // void set_side_length(double side_length) {
    //     side_length_ = side_length;
    // }

    //virtual const char * showAssetCj(void);

    void setVmap(varsmap *_vmap)
    {
        vmap = _vmap;
    }

    std::string id;
    std::string name;
    varsmap compMap;
    varsmap allVars;
    // context for asset
    varsmap *vmap;

    varsmap vmaps;
    VarMapUtils vm;
    //std::map<std::string, std::map<std::string, assetVar*>>compMap;
    //std::map<std::string, std::map<std::string, assetVar*>>allVars;

};

class asset_manager {
protected:
    double side_length_;

public:
    asset_manager()
        : side_length_(0) {};

    virtual ~asset_manager() 
    {
        FPS_ERROR_PRINT(" asset manager running cleanup\n");
        cleanup();
    };
    void setVmap(varsmap *_vmap)
    {
        vmap = _vmap;
    }
//  "/assets/bms":        {
//                 "bms_1":   { 
//                                    "template":"bms_catl_template.json".
//                                    "subs":[
//                                       {"replace":"@@BMS_ID@@","with":"bms_1"},
//                                       {"replace":"@@BMS_IP@@","with":"192.168.1.114"}
//                                       ]
//                                   }
//            }
#if 0
    // configure the asset
    virtual void configure(const char* fname, const char *aname)
    {
        VarMapUtils vm;
        std::vector<std::pair<std::string, std::string>> reps;

        cJSON* cjbase = vm.get_cjson(fname, nullptr);
     
        char * assname = nullptr;
        asprintf(&assname, "/assets/%s", aname);

        cJSON* cj = cjbase->child;
        cJSON * cja = cJSON_GetObjectItem(cjbase, assname);
        cja = cja->child;

        while (cja) 
        {
            cJSON* cjsi;
            cJSON* cjt = cJSON_GetObjectItem(cja, "template");
            FPS_ERROR_PRINT(" %s   >> found asset [%s]\n", __func__, cja->string);
            FPS_ERROR_PRINT(" %s   >> found asset [%s] template [%s]\n", __func__, cja->string, cjt->valuestring?cjt->valuestring:"No template");

            cJSON* cjs = cJSON_GetObjectItem(cja, "subs");
            FPS_ERROR_PRINT(" %s   >> found asset [%s] subs %p isArray %s\n"
                            , __func__, cja->string, cjs
                            , cJSON_IsArray(cjs)?"true":"false"
                            );
            if (cJSON_IsArray(cjs))
            {
                cJSON_ArrayForEach(cjsi, cjs)
                {
                    cJSON* cjsr =cJSON_GetObjectItem(cjsi, "replace");
                    cJSON* cjsw =cJSON_GetObjectItem(cjsi, "with");
                    
                    if(cjsr && cjsw && cjsr->valuestring && cjsw->valuestring)
                    {
                        FPS_ERROR_PRINT(" %s   >> found asset [%s] sub from  [%s] to [%s]\n", __func__, cja->string
                            , cjsr->valuestring
                            , cjsw->valuestring);
                        reps.push_back(std::make_pair(cjsr->valuestring,cjsw->valuestring));
                        //bms * bms = new bms(cja->string);

                    }

                }
            }


            cja = cja->next;
        }
    }
#endif

// junk to be removed
    // void set_side_length(double side_length) {
    //     side_length_ = side_length;
    // }

    // send a command to one or all the assets.
    const char* send_command(const char*dest, const char* cmd)
    {
        const char* res;
        std::string rstr = "";
        if (strcmp(dest, "all")== 0)
        {
            rstr.append("["); // its a list
            for (auto it = assetMap.begin() ; it != assetMap.end(); ++it)
            {
                if(it != assetMap.begin())
                    rstr.append(",");
                res = (it->second)->get_command(it->first.c_str(), cmd);
                rstr.append(res);

                std::cout << " Manager sent command  " << cmd << " to : " << it->first.c_str() << " res: ["<<res<<"] rstr: [" << rstr << "]\n";
                free((void *)res);
            }
            rstr.append("]");

        }
        else
        {
            auto it = assetMap.find(dest);
            if(it != assetMap.end()) {
                res = assetMap[dest]->get_command(dest,cmd);
                rstr.append(res);
                std::cout << " Manager sent command  " << cmd << " to : " << dest << " res: ["<<res<<"] rstr: [" << rstr.c_str() << "]\n";
                free((void *)res);
            }
            else
            {
                std::cout << " Manager dest  [" << dest << "] not in asset map\n";
                for (auto const &x :assetMap) {
                    std::cout << "Item Name :[" << x.first <<"]\n";
                }
            }
        }
        return rstr.c_str();
    }
    // this has to be done in the target class to get the full asset type
    virtual asset* addInstance(const char * name) = 0;

    void configInstance(asset *item);

    // add the asset instance to the asset map
    virtual void mapInstance(asset *item, const char* _name= nullptr)
    {
        if(_name)
            item->setName(_name);
        auto it = assetMap.find(item->name);
        if(it == assetMap.end()) {
            std::cout << " mapped instance " << item->name << " OK\n";
            assetMap[item->name] = item;
        }
        else
        {
            std::cout << " FAIL instance " << item->name << " already mapped\n";
        }
    }

    virtual asset* getInstance(const char* _name)
    {
        auto it = assetMap.find(_name);
        if(it != assetMap.end()) {
            return assetMap[_name];
        }
        return nullptr;
    }

    void cleanup(void)
    {
        for (auto &x : assetMap)
        {
            delete x.second;
        }
        assetMap.clear();
    }
    // this will contain a list of names assets
    //virtual double area() const = 0;

    std::string name;
    //varmap assetMap;
    std::map<std::string, asset*>assetMap;
    // context for asset manager
    varsmap *vmap;
//    VarMapUtils vm;

};

// the types of the manage class factories
typedef asset_manager* createm_t(const char *name);
typedef void destroym_t(asset_manager*);

// the types of the class factories
typedef asset* create_t();
typedef void destroy_t(asset*);
// This function will create a  FIMS message buffer
char* fimsToBuffer(const char* method, const char* uri, const char* replyto, const char* body);
fims_message* bufferToFims(const char *buffer);

#endif
