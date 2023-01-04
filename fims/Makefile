PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

BINS = libfims.so fims_server fims_listen fims_trigger fims_send fims_echo

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/

LIBS_DIR = /usr/local/lib/
SLIBS_DIR = /usr/lib64/

INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_BUILD) -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)

CPPFLAGS += -std=c++11
CPPFLAGS += -Wall -pedantic -Werror

ifeq ($(BUILD_MODE),debug)
	CPPFLAGS += -g -DFPS_DEBUG_MODE
else ifeq ($(BUILD_MODE),test)
	CPPFLAGS += -DFPS_TEST_MODE
	LIBS += -lgtest -lgmock
	INCLUDES_BUILD += -Itests/
else
	BUILD_MODE=release
	CPPFLAGS += -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIBS += -L$(OBJ_DIR)
LIBS += -L$(BUILD_DIR)
LIBS += -L$(LIBS_DIR) -L$(SLIBS_DIR) -Wl,-rpath,$(LIBS_DIR)
LIST = $(addprefix $(BUILD_DIR), $(BINS))

all:	build $(LIST)


$(BUILD_DIR)libfims.so:	$(OBJ_DIR)libfims.o $(OBJ_DIR)libaes.o $(OBJ_DIR)libtime.o
	$(CXX) -shared -o $@ $^ $(INCLUDES) $(LIBS) -lssl -lcrypto


$(BUILD_DIR)fims_server: $(OBJ_DIR)fims_server.o 
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -pthread -lfims

$(BUILD_DIR)fims_send:	$(OBJ_DIR)fims_send.o 
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS)  -lfims

$(BUILD_DIR)fims_listen: $(OBJ_DIR)fims_listener.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -lfims 

$(BUILD_DIR)fims_trigger:	$(OBJ_DIR)fims_trigger.o $(OBJ_DIR)libfims.o
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -lfims -lcjson

$(BUILD_DIR)fims_echo:	$(OBJ_DIR)fims_echo.o 
	$(CXX) -o $@ $^ $(INCLUDES) $(LIBS) -lfims

$(OBJ_DIR)libfims.o:	$(PROJECT_ROOT)src/libfims.cpp
	$(CXX) -c $(CPPFLAGS) -fpic -o $@ $< $(INCLUDES_BUILD)

$(OBJ_DIR)libaes.o: 	$(PROJECT_ROOT)src/libaes.cpp 
	$(CXX) -c $(CPPFLAGS) -fpic -o $@ $< $(INCLUDES_BUILD)

$(OBJ_DIR)libtime.o:	$(PROJECT_ROOT)src/gen_time.cpp
	$(CXX) -c $(CPPFLAGS) -fpic -o $@ $< $(INCLUDES_BUILD)

$(OBJ_DIR)%.o:		$(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $< $(INCLUDES_BUILD)

.PHONY: build
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR)

.PHONY: clean
clean:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)fims*
	rm -rf $(INCLUDES_DIR)fims
	rm $(LIBS_DIR)libfims.so

.PHONY: install
install:
	rm -rf $(BIN_DIR)fims
	cp $(BUILD_DIR)fims_server $(BUILD_DIR)fims_listen $(BUILD_DIR)fims_trigger $(BUILD_DIR)fims_send $(BUILD_DIR)fims_echo $(BIN_DIR)
	mkdir -p $(INCLUDES_DIR)fims
	cp include/fims.h include/libfims.h include/fps_utils.h include/defer.hpp $(INCLUDES_DIR)fims
	cp $(BUILD_DIR)libfims.so $(LIBS_DIR)
