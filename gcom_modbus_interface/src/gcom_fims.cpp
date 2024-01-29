// gcom_fims.cpp
// p. wilshire
// 11_08_2023
// self review 11_23_2023

#include <chrono>
#include <thread>
#include <iostream>
#include <any>
#include <future>
#include <sys/uio.h>    // for receive timouet calls
#include <sys/socket.h> // for receive timouet calls

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <errno.h>  //For errno - the error number

#include "fims/libfims.h"
#include "fims/defer.hpp"

#include "gcom_config.h"
#include "gcom_iothread.h"
#include "gcom_fims.h"
#include "logger/logger.h"
#include "shared_utils.h"

using namespace std::chrono_literals;
using namespace std::string_view_literals; // for sv

std::thread fimsThread;
std::thread processThread1;
std::thread processThread2;
std::thread processThread3;
std::thread collectThread1;

uint64_t set_any_to_uint64(struct cfg &myCfg, std::shared_ptr<cfg::io_point_struct> io_point, std::any val);
void get_stats(std::stringstream &ss, struct cfg &myCfg);


// fims helper functions:
bool send_pub(fims &fims_gateway, std::string_view uri, std::string_view body) noexcept
{
    return fims_gateway.Send(fims::str_view{"pub", sizeof("pub") - 1}, fims::str_view{uri.data(), uri.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}, fims::str_view{body.data(), body.size()});
}
bool send_set(fims &fims_gateway, std::string_view uri, std::string_view body) noexcept
{
    return fims_gateway.Send(fims::str_view{"set", sizeof("set") - 1}, fims::str_view{uri.data(), uri.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}, fims::str_view{body.data(), body.size()});
}
// NOTE(WALKER): This is only for emit_event really (not used anywhere else)
bool send_post(fims &fims_gateway, std::string_view uri, std::string_view body) noexcept
{
    return fims_gateway.Send(fims::str_view{"post", sizeof("post") - 1}, fims::str_view{uri.data(), uri.size()}, fims::str_view{nullptr, 0}, fims::str_view{nullptr, 0}, fims::str_view{body.data(), body.size()});
}

void emit_event(fims* pFims, const char* source, const char* message, int severity)
{
    if (pFims->Connected())
    {
        std::stringstream full_message;
        full_message << "{\"source\":\"" << source << "\",";
        full_message << "\"message\":\"" << message << "\",";
        full_message << "\"severity\":" << severity << "}";
        std::string full_message_str =full_message.str();
        pFims->Send("post", "/events", nullptr, full_message_str.c_str(), nullptr);
    }
}

ioDeque<std::shared_ptr<IO_Fims>> io_fimsChan;        // Use Channel to store IO_Fims objects
ioDeque<std::shared_ptr<IO_Fims>> io_fimsProcessChan; // Use Channel to send IO_Fims to the process
ioDeque<int> io_fimsProcessWakeChan;                  // Use Channel to send IO_Fims to the process
ioDeque<std::shared_ptr<IO_Fims>> io_fimsCollectChan; // Use Channel to send IO_Fims to the process
ioDeque<int> io_fimsCollectWakeChan;                  // Use Channel to send IO_Fims to the process
ioDeque<uint64_t> io_fimsListenChan;                  // Use Channel to ack an incoming message

/// @brief
///  crete an io_fims object if we dont have any old ones.
// but we do crash and burn if we try to make more that max_fims.

/// @param myCfg
/// @return
std::shared_ptr<IO_Fims> make_fims(struct cfg &myCfg)
{
    std::shared_ptr<IO_Fims> io_fims;
    int fims_wait = 0;
    int max_fims_wait = myCfg.max_fims_wait; // after this we give up

    while (!io_fimsChan.peekpop(io_fims))
    { // Assuming receive will return false if no item is available.
        if(myCfg.num_fims < myCfg.max_fims)
        {
            io_fims = std::make_shared<IO_Fims>(myCfg.connection.data_buffer_size);
            myCfg.num_fims++;
            return (io_fims);
        }
        fims_wait++;
        if (fims_wait> max_fims_wait)
        {
            std::cout << " failed to get a released io_fims buffer " << std::endl;
            myCfg.keep_fims_running = false;
            return nullptr;
        }
        // we've got to get one sometime or have we
        std::this_thread::sleep_for(10ms);
    }

    return (io_fims);
}

/// @brief
/// send an io_fims object to the fimsProcess Channel
/// @param io_fims
/// @param myCfg
/// @return
bool send_fims(std::shared_ptr<IO_Fims> io_fims, struct cfg &myCfg)
{
    io_fimsProcessChan.send(std::move(io_fims));
    io_fimsProcessWakeChan.send(1);
    return true;
}

/// @brief
///  returns an io_fims object to our buffer pool
/// @param io_fims
/// @param myCfg
/// @return
bool save_fims(std::shared_ptr<IO_Fims> io_fims, struct cfg &myCfg)
{
    io_fimsChan.fsend(std::move(io_fims));
    return true;
}

/// @brief
/// temp used to process the io_fims objects in test mode
/// @param io_fims
/// @param myCfg
/// @return
bool collect_fims(std::shared_ptr<IO_Fims> io_fims, struct cfg &myCfg)
{
    io_fimsCollectChan.send(std::move(io_fims));
    io_fimsCollectWakeChan.send(1);
    return true;
}

/// @brief
/// run a fims receive using an io_fims object
/// @param fims_gateway
/// @param io_fims
/// @return
bool gcom_recv_raw_message(fims &fims_gateway, std::shared_ptr<IO_Fims> io_fims) noexcept
{
    int connection = fims_gateway.get_socket();

    struct iovec bufs[2];
    bufs[0].iov_base = &io_fims->meta_data;
    bufs[0].iov_len = sizeof(Meta_Data_Info);
    bufs[1].iov_base = (void *)io_fims->fims_input_buf;
    bufs[1].iov_len = io_fims->fims_input_buf_len - 1;

    io_fims->bytes_read = readv(connection, bufs, 2); // sizeof(bufs) / sizeof(*bufs));
    io_fims->error = 0;

    if ((int)io_fims->bytes_read <= (int)0)
    {
        int err = errno;
        io_fims->error = err;

        // TODO handle other fims errors
        if (err != 11)
            FPS_ERROR_LOG(" read  err %d [%s]", err, strerror(err));
        // if (err == 22)
        //     close(connection);
    }
    return (int)io_fims->bytes_read > (int)0;
}

