# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo
# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -rdynamic 

# Default target
all: clean src/sender src/receiver 

# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer 
debug: clean src/sender src/receiver

# We use an implicit rule to build an executable named 'chat'
src/sender: src/sender.o src/list_packet.o src/network.o src/packet_implem.o src/utils.o -lz
src/receiver: src/receiver.o src/network.o src/packet_implem.o src/utils.o src/sorted_queue.o -lz

.PHONY: clean

clean:
	@rm -f src/sender src/receiver src/sender.o src/receiver.o src/packet_implem.o src/network.o src/list_packet.o src/utils.o src/sorted_queue.o
