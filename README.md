# ccsh — Compact C Shell

A small yet flexible Unix‑like shell in C featuring:
- Interactive prompt with colors and command history
- Command execution, pipes, and I/O redirection
- Background job management with `jobs` and `fg`
- Command aliasing and globbing (wildcard expansion)
- Built-in commands: cd, pwd, exit, help, alias, unalias
- Command history & arrow keys via readline (when available)
- Signal handling (Ctrl+C, Ctrl+Z)
- Cross-platform support (Linux/macOS)

## Installation

### Quick Start
```sh
git clone https://github.com/yourname/ccsh.git
cd ccsh
make
```

### Build Options

**Standard Build** (dynamic linking):
```sh
make
```

**Static Binary** (self-contained, no external dependencies):
```sh
make static
```

**Debug Build** (with debug symbols):
```sh
make debug
```

**Clean Build** (remove all build artifacts):
```sh
make clean
```

### Dependencies

**Required:**
- **GCC/Clang**: C compiler
- **Make**: Build system
- **Readline**: Command line editing (optional, falls back to basic input)

**Installation by Platform:**

**Ubuntu/Debian:**
```sh
sudo apt-get install build-essential libreadline-dev
```

**macOS:**
```sh
brew install readline
```

**CentOS/RHEL:**
```sh
sudo yum install gcc make readline-devel
```

**Fedora:**
```sh
sudo dnf install gcc make readline-devel
```

**Arch Linux:**
```sh
sudo pacman -S base-devel readline
```

## Distribution

### Creating Distribution Packages

To create a distributable package:

```sh
./build-and-package.sh
```

This script:
1. **Builds the shell**: Compiles ccsh with appropriate flags for your platform
2. **Creates package structure**: Organizes files in a platform-specific directory
3. **Generates scripts**: Creates installation and uninstallation scripts
4. **Packages everything**: Creates a compressed `.tar.gz` file with checksum

### What's Generated

The `dist/` folder contains:
```
dist/
├── ccsh-1.0.0-Darwin-arm64/          # Platform-specific directory
│   ├── ccsh                          # Compiled executable
│   ├── README.md                      # Documentation
│   ├── install.sh                     # Installation script
│   ├── uninstall.sh                   # Uninstallation script
│   ├── test.sh                        # Test script
│   ├── PACKAGE_INFO                   # Package information
│   └── docs/                          # Documentation files
├── ccsh-1.0.0-Darwin-arm64.tar.gz    # Compressed package
└── ccsh-1.0.0-Darwin-arm64.tar.gz.sha256  # Checksum for verification
```

### Platform Detection

The script automatically detects your platform:
- **macOS**: `ccsh-1.0.0-Darwin-arm64.tar.gz`
- **Linux x86_64**: `ccsh-1.0.0-Linux-x86_64.tar.gz`
- **Linux ARM**: `ccsh-1.0.0-Linux-arm64.tar.gz`

### Using the Package

1. **Extract**: `tar -xzf ccsh-1.0.0-Darwin-arm64.tar.gz`
2. **Install**: `cd ccsh-1.0.0-Darwin-arm64 && ./install.sh`
3. **Use**: `ccsh` or set as default shell

## Usage

### Basic Usage
```sh
./ccsh
```

### Set as Default Shell

**Option 1: User-specific installation**
```sh
# Install to user directory
./build-and-package.sh
cd dist/ccsh-1.0.0-$(uname -s)-$(uname -m)
./install.sh

# Add to /etc/shells (requires sudo)
echo "$HOME/.local/bin/ccsh" | sudo tee -a /etc/shells

# Change your shell
chsh -s "$HOME/.local/bin/ccsh"
```

**Option 2: System-wide installation**
```sh
# Install system-wide (requires sudo)
sudo ./install.sh

# Add to /etc/shells
echo "/usr/local/bin/ccsh" | sudo tee -a /etc/shells

# Change your shell
chsh -s /usr/local/bin/ccsh
```

**Note**: Log out and log back in to apply the shell change.

### Uninstall
```sh
# If installed user-specific
~/.local/bin/uninstall.sh

# If installed system-wide
sudo /usr/local/bin/uninstall.sh
```

## Configuration

### Shell Configuration File

Create `~/.ccshrc` for persistent configuration:

```sh
# Aliases
alias ll="ls -lah"
alias g="git"
alias ..="cd .."
alias ...="cd ../.."

# Custom prompt
export CCSH_PROMPT="ccsh> "

# Welcome message
echo "Welcome to ccsh, $(whoami)!"
echo "Current directory: $(pwd)"

# Custom functions
function mkcd() {
    mkdir -p "$1" && cd "$1"
}

# Environment variables
export EDITOR=vim
export PATH="$HOME/.local/bin:$PATH"
```

### Plugin Support

Create plugin files (e.g., `~/.ccsh_plugins/aliases.ccsh`):

```sh
# ~/.ccsh_plugins/aliases.ccsh
alias dev="cd ~/projects"
alias docs="cd ~/documents"
alias temp="cd /tmp"
```

Load plugins in your `.ccshrc`:

