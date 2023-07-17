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

# Count datapoints (field entries) in an Influx measurement with the given tag
print('Influx field entry counts')
influx_client = InfluxDBClient('localhost', 8086, database=database)
start_time_epoch = int(start_time.timestamp() * (10 ** 9))
end_time_epoch = int((start_time+time_interval).timestamp() * (10 ** 9))
measurements = influx_client.query('''SHOW MEASUREMENTS''').get_points()
for measurement in measurements:
    measurement_name = measurement['name']
    sources = influx_client.query(f'''SHOW TAG VALUES FROM "{measurement_name}" WITH KEY = "source"''').get_points()

    for source in sources:
        source_value = source['value']
        fields = influx_client.query(f'''SHOW FIELD KEYS FROM "{measurement_name}"''').get_points()

        source_datapoint_count = 0
        num_fields = 0
        for field in fields:
            field_key = field['fieldKey']
            field_counts = influx_client.query(f'''SELECT count("{field_key}") FROM "{measurement_name}" WHERE ("source" = '{source_value}') AND time >= {start_time_epoch} AND time < {end_time_epoch}''').get_points()
            # should only have at most one count, and thus only iterate once or not at all
            for field_count in field_counts:
                source_datapoint_count += field_count['count']
                if field_count['count'] != 0:
                    num_fields += 1
        print(f'Measurement: {measurement_name}, Source: {source_value}, Non-Empty Fields Count: {num_fields}, Field Entry Count: {source_datapoint_count}')
print()

# Count documents in a Mongo collection
print('Mongo document counts')
mongo_client = MongoClient('localhost', 27017)
start_id = ObjectId.from_datetime(start_time)
end_id = ObjectId.from_datetime(start_time + time_interval)
for collection_name in mongo_client[database].list_collection_names():
    count = mongo_client[database][collection_name].count_documents({"_id": {"$gte": start_id, "$lt": end_id}})
    print(f'Collection: {collection_name}, Count: {count}')