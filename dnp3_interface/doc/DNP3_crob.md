## DNP3 Crob (Control Relay Output Block)

P Wilshire 02-18-2022

# Crob output 

The CROB (Control Relay Output Block) is one of the command objects that the DMP3 Client can use to control the outstation.

The 9.3 release did not fully manage the object. It was hard coded to "make it work".

The 10.1 release now fully covers the CROB options and provides a flexible means to configure the system.


# Crob messages

Extract from 
```
opendnp3-3.1.1/cpp/lib/include/opendnp3/gen/OperationType.h
```

Used in conjunction with Trip Close Code in a CROB to describe what action to perform
Refer to section A.8.1 of IEEE 1815-2012 for a full description

```
enum class OperationType : uint8_t
{
  /// Do nothing.
  NUL = 0x0,
  /// Set output to active for the duration of the On-time.
  PULSE_ON = 0x1,
  /// Non-interoperable code. Do not use for new applications.
  PULSE_OFF = 0x2,
  /// Set output to active.
  LATCH_ON = 0x3,
  /// Set the output to inactive.
  LATCH_OFF = 0x4,
  /// Undefined.
  Undefined = 0xFF
};
```


Most importantly the system can use "LATCH_ON" to define the object true state 
and "LATCH_OFF" to define the object false state.


# Client input messages

In this implementation, a wide range of options are available to set the CROB state at the client site.

```
fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  true}'

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  false}'

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  "LATCH_ON"}'

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  "LATCH_OFF"}'

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  3}'

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  4}'
```

The default configuration will allow "true" to set the "LATCH_ON" state internally and result in a "true" output from te server.

The other input options are all available for a CROB input all will provide the same result.


```

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  "LATCH_ON"}'

fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  3}'

```

# CROB client configuration

This is a sample of the client configuration file for a CROB input.
Note that all the default options are covered.

```
{
    "registers": [
        {
            "type": "CROB",
            "map": [
                {
                    "id": "remote_enable_flag",
                    "offset": 0,
                    "name": "Start Site Command",
                    "uri": "/site/operation"
                },
                {
                    "id": "remote_disable_flag",
                    "offset": 1,
                    "name": "Disable Site Command",
                    "uri": "/site/operation"
                }
            ]
        }
    ]
}
```


Two optional config items can be added to the configuration to change the default options if needed.

```
    "crob_true": "LATCH_ON",
    "crob_false": "LATCH_OFF"
```

or 

```
    "crob_true": 3,
    "crob_false": 4

```

On the client side, these options define the meaning of the "true" or "false" inputs to the system.


# Crob server configuration


A wider range of output options are available
```

   {
            "type": "CROB",
            "map": [
                {
                    "id": "remote_enable_flag",
                    "offset": 0,
                    "name": "Start Site Command",
                    "uri": "/site/operation",
                    "crob_string" :true
                },
                {
                    "id": "remote_disable_flag",
                    "offset": 1,
                    "name": "Disable Site Command",
                    "uri": "/site/operation",
                    "crob_string" : false,
                    "crob_true":"LATCH_ON",
                    "crob_false":"LATCH_OFF"
                }
   }

```

Here is a summary

```

"crob_string" : true  means that the value received from the server is displayed as a string , the value must be one of the predefined crob object types.
"crob_int" : true  overrides crob_string means that the value received from the server is displayed as an integer object types.
"crob_true" : "LATCH_ON:  defines the incoming crob type that will result in a true default output.
"crob_false" : "LATCH_OFF:  defines the incoming crob type that will result in a true default output.

```

# Crob output messages.

First here are the results with crob_int:true configured.
This is useful to discover what the client is sending as crob messages.


``` 

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":1}
Timestamp:   2022-02-18 13:49:00.93297

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":2}

Timestamp:   2022-02-18 13:49:00.660790
Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":3}

Timestamp:   2022-02-18 13:49:01.58602
Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":4}
Timestamp:   2022-02-18 13:49:01.255601

```

Next with "crob_string" set true in the server config we can see the string value and the "ovalue" that would be used.


```

Method:  set
Uri:     /site/operation/remote_disable_flag
Body:    {"value":"PULSE_ON"}
Timestamp:   2022-02-18 13:52:30.742838

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":"PULSE_OFF"}
Timestamp:   2022-02-18 13:52:31.347608

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":"LATCH_ON","ovalue":true}
Timestamp:   2022-02-18 13:52:31.892976

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value":"LATCH_OFF","ovalue":false}
Timestamp:   2022-02-18 13:52:32.118773

```

Note the "ovalue" field which shows the true/false interpretation of the string value.


Next with both those options set false

```
ReplyTo: (null)
Body:    {"value": false}
Timestamp:   2022-02-18 13:54:58.69774
Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": true}
Timestamp:   2022-02-18 13:55:00.571285
Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": false}
Timestamp:   2022-02-18 13:55:00.783639
```



Lastly , if needed, meaning of LATCH_OFF/LATCH_ON  can be inverted.
Or PULSE_ON/OFF  can be used instead.

Change the old config  
```
    {
        "id": "remote_disable_flag",
        "offset": 1,
        "name": "Disable Site Command",
        "uri": "/site/operation",
        "crob_string" : true,
        "crob_true": "LATCH_ON",
        "crob_false": "LATCH_OFF"
    }
```

To this

```
    {
        "id": "remote_disable_flag",
        "offset": 1,
        "name": "Disable Site Command",
        "uri": "/site/operation",
        "crob_string" : true,
        "crob_true": "LATCH_OFF,
        "crob_false":"LATCH_ON"
    }
```

And we get ...

```
Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": "LATCH_ON", "ovalue": false}
Timestamp:   2022-02-18 14:01:10.867168

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": "LATCH_OFF", "ovalue": true}
Timestamp:   2022-02-18 14:01:11.83283

```

The "ovalue" is the output value we'd get if both "crob_int" and "crob_string" config items were missing or false.

Try this server config.

This configures the "PULSE_ON" crob state to provide the "true" output value.

The other states are ignored except for "LATCH_OFF" that triggers a "false" output.


```
{
    "id": "remote_disable_flag",
    "offset": 1,
    "name": "Disable Site Command",
    "uri": "/site/operation",
    "crob_int" : false,
    "crob_string" : true,
    "crob_true": "PULSE_ON",
    "crob_false": "LATCH_OFF"

}
```

Resulting output. 

```
Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": "PULSE_ON", "ovalue": true}
Timestamp:   2022-02-18 14:08:55.280242

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": "PULSE_OFF"}
Timestamp:   2022-02-18 14:08:55.884548

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": "LATCH_ON"}
Timestamp:   2022-02-18 14:08:56.328087

Method:  set
Uri:     /site/operation/remote_disable_flag
ReplyTo: (null)
Body:    {"value": "LATCH_OFF", "ovalue": false}
Timestamp:   2022-02-18 14:08:56.548895

```

# Conclusion 

These options provide a way to assist with the set up of a server with a CROB interface to a customer provided client system.
Or to adapt the FlexGen outputs to a client to match the needs for a customer server. 

With simple config changes the integration team can visualize the data coming from the client and then configure the server as needed.

It is expected that the "true"/"false" outputs will be used in the final system but they can easily be configured to match different Client DNP3 messages during commissioning.