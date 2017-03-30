INCLUDE_DIRS := ../../include

COMMON_FLAGS += $(foreach includedir, $(INCLUDE_DIRS), -I$(includedir))
CFLAGS += $(COMMON_FLAGS)

%.o: %.cpp
	$(Q)$(CC) $(CFLAGS) -c $<