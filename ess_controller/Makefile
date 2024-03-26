PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

$(info PROJECT_ROOT="$(PROJECT_ROOT)")
$(info MAKEFILE_LIST="$(MAKEFILE_LIST)")

#BINS = libess.so libessfunc.so ess_controller  gpio_controller 
BINS = libess.so libessfunc.so ess_controller gpio_controller
TEST_BINS = test_vmap0 test_vmap1 test_vmap_act_remap test_all_func test_valueChanged
# TEST_BINS += test_calculator_func test_deadband gtest_check_table_var 
#TEST_BINS += gtest_monitor_condition gtest_monitor_var_v2 gtest_calculate_var gtest_dbi gtest_handle_cmd gtest_update_to_dbi
#TEST_BINS += test_logger_chrono
TEST_BINS += test_geturi
TEST_BINS += FlexPack
TEST_BINS += test_vecval
# TEST_BINS += gtest_vecval
# TEST_BINS += gtest_deadband
# TEST_BINS += gtest_vmap0
# TEST_BINS += gtest_vmap1

BIN_DIR = /usr/local/bin/
INCLUDES_DIR = /usr/local/include/
SINCLUDES_DIR= /usr/include/

LIBS_DIR = /usr/local/lib/
SLIBS_DIR = /usr/lib64/

INCLUDES_BUILD = -Iinclude/
INCLUDES = -I$(INCLUDES_BUILD) -I$(INCLUDES_DIR) -I$(SINCLUDES_DIR)
THREADS = -pthread

#CPPFLAGS += -g -std=c++11 -Wall -pedantic -Werror  -Wno-format-security -Wno-format-truncation
#CPPFLAGS += -g 
CPPFLAGS += -std=c++11

CPPFLAGS += -Wall -Wno-format-security -Wno-format-truncation -Werror
GITBRANCH  = $(shell git branch | grep \* | cut -d ' ' -f2)
GITCOMMIT  = $(shell git log --pretty=format:'%h' -n 1)
GITVERSION = $(shell git rev-list --count $(GITCOMMIT))
GITTAG     = $(shell git describe --match v* --abbrev=0 --tags HEAD --always)

$(info GITBRANCH = "$(GITBRANCH)")
$(info GITCOMMIT = "$(GITCOMMIT)")
$(info GITVERSION= "$(GITVERSION)")
$(info GITTAG    = "$(GITTAG)")
CPPFLAGS += -DGITBRANCH="\"$(GITBRANCH)\""
CPPFLAGS += -DGITCOMMIT="\"$(GITCOMMIT)\""
CPPFLAGS += -DGITVERSION="\"$(GITVERSION)\""
CPPFLAGS += -DGITTAG="\"$(GITTAG)\""
CPPFLAGS += -DSPDLOG_COMPILED_LIB

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
FUN_DIR = build/$(BUILD_MODE)_funcs/
TEST_DIR = build/$(BUILD_MODE)_test/
TSRC_DIR = $(PROJECT_ROOT)test

FUNC_DIR = $(PROJECT_ROOT)funcs
SRC_DIR = $(PROJECT_ROOT)src
INC_DIR = $(PROJECT_ROOT)include
SL_FUNC_DIR = $(PROJECT_ROOT)SL_files

LIBS = -L$(OBJ_DIR)
#LIBS += -L$(PROJECT_ROOT)lib64
LIBS += -L$(BUILD_DIR)
LIBS += -L$(LIBS_DIR) -L$(SLIBS_DIR) -Wl,-rpath,$(LIBS_DIR) -lcjson -lspdlog
#LIBS +=
LIBS += -lfims -pthread
LIST = $(addprefix $(BUILD_DIR), $(BINS))
TEST_LIST = $(addprefix $(BUILD_DIR), $(TEST_BINS))

LIB_LIST = $(OBJ_DIR)libess.so, $(OBJ_DIR)libessfunc.so

