CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo
# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS+= -lz

all: clean sender receiver

receiver: 
	cc ${CFLAGS} src/receiver.c src/utils.c src/sorted_queue.c src/network.c src/packet_implem.c -DDEBUG=0 -DPROGRAM_NAME=\"receiver\" -o receiver ${LDFLAGS}
sender:	
	cc ${CFLAGS} src/sender.c src/utils.c src/list_packet.c src/network.c src/packet_implem.c -DDEBUG=0 -DPROGRAM_NAME=\"sender\" -o sender ${LDFLAGS}

.PHONY: clean

clean:
	@rm -f src/sender src/receiver
