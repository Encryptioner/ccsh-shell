# Makefile Quick Reference

## Common Make Commands

```bash
make              # Build default target (usually 'all')
make all          # Build all targets
make clean        # Remove build artifacts
make install      # Install the program
make uninstall    # Remove installed files
make test         # Run tests
make run          # Build and run
make static       # Build static binary
make debug        # Build with debug symbols
make release      # Build optimized release version
```

## Makefile Variables

```makefile
CC = gcc                    # C compiler
CFLAGS = -Wall -Wextra -O2 # Compiler flags
LDFLAGS = -lreadline       # Linker flags
TARGET = ccsh              # Executable name
SOURCES = main.c           # Source files
OBJECTS = main.o           # Object files
```

## Common Compiler Flags

```bash
-Wall          # Enable all common warnings
-Wextra        # Enable extra warnings
-O0            # No optimization
-O1            # Basic optimization
-O2            # More optimization
-O3            # Maximum optimization
-g             # Include debug symbols
-DDEBUG        # Define DEBUG macro
-static        # Static linking
-shared        # Shared library
-c             # Compile only (no linking)
-o filename    # Output filename
```

## Makefile Patterns

### Basic Rule
```makefile
target: dependencies
	command
```

### Pattern Rules
```makefile
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
```

### Variables in Rules
```makefile
$@    # Target name
$<    # First dependency
$^    # All dependencies
$*    # Stem (for pattern rules)
```

### Conditional Compilation
```makefile
ifeq ($(OS),Darwin)
    # macOS specific
else
    # Linux specific
endif
```

## Phony Targets
```makefile
.PHONY: all clean test install
```

## Advanced Features

### Multiple Configurations
```makefile
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

release: CFLAGS += -DNDEBUG -O3
release: $(TARGET)
```

### Automatic Dependencies
```makefile
-include $(SOURCES:.c=.d)

%.d: %.c
	$(CC) -MM $< > $@
```

### Cross-Compilation
```makefile
# For ARM
CC = arm-linux-gnueabi-gcc

# For Windows
CC = x86_64-w64-mingw32-gcc
```

## Distribution Commands

### Build for Distribution
```bash
make clean
make static
```

### Create Package
```bash
./build-and-package.sh
```

### Install System-Wide
```bash
sudo make install
```

### Uninstall
```bash
sudo make uninstall
```

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install build-essential libreadline-dev
   
   # macOS
   brew install readline
   ```

2. **Permission Issues**
   ```bash
   chmod +x ccsh
   ```

3. **Library Path Issues**
   ```bash
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
   ```

### Debugging Make

```bash
make -n          # Show what would be executed (dry run)
make -d          # Show detailed debugging info
make --debug=v   # Verbose debugging
```

## Best Practices

1. **Always include clean target**
   ```makefile
   clean:
   	rm -f $(TARGET) *.o *.d
   ```

2. **Use variables for reusability**
   ```makefile
   CC = gcc
   CFLAGS = -Wall -Wextra -O2
   ```

3. **Include .PHONY for non-file targets**
   ```makefile
   .PHONY: all clean test install
   ```

4. **Handle dependencies properly**
   ```makefile
   $(TARGET): $(SOURCES)
   	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
   ```

5. **Use conditional compilation for cross-platform**
   ```makefile
   ifeq ($(OS),Darwin)
       # macOS specific
   else
       # Linux specific
   endif
   ```

## Example Complete Makefile

```makefile
# Variables
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lreadline
TARGET = ccsh
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

# Phony targets
.PHONY: all clean test install uninstall

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build
release: CFLAGS += -DNDEBUG -O3
release: $(TARGET)

# Static build
static: LDFLAGS += -static
static: $(TARGET)

# Clean
clean:
	rm -f $(TARGET) *.o *.d

# Test
test: $(TARGET)
	./test.sh

# Install
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	chmod +x /usr/local/bin/$(TARGET)

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  all      - Build $(TARGET)"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release"
	@echo "  static   - Build static binary"
	@echo "  clean    - Remove build artifacts"
	@echo "  test     - Run tests"
	@echo "  install  - Install system-wide"
	@echo "  uninstall- Remove installed files"
	@echo "  help     - Show this help"
``` 