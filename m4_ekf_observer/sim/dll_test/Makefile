# Makefile — power_calculator.dll (MinGW, 两步编译)
CC       = /d/mingw64/mingw64/bin/gcc
CFLAGS   = -O2 -Wall -std=c11
LDFLAGS  = -shared -lm

POWER_SRC = ../../src/RX32G410_FW_HAL_V1.3N/Projects/BaseClass/src/power_calculator.c
POWER_INC = ../../src/RX32G410_FW_HAL_V1.3N/Projects/BaseClass/inc
STUB_INC  = stub

INCLUDES = -I$(POWER_INC) -I$(STUB_INC)
CFLAGS  += -Wno-unused-variable -Wno-unused-function -Wno-missing-braces -Wno-int-conversion

TARGET = power_calculator.dll
OBJS   = dll_bridge.o power_calculator.o

.PHONY: all clean

all: $(TARGET)

power_calculator.o: $(POWER_SRC) $(POWER_INC)/power_calculator.h $(STUB_INC)/api_hrtim.h
	$(CC) $(CFLAGS) $(INCLUDES) -include $(STUB_INC)/api_hrtim.h -c $(POWER_SRC) -o $@

dll_bridge.o: dll_bridge.c $(POWER_INC)/power_calculator.h
	$(CC) $(CFLAGS) $(INCLUDES) -c dll_bridge.c -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
