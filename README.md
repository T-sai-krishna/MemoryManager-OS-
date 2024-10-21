# Memory Management System

A comprehensive implementation of a virtual memory management system that simulates paging and process management for operating systems.

## Features

### Memory Management
- Dynamic memory allocation with paging
- Virtual memory support with swap operations
- Page table management per process
- LRU (Least Recently Used) page replacement algorithm
- Memory protection and address bounds checking
- Physical and virtual address translation

### Process Management
- Process creation and termination
- Process state tracking
- Memory swapping between main and virtual memory
- Process execution simulation

### Command Support
- Process loading and execution
- Memory operation simulation (add, subtract, load, print)
- Memory state inspection
- Page table examination

## System Commands

| Command | Description | Example |
|---------|-------------|---------|
| `load <filename>` | Load program into memory | `load program1.txt` |
| `run <pid>` | Execute process | `run 1` |
| `kill <pid>` | Terminate process | `kill 1` |
| `listpr` | List all active processes | `listpr` |
| `pte <pid> <file>` | Print page table entries | `pte 1 pagetable.txt` |
| `pteall <file>` | Print all page tables | `pteall alltables.txt` |
| `swapout <pid>` | Move process to virtual memory | `swapout 1` |
| `swapin <pid>` | Move process to main memory | `swapin 1` |
| `print <addr> <len>` | Display memory contents | `print 0 100` |

## Program Instructions

Programs loaded into the system can use these instructions:
- `add x, y, z`: Add values from addresses x and y, store in z
- `sub x, y, z`: Subtract value at y from x, store in z
- `print x`: Display value at address x
- `load a, y`: Store value a at address y

## File Formats

### Executable Files
```
<size_in_KB>
<instruction_1>
<instruction_2>
...
```

Example:
```
4
load 42, 100
add 100, 200, 300
print 300
```

### Input Command File
```
load program1.txt
run 1
print 0 100
kill 1
exit
```

## Building the System

### Prerequisites
- C++11 compatible compiler
- Make (optional)
- Standard C++ libraries

### Compilation
```bash
# Using g++
g++ -std=c++11 -o memmanager main.cpp MemoryManager.cpp -Wall
## Usage

```bash
./memmanager -M <main_memory_size> -V <virtual_memory_size> -P <page_size> -i <input_file> -o <output_file>
```

Parameters:
- `M`: Main memory size in KB
- `V`: Virtual memory size in KB
- `P`: Page size in bytes
- `i`: Input command file
- `o`: Output file for results

Example:
```bash
./memmanager -M 1024 -V 2048 -P 512 -i commands.txt -o output.txt
```
