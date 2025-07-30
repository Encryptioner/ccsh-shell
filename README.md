# ccsh - Compact C Shell

A small yet powerful Unix-like shell written in C, featuring interactive command line editing, job control, aliases, and more.

## Features

- **Interactive Command Line**: Full readline support with history, tab completion, and line editing
- **Job Control**: Background job management with `jobs`, `fg`, and `bg` commands
- **Alias Support**: Command aliases with `alias` and `unalias`
- **Built-in Commands**: `cd`, `pwd`, `exit`, `help`, `jobs`, `fg`, `alias`, `unalias`
- **Redirection**: Input/output redirection with `>`, `>>`, `<`
- **Pipelines**: Command chaining with `|`
- **Signal Handling**: Proper Ctrl+C handling
- **Cross-Platform**: Works on macOS, Linux, FreeBSD, OpenBSD, and NetBSD

## Cross-Platform Support

### Supported Platforms

| Platform | Package Manager | Dependencies |
|----------|----------------|--------------|
| **Ubuntu/Debian** | `apt` | `build-essential libreadline-dev` |
| **CentOS/RHEL** | `yum` | `Development Tools readline-devel` |
| **Fedora** | `dnf` | `Development Tools readline-devel` |
| **Arch Linux** | `pacman` | `base-devel readline` |
| **macOS** | `brew` | `readline` |
| **FreeBSD** | `pkg` | `gcc readline` |
| **OpenBSD** | `pkg_add` | `gcc readline` |
| **NetBSD** | `pkgin` | `gcc readline` |

### Quick Start

#### Automatic Setup (Recommended)
```bash
# Clone the repository
git clone <repository-url>
cd ccsh-shell

# Install dependencies and build
./build-and-package.sh -a
```

#### Manual Setup
```bash
# 1. Install dependencies (see platform-specific instructions below)
# 2. Build the project
make all

# 3. Run the shell
./ccsh
```

## Platform-Specific Installation

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential libreadline-dev
```

### CentOS/RHEL/Fedora
```bash
# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install readline-devel

# Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install readline-devel
```

### Arch Linux
```bash
sudo pacman -S --needed base-devel readline
```

### macOS
```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install readline
```

### FreeBSD
```bash
sudo pkg install gcc readline
```

### OpenBSD
```bash
sudo pkg_add gcc readline
```

### NetBSD
```bash
sudo pkgin install gcc readline
```

## Build Options

### Using Makefile
```bash
# Standard build
make all

# Static binary (Linux/BSD)
make static

# Debug build
make debug

# Clean build artifacts
make clean

# Check dependencies
make check-deps

# Show help
make help
```

### Using Build Script
```bash
# Full automated build (install deps + build + test)
./build-and-package.sh -a

# Install dependencies only
./build-and-package.sh -d

# Check dependencies
./build-and-package.sh -c

# Build specific type
./build-and-package.sh -b normal    # Standard build
./build-and-package.sh -b static    # Static binary
./build-and-package.sh -b debug     # Debug build

# Test the build
./build-and-package.sh -t
```

## Usage

### Basic Usage
```bash
# Start the shell
./ccsh

# Run a command and exit
echo "ls" | ./ccsh
```

### Built-in Commands
```bash
ccsh> help          # Show help
ccsh> cd /tmp       # Change directory
ccsh> pwd           # Print working directory
ccsh> jobs          # List background jobs
ccsh> alias ll=ls   # Create alias
ccsh> unalias ll    # Remove alias
ccsh> exit          # Exit shell
```

### Job Control
```bash
ccsh> sleep 10 &    # Run in background
ccsh> jobs          # List background jobs
ccsh> fg 0          # Bring job to foreground
```

### Redirection and Pipelines
```bash
ccsh> ls > files.txt        # Output redirection
ccsh> cat < files.txt       # Input redirection
ccsh> ls | grep .c          # Pipeline
ccsh> echo "test" >> log    # Append redirection
```

## Development

### Project Structure
```
ccsh-shell/
├── main.c                 # Main shell implementation
├── Makefile               # Cross-platform build configuration
├── build-and-package.sh   # Automated build script
├── test.sh                # Test suite
├── README.md              # This file
└── docs/                  # Documentation
```

### Building for Different Platforms

The build system automatically detects your platform and configures the appropriate compiler flags and libraries:

- **Linux**: Uses `-lreadline -lncurses -ltinfo` for static builds
- **macOS**: Uses `-lreadline` (static linking limited)
- **BSD**: Uses `-lreadline -lncurses` for static builds

### Debugging
```bash
# Build with debug symbols
make debug

# Run with debug output
./ccsh
```

## Testing

```bash
# Run the test suite
make test

# Or use the build script
./build-and-package.sh -t
```

## Troubleshooting

### Common Issues

1. **"readline/readline.h: No such file or directory"**
   - Install readline development headers for your platform
   - Use `./build-and-package.sh -d` to install dependencies

2. **Static linking fails on macOS**
   - macOS has limitations with static linking
   - Use `make all` instead of `make static`

3. **Build fails on unknown Linux distribution**
   - Install manually: `gcc`, `make`, `readline-dev`
   - Or use the build script: `./build-and-package.sh -d`

### Platform-Specific Notes

- **macOS**: Static linking is limited due to system restrictions
- **FreeBSD**: May require additional ncurses libraries
- **OpenBSD**: Uses different package manager (`pkg_add`)
- **NetBSD**: Uses `pkgin` package manager

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on multiple platforms
5. Submit a pull request

## Acknowledgments

- GNU Readline library for interactive command line editing
- POSIX standards for shell compatibility
