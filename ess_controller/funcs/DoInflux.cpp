// DoInflux.cpp
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "formatters.hpp"

#ifndef FPS_ERROR_FMT
#define FPS_ERROR_FMT(...) fmt::print(stderr, __VA_ARGS__)
#endif
#define closesocket close

// need to make sure SendDb is registered as a function
// fims_send -m set -r /$$ -u /ess/control/pubs '
// {
//   "SendDb":{"value":0,
//     "ifChanged":false,
//     "actions":{"onSet":[{
//          "func":[{"func":"SendDb","amap":essName}]
//             }]}}
// }'

// fims_send -m set -r /$$ -u /ess/system/dbtest '
// {
//   "int1": 1,
//   "float1": 2.3,
//   "string1": "four"
// }'

// fims_send -m set -r /$$ -u /ess/control/pubs '
// {
//   "SendDb":{"value":1,
//             "db":"pirates",
//             "measure":"test_meas",
//             "table":"/system/dbtest"
//            }
// }'
//
// this will need some more of flex pack stuff integrated
// fims_send -m set -r /$$ -u /flex/full/system/commands/run '
//                     {"value":22,"uri":"/control/pubs:SendDb",
//                           "every":1.0,"offset":0,"debug":0}'

#include "asset.h"

extern "C++" {
int checkAv(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

int findSocket(varsmap& vmap, VarMapUtils* vm, char* host, int port)
{
    int sock = -1;
    char* vname;
    vm->vmlen = asprintf(&vname, "/sockets/tcp:%s_%04d", host, port);
    assetVar* av = vm->getVar(vmap, vname, nullptr);
    if (av)
    {
        sock = av->getiVal();
    }
    free(vname);
    return sock;
}

int setSocket(varsmap& vmap, VarMapUtils* vm, char* host, int port, int sock)

{
    char* vname;
    vm->vmlen = asprintf(&vname, "/sockets/tcp:%s_%04d", host, port);
    vm->setVal(vmap, vname, nullptr, sock);
    free(vname);
    return 0;
}

// Very rough , needs to use channels
// Get a time stamp in nanoseconds.
#include <time.h>
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec)*1000000000)

