cmake_minimum_required(VERSION 3.16)

project(AlgorithmTeachingPlatform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

# 教学平台只保留一套运行目录，宿主程序和插件都输出到这里。
set(ATP_APP_DIR "${CMAKE_BINARY_DIR}/app" CACHE INTERNAL "教学平台运行目录")
set(ATP_PLUGIN_DIR "${ATP_APP_DIR}/plugins" CACHE INTERNAL "插件输出目录")
set(ATP_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd" CACHE PATH "第三方库目录")

# 自动查找常见 Qt 安装位置，减少新电脑首次配置成本。
file(GLOB _qt_candidate_roots LIST_DIRECTORIES true
    "C:/Qt/*/msvc*_64"
    "D:/Qt/*/msvc*_64"
)

foreach(_qt_root IN LISTS _qt_candidate_roots)
    if (EXISTS "${_qt_root}/lib/cmake/Qt6/Qt6Config.cmake")
        list(APPEND CMAKE_PREFIX_PATH "${_qt_root}")
    endif()
endforeach()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(ATP_OPENCV_INCLUDE_DIR "${ATP_THIRD_PARTY_DIR}/opencv/include")
set(ATP_OPENCV_LIB_DIR "${ATP_THIRD_PARTY_DIR}/opencv/lib")
set(ATP_OPENCV_BIN_DIR "${ATP_THIRD_PARTY_DIR}/opencv/bin")
set(ATP_ONNXRUNTIME_INCLUDE_DIR "${ATP_THIRD_PARTY_DIR}/onnxruntime/include")
set(ATP_ONNXRUNTIME_LIB_DIR "${ATP_THIRD_PARTY_DIR}/onnxruntime/lib")
set(ATP_ONNXRUNTIME_BIN_DIR "${ATP_THIRD_PARTY_DIR}/onnxruntime/bin")

if (NOT EXISTS "${ATP_OPENCV_INCLUDE_DIR}/opencv2/opencv.hpp")
    message(FATAL_ERROR "未找到 OpenCV 头文件: ${ATP_OPENCV_INCLUDE_DIR}/opencv2/opencv.hpp")
endif()

file(GLOB ATP_OPENCV_LIBS "${ATP_OPENCV_LIB_DIR}/*.lib")
file(GLOB ATP_OPENCV_DLLS "${ATP_OPENCV_BIN_DIR}/*.dll")
file(GLOB ATP_ONNXRUNTIME_DLLS "${ATP_ONNXRUNTIME_BIN_DIR}/*.dll")

if (NOT ATP_OPENCV_LIBS)
    message(FATAL_ERROR "未找到 OpenCV .lib 文件: ${ATP_OPENCV_LIB_DIR}")
endif()

if (EXISTS "${ATP_ONNXRUNTIME_INCLUDE_DIR}/onnxruntime_cxx_api.h"
    AND EXISTS "${ATP_ONNXRUNTIME_LIB_DIR}/onnxruntime.lib")
    set(ATP_ONNXRUNTIME_ENABLED ON)
    set(ATP_ONNXRUNTIME_LIB "${ATP_ONNXRUNTIME_LIB_DIR}/onnxruntime.lib")
else()
    set(ATP_ONNXRUNTIME_ENABLED OFF)
    message(WARNING "未找到 ONNX Runtime，深度学习插件将不能编译真实推理逻辑。")
endif()

add_subdirectory(contracts)
add_subdirectory(shared)
add_subdirectory(plugins/traditional_vision)
add_subdirectory(plugins/point_cloud)
add_subdirectory(plugins/deep_learning)
add_subdirectory(app)
