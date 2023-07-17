mkdir -p libs
BUILD_DIR=./build/test
LIBS_DIR=./libs
g++ -std=c++11 -o ${BUILD_DIR}/main_mod -I ./include src/main_mod.cpp src/asset.cpp -ldl -lcjson -lfims
g++ -std=c++11 -o ${LIBS_DIR}/libfeed.so -fPIC -shared -I ./include src/feed.cpp -lcjson -lfims
g++ -std=c++11 -o ${LIBS_DIR}/libess.so -fPIC -shared -I ./include src/ess.cpp -lcjson -lfims
g++ -std=c++11 -o ${LIBS_DIR}/libpcs.so -fPIC -shared -I ./include src/pcs.cpp -lcjson -lfims
g++ -std=c++11 -o ${LIBS_DIR}/libbms.so -fPIC -shared -I ./include src/bms.cpp -lcjson -lfims
g++ -std=c++11 -o ${BUILD_DIR}/md -I ./include src/modbus_map.cpp src/modbus_dump.cpp -lcjson -lfims
g++ -std=c++11 -o ${BUILD_DIR}/test_chan -I ./include src/test_chan.cpp -lpthread -lcjson -lfims
g++ -std=c++11 -DASSET_TEST -o ${BUILD_DIR}/ass -I ./include src/asset.cpp -lcjson -lfims
g++ -std=c++11 -DFUNC_TEST -o ${BUILD_DIR}/func -I ./include src/functions.cpp -lcjson -lfims
g++ -std=c++11 -g -o ${BUILD_DIR}/test_fims -I ./include src/test_fims.cpp -lpthread -lcjson -lfims
g++ -std=c++11 -g -o ${BUILD_DIR}/test_bms -I ./include src/test_bms.cpp -lpthread -lcjson -lfims
g++ -std=c++11 -g -o ${BUILD_DIR}/test_vmap0 -I ./include src/test_vmap0.cpp -lpthread -lcjson -lfims
g++ -std=c++11 -g -o ${BUILD_DIR}/test_vmap_action -I ./include src/test_vmap_action.cpp -lpthread -lcjson -lfims
