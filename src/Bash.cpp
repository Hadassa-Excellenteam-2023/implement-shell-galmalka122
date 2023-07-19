//
// Created by gal on 7/1/23.
//

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

#include "Bash.h"

/**
 * Close the read and write ends of a pipe.
 * @param pipe The file descriptor array representing the pipe.
 */
void closePipes(const fd& pipe) {
    if (pipe[0] != -1) {
        close(pipe[0]);
        close(pipe[1]);
    }
}

/**
 * Find the index of a redirection symbol (">" or "<") in the given argument list.
 * @param args The argument list.
 * @param direction The redirection symbol ("<" or ">").
 * @return The index of the redirection symbol in the argument list, or 0 if not found.
 */
size_t findRedirection(const vector<string>& args, const string& direction) {
    auto it = std::find(args.begin(), args.end(), direction);
    if (it != args.end()) {
        return std::distance(args.begin(), it);
    }
    return 0;
}

/**
 * Split a string into a vector of substrings using the specified delimiter.
 * @param str The input string.
 * @param delim The delimiter string.
 * @return A vector of substrings.
 */
vector<string> splitString(string str, const string& delim) {
    vector<string> words;
    size_t pos;
    while ((pos = str.find(delim)) != std::string::npos) {
        words.emplace_back(str.substr(0, pos));
        str.erase(0, pos + delim.length());
    }
    words.emplace_back(str.substr(0, pos));
    return words;
}

/**
 * Constructor for the Bash class.
 * Initializes the pathDirectories vector by splitting the PATH environment variable.
 * Sorts the pathDirectories vector in ascending order.
 */
Bash::Bash() {
    for (auto &path : splitString(PATH, ":")) {
        _pathDirectories.emplace_back(path);
    }
    std::sort(_pathDirectories.begin(), _pathDirectories.end(), [](path& s1, path& s2) {
        return s1 < s2;
    });
}

/**
 * Copy constructor for the Bash class.
 * Performs a deep copy of the queue of ICommand pointers, the vector of path directories,
 * and the pipe file descriptors.
 * @param other The Bash object to be copied.
 */
Bash::Bash(const Bash& other) {
    // Copy the queue of ICommand pointers
    Commands newCommands = other._commands;
    while (!newCommands.empty()) {
        ICommand* command = newCommands.front();
        _commands.emplace(command);
        newCommands.pop();
    }

    // Copy the vector of path directories
    _pathDirectories = other._pathDirectories;

    // Copy the pipe file descriptors
    _prevPipe[0] = other._prevPipe[0];
    _prevPipe[1] = other._prevPipe[1];

    // Copy the input/output file descriptors
    _currPipe[0] = other._currPipe[0];
    _currPipe[1] = other._currPipe[1];
}

/**
 * Move constructor for the Bash class.
 * Moves the queue of ICommand pointers, the vector of path directories,
 * and the pipe file descriptors from the source object to the destination object.
 * @param other The Bash object to be moved.
 */
Bash::Bash(Bash&& other) noexcept {
    // Move the queue of ICommand pointers
    _commands = std::move(other._commands);

    // Move the vector of path directories
    _pathDirectories = std::move(other._pathDirectories);

    // Move the pipe file descriptors
    _prevPipe[0] = other._prevPipe[0];
    _prevPipe[1] = other._prevPipe[1];
    other._prevPipe[0] = 0;
    other._prevPipe[1] = 0;

    // Move the input/output file descriptors
    _currPipe[0] = other._currPipe[0];
    _currPipe[1] = other._currPipe[1];
    other._currPipe[0] = 0;
    other._currPipe[1] = 0;
}

/**
 * Copy assignment operator for the Bash class.
 * Performs a deep copy of the queue of ICommand pointers, the vector of path directories,
 * and the pipe file descriptors from the source object to the destination object.
 * @param other The Bash object to be copied.
 * @return The reference to the destination object.
 */
Bash& Bash::operator=(const Bash& other) {
    if (this != &other) {
        // Copy the queue of ICommand pointers
        Commands newCommands = other._commands;
        while (!newCommands.empty()) {
            ICommand* command = newCommands.front();
            _commands.emplace(command);
            newCommands.pop();
        }

        // Copy the vector of path directories
        _pathDirectories = other._pathDirectories;

        // Copy the pipe file descriptors
        _currPipe[0] = other._currPipe[0];
        _currPipe[1] = other._currPipe[1];

        // Copy the input/output file descriptors
        _prevPipe[0] = other._prevPipe[0];
        _prevPipe[1] = other._prevPipe[1];
    }
    return *this;
}