// note this may this may not work on unix sockets
bool gcom_recv_extra_raw_message(fims &fims_gateway, void *data_buf, uint32_t data_buf_len) noexcept
{
#ifdef FPS_DEBUG_MODE
    bool debug = false;
    if (debug)
        std::cout << " extra buff len set to  " << data_buf_len << std::endl;
#endif

    int connection = fims_gateway.get_socket();
    int flags = fcntl(connection, F_GETFL, 0);
    fcntl(connection, F_SETFL, flags | O_NONBLOCK);
    ssize_t bytes_read = read(connection, data_buf, data_buf_len);
    fcntl(connection, F_SETFL, flags);
#ifdef FPS_DEBUG_MODE
    if (debug)
        if (bytes_read > 0)
        {

            printf(" base %d we read extra stuff %d ", (int)sizeof(Meta_Data_Info), (int)bytes_read);
        }
    if (bytes_read <= 0)
    {
        printf(" base %d we did not read extra stuff %d ", (int)sizeof(Meta_Data_Info), (int)bytes_read);
        // close(connection);
    }
#endif
    return bytes_read > 0;
}

// we still have some work on the design pattern here
// we have a uri and a body.
// the uri will produce a component base and a component id, in the case of a single the uri will also produdce an io_point
// the uri will also produce a load of flags
// the body can produce a single value or an object with the value or an object with the name value with a value
//   '1234'    '{1234}' '{"value":1234}'
// if the uri is not a single then the body can contain an onject with items and values
// '{"item1":1234 ,"item2":{1234}, "item3":{"value":1234}}'
// in all casees we want the uri flags decoded
//       modified uri in uri_req
//       flags in uri req
//       the items in a  vector io_fims->items
//       any item values in a vector io_fims->values

