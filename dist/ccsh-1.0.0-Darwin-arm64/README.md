# ccsh — Compact C Shell

A small yet flexible Unix‑like shell in C featuring:
- Interactive prompt with colors
- Command execution, pipes, I/O redirection
- Background jobs, aliasing, globbing
- Plugin sourcing via `source`
- Command history & arrow keys via readline
- Fully working on Linux/macOS

## Installation

```sh
git clone https://github.com/yourname/ccsh.git
cd ccsh
make
```

## For Static Binary
```sh
make static
```

## Distribution

To create a distributable package:

```sh
./build-and-package.sh
```

This creates a platform-specific package in the `dist/` directory with installation scripts.

## Usage
```sh
./ccsh
```

### Set as default shell
```sh
cp ccsh ~/.ccsh
echo "$HOME/.ccsh" | sudo tee -a /etc/shells
chsh -s "$HOME/.ccsh"
```

### Note
Then, log out/in to apply.

## .ccshrc Configuration

### Create ~/.ccshrc with:
```sh
alias ll="ls -lah"
export CCSH_PROMPT="my-shell> "
echo "Welcome, $(whoami)!"
```

### Note
Supports plugin files via source plugin.ccsh.


## Built‑ins
- cd, pwd, exit, jobs, fg, help, source
- Aliases via .ccshrc (alias name="value")
- Plugins via source <file>

## Testing
```sh
make test
```

## Documentation

- [Shell Introduction](docs/Shell%20Introduction.md) - Overview of shell concepts and features
- [Makefile Introduction](docs/Makefile%20Introduction.md) - How Makefiles work and C program distribution
- [Coding Dive](docs/Coding%20Dive.md) - Technical implementation details
