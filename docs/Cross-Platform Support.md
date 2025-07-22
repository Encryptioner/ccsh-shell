# Cross-Platform Support

This document details the comprehensive cross-platform support implemented in ccsh.

## Overview

ccsh now supports building and running on multiple Unix-like operating systems with automatic platform detection and appropriate configuration.

## Supported Platforms

### Linux Distributions
- **Ubuntu/Debian**: Uses `apt` package manager
- **CentOS/RHEL**: Uses `yum` package manager  
- **Fedora**: Uses `dnf` package manager
- **Arch Linux**: Uses `pacman` package manager
- **Other Linux**: Generic fallback with manual dependency installation

### BSD Systems
- **FreeBSD**: Uses `pkg` package manager
- **OpenBSD**: Uses `pkg_add` package manager
- **NetBSD**: Uses `pkgin` package manager

### macOS
- **macOS**: Uses `brew` package manager (Homebrew)

## Platform Detection

The build system automatically detects the platform using `uname -s` and configures:

1. **Compiler flags**: Platform-specific CFLAGS and LDFLAGS
2. **Library linking**: Appropriate libraries for each platform
3. **Static linking**: Platform-specific static linking configuration

### Detection Logic

```makefile
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    # macOS configuration
else ifeq ($(UNAME_S),Linux)
    # Linux configuration
else ifeq ($(UNAME_S),FreeBSD)
    # FreeBSD configuration
# ... etc
```

## Library Dependencies

### Readline Library
- **Linux**: `-lreadline -lncurses -ltinfo` (static builds)
- **macOS**: `-lreadline` (static linking limited)
- **BSD**: `-lreadline -lncurses` (static builds)

### Platform-Specific Notes

#### Linux
- Static builds require `ncurses` and `tinfo` libraries
- Package names vary by distribution:
  - Ubuntu/Debian: `libreadline-dev`
  - CentOS/RHEL: `readline-devel`
  - Arch: `readline`

#### macOS
- Static linking is limited due to system restrictions
- Uses Homebrew for dependency management
- Requires Xcode Command Line Tools

#### BSD Systems
- FreeBSD: Uses `pkg` package manager
- OpenBSD: Uses `pkg_add` (different syntax)
- NetBSD: Uses `pkgin` package manager

## Build System

### Makefile Features

1. **Platform Detection**: Automatic OS detection
2. **Conditional Compilation**: Platform-specific code paths
3. **Library Configuration**: Appropriate libraries per platform
4. **Build Targets**: Multiple build types (normal, static, debug)

### Build Script Features

1. **Dependency Installation**: Automatic dependency installation
2. **Platform Detection**: Comprehensive platform detection
3. **Error Handling**: Graceful failure with helpful messages
4. **Testing**: Built-in testing capabilities

## Code Changes

### main.c Improvements

```c
/* Cross-platform readline detection */
#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #ifdef HAVE_READLINE
        #include <readline/readline.h>
        #include <readline/history.h>
        #define READLINE_LIB 1
    #else
        #define READLINE_LIB 0
    #endif
#else
    #define READLINE_LIB 0
#endif
```

### Key Improvements

1. **Broader Platform Support**: Extended beyond just macOS/Linux
2. **Conditional Compilation**: `HAVE_READLINE` define for better control
3. **Fallback Support**: Graceful degradation when readline unavailable

## Build Commands

### Standard Build
```bash
make all
```

### Static Binary
```bash
make static
```

### Debug Build
```bash
make debug
```

### Automated Build
```bash
./build-and-package.sh -a
```

## Testing

### Platform Testing
Each platform should be tested for:

1. **Dependency Installation**: Automatic dependency installation
2. **Compilation**: Successful build with appropriate flags
3. **Static Linking**: Static binary creation (where supported)
4. **Runtime**: Basic functionality testing
5. **Readline Features**: Interactive features when available

### Test Commands
```bash
# Check dependencies
./build-and-package.sh -c

# Full build and test
./build-and-package.sh -a

# Platform-specific testing
make test
```

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   - Use `./build-and-package.sh -d` for automatic installation
   - Check platform-specific package names

2. **Static Linking Failures**
   - macOS: Use `make all` instead of `make static`
   - Linux: Ensure `ncurses` and `tinfo` are installed

3. **Unknown Platform**
   - Manual dependency installation required
   - Check `make check-deps` for configuration

### Platform-Specific Issues

#### macOS
- Static linking limitations
- Homebrew dependency management
- Xcode Command Line Tools requirement

#### BSD Systems
- Different package managers
- Different library names
- Different static linking requirements

#### Linux
- Distribution-specific package names
- Different ncurses library configurations

## Future Enhancements

### Planned Improvements

1. **Windows Support**: WSL or Cygwin compatibility
2. **ARM Support**: Better ARM architecture support
3. **Container Support**: Docker-based builds
4. **CI/CD Integration**: Automated cross-platform testing

### Potential Additions

1. **Package Managers**: Support for more package managers
2. **Cross-Compilation**: Build for different architectures
3. **Binary Distribution**: Pre-compiled binaries per platform
4. **Dependency Resolution**: Automatic dependency resolution

## Contributing

When contributing cross-platform improvements:

1. **Test on Multiple Platforms**: Ensure changes work across platforms
2. **Update Documentation**: Keep platform-specific docs current
3. **Add Platform Support**: Extend support to new platforms
4. **Maintain Compatibility**: Don't break existing platform support

## References

- [POSIX Standards](https://pubs.opengroup.org/onlinepubs/9699919799/)
- [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html)
- [Cross-Platform C Development](https://en.wikipedia.org/wiki/Cross-platform) 