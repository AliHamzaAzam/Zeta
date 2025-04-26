cmake_minimum_required(VERSION 3.20)

# Project name and version
project(Parser VERSION 1.0)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(Parser Parser.cpp)
add_executable(Stack Stack.cpp)
add_executable(ParserWindows Parser-win.cpp)
add_executable(StackWindows Stack-win.cpp)

# Include directories
include_directories(src/main/cpp/org/azaleas/parser)

# Add any additional libraries if needed
# target_link_libraries(AzaleasParser <library_name>)

# Add cfg.txt as a resource
configure_file(cfg.txt cfg.txt COPYONLY)
# Add input_strings.txt as resource
configure_file(input_strings.txt input_strings.txt COPYONLY)

