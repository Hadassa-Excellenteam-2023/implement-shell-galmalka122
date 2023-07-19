#include <iostream>

#include "Bash.h"

using std::experimental::filesystem::current_path;

bool getInput(string&);

int main() {
    path cwd;
    Bash bash;
    string input;

    while (true) {
        try {
            cwd = current_path();
            std::cout << cwd.string() << ": ";
            if(getInput(input)){
                if(input == "exit")
                    break;
                bash.start(input);
            }

        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}

bool getInput(string& input){
    if (!std::getline(std::cin, input)) {
        if (std::cin.eof()) {
            std::cout << "End of input. Continuing..." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return false;
        }
        else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please try again." << std::endl;
            return false;
        }
    }
    return true;
}