/// @brief
/// parse the incoming header and infact handle set messages
/// @param myCfg
/// @param io_fims
/// @return
bool parseHeader(struct cfg &myCfg, std::shared_ptr<IO_Fims> io_fims)
{
    bool debug = false;

#ifdef FPS_DEBUG_MODE
    debug = true;
#endif
    auto data_buf_processing_index = 0;
    auto data_buf = io_fims->fims_input_buf;
    auto meta_data = io_fims->meta_data;

    io_fims->method_view = std::string_view{(char *)&data_buf[data_buf_processing_index], meta_data.method_len};
    data_buf_processing_index += meta_data.method_len;
    io_fims->uri_view = std::string_view{(char *)&data_buf[data_buf_processing_index], meta_data.uri_len};
    data_buf_processing_index += meta_data.uri_len;
    io_fims->replyto_view = std::string_view{(char *)&data_buf[data_buf_processing_index], meta_data.replyto_len};
    data_buf_processing_index += meta_data.replyto_len;
    io_fims->process_name_view = std::string_view{(char *)&data_buf[data_buf_processing_index], meta_data.process_name_len};
    data_buf_processing_index += meta_data.process_name_len;
    io_fims->username_view = std::string_view{(char *)&data_buf[data_buf_processing_index], meta_data.username_len};
    data_buf_processing_index += meta_data.username_len;
    io_fims->fims_data_buf = (u8 *)&data_buf[data_buf_processing_index];
    io_fims->fims_data_len = meta_data.data_len;
    io_fims->total_data += meta_data.data_len;

#ifdef FPS_DEBUG_MODE
    if (debug)
        if (io_fims->method_view == "set")
        {
            std::cout << "We got a set method; uri [" << io_fims->uri_view
                      << "]"
                      << std::endl;
        }
#endif

    io_fims->uri_req.clear_uri();
    io_fims->uri_req.set_uri(io_fims->uri_view);

    bool is_local_request = io_fims->uri_req.is_local_request;
    bool is_enable_request = io_fims->uri_req.is_enable_request;
    bool is_disable_request = io_fims->uri_req.is_disable_request;
    bool is_force_request = io_fims->uri_req.is_force_request;
    bool is_unforce_request = io_fims->uri_req.is_unforce_request;
    bool is_full_request = io_fims->uri_req.is_full_request;
    bool is_stats_request = io_fims->uri_req.is_stats_request;
    bool is_server_request = io_fims->uri_req.is_server_request;

    bool needs_body = true;
    if ( 
            is_enable_request 
        ||  is_disable_request
        ||  is_unforce_request
        ||  is_stats_request
        ||  is_server_request
        )
    {
        needs_body = false;
    }
    std::string replyto = "";
    if (io_fims->meta_data.replyto_len > 0)
    {
        replyto = std::string(io_fims->replyto_view);
    }

    // record message length stats (min, max, count, avg length)

    // TODO this has to be single threaded 
    myCfg.fims_message_stats.record_duration((int)meta_data.data_len);

    // parse the fims body into an any structure
    bool ipOk = false;

    if (io_fims->method_view != "get")
    {
        ipOk = gcom_parse_data(io_fims->anyBody, (const char *)io_fims->fims_data_buf, (int)io_fims->fims_data_len, false);
    }

    double tNow = get_time_double();
    if (io_fims->method_view == "set")
    {
        if (!ipOk && needs_body )
        {
            std::string reply_message  = "{\"gcom\":\"Modbus Set Message\",\"status\":\"Failed to decode\"}";
                            
            if (replyto != "" && reply_message != "")
            {
                if(myCfg.fims_gateway.Connected())
                {
                    myCfg.fims_gateway.Send(
                        fims::str_view{"set", sizeof("set") - 1},
                        fims::str_view{replyto.data(), replyto.size()},
                        fims::str_view{nullptr, 0},
                        fims::str_view{nullptr, 0},
                        fims::str_view{reply_message.data(), reply_message.size()});
                }
            }
            return false;
        }

#ifdef FPS_DEBUG_MODE
        if (debug)
            std::cout << "We got a set method " << ((io_fims->uri_req.uri_vec.size()>3)?"single":"multi")
                      << " anyBody  type " << io_fims->anyBody.type().name() 
                      << " local ? "<< (is_local_request?"true":"false")
                      << " force ? "<< (is_force_request?"true":"false")
                      << std::endl;
#endif
        if (io_fims->uri_req.num_uris > 3)
        {
            //std::cout << "We got a single set method; anyBody " << io_fims->anyBody.type().name() << " ";
#ifdef FPS_DEBUG_MODE
            if (debug)
            {
                std::cout << "We got a single set method; anyBody " << io_fims->anyBody.type().name() << " ";

                printAny(io_fims->anyBody, 1);
                std::cout << " uri_vec size " << io_fims->uri_req.num_uris<< std::endl;
            }
#endif
            {
                std::shared_ptr<cfg::io_point_struct> io_point;

                bool single_var = ioPointExists(io_point, myCfg, io_fims->uri_req.uri_vec, io_fims->uri_req.num_uris, "dummy");

                if (single_var)
                {
#ifdef FPS_DEBUG_MODE
                    if (debug)
                    {

                        std::cout << " single var found [" << io_point->id
                                  << "] enabled :" << ((io_point->is_enabled)?"true":"false")
                                  << "] forced :" << ((io_point->is_forced)?"true":"false")
                                  << " val :" << io_point->raw_val
                                  << " forced val :" << io_point->forced_val
                                  << std::endl;
                    }
#endif
                    // TODO take the point lock here
                    if (is_enable_request)
                    {
                        FPS_INFO_LOG("Single var enable [%s]", io_point->id);
                        io_point->is_enabled = true;
                    }
                    if (is_disable_request)
                    {
                        FPS_INFO_LOG("Single var disable [%s]", io_point->id);
                        io_point->is_enabled = false;
                    }
                    if (is_force_request)
                    {
                        FPS_INFO_LOG("Single var forced [%s]", io_point->id);
                        io_point->is_forced = true;
                    }
                    if (is_unforce_request)
                    {
                        FPS_INFO_LOG("Single var unforced [%s]", io_point->id);
                        io_point->is_forced = false;
                    }
                    bool debounce_enable = true;
                    check_item_debounce(debounce_enable, io_point, false);
                    if (debounce_enable)
                    {
                        io_point->process_name = std::string(io_fims->process_name_view);
                        io_point->username = std::string(io_fims->username_view);
#ifdef FPS_DEBUG_MODE
                        if (debug)
                        {
                            std::string local_str("false");
                            if (is_local_request)
                            {
                                local_str = "true";
                            }
                            std::cout << " after debounce[" << io_point->id
                                      << "] process_name :[" << io_point->process_name
                                      << "] username :" << io_point->username
                                      << "] local :" << local_str
                                      << std::endl;
                        }
#endif

                        io_point->set_value = io_fims->anyBody;
                        // uint64_t set_any_to_uint64(std::shared_ptr<cfg::io_point_struct>io_point, std::any val)

                        uint64_t uval = set_any_to_uint64(myCfg, io_point, io_point->set_value);

#ifdef FPS_DEBUG_MODE
                        if (debug)
                        {
                            std::cout << " after set_any  [" << io_point->id
                                      << "] uval :" << uval
                                      << std::endl;
                        }
#endif

                        if (is_force_request)
                        {
                            io_point->forced_val = uval;
                        }
#ifdef FPS_DEBUG_MODE
                        if (debug)
                        {
                            std::cout << " single var processed [" << io_point->id
                                      << "] raw val :" << io_point->raw_val
                                      << " forced val :" << io_point->forced_val
                                      << std::endl;
                        }
#endif
                        std::string reply_message = "";
                        if (is_local_request)
                        {
                            reply_message = "{\"gcom\":\"Modbus Point Local Set\",\"status\":\"Success\"}";
                        }

                        //if (is_force_request || is_unforce_request || is_enable_request || is_disable_request || is_local_request)
                        if (is_force_request || is_unforce_request || is_enable_request || is_disable_request)
                        {
                            if (is_force_request)
                            {
                                reply_message = "{\"gcom\":\"Modbus Point Force\",\"status\":\"Success\"}";
                            }
                            if (is_unforce_request)
                            {
                                reply_message = "{\"gcom\":\"Modbus Point Unforce\",\"status\":\"Success\"}";
                            }
                            if (is_enable_request)
                            {
                                reply_message = "{\"gcom\":\"Modbus Point Enable\",\"status\":\"Success\"}";
                                if(io_point->is_watchdog){
                                    std::shared_ptr<cfg::watchdog_struct> watchdog = io_point->watchdog_point.lock();
                                    watchdog->enable();
                                }
                            }
                            if (is_disable_request)
                            {
                                reply_message = "{\"gcom\":\"Modbus Point Disable\",\"status\":\"Success\"}";
                                if(io_point->is_watchdog){
                                    std::shared_ptr<cfg::watchdog_struct> watchdog = io_point->watchdog_point.lock();
                                    watchdog->disable();
                                }
                            }
                            if (replyto != "" && reply_message != "")
                            {
                                if(myCfg.fims_gateway.Connected())
                                {
                                    myCfg.fims_gateway.Send(
                                        fims::str_view{"set", sizeof("set") - 1},
                                        fims::str_view{replyto.data(), replyto.size()},
                                        fims::str_view{nullptr, 0},
                                        fims::str_view{nullptr, 0},
                                        fims::str_view{reply_message.data(), reply_message.size()});
                                }
                            }
#ifdef FPS_DEBUG_MODE
                            if (debug)
                                std::cout << " single var done [" << io_point->id
                                          << "] raw val :" << io_point->raw_val
                                          << " forced val :" << io_point->forced_val
                                          << std::endl;
#endif
                            return true;
                        }

                        // now we need to decide if we are to send the value on to the server
                        // we need to be a holdin or a coil for this
                        bool send_to_server = ((io_point->register_type == cfg::Register_Types::Coil) || (io_point->register_type == cfg::Register_Types::Holding));
                        if (send_to_server || is_local_request)
                        {
#ifdef FPS_DEBUG_MODE
                            debug = true;
                            if (debug)
                            {
                                std::cout << " single var send_to_server [" << io_point->id
                                          << "] raw val :" << io_point->raw_val
                                          << " forced val :" << io_point->forced_val
                                          << std::endl;
                            }
#endif
                            std::string mode("set");

                            std::shared_ptr<IO_Work> io_work_single = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->off_by_one, io_point->size, io_point->reg16, io_point->reg8, strToWorkType(mode, false));
                            io_work_single->io_points.emplace_back(io_point);
                            io_work_single->replyto = replyto;
                            io_work_single->local = is_local_request;
                            io_work_single->off_by_one = io_point->off_by_one;

                            if ((io_point->register_type == cfg::Register_Types::Coil)|| (io_point->register_type == cfg::Register_Types::Discrete_Input))
                            {
#ifdef FPS_DEBUG_MODE
                                if (debug)
                                {
                                    std::cout << ">>>>" << __func__ << " regtype Coil " << io_point->id << std::endl;
                                }
#endif
                                // io_fims->uri_req;
                                auto bval = get_any_to_bool(io_point, io_fims->anyBody, io_fims->uri_req, io_work_single->buf8);
                                // io_point->reg8[0] = bval;
                                // io_work_single->u8_buff[0] = bval;
                                // io_point->raw_val = bval;

                                if (io_point->is_forced && is_force_request)
                                {
                                    bval = io_point->forced_val;
                                }

                                io_work_single->buf8[0] = bval;
                            }
                            else if ((io_point->register_type == cfg::Register_Types::Holding) || (io_point->register_type == cfg::Register_Types::Input))
                            {
#ifdef FPS_DEBUG_MODE
                                if (debug)
                                {
                                    std::cout << ">>>>" << __func__ << " regtype Holding " << std::endl;
                                }
#endif
                                if (io_point->is_forced && is_force_request)
                                {
                                    uval = io_point->forced_val;
                                }
                                // io_point->raw_val = uval;

                                set_reg16_from_uint64(io_point, uval, io_work_single->buf16);
                                //std::cout << ">>>>" << __func__ << " HOLDING uval " << uval << std::endl;
                            }

                            setWork(io_work_single);
                        }
                        else
                        {
                            std::cout << ">>>>" << __func__ << " not correct type to  send to server " << std::endl;
                        }
                        return true;
                    }
                    else
                    {
                        std::cout << " io_point id [" << io_point->id << " ] skipped ... debounce " << std::endl;
                        // continue;
                    }

                    // clearChan(true);
                }
                else
                {
                    myCfg.fims_set_errors++;
                    if(myCfg.fims_set_errors<16)
                    {
                        std::stringstream ss;
                        ss  
                            << "process_name [" << io_fims->process_name_view
                            << "] username [" << io_fims->username_view
                            << "] uri [" << io_fims->uri_view
                            << "] "<< std::endl;
                        //std::cout << ss.str() << std::endl;
                        FPS_ERROR_LOG("Fims Set  failed : %s", ss.str().c_str());

                    }
                    if (io_fims->meta_data.replyto_len > 0) {

                        if(myCfg.fims_gateway.Connected())
                        {
                            std::string reply_message = "{\"gcom\":\"Modbus Point Unknown\",\"status\":\"Falure\"}";
                            myCfg.fims_gateway.Send(
                                fims::str_view{"set", sizeof("set") - 1},
                                fims::str_view{replyto.data(), replyto.size()},
                                fims::str_view{nullptr, 0},
                                fims::str_view{nullptr, 0},
                                fims::str_view{reply_message.data(), reply_message.size()});
                        }
                    }

                }
            }
        }
        else if (io_fims->uri_req.num_uris <= 3)
        {
            //std::cout << "We got a  multi set method; anyBody " << io_fims->anyBody.type().name() << " " << std::endl;

#ifdef FPS_DEBUG_MODE
            if (debug)
                std::cout << "We got a  multi set method; anyBody " << io_fims->anyBody.type().name() << " " << std::endl;
#endif
            // anyTypeString(io_fims->anyBody);
            if (io_fims->anyBody.type() == typeid(std::map<std::string, std::any>))
            {
                //std::cout << "We got a  map of anys "<< std::endl;
                std::map<std::string, std::any> message_body = std::any_cast<std::map<std::string, std::any>>(io_fims->anyBody);
                std::vector<std::shared_ptr<IO_Work>> work_vec;
                std::vector<std::shared_ptr<cfg::io_point_struct>> io_map_vec;
                for (std::pair<std::string, std::any> key : message_body)
                {
                    std::shared_ptr<cfg::io_point_struct> io_point;

                    // have to look for _disable / _enable  / _force /_unforce

                    bool multi_var = ioPointExists(io_point, myCfg, io_fims->uri_req.uri_vec, io_fims->uri_req.num_uris, key.first);
                    if (multi_var)
                    {
                        if (is_enable_request)
                        {
                            io_point->is_enabled = true;
                        }
                        if (is_disable_request)
                        {
                            io_point->is_enabled = false;
                        }
                        if (is_force_request)
                        {
                            io_point->is_forced = true;
                        }
                        if (is_unforce_request)
                        {
                            io_point->is_forced = false;
                        }
                        bool debounce_enable = true;
                        check_item_debounce(debounce_enable, io_point, false);

                        if (!debounce_enable)
                        {
                            std::cout << " io_point id [" << io_point->id << " ] skipped ... debounce " << std::endl;
                            continue;
                        }
                        io_point->process_name = std::string(io_fims->process_name_view);
                        io_point->username = std::string(io_fims->username_view);
                        io_point->set_value = key.second;
                        uint64_t uval = set_any_to_uint64(myCfg, io_point, io_point->set_value);

                        //std::cout << " io_point id [" << io_point->id << " ] uval [" << uval << "]"<< std::endl;
                        if (is_force_request)
                        {
                            io_point->forced_val = uval;
                        }
                        if (myCfg.allow_multi_sets)
                        {
                            //std::cout << " io_point id [" << io_point->id << " ] allow multi sets " << std::endl;
                            // note: our current modbus server does not process multi sets.
                            io_map_vec.emplace_back(io_point);
                        }
                        else
                        {
                            //std::cout << " io_point id [" << io_point->id << " ] not doing multi sets, added to work_vec " << std::endl;

                            std::shared_ptr<IO_Work> io_work_single = make_work(io_point->register_type, io_point->device_id, io_point->offset,
                                         io_point->off_by_one, io_point->size, io_point->reg16, io_point->reg8, strToWorkType("set", false));
                            io_work_single->off_by_one = io_point->off_by_one;

                            io_work_single->io_points.emplace_back(io_point);
                            work_vec.emplace_back(io_work_single);
                        }
                    }
                }

                std::string reply_message = "";

                if (is_local_request)
                {
                    reply_message = "{\"gcom\":\"Modbus Point Local Set\",\"status\":\"Success\"}";
                }
                if (is_force_request || is_unforce_request || is_enable_request || is_disable_request)
                {
                    if (is_force_request)
                    {
                        reply_message = "{\"gcom\":\"Modbus Point Force\",\"status\":\"Success\"}";
                    }
                    if (is_unforce_request)
                    {
                        reply_message = "{\"gcom\":\"Modbus Point Unforce\",\"status\":\"Success\"}";
                    }
                    if (is_enable_request)
                    {
                        reply_message = "{\"gcom\":\"Modbus Point Enable\",\"status\":\"Success\"}";
                    }
                    if (is_disable_request)
                    {
                        reply_message = "{\"gcom\":\"Modbus Point Disable\",\"status\":\"Success\"}";
                    }
                    if (replyto != "" && reply_message != "")
                    {
                        if(myCfg.fims_gateway.Connected())
                        {
                            myCfg.fims_gateway.Send(
                                fims::str_view{"set", sizeof("set") - 1},
                                fims::str_view{replyto.data(), replyto.size()},
                                fims::str_view{nullptr, 0},
                                fims::str_view{nullptr, 0},
                                fims::str_view{reply_message.data(), reply_message.size()});
                        }
                    }

                    return true;
                }

                if (myCfg.allow_multi_sets)
                {
                    check_work_items(work_vec, io_map_vec, myCfg, "set", false, false);
                }

                int idx = 1;
                int io_work_size = (int)work_vec.size();
                auto forced_str = fmt::format("set_group_{}_{}", (++myCfg.set_idx) % 1000, io_work_size);
                std::string single_str("");
                for (std::shared_ptr<IO_Work> io_work : work_vec)
                {
                    io_work->replyto = replyto;
                    io_work->tNow = tNow;
                    io_work->local = is_local_request;

                    io_work->erase_group = true;
                    if (myCfg.allow_multi_sets)
                    {
                        io_work->work_name = forced_str;
                        io_work->work_group = (int)io_work_size;
                        io_work->work_id = idx;
                        idx++;

                    }
                    else
                    {

                        io_work->work_name = single_str;
                        io_work->work_group = 1;
                        io_work->work_id = 1;
                    }
#ifdef FPS_DEBUG_MODE
                    if (debug)
                        std::cout << __func__ << " io->offset  [" << io_work->offset << "] num_registers :" << io_work->num_registers << std::endl;
#endif
                    // pollWork (io);
                    // work_vec.erase(io_work);

                    if ((io_work->register_type == cfg::Register_Types::Coil) || (io_work->register_type == cfg::Register_Types::Discrete_Input))
                    {
                        int j = 0;
                        for (std::shared_ptr<cfg::io_point_struct> io_point : io_work->io_points)
                        {
#ifdef FPS_DEBUG_MODE
                            if (debug)
                            {
                                std::cout << ">>>>" << __func__ << " regtype Coil " << io_point->id << std::endl;
                            }
#endif
                            // io_fims->uri_req;
                            auto bval = get_any_to_bool(io_point, io_point->set_value, io_fims->uri_req, io_work->buf8);
                            // io_point->reg8[0] = bval;
                            // io_work_single->u8_buff[0] = bval;
                            // 

                            if (io_point->is_forced && is_force_request)
                            {
                                bval = io_point->forced_val;
                            } else {
                                io_point->raw_val = bval;
                            }

                            io_work->buf8[j] = bval;
                            j++;
                        }
                    }
                    else if ((io_work->register_type == cfg::Register_Types::Holding) || (io_work->register_type == cfg::Register_Types::Input))
                    {
                        int j = 0;
                        for (std::shared_ptr<cfg::io_point_struct> io_point : io_work->io_points)
                        {
#ifdef FPS_DEBUG_MODE
                            if (debug)
                            {
                                std::cout << ">>>>" << __func__ << " regtype Holding " << std::endl;
                            }
#endif
                            //uint64_t uval;
                            uint64_t uval = set_any_to_uint64(myCfg, io_point, io_point->set_value);

                            if (io_point->is_forced && is_force_request)
                            {
                                uval = io_point->forced_val;
                            } else{
                                io_point->raw_val = uval;
                            }
                            set_reg16_from_uint64(io_point, uval, &io_work->buf16[j]);
                            j += io_point->size;
                        }
                    }

                    setWork(io_work);
                }
            }
            else if (!io_fims->anyBody.has_value())
            {
                int key_idx = 0;
                if ((io_fims->uri_req.num_uris> 1) && (io_fims->uri_req.uri_vec[0].size() == 0))
                    key_idx = 1;

                try
                {
                    // Using at() for safe access. Catch exceptions if key not found
                    auto &myComp = myCfg.component_io_point_map.at(io_fims->uri_req.uri_vec[key_idx]);
                    // std::cout   << "         func : #2 "<< __func__ << "\n";
                    auto &register_group = myComp.at(io_fims->uri_req.uri_vec[key_idx + 1]);
                    // std::cout   << "         func : #3 "<< __func__ << "\n";
                    for (auto &io_point_pair : register_group)
                    {
                        std::shared_ptr<cfg::io_point_struct> io_point = io_point_pair.second;
                        if (is_enable_request)
                        {
                            io_point->is_enabled = true;
                        }
                        if (is_disable_request)
                        {
                            io_point->is_enabled = false;
                        }
                    }
                    return true;
                }
                catch (const std::out_of_range &)
                {
                }
            }
        }
    }

    else if (io_fims->method_view == "get")
    {
        std::vector<std::shared_ptr<cfg::io_point_struct>> io_map_vec; // this collects all the io points in the fims message
        std::vector<std::shared_ptr<IO_Work>> io_work_vec;             // this is the collection of all the io_work objects

#ifdef FPS_DEBUG_MODE
        if (debug)
        {
            std::cout << "We got a get method; replyto [" << io_fims->replyto_view << "]" << std::endl;
            std::cout << " uri_vec size " << io_fims->uri_req.num_uris<< std::endl;
        }
#endif
        if (is_stats_request || is_server_request)
        {
 
            if (io_fims->meta_data.replyto_len > 0)
            {

                std::stringstream ss;
                if (is_stats_request && is_server_request)
                {
                    ss << "[\n";
                    //std::cout << " got stats AND server " << std::endl;
                }

                bool first = true;
                if (is_stats_request)
                {
                    if (first) first = false; else ss << ",";
                    get_stats(ss, myCfg);
                    //std::cout << " got stats " << ss.str() << std::endl;
                    //send_set(myCfg.fims_gateway, io_fims->replyto_view, std::string_view(ss.str()));
                    //return true;
                }
                if (is_server_request)
                {
                    if (first) first = false; else ss << ",";
                    //std::stringstream ss;
                    ss << "{\n";
                    showServerConfig(myCfg, ss);
                    ss << "\n}\n";
                    //std::cout << " got server " << ss.str() << std::endl;
                }

                if (is_stats_request && is_server_request)
                {
                    ss << "]\n";
                    //std::cout << " got stats AND server " << std::endl;
                }
                if(myCfg.fims_gateway.Connected())
                {
                    send_set(myCfg.fims_gateway, io_fims->replyto_view, std::string_view(ss.str()));
                }
                return true;
            }
        }
        

        if (io_fims->uri_req.num_uris > 3)
        {
            std::shared_ptr<cfg::io_point_struct> io_point;
            std::vector<std::shared_ptr<IO_Work>> work_vec;

            auto single_var = ioPointExists(io_point, myCfg, io_fims->uri_req.uri_vec, io_fims->uri_req.num_uris,"dummy");
            if (single_var)
            {
//#ifdef FPS_DEBUG_MODE
                if (debug)
                    std::cout << " single var found " << io_point->id << "\n";
//#endif
                std::string replyto = "";
                if (io_fims->meta_data.replyto_len > 0)
                {
                    replyto = std::string(io_fims->replyto_view);
                }

                std::string mode("get");

                std::shared_ptr<IO_Work> io_work_single = make_work(io_point->register_type, io_point->device_id, io_point->offset, io_point->off_by_one, io_point->size, io_point->reg16, io_point->reg8, strToWorkType(mode, false));
                io_work_single->io_points.emplace_back(io_point);
                io_work_single->replyto = replyto;
                io_work_single->local = is_local_request;
                io_work_single->unforced = is_unforce_request;
                io_work_single->full = is_full_request;
                io_work_single->off_by_one = io_point->off_by_one;

                if (io_point->register_type == cfg::Register_Types::Coil || io_point->register_type == cfg::Register_Types::Discrete_Input)
                {
#ifdef FPS_DEBUG_MODE
                    if (debug)
                    {
                        std::cout << ">>>>" << __func__ << " regtype Coil " << io_point->id << std::endl;
                    }
#endif
                    // io_fims->uri_req;
                    auto bval = io_point->raw_val;

                    if (io_point->is_forced && is_force_request)
                    {
                        bval = io_point->forced_val;
                    }

                    io_work_single->buf8[0] = bval;
                }
                else if (io_point->register_type == cfg::Register_Types::Holding || io_point->register_type == cfg::Register_Types::Input)
                {
#ifdef FPS_DEBUG_MODE
                    if (debug)
                    {
                        if (io_point->register_type == cfg::Register_Types::Holding)

                            std::cout << ">>>>" << __func__ << " regtype Holding " << std::endl;
                        else if (io_point->register_type == cfg::Register_Types::Input)
                            std::cout << ">>>>" << __func__ << " regtype Input " << std::endl;
                    }
#endif
                    uint64_t uval = io_point->raw_val;
                    if (io_point->is_forced && is_force_request)
                    {
                        uval = io_point->forced_val;
                    }

                    set_reg16_from_uint64(io_point, uval, io_work_single->buf16);
                }

                pollWork(io_work_single);
            }

            else
            {
                myCfg.fims_get_errors++;
                if(myCfg.fims_get_errors<16)
                {
                    std::stringstream ss;
                    ss  
                        << "process_name [" << io_fims->process_name_view
                        << "] username [" << io_fims->username_view
                        << "] uri [" << io_fims->uri_view
                        << "] ";
                    //std::cout << ss.str() << std::endl;
                    FPS_ERROR_LOG("Fims Get  failed : %s", ss.str().c_str());


                }
                if (io_fims->meta_data.replyto_len > 0) {

                    if(myCfg.fims_gateway.Connected())
                    {
                        std::string reply_message = "{\"gcom\":\"Modbus Point Unknown\",\"status\":\"Falure\"}";
                        myCfg.fims_gateway.Send(
                            fims::str_view{"set", sizeof("set") - 1},
                            fims::str_view{replyto.data(), replyto.size()},
                            fims::str_view{nullptr, 0},
                            fims::str_view{nullptr, 0},
                            fims::str_view{reply_message.data(), reply_message.size()});
                    }
                }

                // TODO debug
                //std::cout << " single var NOT found ["<<io_fims->uri_req.uri_vec[3]<< "]\n";
            }
        }
        else
        {
#ifdef FPS_DEBUG_MODE
            debug = false;
            if (debug)
                std::cout << __func__ << " multi uri found [" << io_fims->uri_view << "]\n";
#endif
            std::vector<std::shared_ptr<IO_Work>> work_vec;
            std::vector<std::shared_ptr<cfg::io_point_struct>> io_map_vec;
            std::vector<std::shared_ptr<cfg::io_point_struct>> io_map_vec_pruned;
            add_all_component_points_to_io_vec(io_map_vec, myCfg, io_fims->uri_req.uri_vec, false);

            for (std::shared_ptr<cfg::io_point_struct> io_point : io_map_vec)
            {
                if ((is_disable_request && !io_point->is_enabled) 
                || (is_enable_request && io_point->is_enabled) 
                || (is_force_request && io_point->is_forced) 
                || (is_unforce_request && !io_point->is_forced) 
                || (!is_force_request && !is_unforce_request && !is_disable_request && !is_enable_request))
                {
                    if (io_point->is_enabled)
                        io_map_vec_pruned.emplace_back(io_point);
                }
            }

            check_work_items(work_vec, io_map_vec_pruned, myCfg, "get", true, false);
            int idx = 1;
            int io_work_size = (int)work_vec.size();
            auto forced_str = fmt::format("get_group_{}_{}", (++myCfg.get_idx) % 1000, io_work_size);

            for (std::shared_ptr<IO_Work> io_work : work_vec)
            {
                io_work->replyto = replyto;
                io_work->tNow = tNow;
                io_work->local = io_fims->uri_req.is_local_request;
                io_work->full = is_full_request;
                
                io_work->work_name = forced_str;
                io_work->erase_group = true;
                io_work->work_group = (int)io_work_size;
                io_work->work_id = idx;
                idx++;
#ifdef FPS_DEBUG_MODE
                if (debug)
                    std::cout << __func__ << " io->offset  [" << io_work->offset << "] num_registers :" << io_work->num_registers << std::endl;
#endif
                // pollWork (io);
                // work_vec.erase(io_work);

                if (io_work->register_type == cfg::Register_Types::Coil || io_work->register_type == cfg::Register_Types::Discrete_Input)
                {
                    int j = 0;
                    for (std::shared_ptr<cfg::io_point_struct> io_point : io_work->io_points)
                    {
#ifdef FPS_DEBUG_MODE
                        if (debug)
                        {
                            std::cout << ">>>>" << __func__ << " regtype Coil " << io_point->id << std::endl;
                        }
#endif
                        // io_fims->uri_req;
                        auto bval = io_point->raw_val;
                        // io_point->reg8[0] = bval;
                        // io_work_single->u8_buff[0] = bval;

                        if (io_point->is_forced && is_force_request)
                        {
                            bval = io_point->forced_val;
                        }

                        io_work->buf8[j] = bval;
                        j++;
                    }
                }
                else if (io_work->register_type == cfg::Register_Types::Holding || io_work->register_type == cfg::Register_Types::Input)
                {
                    int j = 0;
                    for (std::shared_ptr<cfg::io_point_struct> io_point : io_work->io_points)
                    {
#ifdef FPS_DEBUG_MODE
                        if (debug)
                        {
                            std::cout << ">>>>" << __func__ << " regtype Holding " << std::endl;
                        }
#endif
                        uint64_t uval = io_point->raw_val;
                        if (io_point->is_forced && is_force_request)
                        {
                            uval = io_point->forced_val;
                        }

                        set_reg16_from_uint64(io_point, uval, &io_work->buf16[j]);
                        j += io_point->size;
                    }
                }

                pollWork(io_work);
            }
        }
    }
    else if (io_fims->method_view == "pub")
    {
        myCfg.fims_pub_errors++;
        if(myCfg.fims_pub_errors<16)
        {
            std::stringstream ss;
            ss  
                << "process_name [" << io_fims->process_name_view
                << "] username [" << io_fims->username_view
                << "] uri [" << io_fims->uri_view
                << "] "<< std::endl;
            //std::cout << ss.str() << std::endl;
            FPS_ERROR_LOG("Fims Pub failed : %s", ss.str().c_str());

        }

    }
    else // method not supported by gcom_client (it's not set, pub, or get)
    {
        std::cout << "We got a mystery  method [" << io_fims->method_view << "] len :" << meta_data.method_len << std::endl;

            //     FPS_ERROR_LOG("Listener for : %s, from sender: %s method %s is not supported. Message dropped",
            //                   sys.fims_dependencies->name.c_str(),
            //                   sys.fims_dependencies->process_name_view,
            //                   sys.fims_dependencies->method_view);
            //     FPS_LOG_IT("fims_method_error");

            //     if (!sys.fims_dependencies->replyto_view.empty())
            //     {
            //         static constexpr auto err_str = "Unsupported fims method"sv;

            //         if (!send_set(sys.fims_dependencies->fims_gateway, sys.fims_dependencies->replyto_view, err_str))
            //         {
            //             FPS_ERROR_LOG("Listener for '%s', could not send replyto fims message.",
            //                           sys.fims_dependencies->name.c_str());
            //             FPS_LOG_IT("fims_send_error");

            //             return false;
            //         }
            //     }
            //     return false;
    }

    if (meta_data.data_len > static_cast<uint32_t>(io_fims->fims_data_buf_len))
    {
        // FPS_ERROR_LOG
        FPS_ERROR_LOG("Fims receive buffer is too small. Recommend increasing data_buf_len to at least %d\n", meta_data.data_len);
        auto orig_len = (int)io_fims->fims_data_buf_len;
        FPS_ERROR_LOG("Extra data offset %d required  %d\n", (int)io_fims->fims_data_buf_len, meta_data.data_len - orig_len);
        // io_fims->reset_fims_data_buf((int)(meta_data.data_len * 1.5));

        // gcom_recv_extra_raw_message(myCfg.fims_gateway, &myCfg.fims_input_buf[orig_len], meta_data.data_len-orig_len);

        // FPS_LOG_IT("fims_receive_buffer");
        return false;
    }

    return true;
}

