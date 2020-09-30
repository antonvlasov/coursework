cmake_minimum_required(VERSION 3.7.0)

project(coursework)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED Widgets Core Gui Network Sql WebEngineWidgets)

set(SRC_FILES databasemanager.h 
    mainwindow.h 
    parser.h
    databasemanager.cpp 
    main.cpp 
    mainwindow.cpp 
    parser.cpp
    mainwindow.ui
    resources/resourses.qrc
)

if (WIN32)
add_executable(coursework WIN32 ${SRC_FILES})
else()
  add_executable(coursework ${SRC_FILES})
endif()

target_link_libraries(coursework Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql Qt5::WebEngineWidgets)