FUNC_OBJECTS = chrono_utils.o
FUNC_OBJECTS += LogFuncs.o
FUNC_OBJECTS += AlarmFaultHandler.o  
FUNC_OBJECTS += CheckAmComms.o
FUNC_OBJECTS += CheckAmTimestamp.o
FUNC_OBJECTS += CheckMonitorVar.o 
FUNC_OBJECTS += CheckMonitorVar_v2.o   
FUNC_OBJECTS += CheckDbiVar.o    
FUNC_OBJECTS += CheckTableVar.o         
FUNC_OBJECTS += CalculateVar.o
FUNC_OBJECTS += RunKeyCmd.o
#FUNC_OBJECTS += module_faultFuncs.o      
#FUNC_OBJECTS += module_runFuncs.o      
#FUNC_OBJECTS += FaultFuncs.o   
FUNC_OBJECTS += UpdateSysTime.o
FUNC_OBJECTS += AlarmsLogsFaults.o   
FUNC_OBJECTS += CheckAmHeartbeat.o  
FUNC_OBJECTS += CheckEssStatus.o    
FUNC_OBJECTS += HandleAssetHeartbeat.o  
FUNC_OBJECTS += SimHandleHeartbeat.o
FUNC_OBJECTS += SimSbmu.o
FUNC_OBJECTS += SimBms.o
FUNC_OBJECTS += SimPcs.o
#FUNC_OBJECTS += HandlePowerLimit.o
FUNC_OBJECTS += HandlePower.o
FUNC_OBJECTS += CheckEssStatus.o
FUNC_OBJECTS += SchedFunctions.o
FUNC_OBJECTS += GpioControllerFuncs.o
FUNC_OBJECTS += SendClearFaultCmd.o
FUNC_OBJECTS += FaultFuncs.o
FUNC_OBJECTS += RunFuncs.o      
FUNC_OBJECTS += GpioRework.o
FUNC_OBJECTS += InitAMFuncs.o
FUNC_OBJECTS += MathMovAvg.o
FUNC_OBJECTS += DoInflux.o
FUNC_OBJECTS += RunSystemCmd.o
FUNC_OBJECTS += FlexPack.o
FUNC_OBJECTS += HandleCmd.o
FUNC_OBJECTS += DbiFuncs.o
FUNC_OBJECTS += SlewFuncs.o
FUNC_OBJECTS += PubFuncs.o
FUNC_OBJECTS += UpdateToDbi.o
FUNC_OBJECTS += SaveToDbi.o
FUNC_OBJECTS += CheckValueChanged.o
FUNC_OBJECTS += BalancePower.o
FUNC_OBJECTS += OutputHandler.o
FUNC_OBJECTS += InputHandler.o
FUNC_OBJECTS += ScheduledEnableFunctions.o

#datamap simulink source code files. uncomment for testing
# FUNC_OBJECTS += Reference.o
# FUNC_OBJECTS += Reference_Interface.o
# FUNC_OBJECTS += func1.o
# FUNC_OBJECTS += func2.o
# FUNC_OBJECTS += func3.o
# FUNC_OBJECTS += DC_Augmentation.o
# FUNC_OBJECTS += DC_Augmentation_Interface.o
# FUNC_OBJECTS += HighLevelController.o
# FUNC_OBJECTS += HighLevelController_Interface.o
# FUNC_OBJECTS += LowLevelController.o
# FUNC_OBJECTS += LowLevelController_Interface.o
#$(FUNC_LIST)


#OBJ_LIST = $(addprefix $(OBJ_DIR), $(OBJECTS))
#LIB_OBJ_LIST = $(addprefix $(OBJ_DIR), $(LIB_OBJECTS))FUNC_OBJECTS = AlarmFaultHandler.o  
ESS_OBJECTS += assetFunc.o
ESS_OBJECTS += asset.o
ESS_OBJECTS += assetVar.o
ESS_OBJECTS += varMapUtils.o
ESS_OBJECTS += scheduler.o
ESS_OBJECTS += i2cbusses.o
ESS_OBJECTS += vmActions.o
ESS_OBJECTS += vmConfig.o
ESS_OBJECTS += dbi_funcs.o
ESS_OBJECTS += dataMapCore.o
ESS_OBJECTS += dataMapThread.o
ESS_OBJECTS += dataMapUtils.o
ESS_OBJECTS += AssetSetupUtility.o
ESS_OBJECTS += FunctionUtility.o
ESS_OBJECTS += InfoMessageUtility.o
ESS_OBJECTS += DataUtility.o
ESS_OBJECTS += SiteCommandUtility.o




#ESS_OBJECTS += version.o git_build.o git_commit.o git_tag.o


FUNC_OBJ_LIST = $(addprefix $(FUN_DIR), $(FUNC_OBJECTS))
ESS_OBJ_LIST = $(addprefix $(OBJ_DIR), $(ESS_OBJECTS))


