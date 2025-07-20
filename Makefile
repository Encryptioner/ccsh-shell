CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lreadline

# Detect OS for static linking
UNAME_S := $(shell uname -s)

.PHONY: all clean run test static

all: ccsh

ccsh: main.c
	$(CC) $(CFLAGS) main.c -o ccsh $(LDFLAGS)

static: main.c
ifeq ($(UNAME_S),Darwin)
	@echo "Static linking not fully supported on macOS"
	@echo "Building with standard linking instead..."
	$(CC) $(CFLAGS) main.c -o ccsh $(LDFLAGS)
else
	@echo "Building static binary..."
	$(CC) $(CFLAGS) -static main.c -o ccsh $(LDFLAGS)
endif

run: ccsh
	./ccsh

clean:
	rm -f ccsh *.o

test: ccsh test.sh
	chmod +x test.sh
	./test.sh
