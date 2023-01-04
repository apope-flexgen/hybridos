## pyFims Interface 

After installation you can import pyfims_rewrite into your python program.
Give it an alias of pyfims.


```
import sys
import time
sys.path.append(".")
from pyfims_rewrite import pyfims

```


# Set up and make a connection to the server


The argument is the process name for the connection. This must be unique.


```
p = pyfims()
x = p.Connect("pyfims_test2")

```
The status x[1] is zero for a good connection.

```
[43, 0, 'Connect OK']
```

This error means that the fms server is not running.

```
[None, -1, FileNotFoundError(2, 'No such file or directory')]
```  


# Subscribe to one or more uris

Once connected you can subscribe to one or more uris. 

Note that the appliation is "automatically" subscribed to the "process_name"
    
``` 
    # subscribe to more than one
    x = p.Subscribe(["/components/bms_9/cap","/foo"])

    # or

    # subscribe to  just one
    x = p.Subscribe("/components/bms_9/cap")

    # get subscribe details
    if debug:
        print("after subscribe")
        print(x)
        #['{"userName":"fims_server", "body": SUCCESS}', [], 'Success']
```

# Wait for an input  

The argument is a timeout in seconds.

```

    x = p.Receive(10)

    if debug : print(x)

    #['{"method":"set", "uri":"/pyfims_test2", "userName":"fims_send", "body": 44556}', [], 'Success']
```
 x[0] is jso string containing the message.

# Send a message  

Send a message 
   method
   uri
   message

The message can be a "set", "pub"  or a "get" . 
There are no restrictions on the message method.

Only one uri should be specified , you do not have to be subscribed to the URI.
The Body can be a string, a number or a json object.

```
    x = p.Send("set", "/components/bms_1/cap", "99")

    if debug: 
        print ("after set")
        print(x)

    x = p.Send("set", "/components/bms_1/cap", 99)
    if debug: 
        print ("after set")
        print(x)

    if debug: 
        print ("after set")
        print(x)

    x = p.Send("get", "/components/bms_1/cap")
    if debug:
        print ("after get")
        print(x)
```
The response from a successful send shows the (len, status, Mesage)
```
[65, 0, 'Success']

```
## Close the connection

```
    p.Close()
```
