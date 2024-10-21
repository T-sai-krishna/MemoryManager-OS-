#include <bits/stdc++.h>
using namespace std;
#include <fstream>
typedef long long ll;
int P, M, V;
class Instruction
{
public:
    string operation;
    int a;
    int b;
    int c;
};
class Process
{
public:
    int pid;
    int size;
    bool inmm;
    int prior;
    vector<pair<int, int>> pt;
    vector<Instruction *> instructions;
};
void cleardata(Process *dup, vector<int> &mainMemory, vector<int> &virtualMemory, vector<int> &statusOfMMP, vector<int> &statusOfVMP)
{
    if (dup->inmm)
    {
        for (int i = 0; i < dup->pt.size(); i++)
        {
            int add = (dup->pt)[i].second * P;
            for (int j = 0; j < M; j++)
            {
                mainMemory[add + j] = 0;
            }
            statusOfMMP[add] = 0;
        }
    }
    else
    {
        for (int i = 0; i < dup->pt.size(); i++)
        {
            int add = (dup->pt)[i].second * P;
            for (int j = 0; j < V; j++)
            {
                mainMemory[add + j] = 0;
            }
            statusOfVMP[add] = 0;
        }
    }
}
void bringToMM(Process *proc, vector<int> &mainMemory, vector<int> &virtualMemory, vector<int> &statusOfMMP, vector<int> &statusOfVMP)
{
    // vector<int> emptyPages;
    int count = 0;
    for (int i = 0; i < M / P && count < ((proc->size) / P); i++)
    {
        if (statusOfMMP[i] == 0)
        {
            statusOfMMP[i] = proc->pid;
            statusOfVMP[(proc->pt)[count].second] = 0;
            for (int j = 0; j < P; j++)
            {
                mainMemory[j + i * P] = virtualMemory[j + (proc->pt)[count].second * P];
                virtualMemory[j + (proc->pt)[count].second * P] = 0;
            }
            (proc->pt)[count] = make_pair(count, i);
            count++;
        }
    }
    proc->inmm = true;
}
void bringToVM(Process *proc, vector<int> &mainMemory, vector<int> &virtualMemory, vector<int> &statusOfMMP, vector<int> &statusOfVMP)
{
    int count = 0;
    // outputFile << proc->pid << "%%%" << endl;
    // for (int i = 0; i < proc->pt.size();i++)
    // {
    //     outputFile << proc->pt[i].second << " ";
    // }
    // outputFile << endl;
    for (int i = 0; i < V / P && count < ((proc->size) / P); i++)
    {
        if (statusOfVMP[i] == 0)
        {
            // outputFile << "*****" << i << " ";
            statusOfVMP[i] = proc->pid;
            statusOfMMP[(proc->pt)[count].second] = 0;
            for (int j = 0; j < P; j++)
            {
                //     if (j + ((proc->pt)[count].second) * P == 1001)
                //     {
                //         outputFile << "+++++++" << (proc->pt)[count].second << " j: " << j << endl;
                //     }
                virtualMemory[j + i * P] = mainMemory[j + ((proc->pt)[count].second) * P];
                // outputFile << j + i * P << " ";
                mainMemory[j + ((proc->pt)[count].second) * P] = 0;

                // mainMemory[j + i * P] = virtualMemory[j + (proc->pt)[count].second];
                // virtualMemory[j + (proc->pt)[count].second] = 0;
                // (proc->pt)[count] = make_pair(count, i);
            }
            (proc->pt)[count] = make_pair(count, i);
            count++;
        }
    }
    proc->inmm = false;
}
int main(int argc, char *argv[])
{
    // outputFile << "HI" << endl;
    // outputFile << argc << endl;
    // for (int i = 0; i < argc;i++)
    // {
    //     outputFile << argv[i] << endl;
    // }
    time_t now = time(0);
    char *dt = ctime(&now);

    M = stoi(argv[2]);
    V = stoi(argv[4]);
    P = stoi(argv[6]);
    M = M * 1024;
    V = V * 1024;
    vector<int> mainMemory(M, 0);
    vector<int> virtualMemory(V, 0);
    int nopM = M / P, nopV = V / P;
    // outputFile << nopM << " " << nopV << endl;
    vector<int> statusOfMMP(nopM, 0);
    vector<int> statusOfVMP(nopV, 0);
    int pidCounter = 1, runCounter = 1;
    ifstream inputFile(argv[8]);
    // outputFile << !inputFile << endl;
    ofstream outputFile(argv[10]);
    string command = "";
    // vector<Process *> processes;
    map<int, Process *> processes;
    while (getline(inputFile, command))
    {
        // outputFile << command<<endl;
        stringstream s(command);
        string word = "";
        getline(s, word, ' ');
        // outputFile << "HI"<<" "<<word<<" "<<command<<endl;
        if (word == "load")
        {

            while (getline(s, word, ' '))
            {
                // outputFile << word << ",";
                Process *dup = new Process();
                ifstream file(word);
                if (!file)
                {
                    outputFile << word << " could not be loaded - file does not exist" << endl;
                }
                else
                {
                    string lineInFile = "";
                    getline(file, lineInFile);
                    int si = 1024 * stoi(lineInFile);
                    dup->size = si;
                    dup->pid = pidCounter;
                    dup->prior = 0;
                    int nopreq = si / P;
                    // outputFile << nopreq << " ";
                    int nofpInMM = 0, nofpInVM = 0;
                    for (int i = 0; i < nopM; i++)
                    {
                        if (statusOfMMP[i] == 0)
                            nofpInMM++;
                    }
                    for (int i = 0; i < nopV; i++)
                    {
                        if (statusOfVMP[i] == 0)
                            nofpInVM++;
                    }
                    if (nofpInMM >= nopreq)
                    {
                        outputFile << word << " is loaded in main memory and is assigned process id " << pidCounter << endl;
                        int count = 0;
                        dup->inmm = true;
                        for (int i = 0; i < nopM && count < nopreq; i++)
                        {
                            if (statusOfMMP[i] == 0)
                            {
                                (dup->pt).push_back(make_pair(count, i));
                                // outputFile << i << " ";
                                statusOfMMP[i] = pidCounter;
                                count++;
                            }
                            // outputFile << statusOfMMP[i] << " ";
                        }
                        // outputFile << endl;
                    }
                    else if (nofpInVM >= nopreq)
                    {
                        outputFile << word << " is loaded in virtual memory and is assigned process id " << pidCounter << endl;
                        int count = 0;
                        dup->inmm = false;
                        for (int i = 0; i < nopV && count < nopreq; i++)
                        {
                            if (statusOfVMP[i] == 0)
                            {
                                (dup->pt).push_back(make_pair(count, i));
                                statusOfVMP[i] = pidCounter;
                                count++;
                            }
                        }
                    }
                    else
                    {
                        outputFile << word << " could not be loaded - memory is full" << endl;
                        continue;
                    }

                    while (getline(file, lineInFile))
                    {
                        stringstream ss(lineInFile);
                        string word2 = "";
                        Instruction *dup2 = new Instruction();
                        getline(ss, word2, ' ');
                        dup2->operation = word2;
                        if (word2 == "add" || word2 == "sub")
                        {
                            getline(ss, word2, ',');
                            dup2->a = stoi(word2);
                            // outputFile << stoi(word2) << ",";
                            getline(ss, word2, ',');
                            dup2->b = stoi(word2);
                            // outputFile << stoi(word2) << ",";
                            getline(ss, word2, ',');
                            dup2->c = stoi(word2);
                            // outputFile << stoi(word2) << ",";
                        }
                        else if (word2 == "print")
                        {
                            getline(ss, word2, ',');
                            dup2->a = stoi(word2);
                        }
                        else if (word2 == "load")
                        {
                            getline(ss, word2, ',');
                            dup2->a = stoi(word2);
                            getline(ss, word2, ',');
                            dup2->b = stoi(word2);
                        }
                        dup->instructions.push_back(dup2);
                    }
                }
                processes[pidCounter++] = dup;
                // make pagetable
            }
        }
        else if (word == "run")
        {
            getline(s, word, ' ');
            int pidofp = stoi(word);
            if (processes.find(pidofp) == processes.end())
            {
                outputFile << "pid not found" << endl;
                continue;
            }
            Process *cprocess = processes[pidofp];
            int nopreq = cprocess->size;
            nopreq /= V;
            if (!cprocess->inmm) // process is in virtual memory
            {
                int nofpInMM = 0, nofpInVM = 0;
                for (int i = 0; i < nopM; i++)
                {
                    if (statusOfMMP[i] == 0)
                        nofpInMM++;
                }
                for (int i = 0; i < nopV; i++)
                {
                    if (statusOfVMP[i] == 0)
                        nofpInVM++;
                }
                if (nofpInMM >= nopreq) // direct swapin
                {
                    bringToMM(cprocess, mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
                }
                else // swap out things and goon
                {
                    set<int> s1;
                    for (int i = 0; i < nopM; i++)
                    {
                        if (statusOfMMP[i] != 0)
                            s1.insert(statusOfMMP[i]);
                    }
                    vector<pair<int, int>> priorityvalues;
                    for (auto x : s1)
                    {
                        priorityvalues.push_back(make_pair(processes[x]->prior, processes[x]->pid));
                    }
                    sort(priorityvalues.begin(), priorityvalues.end());
                    int nopshouldrem = 0;
                    int nopremoving = 0;
                    int availp = nofpInMM;
                    for (int i = 0; i < priorityvalues.size(); i++)
                    {
                        availp = availp + (processes[priorityvalues[i].second]->size) / P;
                        nopshouldrem++;
                        nopremoving += (processes[priorityvalues[i].second]->size) / P;
                        if (availp >= nopreq)
                        {
                            break;
                        }
                    }
                    if (availp < nopreq || nopremoving > nofpInVM)
                    {
                        outputFile << "ERROR: NO SPACE" << endl;
                        continue;
                    }
                    for (int i = 0; i < nopshouldrem; i++) // remove these processes and add to vadd
                    {
                        bringToVM(processes[priorityvalues[i].second], mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
                    }
                    // now bring our process to mm
                    bringToMM(cprocess, mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
                }
            }
            // run the process
            cprocess->prior = runCounter++;
            int nocom = cprocess->instructions.size();
            for (int i = 0; i < nocom; i++)
            {
                Instruction *cinstruction = cprocess->instructions[i];
                string ss = cinstruction->operation;
                if (ss == "load")
                {
                    int virtadd = cinstruction->b;
                    if (virtadd < 0 || virtadd >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd << " specified for process id " << pidofp << endl;
                        break;
                    }
                    int pfn = virtadd / V;
                    int offset = virtadd % V;
                    int phyadd = (cprocess->pt)[pfn].second * P + offset;
                    // converted vir to phy
                    mainMemory[phyadd] = cinstruction->a;
                    outputFile << "Command: load " << cinstruction->a << "," << virtadd << ";"
                               << " Result: Value of " << cinstruction->a << " is now stored in addr " << virtadd << endl;
                }
                else if (ss == "add")
                {
                    int virtadd1 = cinstruction->a;
                    int pfn1 = virtadd1 / V;
                    int offset1 = virtadd1 % V;
                    int phyadd1 = (cprocess->pt)[pfn1].second * P + offset1;
                    if (virtadd1 < 0 || virtadd1 >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd1 << " specified for process id " << pidofp << endl;
                        break;
                    }
                    int val1 = mainMemory[phyadd1];
                    int virtadd2 = cinstruction->b;
                    int pfn2 = virtadd2 / V;
                    int offset2 = virtadd2 % V;
                    int phyadd2 = (cprocess->pt)[pfn2].second * P + offset2;
                    if (virtadd2 < 0 || virtadd2 >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd2 << " specified for process id " << pidofp << endl;
                        break;
                    }
                    int val2 = mainMemory[phyadd2];
                    int virtadd3 = cinstruction->c;
                    int pfn3 = virtadd3 / V;
                    int offset3 = virtadd3 % V;
                    int phyadd3 = (cprocess->pt)[pfn3].second * P + offset3;
                    if (virtadd3 < 0 || virtadd3 >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd3 << " specified for process id " << pidofp << endl;
                        break;
                    }
                    mainMemory[phyadd3] = val1 + val2;
                    // int val3 = mainMemory[phyadd1];
                    outputFile << "Command: add " << virtadd1 << ", " << virtadd2 << ", " << virtadd3 << "; Result: Value in addr " << virtadd1 << " = " << val1 << ", addr " << virtadd2 << " = " << val2 << ", addr " << virtadd3 << " = " << val1 + val2 << endl;
                }
                else if (ss == "sub")
                {
                    int virtadd1 = cinstruction->a;
                    int pfn1 = virtadd1 / V;
                    int offset1 = virtadd1 % V;
                    int phyadd1 = (cprocess->pt)[pfn1].second * P + offset1;
                    if (virtadd1 < 0 || virtadd1 >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd1 << " specified for process id " << pidofp << endl;
                        break;
                    }
                    int val1 = mainMemory[phyadd1];
                    int virtadd2 = cinstruction->b;
                    int pfn2 = virtadd2 / V;
                    int offset2 = virtadd2 % V;
                    int phyadd2 = (cprocess->pt)[pfn2].second * P + offset2;
                    if (virtadd2 < 0 || virtadd2 >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd2 << " specified for process id " << pidofp << endl;
                        break;
                    }
                    int val2 = mainMemory[phyadd2];
                    int virtadd3 = cinstruction->c;
                    int pfn3 = virtadd3 / V;
                    int offset3 = virtadd3 % V;
                    int phyadd3 = (cprocess->pt)[pfn3].second * P + offset3;
                    if (virtadd3 < 0 || virtadd3 >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd3 << " specified for process id " << pidofp << endl;
                        break;
                    }
                    mainMemory[phyadd3] = val1 - val2;
                    // int val3 = mainMemory[phyadd1];
                    outputFile << "Command: add " << virtadd1 << ", " << virtadd2 << ", " << virtadd3 << "; Result: Value in addr " << virtadd1 << " = " << val1 << ", addr" << virtadd2 << " = " << val2 << ", addr " << virtadd3 << " = " << val1 - val2 << endl;
                }
                else if (ss == "print")
                {
                    int virtadd = cinstruction->a;
                    int pfn = virtadd / V;
                    int offset = virtadd % V;
                    int phyadd = (cprocess->pt)[pfn].second * P + offset;
                    if (virtadd < 0 || virtadd >= cprocess->size)
                    {
                        outputFile << "Invalid Memory Address " << virtadd << " specified for process id " << pidofp << endl;
                        break;
                    }
                    outputFile << "Command: print " << virtadd << "; Result: Value in addr " << virtadd << " = " << mainMemory[phyadd] << endl;
                }
            }
        }
        else if (word == "kill")
        {
            getline(s, word, ' ');
            int cpid = stoi(word);
            if (processes.find(cpid) == processes.end())
            {
                outputFile << "pid is invalid" << endl;
                continue;
            }
            cleardata(processes[cpid], mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
            processes.erase(cpid);
        }
        else if (word == "listpr")
        {
            set<int> s1, s2;
            for (int i = 0; i < nopM; i++)
            {
                if (statusOfMMP[i] != 0)
                    s1.insert(statusOfMMP[i]);
            }
            for (int i = 0; i < nopV; i++)
            {
                if (statusOfVMP[i] != 0)
                    s2.insert(statusOfVMP[i]);
            }
            outputFile << "Processes in Main Memory : " << endl;
            for (auto x : s1)
                outputFile << x << " ";
            outputFile << endl;
            outputFile << "Processes in Virtual Memory : " << endl;
            for (auto x : s2)
                outputFile << x << " ";
            outputFile << endl;
        }
        else if (word == "pte")
        {
            getline(s, word, ' ');
            int cpid = stoi(word);
            if (processes.find(cpid) == processes.end() || processes[cpid]->inmm != true)
            {
                outputFile << "pid is invalid" << endl;
                continue;
            }
            getline(s, word, ' ');
            ofstream outFile(word, ios::app);
            Process *dup = processes[cpid];
            outFile << "Date and time: " << dt << endl;
            outFile << "page table for process pid: " << cpid << endl;
            for (int i = 0; i < dup->pt.size(); i++)
            {
                outFile << "VPN: " << dup->pt[i].first << " PFN: " << dup->pt[i].second << endl;
            }
            outFile.close();
            // openedfiles.push_back()
        }
        else if (word == "pteall")
        {
            getline(s, word, ' ');
            ofstream outFile(word, ios::app);
            outFile << "printing pt's for all processes" << endl;
            set<int> s1;
            for (int i = 0; i < nopM; i++)
            {
                if (statusOfMMP[i] != 0)
                    s1.insert(statusOfMMP[i]);
            }
            int i = 1;
            outFile << "Date and time: " << dt << endl;
            for (auto x : s1)
            {
                Process *dup = processes[x];

                outFile << i << ") page table for process pid: " << x << endl;
                for (int i = 0; i < dup->pt.size(); i++)
                {
                    outFile << "VPN: " << dup->pt[i].first << " PFN: " << dup->pt[i].second << endl;
                }
                i++;
            }
            outFile.close();
        }
        else if (word == "swapout") // make inmm=false, notrun=0
        {
            getline(s, word, ' ');
            int cpid = stoi(word);
            if (processes.find(cpid) == processes.end() || (processes[cpid])->inmm == false)
            {
                outputFile << "PID INVALD" << endl;
                continue;
            }
            int nofpInVM = 0;
            Process *cproc = processes[cpid];
            for (int i = 0; i < nopV; i++)
            {
                if (statusOfVMP[i] == 0)
                    nofpInVM++;
            }
            if (nofpInVM < (cproc->size / P))
            {
                outputFile << "ERROR: NO SPACE TO SWAPOUT" << endl;
                continue;
            }
            bringToVM(cproc, mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
            // outputFile << "<<<<<<<" << mainMemory[5098] << endl;
        }
        else if (word == "swapin")
        {
            getline(s, word, ' ');
            int cpid = stoi(word);
            if (processes.find(cpid) == processes.end() || (processes[cpid])->inmm == true)
            {
                outputFile << "PID INVALD" << endl;
                continue;
            }
            // int nofpInMM = 0;
            Process *cprocess = processes[cpid];
            int nopreq = cprocess->size / P;
            // for (int i = 0; i < nopM; i++)
            // {
            //     if (statusOfMMP[i] == 0)
            //             nofpInMM++;
            // }
            int nofpInMM = 0, nofpInVM = 0;
            for (int i = 0; i < nopM; i++)
            {
                if (statusOfMMP[i] == 0)
                    nofpInMM++;
            }
            for (int i = 0; i < nopV; i++)
            {
                if (statusOfVMP[i] == 0)
                    nofpInVM++;
            }
            if (nofpInMM >= nopreq) // direct swapin
            {
                bringToMM(cprocess, mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
            }
            else // swap out things and goon
            {
                set<int> s1;
                for (int i = 0; i < nopM; i++)
                {
                    if (statusOfMMP[i] != 0)
                        s1.insert(statusOfMMP[i]);
                }
                vector<pair<int, int>> priorityvalues;
                for (auto x : s1)
                {
                    priorityvalues.push_back(make_pair(processes[x]->prior, processes[x]->pid));
                }
                sort(priorityvalues.begin(), priorityvalues.end());
                int nopshouldrem = 0;
                int nopremoving = 0;
                int availp = nofpInMM;
                for (int i = 0; i < priorityvalues.size(); i++)
                {
                    availp = availp + (processes[priorityvalues[i].second]->size) / P;
                    nopshouldrem++;
                    nopremoving += (processes[priorityvalues[i].second]->size) / P;
                    if (availp >= nopreq)
                    {
                        break;
                    }
                }
                if (availp < nopreq || nopremoving > nofpInVM)
                {
                    outputFile << "ERROR: NO SPACE" << endl;
                    continue;
                }
                for (int i = 0; i < nopshouldrem; i++) // remove these processes and add to vadd
                {
                    bringToVM(processes[priorityvalues[i].second], mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
                }
                // now bring our process to mm
                bringToMM(cprocess, mainMemory, virtualMemory, statusOfMMP, statusOfVMP);
            }
        }
        else if (word == "print")
        {
            // outputFile << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
            getline(s, word, ' ');
            int memloc = stoi(word);
            getline(s, word, ' ');
            int length = stoi(word);
            for (int i = 0; i < length; i++)
            {
                outputFile << "Value of " << memloc + i << ": " << mainMemory[memloc + i] << endl;
            }
        }
        else if (word == "exit")
        {
            for (auto x : processes)
            {
                free(x.second);
            }
            return 0;
        }
    }
    // outputFile << M << endl;
    inputFile.close();
    outputFile.close();
    return 0;
}