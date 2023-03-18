body='{
    "connection": "SC",
    "siteId": "san_diego",
    "siteName": "San Diego",
    "siteControllerPort": 9000
}'
/usr/local/bin/fims_send -m set -u /scheduler/configuration "$body"
