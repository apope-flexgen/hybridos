gcom_modbus_client core processing concepts
p wilshire
4/11/2024


Introduction

The gcom modbus clients introduces some different, more streamlined procesing concepts.
Some of the features in g++ 17 are put to good use.

 - shared pointers
 - threads
 - abstract objects
 - channels ( those are ours but they work great)

Other things to consider.

In addition a lot of attention has been placed on keeping the system alive in the face of degrading networks and vendor equipment problems. Network dropouts and delays are handled as  gracefully as possible.
The client does not curl up and die when data distruptions are detectd. Rather, the client will pause requests and try to reestablish the connection.
The client also handles the data sync problem. If a modbus data stream looses sync you have to pull all the data out of the pipeline and then try to recover the stream, this uses a system called modbus_flush.
You loose the current data but should recover the stream.

The system also provides a lot of feedback information 

In general Walker's awesome code, which was almost unmaintanable, has been replaced with a more modular system.
The code is broken into segments and uses more upto date c++17 techniques.



General Flow.

Read Config.

Lets make this simple we read the json config file into a ifstrem 

// read fname into a file object (std::ifstream)
    auto file = std::ifstream(fname, std::ios::in | std::ios::binary);

// Read content into padded_string
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    padded_content = simdjson::padded_string(content);

// Parse it
    auto result = parser.parse(padded_content);

//std::map<std::string, std::any> &jsonMapOfConfig
//Turn it into an abstrat object which is a map of "any's"
jsonMapOfConfig = jsonToMap(result.value());

So there we have it. The custom FlexGen jsonToMap tries to deal with all the objects that simdjson discovers.


Avoid the "complexity" of jsonToMap for now, how do we find and use data inside this object.


well this function "getItemFromMap" will look for "connection.name" inside the map and place what it finds in the 
myCfg.connection.name string.
It allows for a default value of "modbus_client" , it will return false if the object was not found and we dont want any debug.


bool ok = getItemFromMap(jsonMapOfConfig, "connection.name", myCfg.connection.name, std::string("modbus_client"), true, true, false);

Well this function "getItemFromMap" will handle all known object type and coerce the result into the requested result type.
Its a complex template thing.


This concept is re entrant so , if we have a section called "components" in our jsonMapOfConfig we can extract those fields into  the myCfg components area.
ok = extract_components(jsonMapOfConfig, "components", myCfg, false);

A bit more c++ magic here.


    std::optional<std::vector<std::any>> compArray = getMapValue<std::vector<std::any>>(jsonMapOfConfig, query);
    if (compArray.has_value())
    {
        if (debug)
            std::cout << " Found components " << std::endl;
    }
    else
    {
        FPS_ERROR_LOG("Could not find \"components\" in config file.");
        return false;
    }

and here

    std::vector<std::any> rawCompArray = compArray.value();
    for (const std::any &rawComp : rawCompArray)
    {
        if (debug)
            std::cout << " Processing component" << std::endl;
        if (rawComp.type() == typeid(std::map<std::string, std::any>))
        {
        }
    }

So we have created a way to interrogate this mapped object and navigate around the system.


Next we want to alloc a member of the rawCompArray.
The config object has a vector of cfg::component_struct objects. We will have to create each bject but do not want to have to worry about releasing any code ( no free) when the component structure in no longer needed.

```
    std::map<std::string, std::any> jsonComponentMap = std::any_cast<std::map<std::string, std::any>>(rawComp);
    std::shared_ptr<cfg::component_struct> component = std::make_shared<cfg::component_struct>();
    std::string componentId;
    getItemFromMap(jsonComponentMap, "component_id", componentId, std::string("components"), true, true, false);
```

and once one of the components has been completed

```
    if (ok)
        myCfg.components.push_back(component);
```



Note that the myCfg structure is pseudo static.
The use of the shared pointer structures means that the structure will dissolve gracefully when its no longer needed.

No need to free any shared memory structures. They go away, automatically, when they are no longer referenced.


Its worth looking at one of the functions used in this process.


Here is jsonToAny this takes the incoming simdjson element and turns it into a std::any object.

