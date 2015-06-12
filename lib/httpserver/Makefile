include buildinfo.inc
BUILD := $(shell expr $(BUILD) + 1)

DEFINES := -DMAJORVER=\"$(MAJOR)\" -DMINORVER=\"$(MINOR)\" -DPATCHVER=\"$(PATCH)\" -DBUILDVER=\"$(BUILD)\" 
INCLUDES := -Iinclude

CXX      := g++
CXXFLAGS := -g -Wall -pedantic -std=c++11 $(DEFINES) $(INCLUDES)

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXENAME := httpserver

all: $(OBJECTS) 
	@echo "	(LD) -o $(EXENAME) $(OBJECTS)"
	@$(CXX)  $(OBJECTS) $(CXXFLAGS) -o $(EXENAME) -Wl,-Map=http.map
	@echo "  Version: $(MAJOR).$(MINOR).$(PATCH)-$(BUILD)"

%.o : %.cpp
	@echo "	(CPP) $@"
	@$(CXX) $(CXXFLAGS) -MMD -c -o $@  $<


-include $(SOURCES:%.cpp=%.d)


buildinfo.inc : $(OBJECTS)
	@echo "MAJOR :=$(MAJOR)" > buildinfo.inc
	@echo "MINOR :=$(MINOR)" >> buildinfo.inc
	@echo "PATCH :=$(PATCH)" >> buildinfo.inc
	@echo "BUILD :=$(BUILD)" >> buildinfo.inc

.PHONY: clean
clean:
	rm -f $(OBJECTS)
	rm -f $(OBJECTS:%.o=%.d)
	rm -f $(EXENAME) $(EXENAME).exe

