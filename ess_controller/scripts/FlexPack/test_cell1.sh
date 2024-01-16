#!/bin/sh 

function fget() # pass uri as  $1
{
   /usr/local/bin/fims/fims_send -m get -r /$$ -u $1
}

function fset() # pass uri as  $1
{
   /usr/local/bin/fims/fims_send -m set -r /$$ -u $1 $2
}


fset /flex/bms/cell1 '{"value":0,"actions":{"onSet":[{"func":[{"func":"RunCell","amap":"flex"}]}]}}'
fset /flex/bms/cell1 '{"value":2.1,"soc":35}}'
