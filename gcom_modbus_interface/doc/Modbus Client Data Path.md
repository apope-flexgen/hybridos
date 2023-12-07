Modbus Client Data Path

Config Layout
    the config file contains an array of "components"
    each component has an id , and poissibly a deviceId , it can have a hearbeat and a watchdog, 
    it will have a polling frequency and an offset time.
    contains an array of "registers" 
    each register will have a starting offset and a number of registesd ( or data points)
    the numnber of registers include extra registers where a register consists of 2 or 4 words.
    overall the number of reegisters cannot exceed 125 ( I think for Modbus)
    the individual registers are ciontained in a map array.
     Once the raw config onbject is set uo as a map then different mapping structures are created to allow quick accedd to the actual data items.
     here are then main options 
           given a uri and an id where is the configured data item.
           given a device_id, a register type and an offset where is the configured data item.
           I need a list of uris to subscribe to
           I need a list of pub requests sorted by time and offset.
             esch request item should hasve a device_id, register type, offset and number_of _registers.
     there may be more.
  

Client Pubs
     getting Input Input Registers and Holding Registers from the server on a periodic basis and then publishing the data after decoding etc as required by the config in the map item.
     This entails on collecting a series of device_ids , register types , starting offset and number of regs , putting that data in a request object.
     A give pub may have several request objects.
     The current system time is used as an id for the pub request and all the request objects are marked with that same id.
     each request object is then given a sequence number , we know how many of these objects are needed to complete the pub request.

     the IO thread will then process that request and read the data into a request buffer contained in the request object.
     once a read has completred or an error has been detected or the connection timed out the request object is placed on a pubresponse  queue to be procedded by the main thread.

     the main thread will get a collection of request object and then preparea a pub map with the correctly decoded register data.
      if a request object returns with an error . that error is reported  and that block skipped.
      the man thread gets the raw data from the modbus_read operation this data , as I mentioned, has to be decoded and the suitable output object created.
      note thet as we buid up the pub map we will have to combine data from several requests so this has to be a "merging"map.
      Once the set of register requests has been completed , with or without errors the main thread will then "publish" the map.
      the format , naked, clothes or full will be honnored as part of the decode operation.
      so the pub simply converts the pub map into a string and pushis it out.
      during the decode operation the system will place the decoded data into the config data map.
      we can use the time of the pub request to determine of thje current pub data should be placed in the data object. we'll have to keep track of when the last data item was stored and not overwrite an new object with an older one.


Client Sets
    Fims input data from the subscribed uris is collected.
    As the input is read into a map the components and ids from the configured map id inspectged , using the special config map extracted from the main map.
    If there is a data match a data request is created sorted by device ids, register types starint offsetsw and number of registers.
     In addition the fims inoput data has to be encoded from the fims data type into the register format required by the modbus protocol. 
    Once the fims input has been processed ( we will allow multi set operations ) the request will be binned , meaning that adjacent offsets of then same type ( allowing for sizes) will be combined into request obeying the max nunber of registers allowed by modbus (125)
    the set of requests are time stamped and numbered. 
    these binned requests are then placed in the IO_thread request queue.
    Once the requests are processed , with or without errors , the request results are returned , like the pub requests, to the main thread.
    The fims reply (if requested) message can be treated in two ways.
    we can send a reply as soon as we get the fims set request and have put the assembled requests into the IO_trhead queue.
    or we can wait for the IKO threads to complete their response and then  reply with the addition of any error codes , if any are detected.

    Once again the fims reply can be clothed , naked or full.
    the output or reply map is created when the IO thread requests are assembled and just adjusted when the reply is returned.

    Any fims set requests for data items not in the config will be ignored.


