# set up build env
export PATH=/usr/bin:$PATH
export LD_LIBRARY_PATH=build/release:/usr/local/lib:/usr/local/lib64:

function fimss() # pass uri as  $1 data as $2
{
   /usr/local/bin/fims/fims_send -m set -r /$$ -u $1 '$2'
}

function fimsg() # pass uri as  $1
{
   /usr/local/bin/fims/fims_send -m get -r /$$ -u $1
}

function fimsl() # pass subs as  $1
{
   if [ $# -gt 1  ]; then 
   	/usr/local/bin/fims/fims_listen  -s $1
   else
   	/usr/local/bin/fims/fims_listen
   fi
}