bool fims_connect(struct cfg & myCfg, bool debug)
{
    std::string name(myCfg.connection.name);
    if(name.empty())
    {
        FPS_ERROR_LOG(" fims_connect name string is empty\n");
        return false;
    }
    const auto conn_id_str = fmt::format("modbus_client_uri@{}", name);
    if (!myCfg.fims_gateway.Connect(conn_id_str.data()))
    {
        FPS_ERROR_LOG("For client init uri \"{}\": could not connect to fims_server\n", name);
        return false;
    }

    const auto sub_string = fmt::format("/modbus_client/{}", name);
    myCfg.subs.push_back(sub_string);

    if (!myCfg.fims_gateway.Subscribe(myCfg.subs))
    {
        FPS_ERROR_LOG("For client with init uri \"{}\": failed to subscribe for uri init\n", name);
        return false;
    }

#ifdef FPS_DEBUG_MODE
    if (debug)
        NEW_FPS_ERROR_PRINT("For client \"{}\" with subs \"{}\": subscribed OK\n", name, myCfg.subs[0]);
#endif
    return true;
}

/*
/// @brief
///   this is a thread , used in test mode , to pick up the used  io_fims messages
/// @param io_fims
/// @param myCfg
/// @param debug
/// @return
*/
static bool runCollectFims(std::shared_ptr<IO_Fims> io_fims, struct cfg & myCfg, bool debug)
{
    std::string name(myCfg.connection.name);

    // std::cout << " Process Fims Message running tid " << io_fims->tId <<  std::endl;
    bool ok = true;
    if (ok)
    {

        if (ok)
        {
            if (0)
                printf("%s \"%s\": id [%f] received a message len %d  [%.*s]\n", __func__, name.c_str(),
                        io_fims->tId, (int)io_fims->bytes_read, (int)io_fims->bytes_read, io_fims->fims_data_buf);
        }
        else
        {
            printf("%s \"%s\": failed with a message len %d \n", __func__, name.c_str(), (int)io_fims->data_buf_len);
        }
    }
    else
    {
        printf("%s \"%s\": did not receive a message \n", __func__, name.c_str());
    }

    return true;
}

