PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

BINS = gcom_modbus_client gcom_modbus_server

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/
SYSD_DIR = /usr/lib/systemd/system

LIBS_DIR = /usr/local/lib/
LIBS_DIR64 = /usr/local/lib64/
SLIBS_DIR = /usr/lib/
SLIBS_DIR64 = /usr/lib64/

INCLUDES_BUILD = -Iinclude/
INCLUDES_TEST = -Itest/unit_tests/include/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)
LIBS = -L$(LIBS_DIR) -L$(LIBS_DIR64) -L$(SLIBS_DIR) -L$(SLIBS_DIR64) -Wl,-rpath=$(LIBS_DIR):$(LIBS_DIR64):$(LIBS_DIR)cpp_17:$(SLIBS_DIR):$(SLIBS_DIR64) -lfims -lmodbus

CPPFLAGS += -std=c++17
CPPFLAGS += -Wall -Wpedantic -Werror 

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
	CPPFLAGS += -DFPS_TEST_MODE -fprofile-arcs -ftest-coverage
	LIBS += -lgtest -lgmock
	BINS = modbus_test
else
	BUILD_MODE=release
	CPPFLAGS +=  -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIST = $(addprefix $(BUILD_DIR), $(BINS))

BUILD_OBJS = $(OBJ_DIR)gcom_config_any.o 
BUILD_OBJS += $(OBJ_DIR)gcom_perf.o 
BUILD_OBJS += $(OBJ_DIR)gcom_timer.o 
BUILD_OBJS += $(OBJ_DIR)gcom_iothreads.o 
BUILD_OBJS += $(OBJ_DIR)load_to_dbi_client.o
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

TEST_OBJS = $(OBJ_DIR)modbus_test.o

# DECODE_TEST_OBJS = $(OBJ_DIR)test_delay_connect.o

IO_THREAD_TEST_OBJS = $(OBJ_DIR)test_check_pubgroup_key.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_check_socket_alive.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_close_modbus_for_thread.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_delay_connect.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_discard_group_callback.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_fast_reconnect_for_thread.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_format_thread_connection_info.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_get_num_threads.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_get_pubgroup.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_handle_connect.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_handle_disconnect.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_handle_point_error.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_handle_thread_error.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_has_lost_connection.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_io_thread_func.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_local_read_bits.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_local_read_registers.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_local_write_bits.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_local_write_registers.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_make_io_thread.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_make_work.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_ok_to_connect.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_poll_work.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_process_group_callback.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_process_io_work_response_thread.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_process_io_work_response.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_read_modbus_io_points.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_read_modbus_io_work.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_run_thread_work.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_set_work.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_setup_modbus_for_thread.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_start_threads.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_stash_work.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_wait_socket_ready.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_write_modbus_io_points.o
IO_THREAD_TEST_OBJS += $(OBJ_DIR)test_write_modbus_io_work.o

TEST_OBJS += $(OBJ_DIR)mock_functions.o
TEST_OBJS += $(BUILD_OBJS)
TEST_OBJS += $(IO_THREAD_TEST_OBJS)
TEST_OBJS += $(DECODE_TEST_OBJS)

all:	build  $(LIST)

$(BUILD_DIR)modbus_server: $(OBJ_DIR)modbus_server.o $(OBJ_DIR)modbus_utils.o 
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)gcom_modbus_server: $(OBJ_DIR)gcom_modbus_server.o $(OBJ_DIR)gcom_modbus_utils.o $(OBJ_DIR)load_to_dbi_server.o 
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)gcom_modbus_server_id: $(OBJ_DIR)gcom_modbus_server_id.o $(OBJ_DIR)gcom_modbus_utils.o 
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lcjson -lspdlog -o $@

$(BUILD_DIR)modbus_client: $(OBJ_DIR)modbus_client.o
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lspdlog -o $@

$(BUILD_DIR)gcom_modbus_client: $(OBJ_DIR)gcom_modbus_client.o $(BUILD_OBJS)
	$(CXX) $^ $(INCLUDES)  $(LIBS) -pthread -lsimdjson -lspdlog -o $@ 

$(BUILD_DIR)modbus_test: $(TEST_OBJS)
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lspdlog -lgcov -o $@
	rm -f $(OBJ_DIR)*.gcov

$(OBJ_DIR)version.o : $(PROJECT_ROOT)src/version.cpp
	$(CXX) -c $(CPPFLAGS) $(GITFLAGS) $(INCLUDES_BUILD) -pthread $< -o $@ 

$(OBJ_DIR)%.o: $(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CPPFLAGS) $(INCLUDES_BUILD) $(if $(filter test,$(BUILD_MODE)),$(INCLUDES_TEST)) -pthread $< -o $@ 

$(OBJ_DIR)%.o: $(PROJECT_ROOT)test/unit_tests/%.cpp
	$(CXX) -c $(CPPFLAGS) $(INCLUDES_TEST) $(INCLUDES) $(INCLUDES_BUILD) -pthread $< -o $@ 

$(OBJ_DIR)%.o: $(PROJECT_ROOT)test/unit_tests/io_threads/%.cpp
	$(CXX) -c $(CPPFLAGS) $(INCLUDES_TEST) $(INCLUDES) $(INCLUDES_BUILD) -pthread $< -o $@ 

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
