cmake_minimum_required(VERSION 3.10.0)
project(leds_ctrl VERSION 0.1.0 LANGUAGES C CXX)


# source files
add_executable(leds_ctrl
    main.cpp
    src/pio_rp1.c
    src/piolib.c
)



# Automatic includes
file(GLOB_RECURSE HEADER_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*)
foreach(header ${HEADER_DIRS})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

target_include_directories(leds_ctrl PRIVATE ${INCLUDE_DIRS})


set_target_properties(leds_ctrl PROPERTIES
    OUTPUT_NAME "leds_ctrl"
)