/*
/// @brief
// process the incoming fims message
/// @param io_fims
/// @param myCfg
/// @param debug
/// @return
*/
static bool runProcessFims(std::shared_ptr<IO_Fims> io_fims, struct cfg & myCfg, bool debug)
{
    std::string name(myCfg.connection.name);

    auto ok = parseHeader(myCfg, io_fims);

    if (ok)
    {
        if (0)
            printf("%s \"%s\": id [%f] received a message len %d  [%.*s]\n", __func__, name.c_str(),
                    io_fims->tId, (int)io_fims->bytes_read, (int)io_fims->bytes_read, io_fims->fims_data_buf);
    }
    else
    {
        // TODO error reporting
        printf("%s \"%s\": failed with a message len %d \n", __func__, name.c_str(), (int)io_fims->data_buf_len);
    }

    return true;
}

/*
/// @brief
    fims_message processor
/// @param myCfg
/// @return
*/
static bool process_thread(struct cfg & myCfg) noexcept
{
    double delay = 0.1;
    int signal = -1;
    bool debug = false;
    bool run = true;
    int jobs = 0;
    bool save = false;

    while (run && myCfg.keep_running)
    {
        std::shared_ptr<IO_Fims> io_fims;

        if (io_fimsProcessWakeChan.receive(signal, delay))
        {
            if (signal == 0)
                run = false;
            if (signal == 1)
            {
                jobs++;
                if (io_fimsProcessChan.receive(io_fims))
                {
                    save = runProcessFims(io_fims, myCfg, debug);

                    if (save)
                    {
                        collect_fims(io_fims, myCfg);
                    }
                }
            }
        }
    }
    {
        FPS_INFO_LOG("%s stopping after %d jobs", __func__, jobs);
    }

    return true;
}