```sh
# Load plugins
if [ -f ~/.ccsh_plugins/aliases.ccsh ]; then
    source ~/.ccsh_plugins/aliases.ccsh
fi
```


## Built-in Commands

### Navigation
- **cd**: Change directory
- **pwd**: Print working directory

### Job Control
- **jobs**: List background jobs
- **fg**: Bring job to foreground

### Shell Control
- **exit**: Exit the shell
- **help**: Show help information

### Alias Management
- **alias**: Create or list aliases
- **unalias**: Remove aliases

### File Operations
- **which**: Find command location in PATH
- **path**: Show PATH environment variable

### Text Processing
- **grep**: Built-in grep with options (-i, -n, -v, -c)

### Examples
```sh
# Navigation
cd /tmp
pwd

# Job control
sleep 10 &
jobs
fg %1

# Aliases
alias ll="ls -lah"
alias g="git"
unalias g

# File operations
which ls
path

# Text processing
grep -i "hello" file.txt
grep -n "error" *.log
```

## Testing

### Run Tests
```sh
make test
```

### Manual Testing
```sh
# Test basic functionality
./ccsh
ccsh> ls -la
ccsh> pwd
ccsh> cd /tmp
ccsh> exit

# Test job control
./ccsh
ccsh> sleep 5 &
ccsh> jobs
ccsh> fg %1

# Test aliases
./ccsh
ccsh> alias ll="ls -lah"
ccsh> ll
ccsh> alias
ccsh> unalias ll

# Test I/O redirection
./ccsh
ccsh> echo "hello" > test.txt
ccsh> cat < test.txt
ccsh> echo "world" >> test.txt
ccsh> cat test.txt

# Test globbing
ccsh> ls *.txt
ccsh> ls test?.txt
```

## Documentation

### Core Documentation
- [Shell Introduction](docs/Shell%20Introduction.md) - Overview of shell concepts and features
- [Makefile Introduction](docs/Makefile%20Introduction.md) - How Makefiles work and C program distribution
- [Coding Dive](docs/Coding%20Dive.md) - Technical implementation details
- [Distribution Process](docs/Distribution%20Process.md) - How the dist/ folder is generated

### Quick References
- [Makefile Quick Reference](docs/Makefile%20Quick%20Reference.md) - Common make commands and patterns
- [Distribution Example](docs/Distribution%20Example.md) - How to use the distribution package

### Learning Path
1. **Start with**: [Shell Introduction](docs/Shell%20Introduction.md) for basic concepts
2. **Build process**: [Makefile Introduction](docs/Makefile%20Introduction.md) for understanding compilation
3. **Implementation**: [Coding Dive](docs/Coding%20Dive.md) for technical details
4. **Distribution**: [Distribution Process](docs/Distribution%20Process.md) for packaging

## Features

### Core Shell Features
- **Command Execution**: Run external programs and built-in commands
- **I/O Redirection**: `<`, `>`, `>>` for input/output redirection
- **Background Jobs**: Run commands in background with `&`
- **Job Control**: Manage background jobs with `jobs` and `fg`
- **Signal Handling**: Handle Ctrl+C and other signals gracefully

### Advanced Features
- **Command History**: Arrow keys for command history (with readline)
- **Alias System**: Create shortcuts for commands
- **Globbing**: Wildcard expansion (`*`, `?`)
- **Built-in Commands**: Essential commands implemented in the shell
- **Cross-platform**: Works on Linux and macOS

### Development Features
- **POSIX Compliance**: Follows POSIX standards for portability
- **Modular Design**: Clean, maintainable C code
- **Comprehensive Testing**: Automated and manual testing
- **Professional Packaging**: Automated distribution system

## Examples

### Basic Usage
```sh
# Start the shell
./ccsh

# Run commands
ccsh> ls -la
ccsh> pwd
ccsh> cd /tmp

# Use aliases
ccsh> alias ll="ls -lah"
ccsh> ll

# Background jobs
ccsh> sleep 10 &
ccsh> jobs
ccsh> fg %1
```

### I/O Redirection
```sh
# Output redirection
ccsh> echo "hello" > file.txt
ccsh> cat file.txt

# Input redirection
ccsh> cat < file.txt

# Append redirection
ccsh> echo "world" >> file.txt

# Error redirection
ccsh> ls nonexistent 2> error.log
```

### Globbing
```sh
# List all .txt files
ccsh> ls *.txt

# List files with specific pattern
ccsh> ls test?.txt

# List files in subdirectories
ccsh> ls **/*.c
```

## Contributing

### Development Setup
```sh
# Clone the repository
git clone https://github.com/yourname/ccsh.git
cd ccsh

# Build for development
make debug

# Run tests
make test

# Create distribution package
./build-and-package.sh
```

### Code Style
- Follow C coding standards
- Use meaningful variable and function names
- Add comments for complex logic
- Test changes thoroughly

### Reporting Issues
- Check existing issues first
- Provide detailed reproduction steps
- Include system information (OS, architecture)
- Test with latest version

## License

This project is open source. See [LICENSE](LICENSE) for details.

## Acknowledgments

- **Readline Library**: For command line editing features
- **POSIX Standards**: For portable shell implementation
- **Unix Philosophy**: For simple, composable design
