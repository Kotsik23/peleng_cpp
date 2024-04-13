cmake_minimum_required(VERSION 3.27)
project(task)

set(CMAKE_CXX_STANDARD 17)

add_executable(task
        main.cpp
        Event.h
        EventTracker.cpp
        EventTracker.h
        Logger.cpp
        Logger.h
        CommandHandler.cpp
        CommandHandler.h)
