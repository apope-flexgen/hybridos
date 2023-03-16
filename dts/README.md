# dts
DTS (Disk To Storage) is a iterative service that tracks a folder for new and existing archives. It iteratively decodes every archive using fims_codec and extracts time series FIMS messages and stores them to database

Currently supports Influx DB and Mongodb.
Stores time series FIMS messages to Influx DB
Stores Events related FIMS messages to Mongo DB