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
LIBS = -LTMW/bin -L$(LIBS_DIR) -L$(LIBS_DIR64) -L$(SLIBS_DIR) -L$(SLIBS_DIR64) -Wl,-rpath=$(LIBS_DIR):$(LIBS_DIR64):$(LIBS_DIR)cpp_17:$(SLIBS_DIR):$(SLIBS_DIR64) -lfims

CXX = g++
CXXFLAGS = -std=c++17 -Wall

TARGET = fims_replay
SRCS = fims_replay.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(INCLUDES) $(LIBS) -lfims

clean:
	rm -f $(TARGET)
