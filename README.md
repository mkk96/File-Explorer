# Terminal File Explorer

A keyboard-driven file explorer for the Linux terminal, written in C++ on top of raw POSIX system calls. It does not use `ncurses`, `system()` or shell utilities.

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Linux](https://img.shields.io/badge/platform-Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![POSIX](https://img.shields.io/badge/API-POSIX%20%2F%20termios-4E4E4E?style=flat-square)

## Overview

The explorer puts the terminal into non-canonical mode with `termios` and reads keystrokes directly. It draws the UI with ANSI escape sequences. Every filesystem operation (listing, copy, move, delete, search) is built on `opendir`/`readdir`, `stat`, `open`/`read`/`write`, `mkdir`, `rename`, `unlink` and `rmdir`.

The app has two modes:

- **Normal mode** (default): browse and navigate the filesystem.
- **Command mode** (press `:`): type commands to copy, move, rename, create, delete, jump to a path or search.

The directory you launch the program from is treated as the application root (home).

## Features

**Normal mode**
- Lists every entry in the current directory, including `.` and `..`, sorted by name.
- Each row shows permissions (`drwxr-xr-x` style), owner, group, size in bytes, last-modified time and name.
- Scrolls vertically when the listing is taller than the terminal. The height is read with `ioctl(TIOCGWINSZ)`.
- Opens directories in place. Files open in their default application via `xdg-open`.
- Back/forward history (two stacks), "up one level" and "home".

**Command mode**
- `copy` and `move` for files and directories, recursively. Copies keep the source permission bits.
- `rename`, `create_file`, `create_dir`, `delete_file`, `delete_dir` (deletes recursively).
- `goto` to jump to a directory.
- `search` to find a file or folder by name, recursively under the current directory.

## Tech stack

- **Language:** C++ (STL: `vector`, `stack`, `string`)
- **System APIs:** `termios`, `ioctl`, `dirent`, `sys/stat`, `fcntl`, `pwd`/`grp`, `fork`/`execl`
- **Platform:** Linux

## Project structure

```
File-Explorer/
├── main.cpp                # Entire application: UI, key handling, file operations
└── AOS_Assignment1.1.pdf   # Original assignment specification
```

## Getting started

### Prerequisites

- Linux
- `g++`: `sudo apt-get install g++`
- `xdg-open` (from `xdg-utils`) if you want to open files from the explorer

### Build and run

```bash
git clone https://github.com/mkk96/File-Explorer.git
cd File-Explorer
g++ main.cpp -o main
./main
```

Use a maximised terminal window. If a single entry wraps onto more than one line, cursor navigation gets out of sync.

## Usage

### Normal mode keys

| Key | Action |
| --- | --- |
| `↑` / `↓` | Move the cursor. The list scrolls at the top/bottom edge |
| `Enter` | Open the selected directory, or open the selected file with `xdg-open` |
| `←` | Go back to the previously visited directory |
| `→` | Go forward |
| `Backspace` | Go up one level |
| `h` | Go to the application root (the launch directory) |
| `:` | Switch to command mode |
| `q` | Quit |

### Command mode

Type a command on the bottom line and press `Enter`. `Esc` returns to normal mode.

```text
copy <source(s)> <destination_directory>
move <source(s)> <destination_directory>
rename <old_name> <new_name>
create_file <file_name> <destination_path>
create_dir <dir_name> <destination_path>
delete_file <file_path(s)>
delete_dir <dir_path(s)>
goto <directory_path>
search <file_or_directory_name>
```

Examples:

```text
copy notes.txt report.pdf ~/backup
move old_dir ~/archive
rename draft.txt final.txt
create_file todo.txt .
create_dir projects ~/
goto ~/projects
search main.cpp
```

**Path rules**
- Relative paths resolve against the current directory.
- Paths starting with `~` or `/` resolve against the application root (the launch directory), not the system root.
- `.` as a destination means the current directory.

`search` prints every match as a path relative to the current directory. Press `Backspace` to return.

## Known limitations

- `copy` and `move` treat an argument as a file if its name contains a `.`, and as a directory otherwise.
- Commands are split on spaces, so names that contain spaces are not supported in command mode.
- Pressing `q` while typing in command mode exits the application, so you can't type names containing a `q` there.
- Only `Backspace` is supported for editing a command. There is no cursor movement within the line.

## Context

Built for Assignment 1 of the Advanced Operating Systems course at IIIT Hyderabad (M.Tech, 2021). `AOS_Assignment1.1.pdf` is the original specification.

---

Made by [Mukul Kumar](https://mkk96.github.io) · [GitHub](https://github.com/mkk96) · [LinkedIn](https://www.linkedin.com/in/mukul-kumar-60322616b/)
