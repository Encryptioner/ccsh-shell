# Distribution Example

This document shows how to use the distribution package created by `build-and-package.sh`.

## Package Contents

The distribution package contains:

```
ccsh-1.0.0-Darwin-arm64/
├── ccsh                   # Main executable
├── install.sh             # Installation script
├── uninstall.sh           # Uninstallation script
├── test.sh                # Test script
├── PACKAGE_INFO           # Package information
```

## Installation Steps

### 1. Extract the Package

```bash
# Download and extract the package
tar -xzf ccsh-1.0.0-Darwin-arm64.tar.gz
cd ccsh-1.0.0-Darwin-arm64
```

### 2. Install ccsh

```bash
# Run the installation script
./install.sh
```

This will:
- Install ccsh to `/usr/local/bin/` (system-wide) or `~/.local/bin/` (user)
- Make the executable file executable
- Provide instructions for setting as default shell

### 3. Test the Installation

```bash
# Test the shell
ccsh
```

You should see the ccsh prompt:
```
ccsh> 
```

### 4. Set as Default Shell (Optional)

```bash
# Add to /etc/shells (requires sudo)
echo '/usr/local/bin/ccsh' | sudo tee -a /etc/shells

# Change your shell
chsh -s /usr/local/bin/ccsh

# Log out and log back in to apply changes
```

## Usage Examples

### Basic Commands

```bash
ccsh> ls -la
ccsh> pwd
ccsh> cd /tmp
ccsh> echo "Hello, ccsh!"
```

### Built-in Commands

```bash
ccsh> help          # Show built-in commands
ccsh> jobs          # Show background jobs
ccsh> fg            # Bring job to foreground
ccsh> exit          # Exit the shell
```

### Aliases and Configuration

Create `~/.ccshrc`:
```bash
# Add aliases
alias ll="ls -lah"
alias g="git"

# Set custom prompt
export CCSH_PROMPT="my-shell> "

# Welcome message
echo "Welcome to ccsh, $(whoami)!"
```

### Plugin Support

Create a plugin file `my-plugin.ccsh`:
```bash
# Custom functions
function greet() {
    echo "Hello, $1!"
}

# Custom aliases
alias weather="curl wttr.in"
```

Load the plugin:
```bash
ccsh> source my-plugin.ccsh
ccsh> greet "World"
Hello, World!
```

## Uninstallation

To remove ccsh:

```bash
# Run the uninstallation script
./uninstall.sh
```

Or manually:
```bash
# Remove the executable
sudo rm -f /usr/local/bin/ccsh

# Remove from /etc/shells if added
sudo sed -i '/ccsh/d' /etc/shells
```

## Verification

### Check Installation

```bash
# Verify executable exists
which ccsh

# Check file permissions
ls -la $(which ccsh)

# Test functionality
ccsh --help 2>/dev/null || echo "ccsh installed successfully"
```

### Verify Package Integrity

```bash
# Check SHA256 checksum
sha256sum -c ccsh-1.0.0-Darwin-arm64.tar.gz.sha256
```

## Troubleshooting

### Common Issues

1. **Permission Denied**
   ```bash
   chmod +x ccsh
   ```

2. **Library Not Found**
   ```bash
   # Install readline on Ubuntu/Debian
   sudo apt-get install libreadline-dev
   
   # Install readline on macOS
   brew install readline
   ```

3. **Shell Not Found**
   ```bash
   # Check if ccsh is in PATH
   echo $PATH | grep -q ccsh || echo "ccsh not in PATH"
   
   # Add to PATH if needed
   export PATH="/usr/local/bin:$PATH"
   ```

### Getting Help

- Check the documentation files included in the package
- Run `ccsh> help` for built-in commands
- Review the README.md for usage examples

## Platform-Specific Notes

### macOS
- Static linking is not fully supported
- Uses standard dynamic linking
- Requires readline library

### Linux
- Full static linking support
- Self-contained executable
- No external dependencies required

### Cross-Platform Distribution
- Different packages for each OS/architecture
- Platform-specific installation scripts
- Consistent user experience across platforms 