uint64_t nanos()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    uint64_t ns = SEC_TO_NS((uint64_t)ts.tv_sec) + (uint64_t)ts.tv_nsec;
    return ns;
}
// needs this
// sock = findSocket(vmap, vm, host,port);  .. done see above
// TODO split this out into the send function and a getFunction so we dont block
int PostHtml(varsmap& vmap, VarMapUtils* vm, char* host, int port, std::string hdr, std::string msg, assetVar* aV)
{
    std::string response;
    struct iovec iv[2];
    struct sockaddr_in addr;
    int sock, len = 0;  // ,ret_code = 0;//, content_length = 0, len = 0;
    // sock = findSocket(vmap, vm, host, port);
    // if(sock < 0)
    {
        // char ch;
        // unsigned char chunked = 0;

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if ((addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        {
            FPS_ERROR_FMT("{} >> inet_addr {} : {} failed\n", __func__, host, port);
            return -1;
        }
        if ((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
        {
            FPS_ERROR_FMT("{} >> sock socket {} : {} failed\n", __func__, host, port);
            return -2;
        }
        if (connect(sock, (struct sockaddr*)(&addr), sizeof(addr)) < 0)
        {
            closesocket(sock);
            FPS_ERROR_FMT("{} >> sock connect {} : {} failed\n", __func__, host, port);
            std::string err = fmt::format("{} >> sock connect {} : {} failed", __func__, host, port);
            aV->setParam("error", err.c_str());
            return -3;
        }
        int enable = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        {
            FPS_ERROR_FMT("{} >> setsockopt(SO_REUSEADDR) failed\n", __func__);
        }
        setSocket(vmap, vm, host, port, sock);
    }
    // else
    // {
    //     FPS_ERROR_FMT("{} >> found socket \n",__func__);

    // }

    iv[0].iov_base = (void*)hdr.c_str();
    iv[0].iov_len = hdr.size();
    iv[1].iov_base = (void*)msg.c_str();
    iv[1].iov_len = msg.size();

    if (writev(sock, iv, 2) < (int)(iv[0].iov_len + iv[1].iov_len))
    {
        FPS_ERROR_FMT("{} >> write sock failed \n", __func__);
        sock = -1;
    }
    else
    {
        response.resize(len = 0x100);

        iv[0].iov_len = len;

        // TODO get a proper response
        iv[0].iov_len = recv(sock, &response[0], response.length(), len = 0);
        //     == size_t(-1))
        FPS_ERROR_PRINT(" %s >> received %d chars [%s] \n", __func__, (int)iv[0].iov_len, response.c_str());
    }
    closesocket(sock);

    return sock;
}

int RecvDb(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    std::string response;
    int debug = 0;

    if (!checkAv(vmap, amap, aname, p_fims, aV))
    {
        return -1;
    }
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

    char* host = (char*)"127.0.0.1";
    int sock = -1;

    if (aV->gotParam("debug"))
    {
        debug = aV->getiParam("debug");
    }

    int port = 8086;
    if (aV->gotParam("host"))
    {
        host = aV->getcParam("host");
    }

    if (aV->gotParam("port"))
    {
        port = aV->getiParam("port");
    }
    sock = findSocket(vmap, vm, host, port);
    // if(aV->gotParam("sock"))
    // {
    //     sock = aV->getiParam("sock");
    // }

    if (sock > -1)
    {
        int len;
        int rc = 0;
        // we may need to expand this, it only gets 256 bytes
        // but the header len should be in th first packet
        // so we can recv again to get the rest.
        response.resize(len = 0x100);

        // TODO get a proper response
        rc = (int)recv(sock, &response[0], response.length(), len = 0);
        if (rc == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                return 0;
            }
            else
            {
                closesocket(sock);
                sock = -1;
                setSocket(vmap, vm, host, port, sock);
            }
        }
        if (rc > 0)
        {
            if (debug)
                FPS_ERROR_PRINT(" %s >> received %d chars [%s] \n", __func__, (int)rc, response.c_str());
        }
    }
    // char* host = (char*) "127.0.0.1";

    // int port = 8086;
    // if(aV->gotParam("host"))
    // {
    //     host = aV->getcParam("host");
    // }

    // if(aV->gotParam("port"))
    // {
    //     port = aV->getiParam("port");
    // }
    // if(!aV->gotParam("host"))
    // {
    //     FPS_ERROR_PRINT("%s >> Please provide table param for [%s]\n"
    //     , __func__
    //     , aV->getfName()
    //     );
    return 0;
}
//
// TODO Well have SendDB and RxDb functions
// if we dont have a connection  then create one
// the PostHtml will write to the out channel only ( to be modified)
// incoming messages should just be OK
// we may need a create db
//
//
int SendDb(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    int debug = 1;

    if (!checkAv(vmap, amap, aname, p_fims, aV))
    {
        return -1;
    }
    char* host = (char*)"127.0.0.1";
    int port = 8086;
    if (aV->gotParam("host"))
    {
        host = aV->getcParam("host");
    }

    if (aV->gotParam("port"))
    {
        port = aV->getiParam("port");
    }

    // if(!aV->gotParam("host"))
    // {
    //     FPS_ERROR_PRINT("%s >> Please provide host param for [%s]\n"
    //     , __func__
    //     , aV->getfName()
    //     );
    //     return 0;
    // }
    if (!aV->gotParam("table"))
    {
        FPS_ERROR_PRINT("%s >> Please provide table param for [%s]\n", __func__, aV->getfName());
        return 0;
    }
    if (!aV->gotParam("db"))
    {
        FPS_ERROR_PRINT("%s >> Please provide db param for [%s]\n", __func__, aV->getfName());
        return 0;
    }
    if (!aV->gotParam("measure"))
    {
        FPS_ERROR_PRINT("%s >> Please provide measure for [%s]\n", __func__, aV->getfName());
        return 0;
    }

    char* db = aV->getcParam("db");
    char* table = aV->getcParam("table");
    char* measure = aV->getcParam("measure");
    essPerf ePerf(am, "flex_influx", table, nullptr);

    if (debug)
        FPS_ERROR_PRINT("%s >> Processing [%s] publishing db [%s] table [%s] as [%s]\n", __func__, aV->getfName(), db,
                        table, measure);
    // create the string
    std::stringstream msg;
    if (vmap.find(table) != vmap.end())
    {
        msg << measure;
        msg << ",mytag=1";
        bool first = true;
        for (auto x : vmap[table])
        {
            assetVar* av = x.second;
            if (first)
            {
                first = false;
                msg << " ";
            }
            else
            {
                msg << ",";
            }
            msg << av->name;
            msg << "=";

            switch (av->type)
            {
                case assetVar::ASTRING:
                    msg << "\"" << av->getcVal() << "\"";
                    break;
                case assetVar::AINT:
                case assetVar::AFLOAT:
                    msg << av->getiVal();
                    break;

                case assetVar::ABOOL:
                    if (av->getbVal())
                    {
                        msg << "true";
                    }
                    else
                    {
                        msg << "false";
                    }
                    break;
                default:
                    msg << "Undef";
                    break;
            }
        }
        msg << " " << nanos();
    }
    std::stringstream hdr;
    hdr << "POST http://localhost:8086/write?";
    hdr << "db=" << db;
    hdr << "&p=s";
    hdr << " HTTP/1.1\r\n";
    hdr << "Host: localhost:8086 \r\n";
    hdr << "Content-Type: application/x-www-form-urlencoded\r\n";
    hdr << "Accept: */*\r\n";
    hdr << "Content-Length: " << msg.str().size() << "\r\n\r\n";

    // POST "http://localhost:8086/write?db=mydb&precision=s"
    //%s /%s?db=%s&u=%s&p=%s&epoch=%s%s HTTP/1.1\r\nHost: %s\r\nContent-Length:
    //%d\r\n\r\n",
    //               method, uri, si.db_.c_str(), si.usr_.c_str(),
    //               si.pwd_.c_str(), si.precision_.c_str(),
    FPS_ERROR_PRINT("%s >> created hdr [%s]\n", __func__, hdr.str().c_str());
    FPS_ERROR_PRINT("%s >> created message [%s]\n", __func__, msg.str().c_str());
    int sock = PostHtml(vmap, vm, host, port, hdr.str(), msg.str(), aV);
    aV->setParam("sock", sock);

    // SendPub(am, table, sendas);
    return 0;
}