file(GLOB CHILDREN LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(CHILD ${CHILDREN})
    message(STATUS "Found child: ${CHILD}")

    if(IS_DIRECTORY ${CHILD})
        if(EXISTS "${CHILD}/CMakeLists.txt")
            message(STATUS "Adding plugin: ${CHILD}")
            add_subdirectory(${CHILD})
        else()
            message(STATUS "Skipping (no CMakeLists): ${CHILD}")
        endif()
    endif()
endforeach()