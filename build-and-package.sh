#!/bin/bash
# build-and-package.sh
# Script to build and package ccsh shell for distribution

set -e  # Exit on any error

# Configuration
VERSION="1.0.0"
NAME="ccsh"
DESCRIPTION="Compact C Shell - A small yet flexible Unix-like shell in C"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "main.c" ] || [ ! -f "Makefile" ]; then
    print_error "This script must be run from the ccsh project root directory"
    exit 1
fi

print_status "Building $NAME version $VERSION..."

# Clean previous builds
print_status "Cleaning previous builds..."
make clean

# Detect OS
OS=$(uname -s)
ARCH=$(uname -m)
print_status "Detected OS: $OS ($ARCH)"

# Build static binary
print_status "Building static binary..."
if [ "$OS" = "Darwin" ]; then
    print_warning "Static linking not fully supported on macOS, using standard linking"
    make
else
    make static
fi

# Check if build was successful
if [ ! -f "ccsh" ]; then
    print_error "Build failed - ccsh executable not found"
    exit 1
fi

# Get file size
EXEC_SIZE=$(du -h ccsh | cut -f1)
print_success "Build completed successfully (Size: $EXEC_SIZE)"

# Create distribution directory
DIST_DIR="dist/$NAME-$VERSION-$OS-$ARCH"
print_status "Creating distribution directory: $DIST_DIR"
mkdir -p "$DIST_DIR"

# Copy files
print_status "Copying files to distribution directory..."
cp ccsh "$DIST_DIR/"
cp README.md "$DIST_DIR/"
cp test.sh "$DIST_DIR/" 2>/dev/null || print_warning "test.sh not found"
cp docs/*.md "$DIST_DIR/" 2>/dev/null || print_warning "docs directory not found"

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

chmod +x "$DIST_DIR/install.sh"

# Create uninstall script
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

chmod +x "$DIST_DIR/uninstall.sh"

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
- Documentation files (if available)

Installation:
1. Extract the package
2. Run: ./install.sh
3. Follow the instructions provided

Usage:
- Run directly: ./ccsh
- Set as default shell: Follow install.sh instructions
EOF

# Create compressed package
cd dist
PACKAGE_NAME="$NAME-$VERSION-$OS-$ARCH.tar.gz"
tar -czf "$PACKAGE_NAME" "$NAME-$VERSION-$OS-$ARCH/"

# Get package size
PACKAGE_SIZE=$(du -h "$PACKAGE_NAME" | cut -f1)

print_success "Package created successfully!"
print_status "Package: $PACKAGE_NAME"
print_status "Size: $PACKAGE_SIZE"
print_status "Location: dist/$PACKAGE_NAME"

# Create checksum
print_status "Creating checksum..."
sha256sum "$PACKAGE_NAME" > "$PACKAGE_NAME.sha256"

# Show package contents
print_status "Package contents:"
tar -tzf "$PACKAGE_NAME" | sed 's/^/  /'

echo ""
print_success "Distribution package ready!"
print_status "You can now distribute: dist/$PACKAGE_NAME"
print_status "Include both the .tar.gz and .sha256 files for verification" 