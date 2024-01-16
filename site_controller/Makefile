PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

BINS = site_controller Site_Controller_Utils.so
OBJECTS = main.o Asset.o Asset_ESS.o Asset_Feeder.o Asset_Solar.o Asset_Generator.o Asset_Manager.o Value_Object.o 
OBJECTS += Fims_Object.o Sequence.o Action.o Slew_Object.o Variable_Regulator.o Step.o Path.o Asset_Cmd_Object.o Site_Manager.o
OBJECTS += Feature.o
OBJECTS += Active_Power_Setpoint.o Frequency_Response.o Energy_Arbitrage.o Freq_Resp_Component.o Target_SOC.o ESS_Calibration.o Generator_Charge.o Manual.o
OBJECTS += AVR.o Watt_Var.o Reactive_Setpoint.o Direct_Power_Factor.o Constant_Power_Factor.o
OBJECTS += Active_Power_Closed_Loop_Control.o Active_Power_POI_Limits.o Reactive_Power_POI_Limits.o Watt_Watt.o LDSS.o ESS_Discharge_Prevention.o Aggregated_Asset_Limit.o Reactive_Power_Closed_Loop_Control.o
OBJECTS += Load_Shed.o Solar_Shed.o
OBJECTS += Watchdog.o
OBJECTS += Charge_Dispatch.o Charge_Control.o
OBJECTS += ESS_Manager.o Feeder_Manager.o Generator_Manager.o Solar_Manager.o Site_Controller_Utils.o Data_Endpoint.o Type_Manager.o 
OBJECTS += version.o git_build.o git_commit.o git_tag.o Configurator.o Input_Sources.o Logger.o Reference_Configs.o Config_Validation_Result.o

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/
TSRC_DIR = /usr/local/share/Site_Controller_Utils

LIBS_DIR = /usr/local/lib/
SLIBS_DIR = /usr/lib64/

INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)
LIBS = -L$(LIBS_DIR) -L$(SLIBS_DIR) -Wl,-rpath,$(LIBS_DIR) -lcjson -lfims -lspdlog
THREADS = -pthread

CPPFLAGS += -std=c++11 -Wall -Wpedantic -Werror -Wextra -D SPDLOG_COMPILED_LIB 

ifeq ($(BUILD_MODE),debug)
	# add FPS_DEVELOPER_MODE (prints to console)
	CPPFLAGS += -D FPS_DEVELOPER_MODE
	# add debugging 
	CPPFLAGS += -g -D FPS_DEBUG_MODE
else ifeq ($(BUILD_MODE),test)
	CPPFLAGS += -g -D FPS_TEST_MODE
	LIBS += -lgtest -lgmock
	INCLUDES_BUILD += -Iunit_tests/
else
	BUILD_MODE=release
	CPPFLAGS += -O2
endif

BUILD_DIR = build/$(BUILD_MODE)/
OBJ_DIR = build/$(BUILD_MODE)_obj/
LIBS += -L$(BUILD_DIR)
LIST = $(addprefix $(BUILD_DIR), $(BINS))
OBJ_LIST = $(addprefix $(OBJ_DIR), $(OBJECTS))
vpath %.cpp src
vpath %.cpp src/Features


all:	build $(LIST)

$(BUILD_DIR)Site_Controller_Utils.so:	$(OBJ_DIR)Site_Controller_Utils.o 
	#linking -l spdlog breaks the build for some reason. Manually adding everything for now. 
	$(CXX) -shared -o $@ $^ $(INCLUDES) -L$(LIBS_DIR) -Wl,-rpath,$(LIBS_DIR) -lcjson -lfims -L$(BUILD_DIR) $(THREADS)

$(BUILD_DIR)site_controller:	$(OBJ_LIST)
	$(CXX) -fpic -o $@ $^ $(INCLUDES) $(LIBS) $(THREADS)

$(OBJ_DIR)Site_Controller_Utils.o:	$(PROJECT_ROOT)src/Site_Controller_Utils.cpp
	$(CXX) -c $(CPPFLAGS) -fpic -o $@ $< $(INCLUDES_BUILD)

$(OBJ_DIR)%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $< $(INCLUDES_BUILD)

.PHONY: build
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR)

.PHONY: clean
clean:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)site_controller
	rm $(LIBS_DIR)Site_Controller_Utils.so

.PHONY: install
install:
	mkdir -p $(TSRC_DIR)
	rm -rf $(BIN_DIR)site_controller
	cp $(BUILD_DIR)site_controller $(BIN_DIR)
	cp $(BUILD_DIR)Site_Controller_Utils.so $(LIBS_DIR)