/**
 * Move assignment operator for the Bash class.
 * Moves the queue of ICommand pointers, the vector of path directories,
 * and the pipe file descriptors from the source object to the destination object.
 * @param other The Bash object to be moved.
 * @return The reference to the destination object.
 */
Bash& Bash::operator=(Bash&& other) noexcept {
    if (this == &other) {
        return *this;  // Self-assignment check
    }

    // Free the resources held by the destination object
    while (!_commands.empty()) {
        delete _commands.front();
        _commands.pop();
    }

    // Move the resources from the source object to the destination object
    _commands = std::move(other._commands);
    _pathDirectories = std::move(other._pathDirectories);
    _currPipe[0] = other._currPipe[0], _currPipe[1] = other._currPipe[1];
    _prevPipe[0] = other._prevPipe[0], _prevPipe[1] = other._prevPipe[1];

    // Set the source object's members to a valid state or nullify them
    other._currPipe[0] = 0;
    other._currPipe[1] = 0;
    other._prevPipe[0] = 0;
    other._prevPipe[1] = 0;

    return *this;
}

/**
 * Start the execution of commands based on the given input.
 * Parses the input into individual commands and sets up the necessary pipes.
 * Executes each command sequentially and waits for the child process to finish before executing the next command.
 * @param input The input string containing the commands.
 * @return 0 on success.
 */
int Bash::start(const string& input) {
    _prevPipe[0] = -1;
    _prevPipe[1] = -1;

    // Split the input into commands and search for pipe sections
    vector<string> commandStrings = splitString(input, " | ");
    size_t redirectionIndex;
    // Create the Command instances
    for (const string& commandStr : commandStrings) {

        vector<string> args = splitString(commandStr, " ");

        if ((redirectionIndex = findRedirection(args, ">")) != 0)
            _commands.emplace(new RedirectionCommand(args, _pathDirectories, redirectionIndex));

        else if ((redirectionIndex = findRedirection(args, "<")) != 0) {
            _commands.emplace(new RedirectionCommand(args, _pathDirectories, redirectionIndex));
        }
        else {
            _commands.emplace(new Command(args, _pathDirectories));
        }
    }

    // Execute each command sequentially
    while (!_commands.empty()) {
        ICommand* currCommand = _commands.front();
        _commands.pop();

        // change directory
        if (currCommand->getCommand() && strcmp(currCommand->getCommand()[0], "cd") == 0) {
            if (chdir(*(currCommand->getCommand() + 1)) == -1) {
                std::cerr << "Failed to change directory." << std::endl;
                exit(1);
            }
            return 0;
        }

        // Create a pipe for the current command's input
        pipe(_currPipe);

        pid_t pid = fork();

        if (pid == 0) {
            doChild(currCommand);
        }

        else if (pid > 0) {
            // Close the previous command's pipe
            closePipes(_prevPipe);

            // Store the current command's pipe for the next iteration
            _prevPipe[0] = _currPipe[0];
            _prevPipe[1] = _currPipe[1];

            // Wait for the child process to finish
            wait(nullptr);
            delete currCommand;

        }

        else {
            // Error forking
            std::cerr << "Fork failed" << std::endl;
            exit(1);
        }
    }

    // Close the last command's pipe
    closePipes(_prevPipe);
    return 0;
}

/**
 * Execute a command in the child process.
 * Redirects the input and output using the appropriate pipes.
 * @param command The command to be executed.
 */
void Bash::doChild(ICommand* command) {
    // Connect the previous command's output to the current command's input
    if (_prevPipe[0] != -1) {
        dup2(_prevPipe[0], STDIN_FILENO);
        closePipes(_prevPipe);
    }

    // Connect the current command's output to the next command's input
    if (!_commands.empty()) {
        dup2(_currPipe[1], STDOUT_FILENO);
        closePipes(_currPipe);
    }

    // Execute the command
    command->execute();
}

/**
 * Destructor for the Bash class.
 * Deletes all the ICommand objects stored in the queue.
 */
Bash::~Bash() {
    while (!_commands.empty()) {
        delete _commands.front();
        _commands.pop();
    }
}
