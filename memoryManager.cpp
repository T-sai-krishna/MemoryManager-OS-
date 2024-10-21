#include "MemoryManager.h"
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iostream>

// Global variables initialization
bool isActive = true;
int mainMemSize, virtMemSize, pageSize, mainPages, virtPages;
int processIdCounter = 1;
std::vector<bool> mainMemoryMap;
std::vector<bool> virtualMemoryMap;
int availableMainPages;
int availableVirtualPages;
std::unordered_map<int, ProcessInfo*> processes;
std::vector<std::pair<bool, int>> mainMemoryData;
std::vector<std::pair<bool, int>> virtualMemoryData;
std::vector<int> recentlyUsed;

int findFirstAvailablePage(std::vector<bool>& memoryMap) {
    for(size_t i = 0; i < memoryMap.size(); i++) {
        if(memoryMap[i]) {
            memoryMap[i] = false;
            return i;
        }
    }
    return -1;
}

int calculatePhysicalAddress(int virtualAddr, const std::vector<int>& pageTable) {
    int virtualPage = virtualAddr / pageSize;
    int offset = virtualAddr % pageSize;
    int physicalPage = pageTable[virtualPage];
    return physicalPage * pageSize + offset;
}

void moveToVirtualMemory(int pid, std::ofstream& output) {
    auto process = processes.find(pid);
    if(process == processes.end()) {
        output << "Error: Process " << pid << " not found\n";
        return;
    }

    if(!process->second->isInMainMemory) {
        output << "Error: Process " << pid << " already in virtual memory\n";
        return;
    }

    int pageCount = process->second->pageTable.size();
    if(availableVirtualPages >= pageCount) {
        std::vector<int> oldPages = process->second->pageTable;
        process->second->pageTable.clear();

        // Allocate new pages in virtual memory
        for(int oldPage : oldPages) {
            mainMemoryMap[oldPage] = true;
            int newPage = findFirstAvailablePage(virtualMemoryMap);
            process->second->pageTable.push_back(newPage);
        }

        // Transfer data
        for(int i = 0; i < process->second->size; i++) {
            int oldAddr = calculatePhysicalAddress(i, oldPages);
            int newAddr = calculatePhysicalAddress(i, process->second->pageTable);
            virtualMemoryData[newAddr] = mainMemoryData[oldAddr];
            mainMemoryData[oldAddr] = {false, 0};
        }

        process->second->isInMainMemory = false;
        availableMainPages += pageCount;
        availableVirtualPages -= pageCount;

        // Update LRU
        auto it = std::find(recentlyUsed.begin(), recentlyUsed.end(), pid);
        if(it != recentlyUsed.end()) {
            recentlyUsed.erase(it);
        }

        output << "Process " << pid << " successfully moved to virtual memory\n";
    } else {
        output << "Error: Insufficient virtual memory space\n";
    }
}

void moveToMainMemory(int pid, std::ofstream& output) {
    auto process = processes.find(pid);
    if(process == processes.end()) {
        output << "Error: Process " << pid << " not found\n";
        return;
    }

    if(process->second->isInMainMemory) {
        output << "Error: Process " << pid << " already in main memory\n";
        return;
    }

    int pageCount = process->second->pageTable.size();
    if(availableMainPages >= pageCount) {
        std::vector<int> oldPages = process->second->pageTable;
        process->second->pageTable.clear();

        // Allocate new pages in main memory
        for(int oldPage : oldPages) {
            virtualMemoryMap[oldPage] = true;
            int newPage = findFirstAvailablePage(mainMemoryMap);
            process->second->pageTable.push_back(newPage);
        }

        // Transfer data
        for(int i = 0; i < process->second->size; i++) {
            int oldAddr = calculatePhysicalAddress(i, oldPages);
            int newAddr = calculatePhysicalAddress(i, process->second->pageTable);
            mainMemoryData[newAddr] = virtualMemoryData[oldAddr];
            virtualMemoryData[oldAddr] = {false, 0};
        }

        process->second->isInMainMemory = true;
        availableMainPages -= pageCount;
        availableVirtualPages += pageCount;

        // Update LRU
        recentlyUsed.push_back(pid);

        output << "Process " << pid << " successfully moved to main memory\n";
    } else {
        // Try LRU replacement
        int requiredPages = pageCount - availableMainPages;
        std::vector<int> processesToSwap;
        int pagesFreed = 0;

        for(auto it = recentlyUsed.begin(); it != recentlyUsed.end() && pagesFreed < requiredPages; ++it) {
            auto proc = processes.find(*it);
            if(proc != processes.end() && proc->second->isInMainMemory) {
                processesToSwap.push_back(*it);
                pagesFreed += proc->second->pageTable.size();
            }
        }

        if(pagesFreed >= requiredPages) {
            for(int pidToSwap : processesToSwap) {
                moveToVirtualMemory(pidToSwap, output);
            }
            moveToMainMemory(pid, output);
        } else {
            output << "Error: Insufficient main memory space even after LRU replacement\n";
        }
    }
}

