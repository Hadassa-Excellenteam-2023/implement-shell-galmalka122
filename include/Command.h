#ifndef BASH_COMMAND_H
#define BASH_COMMAND_H

// Containers
#include <string>
#include <vector>
#include <experimental/filesystem>

#include "ICommand.h"

using std::experimental::filesystem::path;
using std::experimental::filesystem::is_directory;
using std::experimental::filesystem::exists;

using std::string;
using std::vector;

char* stringToCharPtr(const string& str);
char* findPath(const string& curPath, const vector<path>&);

class Command : public ICommand{
public:
    explicit Command(const vector<string>& words, const vector<path>& pathVec);
    Command(const Command& other);
    Command(Command&& other) noexcept;

    Command& operator=(const Command& other) = default;
    Command& operator=(Command&& other) noexcept;

    ~Command() override;

    int execute() override;

    char** getCommand() const override;
private:
    char** argv;
    size_t argc;

};


#endif //BASH_COMMAND_H
