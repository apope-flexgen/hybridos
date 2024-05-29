PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

BINS = gcom_dnp3_server gcom_dnp3_client
BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/

LIBS_DIR = /usr/local/lib/
LIBS_DIR64 = /usr/local/lib64/
SLIBS_DIR = /usr/lib/
SLIBS_DIR64 = /usr/lib64/

INCLUDES_BUILD = -Iinclude/ -ITMW/ -ITMW/tmwscl/tmwtarg/LinIoTarg/ -ITMW/tmwscl/tmwtarg/
INCLUDES_TEST = -Itest/unit_tests/include/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)
LIBS = -LTMW/bin -L$(LIBS_DIR) -L$(LIBS_DIR64) -L$(SLIBS_DIR) -L$(SLIBS_DIR64) -Wl,-rpath=$(LIBS_DIR):$(LIBS_DIR64):$(LIBS_DIR)cpp_17:$(SLIBS_DIR):$(SLIBS_DIR64) -lfims -lmodbus -lssl -lcrypto

# had to use CPPXFLAGS to prevent a conflict with TMW
CPPXFLAGS += -std=c++17
CPPXFLAGS += -Wall -Werror
#CPPXFLAGS += -DSPDLOG_COMPILED_LIB
CPPXFLAGS += -g -DFPS_DEBUG_MODE

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
	CPPXFLAGS += -g -DFPS_DEBUG_MODE
else ifeq ($(BUILD_MODE),test)
	CPPXFLAGS += -g -DFPS_TEST_MODE -DDNP3_TEST_MODE
	BINS = dnp3_test
else
	BUILD_MODE=release
	CPPXFLAGS += -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIST = $(addprefix $(BUILD_DIR), $(BINS))
TMW_SRC_DIR = TMW
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_tmw_utils.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_io_filter.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_flags.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_logger.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_stats.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_heartbeat.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_watchdog.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_fims.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_utils.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_point_utils.o
BUILD_OBJS += $(OBJ_DIR)gcom_dnp3_client_utils.o
BUILD_OBJS += $(OBJ_DIR)version.o

TEST_DIR = test/unit_tests/test_obj/
TEST_OBJ_DIR = build/$(BUILD_MODE)_test/
TEST_LIST = $(addprefix $(TEST_DIR), $(TEST_BINS))
TEST_OBJS += $(TEST_DIR)test_dnp3_client.o
TEST_OBJS += $(TEST_DIR)test_dnp3_fims.o
TEST_OBJS +=  $(TEST_DIR)test_dnp3_stats.o
TEST_OBJS += $(TEST_DIR)define_doctest.o
TEST_OBJS += $(TEST_DIR)test_dnp3_point_utils.o
TEST_OBJS += $(TEST_DIR)test_dnp3_utils.o
TEST_OBJS += $(TEST_DIR)test_dnp3_server.o
TEST_OBJS += $(TEST_DIR)test_dnp3_flags.o
TEST_OBJS += $(TEST_DIR)add_points.o 

all:	build tmw $(LIST)

.PHONY: tmw
tmw:
	cd $(PROJECT_ROOT)TMW && make 
	cd $(PROJECT_ROOT) 

.PHONY: clean_tmw

clean_tmw:
	cd $(PROJECT_ROOT)TMW && make clean 
	cd $(PROJECT_ROOT) 

.PHONY: install_tmw

install_tmw: tmw
	cd $(PROJECT_ROOT)TMW && sh copy_binary.sh
	cd $(PROJECT_ROOT) 

$(BUILD_DIR)dnp3_test: $(TEST_OBJS) $(BUILD_OBJS) $(OBJ_DIR)gcom_dnp3_server.o $(OBJ_DIR)gcom_dnp3_client.o
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lcjson -lspdlog -lIoTarg -ldnp -lutils -o $@

$(BUILD_DIR)gcom_dnp3_server: $(OBJ_DIR)gcom_dnp3_server.o $(BUILD_OBJS)
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lcjson -lspdlog -lIoTarg -ldnp -lutils -o $@

$(BUILD_DIR)gcom_dnp3_client: $(OBJ_DIR)gcom_dnp3_client.o $(BUILD_OBJS)
	$(CXX) $^ $(INCLUDES) $(LIBS) -pthread -lsimdjson -lcjson -lspdlog -lIoTarg -ldnp -lutils -o $@

$(OBJ_DIR)version.o : $(PROJECT_ROOT)src/version.cpp
	$(CXX) -c $(CPPFLAGS) $(GITFLAGS) $(INCLUDES_BUILD) -pthread $< -o $@ 

$(OBJ_DIR)%.o: $(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CPPXFLAGS) $(INCLUDES_BUILD) -pthread $< -o $@ 

$(TEST_DIR)%.o: $(PROJECT_ROOT)test/unit_tests/%.cpp
	$(CXX) -c $(CPPXFLAGS) $(INCLUDES_BUILD) $(INCLUDES_TEST) -pthread $< -o $@ 


.PHONY: build
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(TEST_DIR)

.PHONY: clean

clean: clean_tmw
	rm -fr build
	
.PHONY: clean_gcom
clean_gcom:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)gcom_dnp3_server
	rm -rf $(BIN_DIR)gcom_dnp3_client

.PHONY: install

install:
	rm -rf $(BIN_DIR)gcom_dnp3_server
	cp $(BUILD_DIR)gcom_dnp3_server $(BIN_DIR)
	rm -rf $(BIN_DIR)gcom_dnp3_client
	cp $(BUILD_DIR)gcom_dnp3_client $(BIN_DIR)
	cp gcom_dnp3*service $(BUILD_DIR)
	