#all:	build $(OBJ_DIR)libess.so $(OBJ_DIR)libessfunc.so $(BUILD_DIR)ess_controller $(BUILD_DIR)fims_gpio
#$(LIB_LIST) $(LIST)

all: gitinc build $(LIST) $(TEST_LIST)

gitinc:
	touch include/gitinc.h
	rm -f $(OBJ_DIR)ess_controller.o $(OBJ_DIR)gpio_controller.o

$(BUILD_DIR)libess.so: $(ESS_OBJ_LIST)
	$(CXX) -L$(PROJECT_ROOT)lib64 -fpic -shared -o $@ $^ $(INCLUDES_BUILD) 


$(BUILD_DIR)libessfunc.so: $(FUNC_OBJ_LIST)
	$(CXX) -L$(PROJECT_ROOT)lib64 -fpic -shared -o $@ $^ $(INCLUDES_BUILD)

$(BUILD_DIR)ess_controller:	$(OBJ_DIR)ess_controller.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)new_ess_sched:	$(OBJ_DIR)new_ess_sched.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)new_gpio_sched:	$(OBJ_DIR)new_gpio_sched.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)ess_controller_test:	$(OBJ_DIR)ess_controller_test.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)ess_gpio:	$(OBJ_DIR)ess_gpio.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(OBJ_DIR)i2cbusses.o:  $(SRC_DIR)/i2cbusses.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)vmActions.o:  $(SRC_DIR)/vmActions.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)vmConfig.o:  $(SRC_DIR)/vmConfig.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)dbi_funcs.o:  $(SRC_DIR)/dbi_funcs.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)dataMapCore.o:  $(SRC_DIR)/dataMapCore.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)dataMapThread.o:  $(SRC_DIR)/dataMapThread.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)dataMapUtils.o:  $(SRC_DIR)/dataMapUtils.cpp
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(BUILD_DIR)gpio_controller:	$(OBJ_DIR)gpio_controller.o $(OBJ_DIR)i2cbusses.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_vmap0:	$(TEST_DIR)test_vmap0.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)
	
$(BUILD_DIR)gtest_vmap0:	$(TEST_DIR)gtest_vmap0.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)test_vmap1:	$(TEST_DIR)test_vmap1.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)gtest_vmap1:	$(TEST_DIR)gtest_vmap1.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)test_vmap_act_remap:	$(TEST_DIR)test_vmap_act_remap.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)gtest_check_table_var:	$(TEST_DIR)gtest_check_table_var.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_monitor_condition:	$(TEST_DIR)gtest_monitor_condition.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_handle_cmd:	$(TEST_DIR)gtest_handle_cmd.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_monitor_var_v2:	$(TEST_DIR)gtest_monitor_var_v2.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)test_vecval:	$(TEST_DIR)test_vecval.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_calculate_var:	$(TEST_DIR)gtest_calculate_var.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_update_to_dbi:	$(TEST_DIR)gtest_update_to_dbi.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_vecval:	$(TEST_DIR)gtest_vecval.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)gtest_dbi:	$(TEST_DIR)gtest_dbi.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)test_valueChanged:	$(TEST_DIR)test_valueChanged.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_calculator_func:	$(TEST_DIR)test_calculator_func.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_deadband:	$(TEST_DIR)test_deadband.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)gtest_deadband:	$(TEST_DIR)gtest_deadband.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS) -lgtest

$(BUILD_DIR)test_all_func:	$(TEST_DIR)test_all_func.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_chan_timeq:	$(TEST_DIR)test_chan_timeq.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_sched:	$(TEST_DIR)test_sched.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_ess:	$(TEST_DIR)test_ess.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_logger_chrono:	$(TEST_DIR)test_logger_chrono.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)test_geturi:	$(TEST_DIR)test_geturi.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(BUILD_DIR)FlexPack:	$(TEST_DIR)FlexPack.o
	$(CXX) -o $@ $^ $(INCLUDES) -lessfunc -less $(LIBS)

