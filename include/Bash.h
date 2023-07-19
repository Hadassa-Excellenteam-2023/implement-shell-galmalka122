//
// Created by gal on 7/1/23.
//

#ifndef BASH_BASH_H
#define BASH_BASH_H

#include<queue>
#include <string>

#include "RedirectionCommand.h"

using std::string;


const string PATH = getenv("PATH");


using Commands = std::queue<ICommand*>;
class Bash {

public:
    Bash();
    Bash(const Bash&);
    Bash(Bash&&) noexcept ;
    ~Bash();

    Bash& operator=(const Bash&);
    Bash& operator=(Bash&&) noexcept;
    int start(const string& input);


private:
    Commands _commands;
    vector<path> _pathDirectories;
    fd _prevPipe{}; // Pipe for connecting the previous command's output
    fd _currPipe{}; // Pipe for connecting the current command's input
    void doChild(ICommand* command);

};


#endif //BASH_BASH_H
