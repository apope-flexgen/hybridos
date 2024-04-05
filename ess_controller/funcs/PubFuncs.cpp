#ifndef PUB_FUNCTIONS_CPP
#define PUB_FUNCTIONS_CPP
#include "asset.h"
#include "formatters.hpp"
#include "scheduler.h"

extern "C++" {
int checkAv(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void SendPub(varsmap& vmap, asset_manager* am, const char* uri, const char* puri, assetVar* aV);
int RunPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

// Very rough , needs to use channels
// Get a time stamp in nanoseconds.
#include <time.h>
int RunPub(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    asset_manager* am = aV->am;

    int debug = false;
    if (!checkAv(vmap, amap, aname, p_fims, aV))
    {
        return -1;
    }
    VarMapUtils* vm = am->vm;

    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }
    char* table = nullptr;
    if (aV->gotParam("table"))
    {
        if (debug)
            FPS_PRINT_INFO("{} >> Found table param for   [{}]", __func__, aV->getfName());
        table = aV->getcParam("table");
    }
    if (!table)
    {
        if (aV->gotParam("uri"))
        {
            if (debug)
                FPS_PRINT_INFO("{} >> Found uri param for   [{}]", __func__, aV->getfName());
            table = aV->getcParam("uri");
        }
    }
    if (table == nullptr)
    {
        FPS_PRINT_ERROR("{} >> please provide table or  uri param for   [{}]", __func__, aV->getfName());
        return 0;
    }

    essPerf ePerf(am, "flex_pub", table, nullptr);
    assetVar* runaV = vm->getVar(vmap, table, nullptr);
    if (runaV)
    {
        char* pval = runaV->getcVal();

        FPS_PRINT_INFO("  config var found   [{}] value [{}]", runaV->getfName(), pval ? pval : "no Value");
        if (pval)
        {
            std::vector<std::string> amVec;
            int nmaps = vm->uriSplit(amVec, pval, ",");
            nmaps = (int)amVec.size();

            FPS_PRINT_INFO("  config var value  [{}], nmaps [{}]", pval, nmaps);
            int idx = 0;
            while (nmaps > idx)
            {
                FPS_PRINT_INFO("  idx     [{}] pub [{}]", idx, amVec[idx]);
                SendPub(vmap, am, amVec[idx].c_str(), nullptr, aV);

                idx++;
            }
        }
        return 0;
    }

    char* sendas = nullptr;
    if (aV->gotParam("sendas"))
    {
        sendas = aV->getcParam("sendas");
    }
    if (debug)
        FPS_PRINT_ERROR("{} >> Processing [{}] publishing table [{}] as [{}]", __func__, aV->getfName(), table,
                        sendas ? sendas : "No sendas");
    SendPub(vmap, am, table, sendas, aV);
    return 0;
}

// SendPub
// uri is either a table or a uri
void SendPub(varsmap& vmap, asset_manager* am, const char* uri, const char* puri, assetVar* aV)
{
    bool debug = false;
    if (aV->gotParam("debug"))
    {
        debug = aV->getbParam("debug");
    }
    const char* mode = "{:u}";
    char* pmode = nullptr;

    if (aV->gotParam("mode"))
    {
        pmode = aV->getcParam("mode");
        if (pmode)
        {
            if (strcmp(pmode, "full") == 0)
            {
                mode = "{:f}";
            }
            else if (strcmp(pmode, "naked") == 0)
            {
                mode = "{:n}";
            }
            else if (strcmp(pmode, "clothed") == 0)
            {
                mode = "{:c}";
            }
            else if (strcmp(pmode, "ui") == 0)
            {
                mode = "{:u}";
            }
        }
    }

    // fmt::basic_memory_buffer<char, 10000> buf; // for some reason this doesn't
    // work. Might be a compiler bug?
    std::string output;  // I would like to use the buf but it appears to not want
                         // to work with the buf for some reason?

    char* tmp = nullptr;

    // we may be missing a leading '/'
    std::string suri;
    if (uri[0] != '/')
    {
        suri = "/";
    }
    suri.append(uri);
    std::string wild = "##";

    size_t pos = suri.rfind(wild);
    if (pos != std::string::npos)
    {
        // If found then erase it from string
        suri.erase(pos, wild.length());
        if (debug)
            FPS_PRINT_INFO(" now seeking [{}]", suri);
        for (auto xx : vmap)
        {
            if (debug)
                FPS_PRINT_INFO(" current table [{}] suri [{}]", xx.first, suri);
            if (strncmp(xx.first.c_str(), suri.c_str(), suri.size()) == 0)
            {
                if (debug)
                    FPS_PRINT_INFO(" found [{}]", xx.first);
                SendPub(vmap, am, xx.first.c_str(), puri, aV);
            }
        }
        return;
    }

    {
        auto map = vmap.find(suri.c_str());
        if (map != vmap.end())
        {
            output = std::move(fmt::format(mode, map->second));
        }
        else
        {
            auto aV = am->vm->getVar(vmap, suri.c_str(), nullptr);
            if (aV)
            {
                output = std::move(fmt::format(mode, aV));
                if (!puri)
                {
                    puri = aV->comp.c_str();
                }
            }
            else
            {
                output = std::move(fmt::format(
                    R"({{"{}":{{"value":"uri {} nonexistent"}}}})", suri, suri));
            }
        }
    }
    if (!puri)
    {
        puri = suri.c_str();
    }
    int rc = 0;
    int size = (int)output.size();
    if (tmp)
    {
        size = strlen(tmp);
        rc = (int)am->vm->p_fims->Send("pub", puri, nullptr, tmp);
        free(tmp);
    }
    else
    {
        rc = (int)am->vm->p_fims->Send("pub", puri, nullptr, output.c_str());
    }
    if (rc <= 0)
    {
        FPS_PRINT_ERROR("Pub Failed rc {} size {}", rc, size);
        // ooh do we want to crash here ??
        exit(0);
    }
}
#endif