```
**
 * @brief Convert a simdjson::dom::element to an std::any
 * @param elem simdjson::dom::element JSON element created after parsing using simdjson::dom::parser
 */
std::any jsonToAny(simdjson::dom::element elem)
{
    switch (elem.type()) {
    case simdjson::dom::element_type::INT64:
    {
        int64_t val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            if (val <= INT32_MAX && val >= INT32_MIN)
            {
                return static_cast<int32_t>(val);
            }
            else
            {
                return val;
            }
        }
        break;
    }
    case simdjson::dom::element_type::BOOL:
    {
        bool val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            return val;
        }
        break;
    }
    case simdjson::dom::element_type::UINT64:
    {
        uint64_t val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            if (val <= UINT32_MAX)
            {
                return static_cast<uint32_t>(val);
            }
            else
            {
                return val;
            }
        }
        break;
    }
    case simdjson::dom::element_type::DOUBLE:
    {
        double val;
        if (elem.get(val) == simdjson::SUCCESS)
        {
            return val;
        }
        break;
    }
    case simdjson::dom::element_type::STRING:
    {
        std::string_view sv;
        if (elem.get(sv) == simdjson::SUCCESS)
        {
            return std::string(sv);
        }
        break;
    }
    case simdjson::dom::element_type::ARRAY:
    {
        simdjson::dom::array arr = elem;
        std::vector<std::any> vec;
        for (simdjson::dom::element child : arr)
        {
            vec.push_back(jsonToAny(child));
        }
        return vec;
    }
    case simdjson::dom::element_type::OBJECT:
        return jsonToMap(elem);
    default:
        return std::any(); // Empty std::any
    }
    // If we reach here, something went wrong.
    FPS_ERROR_LOG("Error processing JSON element.");
    return std::any(); // Return empty std::any
}
```
note that it uses , recursively jsonToMAp.

```
/**
 * @brief Convert a simdjson::dom::object to a map<string, any>.
 *
 * @param obj simdjson::dom::object JSON object created after parsing using simdjson::dom::parser
 */
std::map<std::string, std::any> jsonToMap(simdjson::dom::object obj)
{
    std::map<std::string, std::any> map;
    for (auto [key, value] : obj)
    {
        map[std::string(key)] = jsonToAny(value);
    }
    return map;
}
```

So that was not too hard......

Note that we have to use 
 getItemFromMap
and that is a little more complex




/**
 * @brief Given a map<string, any>, lookup the query string in the map, and convert it to type T (if found)
 * or give it a default value (if not found and a valid default is provided).
 *
 * @param map const std::map<std::string, std::any> reference to map to search into
 * @param query the key to look for in the map. Uses dot notation to represent subelements of a larger
 * map (e.g. "connection.ip_address" )
 * @param target where to store the value
 * @param defaultValue the default value to use if the target isn't found
 * @param useDefault true/false value to determine if you use the default when not found
 * @param required do you need a value?
 * @param debug bool value representing whether or not to print messages to help debug code
 *
 * @return true if found or default value was used; false if required and useDefault is false
 */


 here is the code


 ```
 template <typename T>
bool getItemFromMap(const std::map<std::string, std::any> &map, const std::string &query, T &target, const T &defaultValue, bool useDefault, bool required, bool debug)
{

    auto result = getMapValue<T>(map, query);
    if (result.has_value())
    {
        if (debug)
            std::cerr << "Found " << query << " " << result.value() << std::endl;
        target = result.value();
        return true;
    }
    else if (useDefault && required)
    {
        if (debug)
            std::cerr << "Used Default " << query << " " << defaultValue << std::endl;
        target = defaultValue;
        return true;
    }
    else if (required)
    {
        return false;
    }
    target = defaultValue;
    return true;
}
 ```

It all uses getMapValue
```

/**
 * @brief Given a map<string, any>, lookup the query string in the map and convert it to type T
 *
 * @param map const std::map<std::string, std::any> reference to map to search into
 * @param query the key to look for in the map. Uses dot notation to represent subelements of a larger
 * map (e.g. "connection.ip_address" )
 *
 * @return the value (of type T) corresponding to the key found in the map. Returns std::nullopt if not
 * found.
 */
```

