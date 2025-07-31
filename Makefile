CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lreadline

# Detect OS and architecture
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Platform-specific settings
ifeq ($(UNAME_S),Darwin)
    # macOS
    PLATFORM = darwin
    READLINE_CFLAGS = -DHAVE_READLINE
    READLINE_LDFLAGS = -lreadline
    STATIC_LDFLAGS = -lreadline
else ifeq ($(UNAME_S),Linux)
    # Linux
    PLATFORM = linux
    READLINE_CFLAGS = -DHAVE_READLINE
    READLINE_LDFLAGS = -lreadline
    STATIC_LDFLAGS = -lreadline -lncurses -ltinfo
else ifeq ($(UNAME_S),FreeBSD)
    # FreeBSD
    PLATFORM = freebsd
    READLINE_CFLAGS = -DHAVE_READLINE
    READLINE_LDFLAGS = -lreadline
    STATIC_LDFLAGS = -lreadline -lncurses
else ifeq ($(UNAME_S),OpenBSD)
    # OpenBSD
    PLATFORM = openbsd
    READLINE_CFLAGS = -DHAVE_READLINE
    READLINE_LDFLAGS = -lreadline
    STATIC_LDFLAGS = -lreadline -lncurses
else ifeq ($(UNAME_S),NetBSD)
    # NetBSD
    PLATFORM = netbsd
    READLINE_CFLAGS = -DHAVE_READLINE
    READLINE_LDFLAGS = -lreadline
    STATIC_LDFLAGS = -lreadline -lncurses
else
    # Generic Unix - try to detect readline
    PLATFORM = generic
    READLINE_CFLAGS = -DHAVE_READLINE
    READLINE_LDFLAGS = -lreadline
    STATIC_LDFLAGS = -lreadline
endif

.PHONY: all clean run test static debug help check-deps

all: ccsh

ccsh: main.c
	@echo "[INFO] Building ccsh for $(PLATFORM) ($(UNAME_S))..."
	$(CC) $(CFLAGS) $(READLINE_CFLAGS) main.c -o ccsh $(READLINE_LDFLAGS)

static: main.c
ifeq ($(UNAME_S),Darwin)
	@echo "[INFO] Building static binary for macOS..."
	@echo "[WARN] Static linking on macOS may not work as expected"
	$(CC) $(CFLAGS) $(READLINE_CFLAGS) main.c -o ccsh $(STATIC_LDFLAGS)
else
	@echo "[INFO] Building static binary for $(PLATFORM)..."
	$(CC) $(CFLAGS) $(READLINE_CFLAGS) -static main.c -o ccsh $(STATIC_LDFLAGS)
endif

debug: main.c
	@echo "[INFO] Building debug version for $(PLATFORM)..."
	$(CC) $(CFLAGS) -g -DDEBUG $(READLINE_CFLAGS) main.c -o ccsh $(READLINE_LDFLAGS)

run: ccsh
	./ccsh

clean:
	@echo "[INFO] Cleaning build artifacts..."
	rm -f ccsh *.o
	rm -rf ccsh.dsym

test: ccsh test.sh
	@echo "[INFO] Running tests..."
	chmod +x test.sh
	./test.sh

check-deps:
	@echo "[INFO] Checking dependencies for $(PLATFORM)..."
	@echo "Platform: $(PLATFORM) ($(UNAME_S))"
	@echo "Architecture: $(UNAME_M)"
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS) $(READLINE_CFLAGS)"
	@echo "LDFLAGS: $(READLINE_LDFLAGS)"
	@echo "Static LDFLAGS: $(STATIC_LDFLAGS)"

help:
	@echo "Available targets:"
	@echo "  all        - Build ccsh (default)"
	@echo "  static     - Build static binary"
	@echo "  debug      - Build with debug symbols"
	@echo "  run        - Build and run ccsh"
	@echo "  test       - Run tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  check-deps - Show build configuration"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Platform: $(PLATFORM) ($(UNAME_S))"
	@echo "Architecture: $(UNAME_M)"
