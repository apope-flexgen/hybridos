/*
 * vmap basic test
 */

#include "asset.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <mutex>
#include <malloc.h>
#include <pthread.h>
#include <cmath>

#include <cjson/cJSON.h>
#include <fims/libfims.h>
#include <fims/fps_utils.h>

#include "assetFunc.cpp"
#include "chrono_utils.hpp"

std::vector<std::string>* sysVec = nullptr;

char* pull_pfrag(const char* uri, int idx)
{
    char* sp = (char*)uri;
    while (idx)
    {
        if (*sp++ == '/') idx--;
    }

    char* comp_id = strdup(sp);
    char* comp_end = strstr(comp_id, "/");
    if (comp_end)
        *comp_end = 0;
    return comp_id;
}

char* pull_first_uri(const char* uri, int n = 1)
{
    char* comp = strdup(uri);
    char* sp = (char*)comp;
    sp++;
    while (*sp)
    {
        if (*sp++ == '/')
        {
            if (n > 1)
            {
                n--;
            }
            else
            {
                break;
            }

        }
    }
    if (*sp)
    {
        *--sp = 0;
    }
    return comp;
}

int get_nfrags(const char* uri)
{
    int nfrags = 0;
    const char* sp = uri;
    while (*sp)
    {
        if (*sp++ == '/') nfrags++;
    }
    return nfrags;
}

char* pull_last_uri(const char* uri, int n = 1)
{
    int nf = get_nfrags(uri);
    nf -= n;

    char* comp = nullptr;
    char* sp = (char*)uri;
    sp++;
    while (*sp)
    {
        if (*sp++ == '/')
        {
            if (nf > 1)
            {
                nf--;
            }
            else
            {
                //sp++;
                break;
            }
        }
    }
    if (*sp)
    {
        comp = strdup(sp);
    }
    return comp;
}

char* pull_uri(const char* uri, int idx)
{
    char* comp = strdup(uri);
    char* sp = (char*)comp;
    while (*sp && idx)
    {
        if (*sp++ == '/') idx--;
    }
    if (*sp)
    {
        *--sp = 0;
    }
    return comp;
}

char* pull_one_uri(const char* uri, int idx)
{
    char* comp = strdup(uri);
    char* res = nullptr;
    char* sp = (char*)comp;
    char* spt = nullptr;
    while (*sp && idx)
    {
        if (*sp++ == '/') idx--;
    }
    spt = sp;

    while (*sp && *sp != '/')
    {
        sp++;
    }

    if (*sp)
    {
        *sp = 0;
    }
    if (spt)
    {
        res = strdup(spt);
    }
    if (comp)free((void*)comp);
    return res;
}
assetVar* getVar(varsmap& vmap, const char* comp, const char* var = nullptr)
{
    char* mycomp = (char*)comp;
    char* myvar = (char*)var;

    assetVar* av = nullptr;

    if (var == nullptr)
    {
        // break up uri into comp:var
        mycomp = strdup(comp);
        myvar = strstr(mycomp, ":");
        if (myvar)
        {
            *myvar = 0;
            myvar++;
        }
    }

    if (0)FPS_ERROR_PRINT("%s looking for comp [%s] var [%s]\n"
        , __func__
        , mycomp
        , myvar);
    if (vmap.size() > 0)
    {
        auto ic = vmap.find(mycomp);
        if (ic == vmap.end())
        {
            if (0)FPS_ERROR_PRINT("%s NOTE created comp [%s] var [%s]\n"
                , __func__
                , mycomp
                , myvar);

        }
        if (ic != vmap.end())
        {
            if (0)FPS_ERROR_PRINT("%s found comp [%s] looking for var [%s] size %d\n"
                , __func__
                , mycomp
                , myvar
                , (int)vmap[mycomp].size()
            );
            // for (auto it : vmap[mycomp])
            // {
            //     if(0)FPS_ERROR_PRINT("%s found comp [%s] looking for var [%s] found [%s]\n"
            //         , __func__
            //         , mycomp
            //         , myvar
            //         ,it.first.c_str()
            //         );
            //     if(strcmp(myvar,it.first.c_str()) == 0)
            //     {
            //         av = vmap[mycomp][myvar];
            //         break;
            //     }

            // }  
            if (myvar)
            {
                auto iv = vmap[mycomp].find(myvar);
                if (iv != vmap[mycomp].end())
                {
                    av = vmap[mycomp][myvar];
                }
            }
        }
    }
    if (mycomp != (char*)comp)
        free((void*)mycomp);
    return av;

}

