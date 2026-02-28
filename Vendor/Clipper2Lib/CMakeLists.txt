add_library(Clipper2Lib
    src/clipper.engine.cpp
    src/clipper.offset.cpp
    src/clipper.rectclip.cpp
)

target_include_directories(Clipper2Lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)