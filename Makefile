CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lreadline

.PHONY: all clean run test static

all: ccsh

ccsh: main.c
	$(CC) $(CFLAGS) main.c -o ccsh $(LDFLAGS)

static: main.c
	$(CC) $(CFLAGS) -static main.c -o ccsh $(LDFLAGS)

run: ccsh
	./ccsh

clean:
	rm -f ccsh *.o

test: ccsh test.sh
	chmod +x test.sh
	./test.sh
