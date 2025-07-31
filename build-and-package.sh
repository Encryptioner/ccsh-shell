#!/bin/bash

# Cross-platform build script for ccsh
# Supports macOS, Ubuntu/Debian, CentOS/RHEL, FreeBSD, OpenBSD, NetBSD

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Platform detection
detect_platform() {
    case "$(uname -s)" in
        Darwin*)    echo "macos" ;;
        Linux*)     echo "linux" ;;
        FreeBSD*)   echo "freebsd" ;;
        OpenBSD*)   echo "openbsd" ;;
        NetBSD*)    echo "netbsd" ;;
        *)          echo "unknown" ;;
    esac
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install dependencies for different platforms
install_dependencies() {
    local platform=$(detect_platform)
    
    echo -e "${BLUE}[INFO] Detected platform: $platform${NC}"
    
    case $platform in
        "linux")
            if command_exists apt-get; then
                echo -e "${BLUE}[INFO] Installing dependencies for Ubuntu/Debian...${NC}"
                sudo apt-get update
                sudo apt-get install -y build-essential libreadline-dev
            elif command_exists yum; then
                echo -e "${BLUE}[INFO] Installing dependencies for CentOS/RHEL...${NC}"
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y readline-devel
            elif command_exists dnf; then
                echo -e "${BLUE}[INFO] Installing dependencies for Fedora...${NC}"
                sudo dnf groupinstall -y "Development Tools"
                sudo dnf install -y readline-devel
            elif command_exists pacman; then
                echo -e "${BLUE}[INFO] Installing dependencies for Arch Linux...${NC}"
                sudo pacman -S --needed base-devel readline
            else
                echo -e "${YELLOW}[WARN] Unknown Linux distribution. Please install:${NC}"
                echo "  - gcc (or clang)"
                echo "  - make"
                echo "  - readline development headers"
            fi
            ;;
        "macos")
            if command_exists brew; then
                echo -e "${BLUE}[INFO] Installing dependencies via Homebrew...${NC}"
                brew install readline
            else
                echo -e "${YELLOW}[WARN] Homebrew not found. Please install:${NC}"
                echo "  - Xcode Command Line Tools: xcode-select --install"
                echo "  - Homebrew: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
                echo "  - readline: brew install readline"
            fi
            ;;
        "freebsd")
            echo -e "${BLUE}[INFO] Installing dependencies for FreeBSD...${NC}"
            sudo pkg install -y gcc readline
            ;;
        "openbsd")
            echo -e "${BLUE}[INFO] Installing dependencies for OpenBSD...${NC}"
            sudo pkg_add gcc readline
            ;;
        "netbsd")
            echo -e "${BLUE}[INFO] Installing dependencies for NetBSD...${NC}"
            sudo pkgin install gcc readline
            ;;
        *)
            echo -e "${RED}[ERROR] Unsupported platform: $platform${NC}"
            exit 1
            ;;
    esac
}

# Check if readline is available
check_readline() {
    echo -e "${BLUE}[INFO] Checking for readline library...${NC}"
    
    # Try to compile a simple test program
    cat > /tmp/readline_test.c << 'EOF'
#include <stdio.h>
#ifdef HAVE_READLINE
#include <readline/readline.h>
int main() { return 0; }
#else
int main() { return 1; }
#endif
EOF
    
    if gcc -DHAVE_READLINE -lreadline /tmp/readline_test.c -o /tmp/readline_test 2>/dev/null; then
        echo -e "${GREEN}[SUCCESS] Readline library found${NC}"
        rm -f /tmp/readline_test.c /tmp/readline_test
        return 0
    else
        echo -e "${YELLOW}[WARN] Readline library not found${NC}"
        rm -f /tmp/readline_test.c /tmp/readline_test
        return 1
    fi
}

