 mkdir -p build/release
 g++ -o build/release/md -std=c++11 test/modbus_map.cpp test/modbus_dump.cpp  -lcjson