```
template <typename T>
std::optional<T> getMapValue(const std::map<std::string, std::any> &map, const std::string &query) {
    std::stringstream ss(query);
    std::string key;
    const std::map<std::string, std::any> *currentMap = &map;

    try {
        while (std::getline(ss, key, '.')) {
            auto it = currentMap->find(key);
            if (it == currentMap->end()) {
                return std::nullopt;
            }

            if (it->second.type() == typeid(std::map<std::string, std::any>)) {
                currentMap = &std::any_cast<const std::map<std::string, std::any> &>(it->second);
            }
            else if (it->second.type() == typeid(T)) {
                if (!std::getline(ss, key, '.')) {
                    return std::any_cast<T>(it->second);
                }
            }
            else if (typeid(T) == typeid(double) && it->second.type() == typeid(int)) {
                if (!std::getline(ss, key, '.')) {
                    return std::any_cast<T>(static_cast<double>((std::any_cast<int>(it->second))));
                    //return static_cast<double>(std::any_cast<int>(it->second));
                }
            }
            else {
                return std::nullopt;
            }
        }
    }
    catch (const std::bad_any_cast &e) {
        std::cout << __func__ << " did not work " << e.what()<< std::endl;  // TODO tidy this up
        // Handle or log the exception
        return std::nullopt;
    }

    return std::nullopt;
}
```
This could do with a bit of help in handling errors .
It does coerce specific data type from std::any objects

Note that you may have to force the compiler to instantiate the templates
```
// Explicit template instantiation

template std::optional<int> getMapValue<int>(const std::map<std::string, std::any>& map, const std::string& query);
template std::optional<double> getMapValue<double>(const std::map<std::string, std::any>& map, const std::string& query);
template std::optional<std::string> getMapValue<std::string>(const std::map<std::string, std::any>& map, const std::string& query);

```


Fims input.

This is handled by the fims_listener thread.
The main features here are the fact that we are using the raw fims message into a self assigned bufer

```
if(myCfg.num_fims < myCfg.max_fims)
{
    io_fims = std::make_shared<IO_Fims>(myCfg.connection.data_buffer_size);
    myCfg.num_fims++;
    return (io_fims);
}
```

Note that we set up the input buffer size here. This means that we could have a bigger buffer size.

```
    ok &= getItemFromMap(jsonMapOfConfig, "connection.data_buffer_size", myCfg.connection.data_buffer_size, 100000, true, true, false);
    if (ok && (myCfg.connection.data_buffer_size <= 10000 || myCfg.connection.data_buffer_size > 200000))
    {
        FPS_INFO_LOG("data_buffer_size must be between 10000 and 200000. Configured value is [%d]. Using default of 100000.", myCfg.connection.data_buffer_size);
        myCfg.connection.data_buffer_size = 100000;
    }
```


It is currently set at 100k but, sadly, this size is hard coded into the fims objects for most of FlexGen applications.
so, without some significant work we are out of luck if the fims messages get bigger.
The system "could" be set up to expand the fims buffer size if it is determined to be too small.
We also can and we do set up more than one buffer.



io_fims data structure
```
struct IO_Fims {
    u64 total_data = 0;
    int fid;    
    std::string work_name;
    std::string_view method_view;
    std::string_view uri_view;
    std::string_view replyto_view;
    std::string_view process_name_view;
    std::string_view username_view;

    struct myMeta_Data_Info meta_data;
    u32 fims_data_buf_len=FIMS_BUFFER_DEFAULT_LEN;
    u32 data_buf_len=FIMS_BUFFER_DEFAULT_LEN;
    u8* fims_input_buf;
    u8* fims_data_buf;
    u64 fims_data_len;
    size_t bytes_read;
    u64 fims_input_buf_len;
    int error;

    IO_Fims()
    {
        fims_data_buf_len = FIMS_BUFFER_DEFAULT_LEN;
        fims_input_buf_len = FIMS_BUFFER_DEFAULT_LEN;
        fims_input_buf = reinterpret_cast<uint8_t *>(calloc(1,fims_data_buf_len));
    };

    IO_Fims(int buffer_len)
    {
        fims_data_buf_len = buffer_len;
        fims_input_buf_len = buffer_len;
        fims_input_buf = reinterpret_cast<uint8_t *>(calloc(1,fims_data_buf_len));
    };
    ~IO_Fims()
    {
        if(fims_input_buf)
        {
           free(fims_input_buf);
        }
    };
}
```
//make_fims
// @brief
///  create an io_fims object if we dont have any old ones.

