    1  fims_listem
    2  fims_listen
    3  fims_send -m get -r /$$ -u/ess/full/controls/bms
    4  fims_send -m get -r /$$ -u/ess/full/controls/bms } jq
    5  fims_send -m get -r /$$ -u/ess/full/controls/bms | jq
    6  fims_listen
    7  fims_send -m get -r /$$ -u/ess/full/controls/bms | jq
    8  fims_send -m get -r /$$ -u/ess/full/controls/bms/oncmd_test | jq
    9  fims_send -m get -r /$$ -u/ess/full/controls/bms/oncmd_test | jq
   10  fims_send -m get -r /$$ -u/ess/full/controls/bms/oncmd_test | jq
   11  fims_listen
   12  fims_send -m get -r /$$ -u/ess/full/controls/bms/oncmd_test | jq
   13  fims_listen
   14  ps 
   15  ps ax
   16  ess_controller -x &
   17  fims_listen
   18  metrics --help
   19  yum install -y metrics
   20  metrics -h
   21  metrics --help
   22  metrics 
   23  Error: Please supply a path to .json configuration file. Usage: node metrics.js path/to/config/
   24  yum install -y echo
   25  echo
   26  echo --help
   27  /usr/locl/bin/echo -h
   28  /usr/local/bin/echo -h
   29  /usr/local/bin/echo --help
   30  -c string
   31          client config file path for server echo generation only
   32    -ip string
   33          address used in the server files fo r client server connection (default "0.0.0.0")
   34    -mode string
   35          set mode: [modbus | dnp3]
   36    -output string
   37          this is the file path where the server and echo file will be going
   38    -prof string
   39  /usr/local/bin/echo 
   40  /usr/local/bin/echo   
   41  twins
   42  sudo yum install twins
   43  twins
   44  twins
   45  Twins: 2023/04/03 15:50:49 Configuring
   46  Twins: 2023/04/03 15:50:49 Config path argument not found. Usage 'twins /path/to/config'. Trying current working directory
   47  Twins: 2023/04/03 15:50:49 Couldn't read the file twins.json: open twins.json: no such file or directory
   48  ess_controller
   49  ess_controller
   50  [0.000135616s] [info    ] [scheduler      ] hello scheduler [essSched]
   51  [0.0171721s] [info    ] [setVal         ] setVal setting lVal->valuestring to  [ess]
   52  [0.0172149s] [info    ] [setVal         ] setVal setting aVal->valuestring to  [ess]
   53  [0.0172319s] [info    ] [setFunc        ] Note: adding aname [ess] to funMap av 0x7fce24a08830
   54  [0.017242s ] [info    ] [setFunc        ] Note: adding aname [bms] to funMap av 0x7fce24a08830
   55  [0.0172489s] [info    ] [setFunc        ] Note: adding aname [pcs] to funMap av 0x7fce24a08830
   56  ess_controller -h
   57  usage: ess_controller <options> 
   58   name:    [ess]
   59   config:  [ess_init]
   60   cfile:   [Undefined]
   61   subs:    [:/components:/assets:/system:/site:/reload:/misc2:]
   62   options:
   63   -?   print help
   64   -h   print help
   65   -v   print version info
   66   -x   disabled cfg dbi auto load
   67   -s   sets the subs list <:/ess:/components:/site:>
   68   -c   initial dbi config file <ess_config_risen_sungrow>
   69   -n   sets the default name <ess>
   70   -d   sets the config dir <deprecated>
   71   -f   initial config file (before fims setup)
   72  yum install -y dbi
   73  dbi
   74  dbi -h
   75  dbi
   76  dbi --help
   77  yum install -y ftd
   78  ftd
   79  ftd -h
   80  sh-4.2# ftd
   81  sh-4.2# ftd -h
   82  Usage of ftd:
   83    -c string
   84          specify the config file for the process (default "config.json")
   85    -config string
   86          specify the config file for the process (default "config.json")
   87    -logCfg string
   88          If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.
   89  ls /home/docker/git2/integration_dev/config/
   90  ls /home/docker/git2/integration_dev/config/ess_controller/config/scripts/
   91  ls /home/docker/git2/integration_dev/config/ess_controller/config/scripts/dbi.sh 
   92  ls /home/docker/git2/integration_dev/config/site_controller/config/scripts/dbi.sh 
   93  diff  -u /home/docker/git2/integration_dev/config/site_controller/config/scripts/dbi.sh /home/docker/git2/integration_dev/config/ess_controller/config/scripts/dbi.sh 
   94  more /home/docker/git2/integration_dev/config/ess_controller/config/scripts/dbi.sh 
   95  /home/docker/git2/integration_dev/config/ess_controller/config/scripts/dbi.sh 
   96  #!/bin/bash
   97  mapfile -t rpms < /home/config/scripts/rpm.txt
   98  path=$(readlink -f "$1")
   99  echo -e "$path"
  100  fims_send="/usr/local/bin/fims_send"
  101  if [ "$#" -ne 1 ]; then     echo -e "[dbi_load.sh] you have died of dysentery, try again.";     exit 1; fi
  102  more /etc/ssh/sshd_config 
  103  nano /etc/ssh/sshd_config 
  104  fims_send -m set -u /componentsALT_2440 '{"trouble_relay": true}'
  105  fims_send -m set -u /components/alt_2440 '{"trouble_relay": true}'
  106  fims_send -m set -r /$$ -u /components/alt_2440 '{"trouble_relay": true}'
  107  fims_send -m set -r /$$ -u /interfaces/alt_2440 '{"trouble_relay": true}'
  108  fims_send -m pub -u /components/alt_2440 '{"trouble_relay": true}'
  109  fims_send -m pub -u /components/alt_2440 '{"trouble_relay":{"value": true}}'
  110  fims_send -m pub -u /components/alt_2440 '{"trouble_relay":{"value": false}}'
  111  fims_send -m pub -u /components/alt_2440 '{"trouble_relay":{"value": true}}'
  112  fims_send -m pub -u /components/alt_2440 '{"fire_relay":{"value": true}}'
  113  fims_send -m pub -u /components/alt_2440 '{"trouble_relay":{"value": true}}'
  114  fims_send -m pub -u /components/alt_2440 '{"e_stop":{"value": true}}'
  115  fims_send -m pub -u /components/alt_2440 '{"e_stop":{"value": false}}'
  116  fims_send -m pub -u /components/alt_2440 '{"e_stop":true}'
  117  fims_send -m pub -u /components/alt_2440 '{"e_stop":0}'
  118  fims_send -m pub -u /components/alt_2440 '{"e_stop":1}'
  119  fims_send -m pub -u /components/alt_2440 '{"fuse_monitoring":1}'
  120  fims_send -m pub -u /components/sel_2440 '{"input_1":1}'
  121  fims_send -m pub -u /components/sel_2440 '{"input_1":1}'
  122  fims_send -m pub -u /components/sel_2440 '{"input_2":1}'
  123  fims_send -m pub -u /components/sel_2440 '{"input_2":123}'
  124  fims_send -m pub -u /components/sel_2440 '{"input_3":3}'
  125  fims_send -m pub -u /components/sel_2440 '{"input_0":100}'
  126  fims_send -m pub -u /components/sel_2440 '{"input_4":-10}'
  127  fims_send -m pub -u /components/sel_2440 '{"input_5":-10}'
  128  fims_send -m pub -u /components/sel_2440 '{"input_6":1.10}'
  129  fims_send -m pub -u /components/sel_2440 '{"input_6_1":1.10}'
  130  fims_send -m pub -u /components/sel_2440 '{"input_6_2":1.10}'
  131  fims_send -m pub -u /components/sel_2440 '{"input_6_1":1.10}'
  132  fims_send -m pub -u /components/sel_2440 '{"input_6_2":2.10}'
  133  fims_send -m pub -u /components/sel_2440 '{"finput_6_2":2.10}'
  134  fims_send -m pub -u /components/sel_2440 '{"finput_6_2":2.123}'
  135  fims_send -m pub -u /components/sel_2440 '{"finput_6_2":-2.123}'
  136  fims_send -m pub -u /components/sel_2440 '{"finput_6_2":-2.123}'
  137  fims_send -m pub -u /components/sel_2440 '{"finput_6_4":-2.123}'
  138  fims_send -m pub -u /components/sel_2440 '{"input_7_4":7450}'
  139  fims_send -m pub -u /components/sel_2440 '{"input_7_2":7450}'
  140  fims_send -m pub -u /components/sel_2440 '{"input_8_2":7450}'
  141  fims_send -m pub -u /components/sel_2440 '{"input_3_2":3250}'
  142  fims_send -m pub -u /components/sel_2440 '{"input_3_4":3250}'
  143  fims_send -m pub -u /components/sel_2440 '{"input_3_2":3251}'
  144  fims_send -m pub -u /components/sel_2440 '{"input_7_2":7}'
  145  fims_send -m pub -u /components/sel_2440 '{"input_0_4":4}'
  146  fims_send -m pub -u /components/sel_2440 '{"input_0_4":4}'
  147  fims_send -m pub -u /components/sel_2440 '{"finput_6_4":4.56}'
  148  fims_send -m pub -u /components/sel_2440 '{"bs_256":2}'
  149  fims_send -m pub -u /components/sel_2440 '{"bs_256":0}'
  150  fims_send -m pub -u /components/sel_2440 '{"enum_259_4":5}'
  151  fims_send -m pub -u /components/sel_2440 '{"enum_259_4":5}'
  152  fims_send -m pub -u /components/sel_2440 '{"enum_259_4":2}'
  153  fims_send -m pub -u /components/sel_2440 '{"ibits_2000_1":2}'
  154  fims_send -m pub -u /components/sel_2440 '{"ibits_2001_2":3}'
  155  fims_send -m pub -u /components/sel_2440 '{"ibits_2001_2":0}'
  156  fims_send -m pub -u /components/sel_2440 '{"bf_2007_1":7}'
  157  fims_send -m pub -u /components/sel_2440 '{"bf_2007_1":15}'
  158  fims_send -m pub -u /components/sel_2440 '{"bf_2007_1":31}'
  159  fims_send -m pub -u /components/sel_2440 '{"bf_2007_1":63}'
  160  fims_send -m pub -u /components/sel_2440 '{"pack_1":66}'
  161  fims_send -m pub -u /components/sel_2440 '{"pack_1":66}'
  162  fims_send -m pub -u /components/sel_2440 '{"pack_1":66}'
  163  history
  164  pwd
  165  cd /home/docker/git1/hybridos/modbus_interface/
  166  ls scripts
  167  history > scripts/test_hist.sh
