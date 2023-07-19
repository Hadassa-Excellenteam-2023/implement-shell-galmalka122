cmake_minimum_required(VERSION 3.19)
project(bash)

set(CMAKE_CXX_STANDARD 17)
link_libraries(stdc++fs)
# Define a custom target to run the program with sudo

add_executable(bash src/Command.cpp include/Command.h src/Bash.cpp include/Bash.h src/RedirectionCommand.cpp include/RedirectionCommand.h include/ICommand.h)
add_subdirectory(include)
add_subdirectory(src)