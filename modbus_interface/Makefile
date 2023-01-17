PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

BINS = modbus_client modbus_server

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/

LIBS_DIR = /usr/local/lib/
LIBS_DIR64 = /usr/local/lib64/
SLIBS_DIR = /usr/lib/
SLIBS_DIR64 = /usr/lib64/

INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)
LIBS = -L$(LIBS_DIR) -L$(LIBS_DIR64) -L$(SLIBS_DIR) -L$(SLIBS_DIR64) -Wl,-rpath=$(LIBS_DIR):$(LIBS_DIR64):$(LIBS_DIR)cpp_17:$(SLIBS_DIR):$(SLIBS_DIR64) -lfims -lmodbus

CPPFLAGS += -std=c++17
CPPFLAGS += -Wall -Wpedantic -Werror
CPPFLAGS += -DSPDLOG_COMPILED_LIB

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
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIST = $(addprefix $(BUILD_DIR), $(BINS))

all:	build $(LIST)

$(BUILD_DIR)modbus_server: $(OBJ_DIR)modbus_server.o $(OBJ_DIR)modbus_utils.o
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)modbus_client: $(OBJ_DIR)modbus_client.o
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lspdlog -o $@

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
	rm -rf $(BIN_DIR)modbus_interface

.PHONY: install
install:
	rm -rf $(BIN_DIR)modbus_interface
	cp $(BUILD_DIR)modbus_server $(BUILD_DIR)modbus_client $(BIN_DIR)
	cp modbus*service $(BUILD_DIR)
	
