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
