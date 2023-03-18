body='{"value": '
body+=$2
body+='}'
/usr/local/bin/fims_send -m set -u /scheduler/scada/write/$1 "$body"