// Created by gal on 7/1/23.
//

#include "RedirectionCommand.h"
#include <stdexcept>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>

/**
 * Constructor for the RedirectionCommand class.
 * Initializes the command and file information based on the provided words vector.
 * @param words The vector of words representing the command and file information.
 * @param pathVec The vector of path directories.
 * @param index The index in the words vector where the redirection symbol ("<" or ">") is located.
 */
RedirectionCommand::RedirectionCommand(const vector<string>& words, const vector<path>& pathVec, size_t index) {

    _command = new Command({words.begin(), words.begin() + index}, pathVec);
    if (words[index] == "<")
        _output = false;
    else
        _output = true;
    _file = words[index + 1];
}

/**
 * Execute the redirection command.
 * Forks the process and redirects the input/output based on the command and file information.
 * Executes the command in the child process.
 * @return The exit status of the executed command.
 */
int RedirectionCommand::execute() {
    // Fork the process
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to fork process." << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process

        // Redirect input
        if (!_output) {
            redirectInput(_pipeFD[0]);
        }

        // Redirect output
        if (_output) {
            redirectOutput(_pipeFD[1]);
        }

        // Execute the command
        _command->execute();
    }

    if (_pipeFD[0] != -1) {
        close(_pipeFD[0]);
        close(_pipeFD[1]);
    }

    exit(0);
}

/**
 * Redirects the output of the command to the specified file.
 * Opens the file using `open` with write-only, create, and truncate flags.
 * Redirects the standard output (stdout) to the file descriptor of the opened file.
 * Closes the file descriptor and flushes the stream.
 * @param fileDescriptor The file descriptor for the output file.
 */
void RedirectionCommand::redirectOutput(int fileDescriptor) {
    // Open the file using fdopen
    fileDescriptor = open(_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fileDescriptor == -1) {
        std::cerr << "Failed to open file descriptor for output." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Redirect stdout to the file
    if (dup2(fileDescriptor, STDOUT_FILENO) == -1) {
        std::cerr << "Failed to redirect output." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Close the file descriptor and flush the stream
    close(fileDescriptor);
}

/**
 * Redirects the input of the command from the specified file.
 * Opens the file using `open` with read-only flag.
 * Redirects the standard input (stdin) to the file descriptor of the opened file.
 * Closes the file descriptor.
 * @param fileDescriptor The file descriptor for the input file.
 */
void RedirectionCommand::redirectInput(int fileDescriptor) {
    // Open the file using fdopen
    fileDescriptor = open(_file.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
    if (fileDescriptor == -1) {
        std::cerr << "Failed to open file descriptor for input." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Redirect stdin to the file
    if (dup2(fileDescriptor, STDIN_FILENO) == -1) {
        std::cerr << "Failed to redirect input." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Close the file descriptor
    close(fileDescriptor);
}

char **RedirectionCommand::getCommand() const {
    return _command->getCommand();
}
