if [ ${tval} == 0 ] ; then
tval=234
fi

tval=0
#export tval
while [ ${tval} -lt 200 ] ; do 
  /usr/local/bin/fims_send -m set  -u /site/operation/test_signed_int16 ${tval} 
  tval=$(($tval+1))
done
#/usr/local/bin/fims_send -m set  -u /site/operation/test_signed_int16 ${tval} 
#tval=$(($tval+1))
#/usr/local/bin/fims_send -m set  -u /site/operation/test_signed_int16 ${tval} 
#tval=$(($tval+1))
#/usr/local/bin/fims_send -m set  -u /site/operation/test_signed_int16 ${tval} 
#tval=$(($tval+1))