void loadProcess(const std::string& filename, std::ofstream& output) {
    std::ifstream input(filename);
    if(!input.is_open()) {
        output << "Error: Could not open file " << filename << "\n";
        return;
    }

    int size;
    input >> size;
    int requiredPages = (size * 1024 + pageSize - 1) / pageSize;

    ProcessInfo* newProcess = new ProcessInfo();
    newProcess->pid = processIdCounter++;
    newProcess->filename = filename;
    newProcess->size = size * 1024;

    if(availableMainPages >= requiredPages) {
        newProcess->isInMainMemory = true;
        for(int i = 0; i < requiredPages; i++) {
            int page = findFirstAvailablePage(mainMemoryMap);
            newProcess->pageTable.push_back(page);
        }
        availableMainPages -= requiredPages;
        recentlyUsed.push_back(newProcess->pid);
        output << "Process loaded into main memory with PID " << newProcess->pid << "\n";
    } else if(availableVirtualPages >= requiredPages) {
        newProcess->isInMainMemory = false;
        for(int i = 0; i < requiredPages; i++) {
            int page = findFirstAvailablePage(virtualMemoryMap);
            newProcess->pageTable.push_back(page);
        }
        availableVirtualPages -= requiredPages;
        output << "Process loaded into virtual memory with PID " << newProcess->pid << "\n";
    } else {
        delete newProcess;
        output << "Error: Insufficient memory to load process\n";
        return;
    }

    processes[newProcess->pid] = newProcess;
    input.close();
}