// TODO test comp /a/b:c  ok working
template <class T>
T getVar(varsmap& vmap, const char* comp, const char* var, T& value)
{
    assetVar* av = getVar(vmap, comp, var);
    if (av)
    {
        return av->getVal(value);
    }
    return value;
}
// get the asset list if we have one
assetList* getAlist(varsmap& vmap, const char* uri)
{
    assetList* alist = nullptr;
    char* tmp = nullptr;
    // // no leading "/" so its hard to find 
    asprintf(&tmp, "_%s", uri);
    if (tmp)
    {
        assetVar* av = getVar(vmap, tmp, "assetList");
        if (av)
        {
            alist = (assetList*)av->aVar;
        }
        if (0)FPS_ERROR_PRINT(" %s >> looking for alist [%s] av [%p] alist [%p]\n"
            , __func__
            , tmp
            , av
            , alist
        );

        free((void*)tmp);
    }
    return alist;
}
//include/varMapUtils.h: In member function ‘int VarMapUtils::vListPartialSendFims(varsmap&, std::vector<std::basic_string<char> >&, const char*, fims*, const char*)’:
    // ./include/varMapUtils.h:140:55: error: ‘loadAssetList’ was not declared in this scope
    //              int found = loadAssetList(vmap, cj, newuri);
    //                                                        ^
    // ./include/varMapUtils.h: In member function ‘cJSON* VarMapUtils::getMapsCj(varsmap&, const char*, const char*, int)’:
    // ./include/varMapUtils.h:1828:58: error: ‘loadAssetList’ was not declared in this scope
    //              found += loadAssetList(vmap, cj, newuri, opts);
    //                                                           ^
    // ./include/varMapUtils.h:1899:65: error: ‘loadAssetList’ was not declared in this scope
    //                  if(loadAssetList(vmap, cj, x.first.c_str(), opts))int found
    // {"bms":{"summary":{<data>}}}
    //
    int baseVec(std::string& bs, std::vector<std::string>& buri, std::vector<std::string>& turi)
    {
        int rc = 0;
        if (0) FPS_ERROR_PRINT(" %s >> sizes buri [%d] turi [%d] \n"
            , __func__
            , (int)buri.size()
            , (int)turi.size()
        );
        while ((rc < (int)buri.size()) && (rc < (int)turi.size()))
        {
            if (buri[rc] != turi[rc])
            {
                if (0) FPS_ERROR_PRINT(" %s >> uri [%s] furi [%s] break rc %d\n"
                    , __func__
                    , buri[rc].c_str()
                    , turi[rc].c_str()
                    , rc);
                break;
            }
            bs += "/";
            bs += buri[rc];
            rc++;
        }
        return rc;
    }

    int uriSplit(std::vector<std::string>& uriVec, const char* _uri)
    {
        int nfrags = 0;
        std::string uri = _uri;
        std::string key = "/";

        std::size_t startf;
        std::size_t endf = 0;
        do
        {
            if (endf == 0)
            {
                startf = 1;
            }
            else
            {
                std::string furi = uri.substr(startf, (endf - startf));
                if (0) FPS_ERROR_PRINT(" %s >> uri [%s] furi [%s]\n", __func__, _uri, furi.c_str());
                startf = endf + 1;
                uriVec.push_back(furi);
                nfrags++;
            }

        } while ((endf = uri.find(key, startf)) != std::string::npos);
        std::string furi = uri.substr(startf, (endf - startf));
        uriVec.push_back(furi);

        if (0) FPS_ERROR_PRINT(" %s >> last >> uri [%s] furi [%s]\n", __func__, _uri, furi.c_str());
        return nfrags;
    }

    cJSON* createUriListCj(varsmap& vmap, std::string& bs, const char* inuri, cJSON* incj, int options, std::vector<std::string>& uriVec)
    {
        // split uri up into strings
        std::vector<std::string> inVec;
        if (incj == nullptr)
        {
            incj = cJSON_CreateObject();
        }
        //int infrags = 
        uriSplit(inVec, inuri);   //  inVec /status/bms   uriVec /status/bms_1,2,3,4   etc
        for (auto& x : uriVec)
        {
            const char* myuri = x.c_str();
            if (0) FPS_ERROR_PRINT(" %s >> uri [%s] uriVec [%s] opts 0x%04x\n", __func__, inuri, myuri, options);
            assetList* alist = getAlist(vmap, myuri);

            if (0)FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getting cj map for [%s] assetList %p  cj %p\n"
                , __func__
                , myuri
                , alist
                , (void*)incj
            );

            std::vector<std::string> uVec;
            //int infrags = 
            uriSplit(uVec, x.c_str());
            std::string bsx;
            int bvec = baseVec(bsx, inVec, uVec);
            bs = bsx;
            if (0) FPS_ERROR_PRINT(" %s >> uri [%s] uriVec [%s] opts 0x%04x bvec %d bs[%s]\n", __func__, inuri, x.c_str(), options, bvec, bs.c_str());
            cJSON* cji = incj;
            cJSON* cjii = nullptr;
            if (bvec < (int)uVec.size())
            {
                if (0) FPS_ERROR_PRINT(" %s >> bvec small %d need to find / create trees\n", __func__, bvec);
                while (bvec < (int)uVec.size())
                {
                    if (0) FPS_ERROR_PRINT(" %s >> bvec small %d  find / create tree [%s] inuri [%s] \n", __func__, bvec, uVec[bvec].c_str(), inuri);
                    cjii = cJSON_GetObjectItem(cji, uVec[bvec].c_str());
                    if (!cjii)
                    {
                        cjii = cJSON_CreateObject();
                        cJSON_AddItemToObject(cji, uVec[bvec].c_str(), cjii);
                    }
                    // build tree
                    cji = cjii;
                    bvec++;
                }
                // now run getMapsCj on x.c_str() into cjii
                if (0) FPS_ERROR_PRINT(" %s >> running getMapsCj [%s ] into end node\n", __func__, x.c_str());
                if (alist)
                {
                    unsigned int ix = 0;
                    assetVar* av;
                    do
                    {
                        av = alist->avAt(ix++);
                        if (av) av->showvarCJ(cjii, options);
                    } while (av);
                }
                else
                {
                    for (auto& y : vmap[x.c_str()])
                    {
                        if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                        y.second->showvarCJ(cjii, options);
                        if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                        //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                    }
                }
            }
            else
            {
                // now run getMapsCj on x.c_str() into cjii
                if (0) FPS_ERROR_PRINT(" %s >> running getMapsCj [%s ] into end node\n", __func__, x.c_str());
                if (alist)
                {
                    unsigned int ix = 0;
                    assetVar* av;
                    do
                    {
                        av = alist->avAt(ix++);
                        if (av) av->showvarCJ(cji, options);
                    } while (av);
                }
                else
                {

                    for (auto& y : vmap[x.c_str()])
                    {
                        if (0)FPS_ERROR_PRINT("%s >> getting cj for [%s] \n", __func__, y.first.c_str());
                        y.second->showvarCJ(cji, options);
                        if (0)FPS_ERROR_PRINT("%s >> got cj for [%s] \n", __func__, y.first.c_str());
                        //cJSON_AddItemToObject(cj,y.first.c_str(),cji);
                    }
                }
            }
        }
        return incj;
    }

    // 1/ create a list of stuff in varsmap
    // 2/ put items in sysvec order ( if you have a sysVec)
    // 3/ find  matches to the uri
    //std::vector<std::string> nVec;
    int createAssetListCj(varsmap& vmap, const char* uri, std::vector<std::string>* sysVec, int opts, std::vector<std::string>& nVec)
    {
        std::vector<std::string> xVec;
        std::vector<std::string> yVec;
        std::vector<std::string>* yVecp;

        std::vector<std::string> zVec;
        for (auto& x : vmap)
        {
            if (0) FPS_ERROR_PRINT(" %s >> xVec push_back [%s]\n", __func__, x.first.c_str());
            xVec.push_back(x.first);
        }
        if (0) FPS_ERROR_PRINT(" %s >> xVec size [%d]\n", __func__, (int)xVec.size());
        yVecp = &xVec;

        // now extract it from sysVec if we have one
        if (sysVec)
        {
            for (auto& y : *sysVec)
            {
                // y == whole string
                // yend == last /thing
                std::size_t found = y.find_last_of("/");
                //std::cout << " path: " << str.substr(0,found) << '\n';
                std::string yend = y.substr(found + 1);

                if (0) FPS_ERROR_PRINT(" %s >> sysvec y [%s] yend [%s]\n", __func__, y.c_str(), yend.c_str());
                for (auto z : xVec)
                {
                    if ((y == z) || (z.find(yend) != std::string::npos))
                    {
                        //if(z[0]!='_')
                        {
                            if (0) FPS_ERROR_PRINT(" %s >> >> yVec push_back [%s]\n", __func__, z.c_str());
                            yVec.push_back(z);
                        }
                    }
                }
                //nVec.push_back(x.first);
            }
            yVecp = &yVec;

            if (0) FPS_ERROR_PRINT(" %s >> yVec size [%d]\n", __func__, (int)yVec.size());
        }
        for (auto& y : *yVecp)
        {
            std::string suri = uri;
            auto x = y.find(suri);
            if (0) FPS_ERROR_PRINT(" %s >> >> >> yVec option [%s] x %d\n", __func__, y.c_str(), (int)x);
            if (x == 0)
            {
                if (0) FPS_ERROR_PRINT(" %s >> >> >> nVec push_back [%s]\n", __func__, y.c_str());
                nVec.push_back(y);
            }
        }
        int rc = (int)nVec.size();
        if (0) FPS_ERROR_PRINT(" %s >> nVec size [%d]\n", __func__, rc);
        return rc;
    }



