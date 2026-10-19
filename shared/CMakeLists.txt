add_library(algorithm_shared STATIC
    src/image_utils.cpp
    include/algorithm_platform/image_utils.h
)

target_include_directories(algorithm_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ATP_OPENCV_INCLUDE_DIR}
)

target_link_directories(algorithm_shared
    PUBLIC
        ${ATP_OPENCV_LIB_DIR}
)

target_link_libraries(algorithm_shared
    PUBLIC
        Qt6::Gui
        ${ATP_OPENCV_LIBS}
)
