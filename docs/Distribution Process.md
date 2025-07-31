# Distribution Process: How the dist/ Folder is Generated

This document explains the step-by-step process of how the `dist/` folder is created and what happens during the distribution packaging.

## 🔧 The Build and Package Script

The `build-and-package.sh` script automates the entire distribution process. To create a distribution package, run:

```bash
# Full automated process (install deps + build + test + package)
./build-and-package.sh -a

# Or build and package separately
./build-and-package.sh -b normal -p
```

### **Step 1: Environment Setup**
```bash
# Configuration
VERSION="1.0.0"
NAME="ccsh"
DESCRIPTION="Compact C Shell - A small yet flexible Unix-like shell in C"

# Platform Detection
OS=$(uname -s)      # Darwin (macOS) or Linux
ARCH=$(uname -m)    # arm64, x86_64, etc.
```

### **Step 2: Build Process**
```bash
# Clean previous builds
make clean

# Build based on platform
if [ "$OS" = "Darwin" ]; then
    make                    # Standard linking on macOS
else
    make static            # Static linking on Linux
fi
```

**What happens during build:**
1. **Compilation**: `gcc -Wall -Wextra -O2 main.c -o ccsh -lreadline`
2. **Optimization**: `-O2` flag for performance
3. **Warnings**: `-Wall -Wextra` for code quality
4. **Linking**: Links against readline library

### **Step 3: Package Structure Creation**
```bash
# Create distribution directory
DIST_DIR="dist/$NAME-$VERSION-$OS-$ARCH"
mkdir -p "$DIST_DIR"

# Copy files
cp ccsh "$DIST_DIR/"
cp README.md "$DIST_DIR/"
cp test.sh "$DIST_DIR/"
```

**Files included in package:**
- **ccsh**: Compiled executable
- **README.md**: Project documentation
- **test.sh**: Test script

### **Step 4: Script Generation**

#### **Installation Script**
```bash
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
```

#### **Uninstallation Script**
```bash
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
```

### **Step 5: Package Information**
```bash
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
```

### **Step 6: Compression and Checksum**
```bash
# Create compressed package
cd dist
PACKAGE_NAME="$NAME-$VERSION-$OS-$ARCH.tar.gz"
tar -czf "$PACKAGE_NAME" "$NAME-$VERSION-$OS-$ARCH/"

# Create checksum for verification
sha256sum "$PACKAGE_NAME" > "$PACKAGE_NAME.sha256"
```

## 📁 Final Directory Structure

After running `./build-and-package.sh -a` or `./build-and-package.sh -p`, you get:

```
ccsh-shell/
├── main.c                    # Source code
├── Makefile                  # Build configuration
├── build-and-package.sh      # Distribution script
├── README.md                 # Project documentation
├── .gitignore               # Git ignore rules
├── dist/                     # Generated distribution folder
│   ├── ccsh-1.0.0-Darwin-arm64/
│   │   ├── ccsh             # Compiled executable
│   │   ├── README.md        # Documentation
│   │   ├── install.sh       # Installation script
│   │   ├── uninstall.sh     # Uninstallation script
│   │   ├── test.sh          # Test script
│   │   ├── PACKAGE_INFO     # Package metadata
│   ├── ccsh-1.0.0-Darwin-arm64.tar.gz
│   └── ccsh-1.0.0-Darwin-arm64.tar.gz.sha256
└── docs/*                   # Source documentation
```

## 🔍 Platform-Specific Details

### **macOS (Darwin)**
- **OS Detection**: `uname -s` returns "Darwin"
- **Architecture**: `uname -m` returns "arm64" (M1/M2) or "x86_64" (Intel)
- **Build Method**: Standard linking (no static linking)
- **Package Name**: `ccsh-1.0.0-Darwin-arm64.tar.gz`

### **Linux**
- **OS Detection**: `uname -s` returns "Linux"
- **Architecture**: `uname -m` returns "x86_64" or "aarch64"
- **Build Method**: Static linking for self-contained binary
- **Package Name**: `ccsh-1.0.0-Linux-x86_64.tar.gz`

## 🚀 Automation Features

### **Error Handling**
```bash
set -e  # Exit on any error
```

### **Color Output**
```bash
# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
```

### **Progress Reporting**
```bash
print_status "Building $NAME version $VERSION..."
print_success "Build completed successfully (Size: $EXEC_SIZE)"
print_warning "Static linking not fully supported on macOS"
```

### **Validation**
```bash
# Check if build was successful
if [ ! -f "ccsh" ]; then
    print_error "Build failed - ccsh executable not found"
    exit 1
fi
```

## 📦 Package Contents

### **Executable**
- **ccsh**: Compiled shell binary
- **Permissions**: Executable (chmod +x)
- **Size**: ~36KB (macOS) to ~2MB (Linux static)

### **Documentation**
- **README.md**: Project overview and usage
- **PACKAGE_INFO**: Package metadata

### **Scripts**
- **install.sh**: Automated installation
- **uninstall.sh**: Clean removal
- **test.sh**: Basic functionality tests

### **Verification**
- **.tar.gz.sha256**: Checksum for integrity verification

## 🔧 Customization

### **Version Management**
```bash
VERSION="1.0.0"  # Change this for new releases
```

### **Package Name**
```bash
NAME="ccsh"      # Change this for different projects
```

### **Additional Files**
```bash
# Add more files to the package
cp LICENSE "$DIST_DIR/" 2>/dev/null || echo "No LICENSE found"
cp CHANGELOG.md "$DIST_DIR/" 2>/dev/null || echo "No CHANGELOG found"
```

### **Platform-Specific Builds**
```bash
# Cross-compilation example
CC=arm-linux-gnueabi-gcc make clean && make static
```

This automated process ensures consistent, professional distribution packages that work across different platforms and include all necessary components for easy installation and use. 