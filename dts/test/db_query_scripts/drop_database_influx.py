# Copied from stackoverflow https://stackoverflow.com/questions/57450101/how-to-drop-all-influxdb-databases

from influxdb import InfluxDBClient
import os

influx_host = os.getenv('INFLUX_HOST', 'localhost')
db_client = InfluxDBClient(host=influx_host)

db_list = db_client.get_list_database()

for db in db_list:
    db_client.drop_database(db['name'])
