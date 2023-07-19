//
// Created by gal on 7/1/23.
//

#ifndef BASH_REDIRECTIONCOMMAND_H
#define BASH_REDIRECTIONCOMMAND_H
#include <fstream>
#include "Command.h"
using fd = int[2];
class RedirectionCommand  : public ICommand {
public:

    RedirectionCommand(const vector<string>&, const vector<path>&, size_t);
    int execute() override;

private:
    char** getCommand()const override;
    void redirectOutput(int fileDescriptor);
    void redirectInput(int fileDescriptor);
    bool _output;
    ICommand* _command;
    fd _pipeFD;
    string _file;
};


#endif //BASH_REDIRECTIONCOMMAND_H