bool strMatch(const char* str, const char* key)
{
    //char* key2=nullptr;
    if (0)printf("  Match test  for str [%s] key [%s] \n", str, key);

    //char rep = 0;
    int mlen = 0;
    if (key) mlen = strlen(key);
    if (mlen == 0 || (mlen > 0 && key[0] == '*'))
    {
        if (0)printf(" >>>> Match true  for str [%s] key [%s] \n", str, key);
        return true;
    }
    if (!str)
    {
        FPS_ERROR_PRINT(" %s >>>>  no str for match  key [%s] \n", __func__, key);
        return false;
    }
    if (strncmp(str, key, mlen) == 0)
    {
        if (0)printf("     >>>> Match true  for str [%s] key [%s] mlen %d \n", str, key, mlen);
        return true;
    }

    // rep = key[mlen-1];
    // if (rep == '*')
    // {
    //     key2 = strdup(key);
    //     mlen--;
    //     key2[mlen] = 0;
    //     if (strncmp(str, key2, mlen)== 0)
    //     {
    //         Match = true;
    //         if(0)printf(" >>>> Match true  for str [%s] key [%s] \n", str, key);

    //     }
    //     free((void *)key2);
    // }
    // else
    // {
    //     if (strcmp(str, key)== 0)
    //     {
    //         Match = true;
    //         if(0)printf("  >>>> Match true  for str [%s] key [%s] \n", str, key);
    //     }
    // }
    return false;
}