# Build the project
build_project() {
    local build_type=$1
    
    echo -e "${BLUE}[INFO] Building ccsh...${NC}"
    
    case $build_type in
        "normal")
            make clean
            make all
            ;;
        "static")
            make clean
            make static
            ;;
        "debug")
            make clean
            make debug
            ;;
        *)
            echo -e "${RED}[ERROR] Unknown build type: $build_type${NC}"
            exit 1
            ;;
    esac
    
    if [ -f "ccsh" ]; then
        echo -e "${GREEN}[SUCCESS] Build completed successfully${NC}"
        echo -e "${BLUE}[INFO] Binary size: $(ls -lh ccsh | awk '{print $5}')${NC}"
    else
        echo -e "${RED}[ERROR] Build failed${NC}"
        exit 1
    fi
}

# Test the build
test_build() {
    echo -e "${BLUE}[INFO] Testing ccsh...${NC}"
    
    if [ ! -f "ccsh" ]; then
        echo -e "${RED}[ERROR] ccsh binary not found${NC}"
        exit 1
    fi
    
    # Simple test
    echo "echo 'Hello from ccsh'" | ./ccsh > /tmp/ccsh_test.out 2>&1
    if grep -q "Hello from ccsh" /tmp/ccsh_test.out; then
        echo -e "${GREEN}[SUCCESS] Basic functionality test passed${NC}"
    else
        echo -e "${YELLOW}[WARN] Basic functionality test failed${NC}"
    fi
    rm -f /tmp/ccsh_test.out
}

# Create distribution package
create_package() {
    echo -e "${BLUE}[INFO] Creating distribution package...${NC}"
    
    if [ ! -f "ccsh" ]; then
        echo -e "${RED}[ERROR] ccsh binary not found. Build first with -b option${NC}"
        exit 1
    fi
    
    # Configuration
    VERSION="1.0.0"
    NAME="ccsh"
    DESCRIPTION="Compact C Shell - A small yet flexible Unix-like shell in C"
    
    # Platform detection
    OS=$(uname -s)
    ARCH=$(uname -m)
    
    # Get executable size
    EXEC_SIZE=$(ls -lh ccsh | awk '{print $5}')
    
    # Create distribution directory
    DIST_DIR="dist/$NAME-$VERSION-$OS-$ARCH"
    mkdir -p "$DIST_DIR"
    
    echo -e "${BLUE}[INFO] Creating package: $DIST_DIR${NC}"
    
    # Copy files
    cp ccsh "$DIST_DIR/"
    cp README.md "$DIST_DIR/" 2>/dev/null || echo -e "${YELLOW}[WARN] README.md not found${NC}"
    cp test.sh "$DIST_DIR/" 2>/dev/null || echo -e "${YELLOW}[WARN] test.sh not found${NC}"
    
    # Create installation script
    cat > "$DIST_DIR/install.sh" << 'EOF'
#!/bin/bash
# install.sh - Installation script for ccsh

set -e

echo "Installing ccsh..."

# Check if running as root for system-wide installation
if [ "$EUID" -eq 0 ]; then
    INSTALL_DIR="/usr/local/bin"
    echo "Installing system-wide to $INSTALL_DIR"
else
    INSTALL_DIR="$HOME/.local/bin"
    echo "Installing to user directory: $INSTALL_DIR"
    mkdir -p "$INSTALL_DIR"
fi

# Copy executable
cp ccsh "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/ccsh"

echo "ccsh installed successfully to $INSTALL_DIR/ccsh"
echo ""
echo "To use ccsh as your default shell:"
echo "1. Add to /etc/shells (requires sudo):"
echo "   echo '$INSTALL_DIR/ccsh' | sudo tee -a /etc/shells"
echo "2. Change your shell:"
echo "   chsh -s $INSTALL_DIR/ccsh"
echo "3. Log out and log back in"
echo ""
echo "To run ccsh directly:"
echo "   $INSTALL_DIR/ccsh"
EOF

    # Create uninstallation script
    cat > "$DIST_DIR/uninstall.sh" << 'EOF'
#!/bin/bash
# uninstall.sh - Uninstallation script for ccsh

set -e

echo "Uninstalling ccsh..."

# Check if running as root for system-wide installation
if [ "$EUID" -eq 0 ]; then
    INSTALL_DIR="/usr/local/bin"
else
    INSTALL_DIR="$HOME/.local/bin"
fi

if [ -f "$INSTALL_DIR/ccsh" ]; then
    rm -f "$INSTALL_DIR/ccsh"
    echo "ccsh removed from $INSTALL_DIR"
else
    echo "ccsh not found in $INSTALL_DIR"
fi

echo "Uninstallation complete"
EOF

    # Create package information
    cat > "$DIST_DIR/PACKAGE_INFO" << EOF
Package: $NAME
Version: $VERSION
Description: $DESCRIPTION
OS: $OS
Architecture: $ARCH
Build Date: $(date)
Executable Size: $EXEC_SIZE

Files included:
- ccsh (main executable)
- README.md (documentation)
- install.sh (installation script)
- uninstall.sh (uninstallation script)
- test.sh (test script, if available)

Installation:
1. Extract the package
2. Run: ./install.sh
3. Follow the instructions provided

Usage:
- Run directly: ./ccsh
- Set as default shell: Follow install.sh instructions
EOF

    # Make scripts executable
    chmod +x "$DIST_DIR/install.sh"
    chmod +x "$DIST_DIR/uninstall.sh"
    chmod +x "$DIST_DIR/ccsh"
    
    # Create compressed package
    cd dist
    PACKAGE_NAME="$NAME-$VERSION-$OS-$ARCH.tar.gz"
    tar -czf "$PACKAGE_NAME" "$NAME-$VERSION-$OS-$ARCH/"
    
    # Create checksum for verification
    sha256sum "$PACKAGE_NAME" > "$PACKAGE_NAME.sha256"
    
    echo -e "${GREEN}[SUCCESS] Package created: $PACKAGE_NAME${NC}"
    echo -e "${BLUE}[INFO] Package size: $(ls -lh "$PACKAGE_NAME" | awk '{print $5}')${NC}"
    echo -e "${BLUE}[INFO] Checksum: $PACKAGE_NAME.sha256${NC}"
    
    cd ..
}

