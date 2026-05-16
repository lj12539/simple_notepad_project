cmake_minimum_required(VERSION 3.22)
project(simple_notepad_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(
    notepad
    main.cpp
    main_window.h
    main_window.cpp
    text_transform.h
    find_replace_dialog.ui
    word_frequency_dialog.ui
)
target_link_libraries(notepad PRIVATE Qt6::Widgets)

add_executable(
    test_sort
    test_sort.cpp
    sort.h
)
add_custom_command(TARGET notepad POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:notepad>/data
)