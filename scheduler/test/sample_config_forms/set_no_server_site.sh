body='{
    "connection": "SC",
    "siteId": "austin",
    "siteName": "Austin"
}'
/usr/local/bin/fims_send -m set -u /scheduler/configuration "$body"