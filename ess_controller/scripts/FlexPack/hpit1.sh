#!/bin/sh
# hput and hget

URIS='
/components/bms_info/bms_data
/components/bms_info/bms_power
'
OPTS='
/full
/full/naked
'





hput () {
  eval hash"$1"='$2'
}
hget () {
  eval echo '${hash'"$1"'#hash}'
}
hput compfoo "response foo"
hput compfoo2 "response foo2"
hput compfoo3 "response foo3"
echo `hget compfoo` and `hget compfoo2` and `hget compfoo3`


