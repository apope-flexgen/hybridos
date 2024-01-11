#!/bin/sh
# command line example
influx -precision rfc3339 --database brp_northfork -execute 'select sbmu_soc,source from sbmu_modbus_data order by time desc limit 10'