void runProcess(int pid, std::ofstream& output) {
    auto process = processes.find(pid);
    if(process == processes.end()) {
        output << "Error: Process " << pid << " not found\n";
        return;
    }

    if(!process->second->isInMainMemory) {
        moveToMainMemory(pid, output);
    } else {
        auto it = std::find(recentlyUsed.begin(), recentlyUsed.end(), pid);
        if(it != recentlyUsed.end()) {
            recentlyUsed.erase(it);
        }
        recentlyUsed.push_back(pid);
    }

    std::ifstream input(process->second->filename);
    std::string line;
    int dummy; // Skip size line
    input >> dummy;

    while(std::getline(input, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if(command == "add") {
            int addr1, addr2, addr3;
            char comma;
            iss >> addr1 >> comma >> addr2 >> comma >> addr3;
            
            if(addr1 >= process->second->size || addr2 >= process->second->size || 
               addr3 >= process->second->size) {
                output << "Error: Memory access violation\n";
                break;
            }

            int phyAddr1 = calculatePhysicalAddress(addr1, process->second->pageTable);
            int phyAddr2 = calculatePhysicalAddress(addr2, process->second->pageTable);
            int phyAddr3 = calculatePhysicalAddress(addr3, process->second->pageTable);

            int val1 = mainMemoryData[phyAddr1].second;
            int val2 = mainMemoryData[phyAddr2].second;
            mainMemoryData[phyAddr3] = {true, val1 + val2};

            output << "Added " << val1 << " and " << val2 << ", stored " << (val1 + val2) 
                  << " at address " << addr3 << "\n";
        }
        else if(command == "sub") {
            // Similar to add but with subtraction
        }
        else if(command == "print") {
            int addr;
            iss >> addr;
            
            if(addr >= process->second->size) {
                output << "Error: Memory access violation\n";
                break;
            }

            int phyAddr = calculatePhysicalAddress(addr, process->second->pageTable);
            output << "Value at address " << addr << ": " << mainMemoryData[phyAddr].second << "\n";
        }
    }

    input.close();
}

void killProcess(int pid, std::ofstream& output) {
    auto process = processes.find(pid);
    if(process == processes.end()) {
        output << "Error: Process " << pid << " not found\n";
        return;
    }

    if(process->second->isInMainMemory) {
        for(int page : process->second->pageTable) {
            mainMemoryMap[page] = true;
            int start = page * pageSize;
            int end = start + pageSize;
            for(int i = start; i < end; i++) {
                mainMemoryData[i] = {false, 0};
            }
        }
        availableMainPages += process->second->pageTable.size();
        
        auto it = std::find(recentlyUsed.begin(), recentlyUsed.end(), pid);
        if(it != recentlyUsed.end()) {
            recentlyUsed.erase(it);
        }
    } else {
        for(int page : process->second->pageTable) {
            virtualMemoryMap[page] = true;
            int start = page * pageSize;
            int end = start + pageSize;
            for(int i = start; i < end; i++) {
                virtualMemoryData[i] = {false, 0};
            }
        }
        availableVirtualPages += process->second->pageTable.size();
    }

    delete process->second;
    processes.erase(process);
    output << "Process " << pid << " terminated\n";
}

void listProcesses(std::ofstream& output) {
    output << "Main Memory Processes: ";
    for(const auto& process : processes) {
        if(process.second->isInMainMemory) {
            output << process.first << " ";
        }
    }
    output << "\nVirtual Memory Processes: ";
    for(const auto& process : processes) {
        if(!process.second->isInMainMemory) {
            output << process.first << " ";
        }
    }
    output << "\n";
}

void printPageTable(int pid, const std::string& filename, std::ofstream& output) {
    auto process = processes.find(pid);
    if(process == processes.end()) {
        output << "Error: Process " << pid << " not found\n";
        return;
    }

    std::ofstream pteFile(filename, std::ios::app);
    time_t now = time(nullptr);
    auto timeinfo = localtime(&now);
    
    pteFile << "Page Table for Process " << pid << " at " 
            << asctime(timeinfo);
    for(size_t i = 0; i < process->second->pageTable.size(); i++) {
        pteFile << "Virtual Page " << i << " -> Physical Page " 
                << process->second->pageTable[i] << "\n";
    }
    pteFile.close();
}

void printMemory(int location, int length, std::ofstream& output) {
    if(location + length > mainMemSize * 1024) {
        output << "Error: Memory access out of bounds\n";
        return;
    }

    output << "Memory contents from " << location << " to " << (location + length - 1) << ":\n";
    for(int i = location; i < location + length; i++) {
        output << "Address " << i << ": " 
              << (mainMemoryData[i].first ? mainMemoryData[i].second : 0) << "\n";
    }
}

void initializeMemory(int mainSize, int virtSize, int pgSize) {
    mainMemSize = mainSize;
    virtMemSize = virtSize;
    pageSize = pgSize;
    
    mainPages = (mainSize * 1024) / pageSize;
    virtPages = (virtSize * 1024) / pageSize;
    
    mainMemoryMap = std::vector<bool>(mainPages, true);
    virtualMemoryMap = std::vector<bool>(virtPages, true);
    
    availableMainPages = mainPages;
    availableVirtualPages = virtPages;
    
    mainMemoryData = std::vector<std::pair<bool, int>>((mainSize * 1024), {false, 0});
    virtualMemoryData = std::vector<std::pair<bool, int>>((virtSize * 1024), {false, 0});
}
