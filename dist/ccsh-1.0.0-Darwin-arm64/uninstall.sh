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
