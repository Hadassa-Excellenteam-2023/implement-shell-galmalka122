#include "Command.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <unistd.h>


/**
 * Constructor.
 * @param words The vector of words representing the command.
 * @param pathDirectories The vector of directories to search for the command.
 */
Command::Command(const vector<string>& words, const vector<path>& pathDirectories) {
    argc = words.size();
    argv = new char* [argc + 1];
    argv[0] = findPath(words[0], pathDirectories);
    for (size_t i = 1; i < argc; ++i) {
        argv[i] = stringToCharPtr(words[i]);
    }
    argv[argc] = nullptr;
}

/**
 * Copy constructor.
 * @param other The Command object to copy.
 */
Command::Command(const Command& other) {
    if (this != &other) {

        // Increase size by 1 for the nullptr pointer
        this->argv = new char*[other.argc + 1];
        for (size_t i = 0; i < other.argc; ++i) {
            size_t len = strlen(other.argv[i]) + 1;
            // allocate for string and ending \0
            this->argv[i] = new char[len];
            strcpy(this->argv[i], other.argv[i]);
        }
        this->argv[other.argc] = nullptr;
        this->argc = other.argc;
    }
}

/**
 * Move constructor.
 * @param other The Command object to move.
 */
Command::Command(Command&& other) noexcept{
    argv = other.argv;
    argc = other.argc;
    other.argv = nullptr;
    other.argc = 0;
}

/**
 * Move assignment operator.
 * @param other The Command object to move assign.
 * @return Reference to the modified Command object.
 */
Command& Command::operator=(Command&& other) noexcept{
    if (this != &other) {
        delete[] argv;
        argv = other.argv;
        argc = other.argc;
        other.argv = nullptr;
        other.argc = 0;
    }
    return *this;
}

/**
 * Destructor.
 */
Command::~Command() {
    if (argv) {
        for (size_t i = 0; i < argc; ++i) {
            delete[] argv[i];
            argv[i] = nullptr;
        }
        delete[] argv;
        argv = nullptr;
    }
}

/**
 * Executes the command.
 * @return The exit status of the command.
 */
int Command::execute() {

    execvp(argv[0], argv);

    // execvp failed if the control reaches here
    std::cerr << "Failed to execute command: " << argv[0] << std::endl;

    exit(1);
}

char **Command::getCommand() const {
    if(argc > 0)
        return argv;
    return nullptr;
}

/**
 * Converts a string to a character pointer.
 * @param str The input string.
 * @return A dynamically allocated character pointer.
 */
char* stringToCharPtr(const string& str){
    char* cp = new char[str.size() + 1];
    strcpy(cp, str.c_str());
    return cp;
}

/**
 * Finds the path of a command.
 * @param curPath The current path of the command.
 * @param pathDirectories The vector of directories to search for the command.
 * @return The path of the command as a character pointer.
 */
char* findPath(const string& curPath, const vector<path>& pathDirectories)  {
    path commandPath(curPath);
    if (!exists(commandPath)) {

        for (const auto& dir : pathDirectories) {
            path currentPath = dir / commandPath;
            if (exists(currentPath)) {
                return stringToCharPtr(currentPath.string());
            }
        }
    }

    return stringToCharPtr(curPath);
}
