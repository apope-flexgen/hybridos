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

# List fields for each measurement
print('Influx field entry counts')
influx_client = InfluxDBClient('localhost', 8086, database=database)
start_time_epoch = int(start_time.timestamp() * (10 ** 9))
end_time_epoch = int((start_time+time_interval).timestamp() * (10 ** 9))
measurements = influx_client.query('''SHOW MEASUREMENTS''').get_points()
for measurement in measurements:
    measurement_name = measurement['name']
    fields = influx_client.query(f'''SHOW FIELD KEYS FROM "{measurement_name}"''').get_points()

    print(f'Measurement: {measurement_name}')
    for field in fields:
        print(f'Field: {field["fieldKey"]}: {field["fieldType"]}')
print()

# List fields
print('Mongo document counts')
mongo_client = MongoClient('localhost', 27017)
start_id = ObjectId.from_datetime(start_time)
end_id = ObjectId.from_datetime(start_time + time_interval)
for collection_name in mongo_client[database].list_collection_names():
    documents = mongo_client[database][collection_name].find({"_id": {"$gte": start_id, "$lt": end_id}})
    for doc in documents:
        print(f'Collection: {collection_name}, Fields: {doc}')