/// @brief
/// test mode collect io_fims messages
/// @param myCfg
/// @return
static bool collect_thread(struct cfg & myCfg) noexcept
{
    double delay = 0.1;
    int signal = -1;
    bool debug = false;
    bool run = true;
    int jobs = 0;
    bool save = false;
    while (run && myCfg.keep_running)
    {
        std::shared_ptr<IO_Fims> io_fims;
        if (io_fimsCollectWakeChan.receive(signal, delay))
        {
            if (signal == 0)
                run = false;
            if (signal == 1)
            {
                jobs++;
                if (io_fimsCollectChan.receive(io_fims))
                {
                    save = runCollectFims(io_fims, myCfg, debug);

                    if (save)
                    {
                        save_fims(io_fims, myCfg);
                    }
                }
            }
        }
    }
    {
        FPS_INFO_LOG("%s stopping after %d jobs", __func__, jobs);
    }

    return true;
}

/*
/// @brief
    high speed fims mesage dump
    just pass on the IO_Fims task to the process thread.
    allows us to keep up with possible fims traffic
/// @param myCfg
/// @return
*/
static bool listener_thread(std::vector<std::string> & subs, struct cfg & myCfg) noexcept
{
    bool debug = true;
    myCfg.fims_running = true;

    // TODO logs
    // std::cout << __func__ << " connect " << std::endl;

    bool fimsOk = fims_connect(myCfg, debug);
    if (!fimsOk)
    {
        FPS_ERROR_LOG("Fims listener thread:  could not connect. Exiting");
        myCfg.fims_running = false;
        return false;
    }

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(myCfg.fims_gateway.get_socket(), SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) == -1)
    {

        FPS_ERROR_LOG("listener for \"{}\": could not set socket timeout to 2 seconds. Exiting\n", myCfg.client_name);
        myCfg.fims_running = false;
        return false;
    }
    uint64_t myjobs = 0;

    auto io_fims = make_fims(myCfg);
    // main loop:
    while (myCfg.keep_fims_running)
    {
        io_fims->jobs++;

        // u64 data_buf_len = 0;
        bool ok = gcom_recv_raw_message(myCfg.fims_gateway, io_fims);
        if (ok)
        {
            if (myCfg.test_fims)
            {
                io_fimsListenChan.sendb(myjobs);
            }
            else
            {
                myjobs++;
                // std::cout << "listener_thread ::  received message " << std::endl;
                // if(myCfg.fims_gateway.Connected())
                //     {

                send_fims(io_fims, myCfg);
                io_fims = make_fims(myCfg);
            }
        }
        else
        {
            if (io_fims->error != EAGAIN && io_fims->error != EWOULDBLOCK)
            {
                std::cout << "listener_thread :: io_fims error " << io_fims->error << std::endl;
                myCfg.fims_running = false;
                myCfg.keep_fims_running = false;
            }
        }
    }
    {
        // std::lock_guard<std::mutex> lock2(io_output_mutex);
        // FPS_INFO_LOG
        //printf("listen thread stopping after %ld jobs\n", myjobs);
        // CloseModbusForThread(io_thread, debug);
        myCfg.fims_running = false;

    }

    return true;
}

