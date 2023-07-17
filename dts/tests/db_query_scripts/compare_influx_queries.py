import sys
from influxdb import InfluxDBClient
from pymongo import MongoClient
from bson.objectid import ObjectId
from datetime import datetime, timedelta

# Parse arguments
database = sys.argv[1]
start_time = datetime.strptime(sys.argv[2], '%Y/%m/%dT%H:%M:%S')
time_interval = timedelta(minutes=int(sys.argv[3]))
print(f'Start Time: {start_time}, Time Interval: {time_interval}')
print()

# Compare results of two queries
influx_client = InfluxDBClient('localhost', 8086, database=database)
start_time_epoch = int(start_time.timestamp() * (10 ** 9))
end_time_epoch = int((start_time+time_interval).timestamp() * (10 ** 9))

result_1 = influx_client.query(f'''SELECT LAST(*), MAX(*), MEAN(*), MIN(*) FROM "components_ess_real_hs" WHERE time >= {start_time_epoch} AND time < {end_time_epoch} GROUP BY time(1m)''').raw['series'][0]['values']
print(f'Result 1: {result_1}')
print()

result_2 = influx_client.query(f'''SELECT *::field FROM "120_day_rp"."components_ess_real_hs" WHERE time >= {start_time_epoch} AND time < {end_time_epoch}''').raw['series'][0]['values']
print(f'Result 2: {result_2}')
print()

match = True
if len(result_1) != len(result_2):
    match = False
else:
    for i in range(len(result_1)):
        if len(result_1[i]) != len(result_2[i]):
            match = False
            break
        for j in range(len(result_1[i])):
            if result_1[i][j] != result_2[i][j]:
                match = False
print(match)