body='{
    "connection": "FM",
    "sites": [
        {
            "siteId": "austin",
            "siteName": "Austin",
            "siteControllerIP": "172.16.2.82",
            "siteControllerPort": "9000"
        },
        {
            "siteId": "san_diego",
            "siteName": "San Diego",
            "siteControllerIP": "172.16.2.83",
            "siteControllerPort": ":9000"
        }
    ]
}'
/usr/local/bin/fims_send -m set -u /scheduler/configuration "$body"