/// @param myCfg
/// @return
std::shared_ptr<IO_Fims> make_fims(struct cfg &myCfg)
{
    std::shared_ptr<IO_Fims> io_fims;
    io_fims = std::make_shared<IO_Fims>(myCfg.connection.data_buffer_size);
    return (io_fims);
}

```
Here is the modified recieve_raw_message function

```

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
        if (err != EAGAIN) //11
            FPS_ERROR_LOG("Fims read err %d [%s]", err, strerror(err));
        // if (err == EINVAL) //22
        //     close(connection);
    }
    return (int)io_fims->bytes_read > (int)0;
}


The io_fims->fims_input_buf contains the raw data and io_fims->bytes_read is the data size.



The code to make use of the flexible buffer size is here.


```
    auto io_fims = make_fims(myCfg);
    // main loop:
    while (myCfg.keep_fims_running)
    {

        bool ok = gcom_recv_raw_message(myCfg.fims_gateway, io_fims);
        if (ok)
        {
            process_fims_data(io_fims, myCfg);
            // note that if you overwrite io_fims the original io_fims will go out of scope here and auto delete.
            // you can reuse the io_fims buffer by deleting this line. 
            io_fims = make_fims(myCfg);  // this auto deletes the fims buffer.
        }
        else
        {
            if (io_fims->error != EAGAIN && io_fims->error != EWOULDBLOCK)
            {
                //std::cout << "listener_thread :: io_fims error " << io_fims->error << std::endl;
                myCfg.fims_running = false;
                myCfg.keep_fims_running = false;
            }
        }
    }
```


The raw buffer does require some extra handling to get all the fims information out of the header.

```
/// @brief
/// parse the incoming header and infact handle set messages
/// @param myCfg
/// @param io_fims
/// @return
bool parseHeader(struct cfg &myCfg, std::shared_ptr<IO_Fims> io_fims)
{
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
        if (io_fims->method_view == "set")
        {
            std::cout << "We got a set method; uri [" << io_fims->uri_view
                      << "]"
                      << std::endl;
        }
#endif

    std::string replyto = "";
    if (io_fims->meta_data.replyto_len > 0)
    {
        replyto = std::string(io_fims->replyto_view);
    }


    // parse the fims body into an any structure
    bool ipOk = false;

    if (io_fims->method_view != "get")
    {
        // this is where you parse the incoming fims buffer. You will have to turn it into a cJson or simdjson object
        ipOk = gcom_parse_data(io_fims->anyBody, (const char *)io_fims->fims_data_buf, (int)io_fims->fims_data_len, false);
    }


}

```



The fims input is handled by the fims_listener thread. This collects the  fims input message and passes it, using our channels to the process_thread.
Once processed the fims_message buffer is placed on the "recycle" channel. This is used like a lifo queue.
This is a common design pattern used in gcom_modbus


We can handle a high rate of input mesages. We are only limited by process thread speed and the max number of input buffers we permit to be in the system.
When the processFims function has completed it indicates if the buffer is to be recycled.
collect_fims is the fims recycler.
This involves using a holding channel keep the  incoming fims message structures after they have been processed.
The "make_fims"  will create up to (max_fims = 32) buffers.
After that time the make_fims will spin waiting for a free buffer up to the max_fims_wait (100 iterations = 1 sec) time .
The make_fims process will delay 10ms between cycles if it has to wait for a buffer.
So we have a used fims message channel, we get a io_fims buffer from a recycled fims_message , making up to 32 new ones if needed. If still no recycled bufferes are found we will wait for up to 1 second for a free buffer.

