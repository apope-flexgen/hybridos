PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

# we'll use this to determine if we build or not.
# to build
# export PATH=/usr/local/gcc-9.2.0/bin:$PATH
# make
# to run  
# export LD_LIBRARY_PATH=/usr/local/gcc-9.2.0/lib64:$LD_LIBRARY_PATH
# /usr/local/bin/dnp3_interface/dnp3_outstation config/<some_config>
# dirname `which g++`

BINS = dnp3_server dnp3_client
TBINS = test_single test_config test_config_stackload test_config_fims

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/

LIBS_DIR = /usr/local/lib/
#SLIBS_DIR = /usr/local/gcc-9.2.0/lib64/
SLIBS_DIR = /usr/local/gcc-9.1.0/lib64/
WGPP = $(shell expr `which g++`)

XLIBS_DIR = $(shell expr `dirname $(WGPP)`)/../lib64


INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)


CPPFLAGS += -std=c++17
CPPFLAGS += -Wall -pedantic -Werror
CPPFLAGS += -DSPDLOG_COMPILED_LIB

# BUILD_MODE = debug
ifeq ($(BUILD_MODE),debug)
	CPPFLAGS += -g -DFPS_DEBUG_MODE
else ifeq ($(BUILD_MODE),test)
	CPPFLAGS += -DFPS_TEST_MODE
	LIBS += -lgtest -lgmock
else
	BUILD_MODE=release
	CPPFLAGS += -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
TBUILD_DIR = build/$(BUILD_MODE)_test/

OBJ_DIR = build/$(BUILD_MODE)_obj/
TOBJ_DIR = build/$(BUILD_MODE)_obj_test/

LIST = $(addprefix $(BUILD_DIR), $(BINS))
TLIST = $(addprefix $(TBUILD_DIR), $(TBINS))

LIBS = -L/usr/lib -L$(OBJ_DIR) -L$(XLIBS_DIR) -L$(LIBS_DIR) -L/usr/lib64/openssl11 -Wl,--build-id -Wl,-rpath,$(XLIBS_DIR):$(LIBS_DIR):/usr/lib64/ 
LIBS += -lstdc++ -lcjson -lfims -lopendnp3  -lpthread
LIBS += -lssl -lssl3
LIBS += -lcrypto

BLIST = build
BLIST += $(OBJ_DIR)libdmap.so 
BLIST += $(LIST)
BLIST += $(TLIST)

GCC_GT_4805 := 1
#$(shell expr `gcc -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \> 40805)

ifeq "$(GCC_GT_4805)" "0"
     BLIST = nobuild
else 
     BLIST = okbuild
     BLIST += build 
     BLIST += $(OBJ_DIR)libdmap.so 
     BLIST += $(LIST)

endif

#all:	
all:	$(BLIST) $(TLIST)
	echo "blist = $(BLIST) ="
	echo "tlist = $(TLIST) ="

$(OBJ_DIR)libdmap.so:	$(OBJ_DIR)libdmap.o 
	$(CXX) -shared -o $@ $^ $(INCLUDES) $(LIBS)

$(OBJ_DIR)libdmap.o:	$(PROJECT_ROOT)src/dnp3_utils.cpp $(PROJECT_ROOT)include/dnp3_utils.h
	$(CXX) -c $(CPPFLAGS) -fpic -o $@ $< $(INCLUDES_BUILD)

$(BUILD_DIR)dnp3_server: $(OBJ_DIR)dnp3_server.o $(OBJ_DIR)fpsCommandHandler.o $(OBJ_DIR)fpsOutstationApplication.o $(OBJ_DIR)fpsLogger.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap  -lsimdjson -lspdlog -pthread
 
$(BUILD_DIR)dnp3_client: $(OBJ_DIR)dnp3_client.o $(OBJ_DIR)fpsSOEHandler.o $(OBJ_DIR)fpsMasterApplication.o $(OBJ_DIR)fpsCommandCallback.o $(OBJ_DIR)fpsLogger.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap -lsimdjson -lspdlog -pthread

$(TBUILD_DIR)test_single: $(TOBJ_DIR)test_single.o  $(OBJ_DIR)libdmap.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap -lsimdjson -lspdlog -pthread

$(TBUILD_DIR)test_config: $(TOBJ_DIR)test_config.o  $(OBJ_DIR)libdmap.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap -lsimdjson -lspdlog -pthread

$(TBUILD_DIR)test_config_stackload: $(TOBJ_DIR)test_config_stackload.o  $(OBJ_DIR)libdmap.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap -lsimdjson -lspdlog -pthread

$(TBUILD_DIR)test_config_fims: $(TOBJ_DIR)test_config_fims.o  $(OBJ_DIR)libdmap.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -ldmap -lsimdjson -lspdlog -pthread

$(OBJ_DIR)%.o: $(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $< $(INCLUDES_BUILD)

$(TOBJ_DIR)%.o: $(PROJECT_ROOT)test/%.cpp 
	$(CXX) -c $(CPPFLAGS) -o $@ $< $(INCLUDES_BUILD)

.PHONY: build nobuild okbuild
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(TBUILD_DIR) $(TOBJ_DIR)
nobuild:
	mkdir -p NO_$(BUILD_DIR) $(OBJ_DIR)
okbuild:
	mkdir -p OK_$(BUILD_DIR) $(OBJ_DIR)

.PHONY: clean
clean:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)dnp3_server
	rm -rf $(BIN_DIR)dnp3_client

.PHONY: install
install:
	mkdir -p $(BIN_DIR)
	cp $(BUILD_DIR)dnp3_server $(BIN_DIR)
	cp $(BUILD_DIR)dnp3_client $(BIN_DIR)
	cp $(OBJ_DIR)*.so $(LIBS_DIR)
