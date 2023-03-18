body='{
    "connection": "SC",
    "siteId": "austin",
    "siteName": "Austin",
    "siteControllerPort": ":9000"
}'
/usr/local/bin/fims_send -m set -u /scheduler/configuration "$body"