In practice, things run fast, and only 3 fims buffers are needed.
There was thought to allow the fims buffer to be extended if needed but this would then be the only application that could do that.

The fims_message io_fims is created as a shared pointer. This structure is self maintaining using the shared pointer referenece count and does not need to be freed.
io_fims buffers are recycled.


The fims_messages are processed by a process_thread once completed a dummy collect thread is used to return the completed io_fims message structure to the buffer store.

The fims message is handled by 
parseHeader
Ths client only handles "get" and "set" messages.
only "set" messages have a body.
get or set messages may have "requests" or modifiers to manage the characteristics of the fims transaction.

The set messages simply encode the fims body into a std::any structure.

```
 ipOk = gcom_parse_data(io_fims->anyBody, (const char *)io_fims->fims_data_buf, (int)io_fims->fims_data_len, false);

```

The uri is decoded into single variable messages or mutiple, sadly, based on the number of segments in uri.
More than 3  then this is a single uri with a combined uri and variable name.
note that this code is not thread safe when deailing with the connect disconnect operations.

The single point data handling is :-
1/ convert the std::any objext into a uint64 value.
2/ place the uint64 value into an io_work object
3/ send the io_work object to the io_thread.


more details.


An io_work object details a task ( really a work ticket item) for an io_thread to send or receive data from the modbus_server.
For example a fims set requset will contain data items to be sent to one or more server registers. The io_work object will contain the values to be sent to the server. If the server send fails the local values of those io_points should only be updated to show the new values if the "set" succeeds. That way a value in the io_point will show the value from the set operation only if that operation succeeded.
If the io_work details a "get" or poll operation. The values from the server are read into the io_work register and then placed into the io_point object by the response thread.

All io_thread objects use the response thread to updata the data point values and present any results to fims.


once again.
The io_point is used as a holder of information about the point, including its offset, type size etc. 
The io_work object holds a list of io_point to be used in a transaction.
If values are to be placed into the destination( server) io_point they are placed into a  short array of uint16 or uint8 objects attached to the io_work ticket.

The io_thread simpley reads values from the server and places them into the io_work register arrays, or takes values out of the 
io_work reigster arrays and send them to the server.


So in the case of a single register get or set operation , the io_point referred to the the overall request is given to the 
io_work object as follows.

```
io_work_single->io_points.emplace_back(io_point);
```

The multi io_point io_work objects have a bit more work to do.


When a request like a poll or a fims get / set operation requires work on multiple registers an io_work object is created and all the io_points affected are  appended to the io_work->io_points vector.

This vector is not yet organised in an efficient manner for the modbus transfer.
To do this the single vector has to be ordered by device_id , register type, point offset.
Points may be disabled, disconnected or forced. 
In addition only 125 consequtive points can be included in a single modbus transaction.

Note that the actual io_points may be in a well orderered block but they may well be spaced out.
To deal with this the concept of gaps was intoduced.
A gap is the space between the end of one point and the start of the next.
Two points separated buy a gap may be includd in a single transaction.
The server may allow reads of data registers within the gap but no data points or it may respond with an illegal address message.

The single vector or collection of significant io points is split up into one or more io_works objects containing blocks of data items for consumption by the io_threads.

Having done that the io_work objects are collected into a group and sent to the io_threads on either the poll channel or the send channel.

THe config option that enables this "gaps" processing is true by default.
// allow_gaps
//    ok &= getItemFromMap(jsonMapOfConfig, "connection.allow_gaps", myCfg.allow_gaps, true, true, true, false);



Next comes the io_thread handling

The purpose of the io_thread is to send a single io_work item to or receive a block of data from the server.
The server may not like the block, it may not even be connected.

If the server is connected and responds with some kind of error the io_thread will attempt to deal with the problem.
lets start with the somple ons first.

Not Connected.
The io_thread will go into a lets" try to connect mode" It does not slam the server with connection requests but tries a number of times with a 200 millisec gap.
Note that this logic may need some attention.

