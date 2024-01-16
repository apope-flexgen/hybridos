#!/bin/sh
# command line example
influx -precision rfc3339 --database brp_northfork -execute 'select mbmu_soc,source from mbmu_modbus_data order by time desc limit 1'