# Show help
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -d, --deps          Install dependencies"
    echo "  -c, --check         Check dependencies"
    echo "  -b, --build TYPE    Build project (TYPE: normal, static, debug)"
    echo "  -t, --test          Test the build"
    echo "  -p, --package       Create distribution package"
    echo "  -a, --all           Install deps, build, test, and package"
    echo ""
    echo "Examples:"
    echo "  $0 -d              # Install dependencies"
    echo "  $0 -b normal       # Build normally"
    echo "  $0 -b static       # Build static binary"
    echo "  $0 -p              # Create distribution package"
    echo "  $0 -a              # Full build process with packaging"
}

# Main script
main() {
    local install_deps=false
    local check_deps=false
    local build_type="normal"
    local test_build_flag=false
    local package_flag=false
    local all_flag=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -d|--deps)
                install_deps=true
                shift
                ;;
            -c|--check)
                check_deps=true
                shift
                ;;
            -b|--build)
                build_type="$2"
                shift 2
                ;;
            -t|--test)
                test_build_flag=true
                shift
                ;;
            -p|--package)
                package_flag=true
                shift
                ;;
            -a|--all)
                all_flag=true
                shift
                ;;
            *)
                echo -e "${RED}[ERROR] Unknown option: $1${NC}"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Handle --all flag
    if [ "$all_flag" = true ]; then
        install_deps=true
        build_type="normal"
        test_build_flag=true
        package_flag=true
    fi
    
    # Install dependencies if requested
    if [ "$install_deps" = true ]; then
        install_dependencies
    fi
    
    # Check dependencies if requested
    if [ "$check_deps" = true ]; then
        check_readline
    fi
    
    # Build if requested
    if [ "$build_type" != "none" ]; then
        build_project "$build_type"
    fi
    
    # Test if requested
    if [ "$test_build_flag" = true ]; then
        test_build
    fi
    
    # Create package if requested
    if [ "$package_flag" = true ]; then
        create_package
    fi
    
    echo -e "${GREEN}[SUCCESS] All operations completed successfully${NC}"
}

# Run main function with all arguments
main "$@" 