Invalid Data
Server not ready etc 

No undue cause for alarm run a modbus_flush  and then try the io point again.
After two attempts the thread will mark the transaction as failed and moe on the the next.


Illegal Data Address.
This is the big one.
It happens where there is a mis alignment between the client config file and the server configuration.
Any points specified in the client configuration that are not in the corresponding server configuration will not be accepted by the server and the Illegal Data Address error returned for the whole block.
The approach taken by gcom_modbus_client is to attempt to detect the points in error.
Note that this can become confusing since a mosbus server may decide to fill in all data points between the highest and lowest ofsets for a particular group.

Scenario: We tried to communicate with a one or more registers in a block. One or more of the registers failed.
The io_thread will then step through each data point in the block to identify the point failure.
No smart searches just one point at a time.  
If the point has a "gap" the gap will be removed
and the operation repeated.

Once the incorrct point has been identified the point is flagged as "disconnected".
An error message will be produced.
The point will be "disconnected" for 5 seconds before the next attempt to include the point.

The next time the io_work vector is quantized this point will be diconnected.
This approach means that all the invalid points in a group will be detected.

The block will be split up into one extra group one before, the other after the disconnected point.

The actual "start_offset" and "number_of_registers" config options no longer have the same significance.

This feature is intended to solve the problem where a couple of misconfigurd data points stop the data client server communications process.
It is also intended to assist when the vendor has changed point definitions.
It well may cause a little chaos when it is first used because the onus is now on the developer to detect the missing data. 
This will , possibly, also insist  the software team since the missing point cause or point of failure is no longer the software developer.
If the point is really no longer needed , simply remove it fron the configuration and it will no longer be used.
If the io_point IS needed then apply the same logic, go find the correct offset and update the config.


Possible problems.
Some servers are finiky about not allowing multi register operations and some insist on using multi register operations even when addressing single registers.
Config options are available allow for these options. TODO complete these.
This dynamic "binning" of data points may break some systems. They should not but , so far , no provision has been made to designate an io_point as the starting point for an io_block. I may put that inplace before I pass the package over.


Connection timeouts.

This has been one of the big problems with the communications systems.
Up to now the network connections may be unstable an the modbus server / client code stops working and we get a comms Loss. 
Who get the call , the modbus cleint / server code developers.
Can they fix the problem. no the code is not crashing for no reason it is behaving as designed to look like its crashing 
because the network is not working.

The new code will make some pretty serious reconnection attempts before giving up.
One of the features of the Modbus TCP link is the fact that the connection will break if a transaction timeout occurs.
The new code differeneiates bertween a Connection timeout and a Transaction timeout.
A connection timeout could take a little longer since the network may have to route the connection.
The transaction timeout should be a shorter timeout since the link should be established.
The system will wait for the transaction timeout before causing a break in the link.
This is only for TCP (ethernet traffic) more about RTU ( serial links) later.
A heavily worked system with fast polling times should also have quite short transaction timeouts.
The system  may have to use a timeout for each data point. With fast flowing data the syste can get backed up quite quickly.
If the transaction , for some reason , takes   too long  the whole io_thread may get backed up.
The system has some safeguards to reduce this. Group gets can only be backed up by , say three requests.
Timeouts mean have an early detection of dropped communications. Shorter timeput also mean that the io_thread will not have to wait so long for an io problem to be detected.
Even with short transaction timeouts (for example .010 seconds) and 500 data points, a badly behaving connection can take 5 seconds to process.
The things we do not want to happen,if possible , is for dealays on getting (pub) data from preventing modbus sets (coils and holding registers) We are limited in options and remedial actions . The gcom_modbus_client will try to service the data .
At the end of the day the designer will have to use heartbeats and timeouts to monitor the integrity of communicaitons.

Serial Connections.
These are different problems.
Exceeding the transaction timeout on a serial connection does not mean that the connection has been broken.
It may mean interfence or it may mean a broken or insecure physical connection.
The system cannot use the loss of the connection to imply that the link has broken, no need to reconnect to the port. Just try to send the data.
A short transaction timeout should be used, in this case the system will have to be tuned to optimize its performance.