// so here's the thing  rules for set

// simple /set a/b/c {"component":{"value":1234},.....}  Yes
// naked /set a/b/c {"component":1234, ...}  YES
// single    set /a/b/c/component 1234        NO                           /a/b/c/component is mot a table /a/b/c is sor set/add component to /a/b/c
// perhaps       /a/b/c/component '{"value":1234}'    YES

//   get     /a/b/c/component      NO Error
//   get     /a/b/c        Ok but we should search anyway  Error

// get /a/b/c/ should return just /a/b/c

// TODO lock varmap    
// get one or get them all
// added assetList concept for uiObjects to keep order

// global search  given a uri get a list of comps that match it
// /assets  -> /assets/ess /assets/bms /assets/bms_1    etc 
void addCjFrags(cJSON* cj, const char* uri, cJSON* junk)
{
    int nfrags = get_nfrags(uri);
    char* suri = strdup(uri);
    cJSON* cji = cj;
    cJSON* cjf = cj;
    while (nfrags > 0)
    {
        char* firsturi = pull_first_uri(suri);
        if (nfrags == 1)
        {
            cjf = junk;
        }
        else
        {
            cjf = cJSON_GetObjectItem(cji, &firsturi[1]);
        }

        if (!cjf)
        {
            cjf = cJSON_CreateObject();
        }
        cJSON_AddItemToObject(cji, &firsturi[1], cjf);

        nfrags--;
        suri += (strlen(firsturi));
        cji = cjf;
    }
    free((void*)suri);
}

// opts 0x0000     default , full comps , value
// opts 0x0001     default , full comps , naked
// opts 0x0010     dump object , full comps , value
// opts 0x0100     dump object , reduced , value

