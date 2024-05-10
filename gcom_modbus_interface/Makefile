PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

BINS = gcom_modbus_client gcom_modbus_server
#BINS += gcom_modbus_test
#BINS += gcom_modbus_server_id

# BINS = gcom_modbus_test modbus_server modbus_client gcom_modbus_client
# BINS = modbus_client modbus_server gcom_modbus_test
# gcom_modbus_client

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/
SYSD_DIR = /usr/lib/systemd/system

LIBS_DIR = /usr/local/lib/
LIBS_DIR64 = /usr/local/lib64/
SLIBS_DIR = /usr/lib/
SLIBS_DIR64 = /usr/lib64/

INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)
LIBS = -L$(LIBS_DIR) -L$(LIBS_DIR64) -L$(SLIBS_DIR) -L$(SLIBS_DIR64) -Wl,-rpath=$(LIBS_DIR):$(LIBS_DIR64):$(LIBS_DIR)cpp_17:$(SLIBS_DIR):$(SLIBS_DIR64) -lfims -lmodbus

CPPFLAGS += -std=c++17
CPPFLAGS += -Wall -Wpedantic -Werror 
#CPPFLAGS += -Wextra
#CPPFLAGS += -DSPDLOG_COMPILED_LIB
# CPPFLAGS += -DSERVER_DELAY

GITBRANCH  = $(shell git branch | grep \* | cut -d ' ' -f2)
GITCOMMIT  = $(shell git log --pretty=format:'%h' -n 1)
GITVERSION = $(shell git rev-list --count $(GITCOMMIT))
GITTAG     = $(shell git describe --match v* --abbrev=0 --tags HEAD --always)
$(info GITBRANCH = "$(GITBRANCH)")
$(info GITCOMMIT = "$(GITCOMMIT)")
$(info GITVERSION= "$(GITVERSION)")
$(info GITTAG    = "$(GITTAG)")

GITFLAGS += -DGITBRANCH="\"$(GITBRANCH)\""
GITFLAGS += -DGITCOMMIT="\"$(GITCOMMIT)\""
GITFLAGS += -DGITVERSION="\"$(GITVERSION)\""
GITFLAGS += -DGITTAG="\"$(GITTAG)\""

ifeq ($(BUILD_MODE),debug)
	CPPFLAGS += -g -DFPS_DEBUG_MODE
else ifeq ($(BUILD_MODE),test)
	CPPFLAGS += -DFPS_TEST_MODE
	LIBS += -lgtest -lgmock
else
	BUILD_MODE=release
	CPPFLAGS +=  -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIST = $(addprefix $(BUILD_DIR), $(BINS))

TEST_OBJS = $(OBJ_DIR)gcom_modbus_test.o 
TEST_OBJS += $(OBJ_DIR)gcom_config_any.o 
TEST_OBJS += $(OBJ_DIR)gcom_config_test.o 
TEST_OBJS += $(OBJ_DIR)gcom_perf.o 
TEST_OBJS += $(OBJ_DIR)gcom_timer.o 
TEST_OBJS += $(OBJ_DIR)gcom_iothreads.o 
TEST_OBJS += $(OBJ_DIR)gcom_logger.o
TEST_OBJS += $(OBJ_DIR)gcom_modbus_pub.o
TEST_OBJS += $(OBJ_DIR)gcom_modbus_decode.o
TEST_OBJS += $(OBJ_DIR)gcom_fims.o
TEST_OBJS += $(OBJ_DIR)gcom_stats.o
TEST_OBJS += $(OBJ_DIR)gcom_config.o
TEST_OBJS += $(OBJ_DIR)gcom_heartbeat.o
TEST_OBJS += $(OBJ_DIR)gcom_watchdog.o
TEST_OBJS += $(OBJ_DIR)gcom_utils.o
TEST_OBJS += $(OBJ_DIR)version.o

 
#BUILD_OBJS += $(OBJ_DIR)gcom_modbus_client.o
BUILD_OBJS += $(OBJ_DIR)gcom_config_any.o 
BUILD_OBJS += $(OBJ_DIR)gcom_perf.o 
BUILD_OBJS += $(OBJ_DIR)gcom_timer.o 
BUILD_OBJS += $(OBJ_DIR)gcom_iothreads.o 
BUILD_OBJS += $(OBJ_DIR)gcom_logger.o
BUILD_OBJS += $(OBJ_DIR)gcom_modbus_pub.o
BUILD_OBJS += $(OBJ_DIR)gcom_modbus_decode.o
BUILD_OBJS += $(OBJ_DIR)gcom_fims.o
BUILD_OBJS += $(OBJ_DIR)gcom_stats.o
BUILD_OBJS += $(OBJ_DIR)gcom_config.o
BUILD_OBJS += $(OBJ_DIR)gcom_heartbeat.o
BUILD_OBJS += $(OBJ_DIR)gcom_watchdog.o
BUILD_OBJS += $(OBJ_DIR)gcom_utils.o
BUILD_OBJS += $(OBJ_DIR)version.o



all:	build  $(LIST)

$(BUILD_DIR)modbus_server: $(OBJ_DIR)modbus_server.o $(OBJ_DIR)modbus_utils.o 
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)gcom_modbus_server: $(OBJ_DIR)gcom_modbus_server.o $(OBJ_DIR)gcom_modbus_utils.o 
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)gcom_modbus_server_id: $(OBJ_DIR)gcom_modbus_server_id.o $(OBJ_DIR)gcom_modbus_utils.o 
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)modbus_client: $(OBJ_DIR)modbus_client.o
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lspdlog -o $@

$(BUILD_DIR)gcom_modbus_client: $(OBJ_DIR)gcom_modbus_client.o $(BUILD_OBJS)
	$(CXX) $^ $(INCLUDES)  $(LIBS) -pthread -lsimdjson -lspdlog -o $@ 

$(BUILD_DIR)gcom_modbus_test: $(TEST_OBJS) 
	$(CXX) $^ $(INCLUDES)  $(LIBS) -pthread -lsimdjson -lspdlog -o $@ 

$(OBJ_DIR)version.o : $(PROJECT_ROOT)src/version.cpp
	$(CXX) -c $(CPPFLAGS) $(GITFLAGS) $(INCLUDES_BUILD) -pthread $< -o $@ 


$(OBJ_DIR)%.o: $(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CPPFLAGS) $(INCLUDES_BUILD) -pthread $< -o $@ 



# cp $(BUILD_DIR)modbus_server $(BUILD_DIR)modbus_client $(BIN_DIR)

.PHONY: build
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR)

.PHONY: clean
clean:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)gcom_modbus_interface

.PHONY: install
install:
	rm -rf $(BIN_DIR)gcom_modbus_interface
	cp $(BUILD_DIR)gcom_modbus_server $(BUILD_DIR)gcom_modbus_client $(BIN_DIR)
	cp gcom_modbus_client@.service $(BUILD_DIR)
	cp gcom_modbus_server@.service $(BUILD_DIR)
	