Simulating Network Problems
The gcom_modbus_server has a feature that allows it to insert errors into the modbus query response.
The uri used for these controls must be the base uri 

For example the base uri for a server config containing the system section

```
"system": {
        "connection name": "FlexGen_ESS_Controller",
        "ip_address": "0.0.0.0",
        "port": 502,
        "name": "FlexGen_ESS_Controller",
        "id": "/site"
    },
```
will be 
```
"/interfaces/site"
```
To trigger the debug options a "/_bug" request must be added to the base uri

For example:

 insert a delay of 2 seconds into the response time for a single  incoming  modbus query
    fims_send -m pub -u /interfaces/site/_bug '{"sdelay":2000}

 insert a delay of 0.2 seconds into the response time for a all  incoming  modbus querys
    fims_send -m pub -u /interfaces/site/_bug '{"delay":200}

 insert a delay of 4 seconds into the response time for an incoming modbus connect 
    fims_send -m pub -u /interfaces/site/_bug '{"cdelay":4000}




Pub Request Generator

The pub request generator will limit the repeted requests for modbus queries (pub requests) to 3 outstanding requests.
these will be serviced in a period of time based on the connection and transaction timeouts.
If a request is generated with no active threads registering as being connected to  a server, the io_thread process is bypassed and the io_work packets serviced as errors.

Pubs are not generated when there are no io threads registered as connected.
The pub request timeout still runs but the pub request is bypassed.
A maximum of 2 pub request backlogs are created.

These backlogs, in the case of a slow connection will cause the pub request io_work objects  to enter into a backlog.
These requests are cleared as not complete basd on the transfer timeput.

As soon as the connection is restored the pub requests should be serviced in a timely fashion and "normal" operation will be restored.
It is expected that a number of io_work objects will be created,  and saved on the storage channel.
The storage channel is emptied and listed when the system is closed.

```
IO Work Object [10]     Count: 1030     Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [11]     Count: 1030     Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [12]     Count: 1030     Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [13]     Count: 1030     Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [26]     Count: 285      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [28]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [29]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [30]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [31]     Count: 290      Name:    deleted
IO Work Object [32]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [33]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [34]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [35]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [36]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [37]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [38]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [39]     Count: 290      Name: pub_components_flexgen_ess_65_ls   deleted
IO Work Object [1]      Count: 1032     Name: pub_components_flexgen_ess_65_ls   deleted
```



The io_task ( the modbus interface )

The io_task thread pulls io_work objects from the send or poll channels.
This are set up as multi consumer multi producer queues.
An io_work object can be local , in which case the data is set or received from the local io_point storage.
The io_work object will still be processed by the io_thread but no modbus interchange will take place.

Io_thread 0 is the designated local thread. This thread will remain in place and never expect to connect to the server.
It is used for local interaction with the io_points.

On completion of an io_work task. The io_thread will relay the io_work object to the io_response thread.
This response thread has the dutiy to collate all the members of an io_group.
note that ONLY the io_response thread can update the data values in the io_point objects. THis is to provid a single thread access to the data in the points.

Each io_work object can be designated as a single action, or be a member of a multiple io_work actions. 
This is where, for example, in a fims resuest more that one io_point is referenced and the device_id, type and offsets  of eack point require multipleio_work objects,

In addition a formal action like a pub request, will have the pub_request associatd with all the io_work objects that mke up the pub request.
The pub requests all have the same name but each request also has the time of the request creation in its structure.
Each io_work object will also have the same time of creation recorded in its structure.
Since the io_threads can proces different io_work items at different times the group name and the creation time are used to identify the different io_work objects.
The io_work and pub group  creation time recording allows older io_work items to be discarded if they are recieved out of sequence.
A compound "set" group also has the same creation time based grouping management. 
Currently there is no throttling of set objects. The dbounce system can be used but we may need additional set object control.