$(OBJ_DIR)assetFunc.o:	$(SRC_DIR)/assetFunc.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)scheduler.o:	$(SRC_DIR)/scheduler.cpp 
	$(CXX) -c $(CPPFLAGS) -D_JUST_LIB -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)AssetSetupUtility.o:	$(SRC_DIR)/AssetSetupUtility.cpp 
	$(CXX) -c $(CPPFLAGS) -D_JUST_LIB -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)FunctionUtility.o:	$(SRC_DIR)/FunctionUtility.cpp 
	$(CXX) -c $(CPPFLAGS) -D_JUST_LIB -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)InfoMessageUtility.o:	$(SRC_DIR)/InfoMessageUtility.cpp 
	$(CXX) -c $(CPPFLAGS) -D_JUST_LIB -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)DataUtility.o: $(SRC_DIR)/DataUtility.cpp 
	$(CXX) -c $(CPPFLAGS) -D_JUST_LIB -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)SiteCommandUtility.o:	$(SRC_DIR)/SiteCommandUtility.cpp 
	$(CXX) -c $(CPPFLAGS) -D_JUST_LIB -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)ess_controller.o:	$(SRC_DIR)/ess_controller.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)new_ess_sched.o:	$(SRC_DIR)/new_ess_sched.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)new_gpio_sched.o:	$(SRC_DIR)/new_gpio_sched.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)ess_controller_test.o:	$(SRC_DIR)/ess_controller_test.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)gpio_controller.o:	$(SRC_DIR)/gpio_controller.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(FUN_DIR)AlarmFaultHandler.o:	$(FUNC_DIR)/AlarmFaultHandler.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)%.o:	$(SRC_DIR)/%.cpp $(INC_DIR)/%.h
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) -I$(INC_DIR)

$(FUN_DIR)%.o:	$(FUNC_DIR)/%.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(INC_DIR)

$(TEST_DIR)test_sched.o:	$(SRC_DIR)/scheduler.cpp 
	$(CXX) -c $(CPPFLAGS) -D_MAIN -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(INC_DIR)

$(TEST_DIR)test_ess.o:	$(SRC_DIR)/scheduler.cpp 
	$(CXX) -c $(CPPFLAGS) -D_MAIN_ESS -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(INC_DIR)

$(TEST_DIR)%.o:	$(TSRC_DIR)/%.cpp 
	$(CXX) -c $(CPPFLAGS) -D_MAIN -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(INC_DIR)

$(OBJ_DIR)%.so:	$(OBJ_DIR)%.o
	$(CXX)  $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) $(LIBS)

$(FUN_DIR)%.so:	$(FUN_DIR)%.o
	$(CXX)  $(CPPFLAGS) -shared -fpic -o $@ $< $(INCLUDES_BUILD) $(LIBS)


$(FUN_DIR)Reference.o:	$(SL_FUNC_DIR)/Reference.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)Reference_Interface.o:	$(SL_FUNC_DIR)/Reference_Interface.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)func1.o:	$(SL_FUNC_DIR)/func1.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)func2.o:	$(SL_FUNC_DIR)/func2.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)func3.o:	$(SL_FUNC_DIR)/func3.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)DC_Augmentation.o:	$(SL_FUNC_DIR)/DC_Augmentation.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)DC_Augmentation_Interface.o:	$(SL_FUNC_DIR)/DC_Augmentation_Interface.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)HighLevelController.o:	$(SL_FUNC_DIR)/HighLevelController.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)HighLevelController_Interface.o:	$(SL_FUNC_DIR)/HighLevelController_Interface.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)LowLevelController.o:	$(SL_FUNC_DIR)/LowLevelController.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

$(FUN_DIR)LowLevelController_Interface.o:	$(SL_FUNC_DIR)/LowLevelController_Interface.cpp 
	$(CXX) -c $(CPPFLAGS) -shared -fpic -o $@ $^ $(INCLUDES_BUILD) -I$(SL_FUNC_DIR)

.PHONY: build
build:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(FUN_DIR) $(TEST_DIR)

.PHONY: clean
clean:
	rm -fr build

.PHONY: uninstall
uninstall:
	rm -rf $(BIN_DIR)ess_controller $(BIN_DIR)gpio_controller
	rm -f $(LIBS_DIR)libessfunc.so
	rm -f $(LIBS_DIR)libess.so


.PHONY: install
install:
	cp $(BUILD_DIR)*.so  $(LIBS_DIR)
	cp $(BUILD_DIR)ess_controller $(BIN_DIR)
	cp $(BUILD_DIR)gpio_controller $(BIN_DIR)

  
