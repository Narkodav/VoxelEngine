cmake_minimum_required(VERSION 3.20)

# Use folder name as plugin name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

message(STATUS "Building plugin: ${PLUGIN_NAME}")

file(GLOB_RECURSE SRC_FILES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${PLUGIN_NAME} SHARED ${SRC_FILES})

target_compile_features(${PLUGIN_NAME} PUBLIC cxx_std_20)

target_compile_options(${PLUGIN_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_include_directories(${PLUGIN_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        PluginFramework
        CommonApi
        Containers::Containers
        imgui
        GraphicsWrapper
        JsonParser
        Clipper2Lib
)

# Output to shared plugin folder
set_target_properties(${PLUGIN_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)

# Windows export define (optional but useful)
target_compile_definitions(${PLUGIN_NAME}
    PRIVATE ${PLUGIN_NAME}_EXPORTS
)