Io_Thread Modbus communications.
The io_thread  is given an io_work object and it has to exchange the items in its data buffer e=with the values on the server.
The io_thread hs to maintain the connection status with the server, When an io_thread is not connected it will attempt to reconnect.
THere are hard coded pauses in the logic for these reconnection attempts.
If an io_thread has a valid connection with the server it can process , or attempt to process, an io_work items.
This will be to either send or receive register or bit information with the server.
This transaction will be subject to a timeout.
With a TCP (Ethernet) connection , if the io_thread times out , it will have to be reconnected.
With a RDU (Serial) connection the io_threa doe not need to restore the connection, the connection is to the local serial port and it i not broken in the case od a f=timeout.

There are several other errors that can arise in the case of a data transfer attempt.
Some may be solved with a retry of the data transfer ( server busy for example) 
Other are cause by loss of sync with the modbus connection.
The "Invalid Data" error is a very common one. 
This would in the original modbus_client result in a dropped connection.
THe new gcom_modbus_client recognises this error condition as a data sync failure and issues a modbus_flush command.
This should have the effect of clearing the modbus data channel from any partially decoded commands and preparin the connection for a new query and response sequence.

The io_thread will immediately retry the failed transfer one time.

The most significant transfer error is  "Illegal Data Address" this is really bad news.
There is, possibly, a mismatch between the client and server configs.
The client is trying to reach a register that the server does not accept.

The gcom_modbus_client knows that one data point offset in the io_work object is in error.
THe system then will try each data point, one by one, and try to isolate the illegal address.
This requires an individual transaction for each of the points in a group.

If one or more io_points are discovered as the reason for the Illegal Data Address status, those points are marked as disconnected.
Future attempts to include these points in an io_work transaction are ignored.

The original io_work object will be fragmented to bypass the Illegal Address points.
An error message will be generated . 
This may be due to a server misconfiguration (after an upgrade) or it may be a "simple" error.
Just to remind the user, such points are reconnected every 5 seconds to cover the case of a temporary failure in the server.


The config parameter that controls all of this is

connection.auto_disable

It is defaulted to true.



    // data_buffer_size
    ok &= getItemFromMap(jsonMapOfConfig, "connection.data_buffer_size", myCfg.connection.data_buffer_size, 100000, true, true, false);
    if (ok && (myCfg.connection.data_buffer_size <= 10000 || myCfg.connection.data_buffer_size > 200000))
    {
        FPS_INFO_LOG("data_buffer_size must be between 10000 and 200000. Configured value is [%d]. Using default of 100000.", myCfg.connection.data_buffer_size);
        myCfg.connection.data_buffer_size = 100000;
    }

    // auto_disable
    ok &= getItemFromMap(jsonMapOfConfig, "connection.auto_disable", myCfg.auto_disable, true, true, true, false);

    // allow_multi_sets turn off to limit sets to just one register
    ok &= getItemFromMap(jsonMapOfConfig, "connection.allow_multi_sets", myCfg.allow_multi_sets, true, true, true, false);

    // allow_multi_gets
    ok &= getItemFromMap(jsonMapOfConfig, "connection.allow_multi_gets", myCfg.allow_multi_gets, true, true, true, false);

    // force_multi_sets ; use multi even for just one register
    ok &= getItemFromMap(jsonMapOfConfig, "connection.force_multi_sets", myCfg.force_multi_sets, false, true, true, false);

    // force_multi_gets ; use multi even for just one register
    ok &= getItemFromMap(jsonMapOfConfig, "connection.force_multi_gets", myCfg.force_multi_gets, false, true, true, false);






Modbus server fims interface
this still uses the original fims format 


So this message will not work 
    fims_send -m pub -u /site/ess_ls/active_power 34567 

but these will work
    fims_send -m pub -u /components/ess_hs '{"active_power": 34567 }' 
    fims_send -m pub -u /components/ess_hs '{"active_power": {"value":34567 }}' 

    fims_send -m pub -u /components/ess_hs '{"active_power": 34567 ,"reactive_power": 34567 }' 
    fims_send -m pub -u /components/ess_hs '{"active_power": {"value":34567 }, "reactive_power": {"value":34567 }}' 


 










