/// @brief
/// we have a double buffered thread system for fims
/// @param myCfg
/// @return

bool start_process(struct cfg & myCfg)
{
    myCfg.keep_running = true;
    processThread1 = std::thread(process_thread, std::ref(myCfg));
    //    processThread2 = std::thread(process_thread, std::ref(myCfg));
    //    processThread3 = std::thread(process_thread, std::ref(myCfg));
    collectThread1 = std::thread(collect_thread, std::ref(myCfg));

    return true;
}

bool start_fims(std::vector<std::string> & subs, struct cfg & myCfg)
{
    //std::cout << " fims starting " << std::endl;
    myCfg.keep_fims_running = true;
    fimsThread = std::thread(listener_thread, std::ref(subs), std::ref(myCfg));

    return true;
}

bool stop_process(struct cfg & myCfg)
{
    myCfg.keep_running = false;
    processThread1.join();
    //    processThread2.join();
    //    processThread3.join();
    collectThread1.join();

    return true;
}

bool stop_fims(struct cfg & myCfg)
{
    //std::cout << " fims stopping " << std::endl;
    myCfg.keep_fims_running = false;
    fimsThread.join();

    return true;
}

///////////////////// built in test code follows

/// @brief
/// simple stress test for fims
/// @param myCfg
/// @param debug
/// @return
bool test_fims_connect(struct cfg & myCfg, bool debug)
{
    //std::string name("myname");
    std::string name(myCfg.connection.name);

    const auto sub_string = fmt::format("/components/{}", "comp_sel_2440");

    std::vector<std::string> subs;
    subs.emplace_back(sub_string);

    // start fims threads
    start_process(myCfg);

    start_fims(subs, myCfg);

    std::cout << " Now sleeping for 2 seconds" << std::endl;
    std::this_thread::sleep_for(2000ms);

    const auto conn_test_str = fmt::format("modbus_client_test@{}", name);
    if (!myCfg.fims_test.Connect(conn_test_str.data()))
    {
        FPS_ERROR_LOG("For client testuri \"{}\": could not connect to fims_server\n", name);
        return false;
    }

    myCfg.test_fims = true;

    // bool ok;
    double tstart = get_time_double();
    int i;
    int max_count = 3000;
    double delay = 0.1;
    uint64_t signal;

    // std::chrono::microseconds delay(10);

    for (i = 0; i <= max_count; ++i)
    {
        double tNow = get_time_double();
        const auto body = fmt::format("{{\"time\":{},\"item\":{},\"max\":{}}}", tNow, i, max_count);
        // std::cout << " send message # " << i <<  std::endl;
        if (!myCfg.fims_test.Send(
                fims::str_view{"set", sizeof("set") - 1},
                fims::str_view{sub_string.data(), sub_string.size()},
                fims::str_view{nullptr, 0},
                fims::str_view{nullptr, 0},
                fims::str_view{body.data(), body.size()}))
        {
            FPS_ERROR_LOG("For client with inti uri \"{}\": failed to send a fims set message\n", name);
            return false;
        }
        // std::cout << " message  sent [" << body << "]"<< std::endl;
        if (io_fimsListenChan.receive(signal, delay))
        {
            if (0)
            {
                double tRx = get_time_double();
                std::cout << " message  received [" << signal << "] time mS: " << (tRx - tNow) * 1000.0 << std::endl;
            }
        }
        // std::this_thread::sleep_for(delay);
    }

    double tdone = get_time_double();
    std::cout << i << " Messages sent in : " << tdone - tstart << " seconds " << std::endl;

    std::cout << " Now sleeping for 2 seconds" << std::endl;
    std::this_thread::sleep_for(2000ms);

    stop_fims(myCfg);
    stop_process(myCfg);
    std::cout << " gcom_io_fims count :" << io_fims_count << std::endl;
    std::stringstream ss;
    myCfg.fims_message_stats.showNum(ss, "fims message sizes");
    std::cout << "                >>>" << ss.str() << std::endl;

    return true;
}

