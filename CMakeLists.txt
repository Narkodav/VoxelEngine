cmake_minimum_required(VERSION 3.20)
project(VoxelEngine)

file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME}
    # ${SRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/StorageCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/AssetCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/VoxelCullingCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Renderer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameData/ResourceCache.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/WorldManagement/Generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WorldManagement/WorldGrid.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic> #-fsanitize=address -fno-omit-frame-pointer -g>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> #/fsanitize=address>
)

#target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Vulkan REQUIRED)

set(CommonApi_Directory "E:/Program Files (x86)/Code/C_code/libraries/CommonApi")
add_subdirectory(
    ${CommonApi_Directory}
    ${CMAKE_CURRENT_BINARY_DIR}/CommonApi
)

find_package(Containers CONFIG REQUIRED)

set(GraphicsWrapper_Directory "E:/Program Files (x86)/Code/C_code/libraries/GraphicsWrapper")
add_subdirectory(
    ${GraphicsWrapper_Directory}
    ${CMAKE_CURRENT_BINARY_DIR}/GraphicsWrapper
)

target_link_libraries(${PROJECT_NAME} PRIVATE GraphicsWrapper)

set(JsonParser_Directory "E:/Program Files (x86)/Code/C_code/libraries/JsonParser")
add_subdirectory(
    ${JsonParser_Directory}
    ${CMAKE_CURRENT_BINARY_DIR}/JsonParser
)

target_link_libraries(${PROJECT_NAME} PRIVATE JsonParser)

add_subdirectory(Vendor/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

add_subdirectory(Vendor/Clipper2Lib)
target_link_libraries(${PROJECT_NAME} PRIVATE Clipper2Lib)

# for imgui alpha blending
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi)
    add_definitions(-D_WIN32_WINNT=0x0600)
endif()