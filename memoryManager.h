#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>

struct ProcessInfo {
    int pid;
    std::string filename;
    int size;
    bool isInMainMemory;
    std::vector<int> pageTable;
};

// External declarations
extern bool isActive;
extern int mainMemSize, virtMemSize, pageSize, mainPages, virtPages;
extern int processIdCounter;
extern std::vector<bool> mainMemoryMap;
extern std::vector<bool> virtualMemoryMap;
extern int availableMainPages;
extern int availableVirtualPages;
extern std::unordered_map<int, ProcessInfo*> processes;
extern std::vector<std::pair<bool, int>> mainMemoryData;
extern std::vector<std::