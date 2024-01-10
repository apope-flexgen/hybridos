#!/bin/sh
mkdir -p build/release
# g++ --std=c++17 -g -o build/release/t -I include test/test_data_map_json.cpp  \
#     build/release_obj/simdjson.o -L/usr/local/lib/ -L/usr/lib64/ \
#     -Wl,-rpath,/usr/local/lib/ -lpthread -lfims

# g++ -g --std=c++17 -o build/release/te -I include test/test_event_threads.cpp  -lpthread

./package_utility/version.sh
g++ -g --std=c++17 -o build/release/tv -I include test/test_version.cpp git*.o  -lpthread
