SRCS := $(wildcard *.cpp)
INCLUDE_DIRS := ./include /usr/local/include
LDFLAGS := -L/usr/local/lib

# Debug
# DEBUG := 1

ifeq ($(DEBUG), 1)
	COMMON_FLAGS += -g
endif
LIBRARIES := json-c glog gflags
DEPS := src/json-c/parse_json.d

.PHONY: clean

COMMON_FLAGS += $(foreach includedir, $(INCLUDE_DIRS), -I$(includedir))
CXXFLAGS += $(COMMON_FLAGS)
LDFLAGS += $(foreach library, $(LIBRARIES), -l$(library))
LDFLAGS += $(COMMON_FLAGS)

all: json_reader

%.o: %.cpp
	$(Q)$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o
	$(Q)$(CXX) $(CXXFLAGS) -MM $*.cpp > $*.d
%: %.o
	$(Q)$(CXX) $< -o $@ $(CXXFLAGS)

json_reader: src/reader.o src/json-c/parse_json.o 
	$(Q)$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	@rm -f json_reader
	@rm -rf *.o *.d

-include $(DEPS)
