//
// Created by gal on 7/1/23.
//

#ifndef BASH_ICOMMAND_H
#define BASH_ICOMMAND_H

class ICommand{

public:
    virtual char** getCommand() const = 0;
    virtual int execute() = 0;
    virtual ~ICommand() = default;
};

#endif //BASH_ICOMMAND_H
