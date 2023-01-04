import json
import requests

# get JSON string data from CityBike NYC using web requests library
json_response= requests.get("https://feeds.citibikenyc.com/stations/stations.json")
# check type of json_response object
print(type(json_response.text))
# load data in loads() function of json library
bike_dict = json.loads(json_response.text)
#check type of news_dict
print(type(bike_dict))
# now get stationBeanList key data from dict
print(bike_dict['stationBeanList'][0]) 

