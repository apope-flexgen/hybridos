contents=$(sed -r 's|["]|\\"|g' /usr/local/etc/config/washer/query.wsdl)
body=$(printf '{"document": "%s"}' "$contents")

fims_send -m set -u /dbi/washer/queryDocument "$body"