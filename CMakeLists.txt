cmake_minimum_required(VERSION 3.5)

project(web_project CXX)

# enable C and C++ language
enable_language(C CXX)

# Add sub-directories
add_subdirectory(server)
add_subdirectory(client)

