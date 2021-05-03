cmake_minimum_required(VERSION 3.16.3)
project(ProcessID)
add_executable(${PROJECT_NAME} main.c)

# Making sure it can be executed on older machines (c++98?)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 98)

# Shouldn't this be c90 or c99 standard?
#set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 90) # or 99