cJSON* getMapsCj(varsmap& vmap, const char* inuri = nullptr, const char* var = nullptr, int opts = 0, const char* origuri = nullptr, cJSON* cji = nullptr)
    {
        if (0)FPS_ERROR_PRINT("%s >> getting cj maps uri [%s] var [%s] opts 0x%04x\n"
            , __func__
            , inuri ? inuri : "noURI"
            , var ? var : "noVar"
            , opts
        );
        int found = 0;
        cJSON* cj = nullptr;  //cJSON_CreateObject();
        char* uri = (char*)inuri;
        // if(uri)
        // {
        //     if(strstr(uri,"/naked"))
        //     {
        //         if(0)FPS_ERROR_PRINT("%s >> stripping /naked off the uri [%s] \n", __func__, inuri);
        //         uri = strdup(&inuri[strlen(uri) - strlen("/naked")]);
        //         if(0)FPS_ERROR_PRINT("%s >> striped /naked off the uri now[%s] \n", __func__, uri);
        //         opts = 1;
        //     }
        //     if(0)FPS_ERROR_PRINT("%s >> using uri [%s] var [%s]\n", __func__, uri, var?var:"noVar");
        // }
        // we want a whole table at least one 
               // get /assets/bms should retun
    //     {
    // "summary": {...},
    // "bms_1": {...},
    // "bms_2": {...},
    // "bms_3": {...},
    // "bms_4": {...},
    // )
 // path 1 uri and no var 
 // NOTE this will return the table name as the first object.

        if (uri && !var)
        {

            std::vector<std::string> nVec;
            std::string bs;
            //int rc = 
            createAssetListCj(vmap, uri, sysVec, opts, nVec);
            cJSON* cjm = createUriListCj(vmap, bs, uri, nullptr, opts, nVec);
            {
                char* tmp = cJSON_Print(cjm);
                if (0)FPS_ERROR_PRINT("%s >> used createUriList from uri [%s] basevec [%s] tmp \n>>%s<<\n", __func__, uri, bs.c_str(), tmp);
                //if (uri != inuri) free((void*)uri);
                if (tmp) free((void*)tmp);
                cJSON* cjx = cJSON_CreateObject();
                cJSON_AddItemToObject(cjx, bs.c_str(), cjm);
                if (uri != inuri) free((void*)uri);
                return cjx;
                //cJSON_Delete(cjm);
            }

            
        }
        // path 2 we have a uri and a var
        else if (uri && var)
        {
            auto x = vmap.find(uri);
            if (x != vmap.end())
            {
                auto y = vmap[uri].find(var);
                if (y != vmap[uri].end())
                {
                    found++;
                    cJSON* cji = cJSON_CreateObject();
                    if (0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] found %d \n", __func__, uri, y->first.c_str(), found);
                    // TODO need opts here 
                    y->second->showvarValueCJ(cji, opts);
                    // char* tmp = cJSON_PrintUnformatted(cji);
                    // if(tmp)
                    // {
                    //     if(0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] as [%s] \n", __func__, uri, y->first.c_str(), tmp);
                    //     free((void *)tmp);
                    // }
                    cJSON* cj = cji;
                    if (opts == 1 && cji->child)
                    {
                        cj = cJSON_Duplicate(cji->child, true);
                        // tmp = cJSON_PrintUnformatted(cji->child);
                        // if(tmp)
                        // {
                        //     if(0)FPS_ERROR_PRINT(" %s >> final  cj child for uri [%s] var [%s] as [%s] \n", __func__
                        //             , uri
                        //             , var?var:"noVar"
                        //             , tmp);
                        //     free((void *)tmp);
                        // }
                        cJSON_Delete(cji);
                    }
                    if (uri != inuri) free((void*)uri);
                    return cj;

                }
            }
        }
        else
            // get all the objects
        {
            cJSON* cj = cJSON_CreateObject();

            if (0)FPS_ERROR_PRINT(" %s >>  get them all opts [0x%04x] \n"
                , __func__
                , opts
            );
            int found = 0;

            for (auto& x : vmap)
            {
                // if(0)FPS_ERROR_PRINT(" %s >>  get them all comp [%s] \n"
                // , __func__
                // , x.first.c_str()
                // );

                //cJSON* cjx = loadAssetList(vmap, x.first.c_str(), found, opts);
                if (0)FPS_ERROR_PRINT(" %s >>  get them all comp [%s]  assetlists \n"
                    , __func__
                    , x.first.c_str()
                );

                cJSON* cji = cJSON_CreateObject();
                assetList* alist = getAlist(vmap, x.first.c_str());

                if (0)FPS_ERROR_PRINT(" %s >>       >>>>>>>>>>>>>>>>>>>>query alist >>>>>>>>>>>>>>>>>>>>>>>>>>getting cj for comp [%s]  alist %p\n"
                        , __func__
                        , x.first.c_str()
                        , (void*)alist
                        );
                if (alist)
                {
                    unsigned int ix = 0;
                    assetVar* av;
                    do
                    {
                        av = alist->avAt(ix++);
                        if (av) av->showvarCJ(cji, opts);
                    } while (av);
                    found++;
                }
                else
                {
                    for (auto& y : vmap[x.first])
                    {
                        if (y.second != nullptr)
                        {
                            y.second->showvarCJ(cji, opts);
                            found++;
                        }
                        else
                        {
                            FPS_ERROR_PRINT(" %s >>       NOTE no map for var [%s] \n", __func__, y.first.c_str());
                        }
                    }
                }
                // shoud really compare x.first.c.str() with cji->string
                //cJSON_AddItemToObject(cj, x.first.c_str(), cji->child?cji->child:cji);
                // if (opts & 0x0001)
                // {
                //     // TODO get showvarCJ to work properly for naked stuff
                //     cJSON* cjii = cJSON_Duplicate(cji->child, true);
                //     cJSON_AddItemToObject(cj, x.first.c_str(), cjii);
                //     cJSON_Delete(cji);
                // }
                // else
                {
                    cJSON_AddItemToObject(cj, x.first.c_str(), cji);

                }
                found++;
            }
            if (cj)
            {
                char* tmp = cJSON_PrintUnformatted(cj);
                if (tmp)
                {
                    if (0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] as [%s] \n", __func__
                        , uri
                        , var ? var : "noVar"
                        , tmp);
                    free((void*)tmp);
                }
            }
            return cj;
        }
        if (0)printf(" <<<<<<<<<<<<<<<<<<<<<<<<<<<got cj %p maps  found [%d]\n", (void*)cj, found);
        if (found == 0)
        {
            cJSON_Delete(cj);
            cj = nullptr;
        }
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (tmp)
            {
                if (0)FPS_ERROR_PRINT(" %s >> getting cj for uri [%s] var [%s] as [%s] \n", __func__
                    , uri
                    , var ? var : "noVar"
                    , tmp);
                free((void*)tmp);
            }
        }
        if (uri != inuri) free((void*)uri);
        return cj;
    }