/// @brief
/// different test sending test URI's
/// @param myCfg
/// @param uri
/// @param method
/// @param body
/// @param debug
/// @return
bool test_fims_send_uri(struct cfg & myCfg, const char *uri, const char *method, const char *body, bool debug)
{
    if (body)
        std::cout << __func__ << " uri " << uri << " method : " << method << " body [" << body << "]" << std::endl;
    else
        std::cout << __func__ << " uri " << uri << " method : " << method << " body ["
                    << "]" << std::endl;

    std::string name("myname");
    const auto sub_string = fmt::format("/components/{}", "comp_sel_2440");

    myCfg.test_fims = true;

    std::vector<std::string> subs;
    subs.emplace_back(sub_string);

    start_process(myCfg);
    double tstart = get_time_double();
    double delay = 0.1;
    uint64_t signal;

    double tNow = get_time_double();
    if (!myCfg.fims_test.Send(
            fims::str_view{uri, strlen(uri)},
            fims::str_view{method, strlen(method)},
            fims::str_view{"/test/resp", sizeof("/test/resp") - 1},
            fims::str_view{nullptr, 0},
            body ? fims::str_view{body, strlen(body)} : fims::str_view{nullptr, 0}))

    {
        FPS_ERROR_LOG("For client with inti uri \"{}\": failed to send a fims set message\n", name);
        return false;
    }
    // std::cout << " message  sent [" << body << "]"<< std::endl;
    if (io_fimsListenChan.receive(signal, delay))
    {
        double tRx = get_time_double();
        std::cout << " message  received [" << signal << "] time mS: " << (tRx - tNow) * 1000.0 << std::endl;
    }

    double tdone = get_time_double();
    std::cout << " Message sent in : " << tdone - tstart << " seconds " << std::endl;
    stop_process(myCfg);
    std::stringstream ss;
    myCfg.fims_message_stats.showNum(ss, "fims message sizes");
    std::cout << "                >>>" << ss.str() << std::endl;

    return true;
}

// see the stuff on archive/src/gcom_fims.cpp it no longer lives here
