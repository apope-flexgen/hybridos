First the config definition files 
Thse provide  name, initial config file, fims subs list and any blocked uris .

This file is read in before fims subs are set up.
This is why the "blocked" items are important.

Blocked uris are "soft blocked". 

We are subscribed to that uri, we get the header, then discard the rest of the message.
 
This one is for the ess_1 device 
It subscribes to 

    /components/ess_1
    /assets
    /system
    /site/ess_1

The /ess_1 prefix can be used to bypass any missing subscriptions.

Ess_1 also soft blocks , at start up and uris that are subscribed to but are not needed or wanted.
These blocks can be modified ( change true to false ) at run time.
This stops the system collecting unwanted data.


configs/ess_controller/ess_1_file.json
{
    "/sysconfig/default":
    {
        "Help": " This is the default system config file",
        "Subs":":/components/ess_1:/assets:/system/ess_1:/site/ess_1:",
        "Config":"ess_init_dual_1",
        "EssName": "ess_1"
    }
    ,
    "/blockeduris/pub" :
    {
        "/assets/ess/summary":true,
        "/assets/ess/ess_2":false
    },
    "/blockeduris/set" :
    {
        "/assets/ess/summary":true,
        "/assets/ess/ess_2":false
    }

}

configs/ess_controller/ess_2_file.json
{
    "/sysconfig/default": 
    {
        "Help": " This is the default system config file",
        "Subs":":/components/ess_2:/assets:/system:/site/ess_2:",
        "Config":"ess_init_dual_2",
        "EssName": "ess_2"
    }
    ,
    "/blockeduris/pub" :
    {
        "/components/ess_1":true,
        "/system/ess_1":true,
        "/assets/ess/summary":true,
        "/assets/ess_1":true
    },
    "/blockeduris/set" :
    {
        "/components/ess_1":true,
        "/system/ess_1":true,
        "/assets/ess/summary":true,
        "/assets/ess_1":true
    }

}



The system can now be started up with a single "-f" argument.
(
   systemctl enable ess_controller@ess_1_file  
)

Or you can start them from the command line.

sh-4.2# build/release/ess_controller -f ess_1_file      >/tmp/ess1_out 2>&1 &

sh-4.2# build/release/ess_controller -f ess_2_file      >/tmp/ess2_out 2>&1 &



Now you can send data to the individual ess controllers.

The modbus_clients will be configured to direct their pubs to the correct ess_controllers.

sh-4.2# fims_send -m pub -u /components/ess_1/bms_rack_01 '{"temp":34.56}'
sh-4.2# fims_send -m pub -u /components/ess_2/bms_rack_01 '{"temp":24.56}'


The name> prefix can be used to direct queries to a specific ess_controller
(in this case ess_1 or ess_2)
 

sh-4.2# fims_send -m get -r /$$ -u /ess_1/components/ess_1/bms_rack_01
{"temp":{"value":34.56}}

sh-4.2# fims_send -m get -r /$$ -u /ess_2/components/ess_2/bms_rack_01 
{"temp":{"value":24.56}}


But you cannot get data sent to the ess_1 controller from the ess_2 controller.

sh-4.2# fims_send -m get -r /$$ -u /ess_2/components/ess_1/bms_rack_01
   <<Receive Timeout.>>

Other topics 

... Sharing data
... Handling cross system controls