int main(int argc, char* argv[])
{
    printf("I am running\n");

    // // this is our main data map
    varsmap vmap;


    // // this is our map utils factory
    VarMapUtils vm;
    int rc;
    const char* var1 = "{\"start_stop\":{\"value\":0,"
        "\"actions\":{"
        "\"onSet\":{"
        "\"bitfield\":["
        "{ \"inValue\":0, \"uri\": \"/system/new_controls\",\"var\":\"oncmd\",       \"outValue\": \"nice\" },"
        "{ \"inValue\":1, \"uri\": \"/system/new_controls\",\"var\":\"kacclosecmd\", \"outValue\": 34.5 },"
        "{ \"inValue\":8, \"uri\": \"/system/new_controls\",\"var\":\"offcmd\",       \"outValue\": true },"
        "{ \"inValue\":9, \"uri\": \"/system/new_controls\",\"var\":\"kacopencmd\",       \"outValue\": true }"
        "]"
        "}"
        "}"
        "}"
        "}";
    const char* rep1 = "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"bitfield\":["
        "{\"inValue\":0,\"uri\":\"/system/new_controls\",\"var\":\"oncmd\",\"outValue\":\"nice\"},"
        "{\"inValue\":1,\"uri\":\"/system/new_controls\",\"var\":\"kacclosecmd\",\"outValue\":34.5},"
        "{\"inValue\":8,\"uri\":\"/system/new_controls\",\"var\":\"offcmd\",\"outValue\":true},"
        "{\"inValue\":9,\"uri\":\"/system/new_controls\",\"var\":\"kacopencmd\",\"outValue\":true}"
        "]}}}}";

    const char* rep2 = "{\"/assets/bms_1\":"
        "{\"start_stop\":{\"value\":0,\"actions\":{\"onSet\":{\"bitfield\":["
        "{\"inValue\":0,\"outValue\":\"nice\",\"uri\":\"/system/new_controls\",\"var\":\"oncmd\"},"
        "{\"inValue\":1,\"outValue\":34.5,\"uri\":\"/system/new_controls\",\"var\":\"kacclosecmd\"},"
        "{\"inValue\":8,\"outValue\":true,\"uri\":\"/system/new_controls\",\"var\":\"offcmd\"},"
        "{\"inValue\":9,\"outValue\":true,\"uri\":\"/system/new_controls\",\"var\":\"kacopencmd\"}"
        "]}}}}}";


    rc = vm.testRes(" Test 1", vmap
        , "set"
        , "/components/bms_1"
        , var1
        , rep1
    );

    {
        cJSON* cj = vm.getMapsCj(vmap);
        char* res = cJSON_Print(cj);
        printf("vmap after test 1 \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
        //return 0;
    }

    rc = vm.testRes(" Test 2", vmap
        , "get"
        , "/components/bms_1"
        , nullptr
        , rep2
    );

    rc = vm.testRes(" Test 3", vmap
        , "set"
        , "/components/bms_1/start_stop"
        , "4"
        , "{\"start_stop\":4}"
    );

    rc = vm.testRes(" Test 4", vmap
        , "set"
        , "/components/bms_1"
        , "{\"start_stop\":3}"
        , "{\"start_stop\":3}"
    );

    rc = vm.testRes(" Test 5", vmap
        , "get"
        , "/system/new_controls/kacclosecmd"
        , nullptr
        , "{\"value\":34.5}"
    );

    rc = vm.testRes(" Test 6", vmap
        , "get"
        , "/system/new_controls/oncmd"
        , nullptr
        , "{\"value\":\"nice\"}"
    );


    // rc = vm.testRes(" Test 1", vmap
    //         ,"set"
    //         ,"/components/vmap_test"
    //         ,"{\"test_int\": 123,\"test_float\":456.78,\"test_string\":\"some string thing\" }"
    //         ,"{\"test_int\":123,\"test_float\":456.78,\"test_string\":\"some string thing\"}"
    //     );
    // rc = vm.testRes(" Test 1a", vmap
    //         ,"set"
    //         ,"/components/vmap_test"
    //         ,"{\"test_int\":123,\"test_float\":456.78,\"test_string2\":{\"value\":\"some string value\"}}"
    //         ,"{\"test_int\":123,\"test_float\":456.78,\"test_string2\":{\"value\":\"some string value\"}}"
    //     );

    // rc = vm.testRes(" Test 2", vmap
    //         ,"set"
    //         ,"/system/status"
    //         ,"{\"status\": \"standby\",\"soc\":100,\"active_current_setpoint\":2000 }"
    //         ,"{\"status\":\"standby\",\"soc\":100,\"active_current_setpoint\":2000}"
    //     );

    // rc = vm.testRes(" Test 3", vmap
    //         , "get"
    //         , "/system/status"
    //         , nullptr
    //         , "{\"active_current_setpoint\":{\"value\":2000},\"soc\":{\"value\":100},\"status\":{\"value\":\"standby\"}}"
    //     );

    // rc = vm.testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );



// rc = vm.testRes(" Test 4", vmap
    //         , "get"
    //         , "/system/status/soc"
    //         , nullptr
    //         , "{\"value\":100}"
    //     );
    {
        cJSON* cj = /*vm.*/getMapsCj(vmap);
        char* res = cJSON_Print(cj);

        printf("vmap at end default options \n%s\n", res);
        rc = 0; // -Wall
        if (rc == 0) free((void*)res);
        cJSON_Delete(cj);
    }
    //     vmap at end default options
    // {
    //         "/components/bms_1":    {
    //                 "start_stop":   {
    //                         "value":        3
    //                 }
    //         },
    //         "/components/bms_1/start_stop": {
    //                 "start_stop":   {
    //                         "value":        4
    //                 }
    //         },
    //         "/system/new_controls": {
    //                 "kacclosecmd":  {
    //                         "value":        34.5
    //                 },
    //                 "kacopencmd":   {
    //                         "value":        true
    //                 },
    //                 "offcmd":       {
    //                         "value":        true
    //                 },
    //                 "oncmd":        {
    //                         "value":        "nice"
    //                 }
    //         }
    // }

    {
        cJSON* cj = /*vm.*/getMapsCj(vmap, nullptr, nullptr, 0x0010);
        char* res = cJSON_Print(cj);

        printf("vmap at end dump options \n%s\n", res);
        rc = 0; // -Wall
        if (rc == 0) free((void*)res);
        cJSON_Delete(cj);
    }
    // vmap at end dump options
    // {
    //     "/components/bms_1":    {
    //             "start_stop":   {
    //                     "value":        3,
    //                     "actions":      {
    //                             "onSet":        {
    //                                     "bitfield":     [{
    //                                                     "inValue":      0,
    //                                                     "outValue":     "nice",
    //                                                     "uri":  "/system/new_controls",
    //                                                     "var":  "oncmd"
    //                                             }, {
    //                                                     "inValue":      1,
    //                                                     "outValue":     34.5,
    //                                                     "uri":  "/system/new_controls",
    //                                                     "var":  "kacclosecmd"
    //                                             }, {
    //                                                     "inValue":      8,
    //                                                     "outValue":     true,
    //                                                     "uri":  "/system/new_controls",
    //                                                     "var":  "offcmd"
    //                                             }, {
    //                                                     "inValue":      9,
    //                                                     "outValue":     true,
    //                                                     "uri":  "/system/new_controls",
    //                                                     "var":  "kacopencmd"
    //                                             }]
    //                             }
    //                     }
    //             }
    //     },
    //     "/components/bms_1/start_stop": {
    //             "start_stop":   {
    //                     "value":        4
    //             }
    //     },
    //     "/system/new_controls": {
    //             "kacclosecmd":  {
    //                     "value":        34.5
    //             },
    //             "kacopencmd":   {
    //                     "value":        true
    //             },
    //             "offcmd":       {
    //                     "value":        true
    //             },
    //             "oncmd":        {
    //                     "value":        "nice"
    //             }
    //     }
    // }

    {
        cJSON* cj = /*vm.*/getMapsCj(vmap, "/components/bms_1");
        char* res = cJSON_Print(cj);
        printf("components/bms_1 default at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    // this is correct I think  but we did not get the child 
    //"/components/bms_1/start_stop": {
    //             "start_stop":   {
    //                     "value":        4
    //             }

    //We got this 
    //     components/bms_1 default at end
    // {
    //         "start_stop":   {
    //                 "value":        3
    //         }
    // }

    //We should have got  this
    // but we may need to add a search option 0x1000 to force the full expansion. 
    //     components/bms_1 default at end
    // {
    //         "start_stop":   {
    //                 "value":        3,
    //                 "start_stop": {
    //                         "value":        4,
    //                             }
    //         }
    // }

    {
        cJSON* cj = /*vm.*/getMapsCj(vmap, "/components");
        char* res = cJSON_Print(cj);
        printf("components  (should trigger a search)  default at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    // {
       //     "/components/bms_1/start_stop": {
       //             "start_stop":   {
       //                     "value":        4
       //             }
       //     },
       //     "/components/bms_1":    {
       //             "start_stop":   {
       //                     "value":        3
       //             }
       //     }
       // }

    {
        cJSON* cj = /*vm.*/getMapsCj(vmap, "/components", nullptr, 0x0100);
        char* res = cJSON_Print(cj);
        printf("components  (should trigger a search)  reduced comp at end \n%s\n", res);
        free((void*)res);
        //unstable 
        cJSON_Delete(cj);
    }
    // this is probably broken we get this ..... sigh
     // components  (should trigger a search)  reduced comp at end
     // {
     //         "bms_1":        {
     //                 "start_stop":   {
     //                 }
     //         }
     // }
   // this is probably broken we should get this
     // components  (should trigger a search)  reduced comp at end
     // {
     //         "bms_1":        {
     //
     //                 "start_stop":   {
     //                      "value": 3,
     //                        "start_stop": {
     //                            "value":4
     //                            }
     //                 }
     //         }
     // }

    {
        cJSON* cj = /*vm.*/getMapsCj(vmap, "/components", nullptr, 0x0101);
        char* res = cJSON_Print(cj);
        printf("components  (should trigger a search) naked ,  reduced comp at end \n%s\n", res);
        free((void*)res);
        //unstable 
        cJSON_Delete(cj);
    }
    // this is probably broken we get this ..... sigh
 // components  (should trigger a search)  reduced comp at end
 // {
 //         "bms_1":        {
 //                 "start_stop":   {
 //                 }
 //         }
 // }
// this is probably broken we should get this  actually for now we cannot represent the strucure properly 
// need to change the loader...
//but the values should be naked...
  // components  (should trigger a search)  reduced comp at end
  // {
  //         "bms_1":        {
  //
  //                 "start": 3,
  //                 "start_stop": {"start":4}
  //                 }
  //         }
  // }


// // delete bms_man;
    vm.clearVmap(vmap);

    return 0;

}
