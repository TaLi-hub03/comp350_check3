#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

vector<string> paths = {"/bin"};

void printError() {
    const char* error_message = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char* argv[]) {
    string input;
    bool interactive = true;
    if (argc > 2) {
        printError();
        exit(1);
    }
    if (argc == 2) {
        interactive = false;
        ifstream batchFile(argv[1]);
        if (!batchFile.is_open()) {
            printError();
            exit(1);
        }
        while (getline(batchFile, input)) {
            input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
            if (input.empty()) continue;
            stringstream ss(input);
            vector<string> args;
            string arg;
            while (ss >> arg) args.push_back(arg);
            if (args.empty()) continue;
            string command = args[0];
            if (command == "exit") {
                if (args.size() != 1) { printError(); continue; }
                cout << "Aggie Shell - TaliahLilly" << endl;
                exit(0);
            } else if (command == "cd") {
                if (args.size() != 2) { printError(); continue; }
                if (chdir(args[1].c_str()) != 0) printError();
            } else if (command == "path") {
                paths.clear();
                for (size_t i = 1; i < args.size(); ++i)
                    paths.push_back(args[i]);
            } else {
                bool redirected = false;
                string outputFile;
                vector<string> cmdArgs;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (args[i] == ">") {
                        if (i + 1 >= args.size() || i + 2 != args.size()) { 
                            printError(); 
                            cmdArgs.clear(); 
                            break; 
                        }
                        redirected = true;
                        outputFile = args[i + 1];
                        break;
                    } else {
                        cmdArgs.push_back(args[i]);
                    }
                }
                if (cmdArgs.empty()) continue;
                bool executed = false;
                for (const string &p : paths) {
                    string fullPath = p + "/" + cmdArgs[0];
                    if (access(fullPath.c_str(), X_OK) == 0) {
                        pid_t pid = fork();
                        if (pid == 0) {
                            if (redirected) {
                                int fd = open(outputFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
                                if (fd < 0) { printError(); exit(1); }
                                dup2(fd, STDOUT_FILENO);
                                dup2(fd, STDERR_FILENO);
                                close(fd);
                            }
                            vector<char*> execArgs;
                            for (auto &s : cmdArgs) execArgs.push_back(const_cast<char*>(s.c_str()));
                            execArgs.push_back(nullptr);
                            execv(fullPath.c_str(), execArgs.data());
                            printError();
                            exit(1);
                        } else if (pid > 0) {
                            wait(nullptr);
                            executed = true;
                            break;
                        } else {
                            printError();
                            executed = true;
                            break;
                        }
                    }
                }
                if (!executed) printError();
            }
        }
        return 0;
    }

    while (true) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        cout << "ash " << cwd << " > ";
        if (!getline(cin, input)) {
            cout << "\nAggie Shell - TaliahLilly" << endl;
            exit(0);
        }
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        if (input.empty()) continue;
        stringstream ss(input);
        vector<string> args;
        string arg;
        while (ss >> arg) args.push_back(arg);
        if (args.empty()) continue;
        string command = args[0];
        if (command == "exit") {
            if (args.size() != 1) { printError(); continue; }
            cout << "Aggie Shell - TaliahLilly" << endl;
            exit(0);
        } else if (command == "cd") {
            if (args.size() != 2) { printError(); continue; }
            if (chdir(args[1].c_str()) != 0) printError();
        } else if (command == "path") {
            paths.clear();
            for (size_t i = 1; i < args.size(); ++i)
                paths.push_back(args[i]);
        } else {
            bool redirected = false;
            string outputFile;
            vector<string> cmdArgs;
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] == ">") {
                    if (i + 1 >= args.size() || i + 2 != args.size()) { 
                        printError(); 
                        cmdArgs.clear(); 
                        break; 
                    }
                    redirected = true;
                    outputFile = args[i + 1];
                    break;
                } else {
                    cmdArgs.push_back(args[i]);
                }
            }
            if (cmdArgs.empty()) continue;
            bool executed = false;
            for (const string &p : paths) {
                string fullPath = p + "/" + cmdArgs[0];
                if (access(fullPath.c_str(), X_OK) == 0) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        if (redirected) {
                            int fd = open(outputFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
                            if (fd < 0) { printError(); exit(1); }
                            dup2(fd, STDOUT_FILENO);
                            dup2(fd, STDERR_FILENO);
                            close(fd);
                        }
                        vector<char*> execArgs;
                        for (auto &s : cmdArgs) execArgs.push_back(const_cast<char*>(s.c_str()));
                        execArgs.push_back(nullptr);
                        execv(fullPath.c_str(), execArgs.data());
                        printError();
                        exit(1);
                    } else if (pid > 0) {
                        wait(nullptr);
                        executed = true;
                        break;
                    } else {
                        printError();
                        executed = true;
                        break;
                    }
                }
            }
            if (!executed) printError();
        }
    }

    return 0;
}
