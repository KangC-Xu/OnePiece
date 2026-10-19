add_executable(AlgorithmTeachingPlatform
    main.cpp
    main_window.cpp
    main_window.h
    plugin_loader.cpp
    plugin_loader.h
    algorithm_runner.cpp
    algorithm_runner.h
)

set_target_properties(AlgorithmTeachingPlatform PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${ATP_APP_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ATP_APP_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ATP_APP_DIR}"
)

target_include_directories(AlgorithmTeachingPlatform
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(AlgorithmTeachingPlatform
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        algorithm_contracts
)

add_dependencies(AlgorithmTeachingPlatform
    traditional_vision_plugin
    point_cloud_plugin
    deep_learning_plugin
)

if (ATP_OPENCV_DLLS)
    add_custom_command(TARGET AlgorithmTeachingPlatform POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ATP_OPENCV_DLLS}
            "$<TARGET_FILE_DIR:AlgorithmTeachingPlatform>"
        COMMENT "复制 OpenCV 运行库"
    )
endif()

if (ATP_ONNXRUNTIME_DLLS)
    add_custom_command(TARGET AlgorithmTeachingPlatform POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ATP_ONNXRUNTIME_DLLS}
            "$<TARGET_FILE_DIR:AlgorithmTeachingPlatform>"
        COMMENT "复制 ONNX Runtime 运行库"
    )
endif()

get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
if (QT_QMAKE_EXECUTABLE)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(QT_DEPLOY_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")
endif()

if (EXISTS "${QT_DEPLOY_EXECUTABLE}")
    add_custom_command(TARGET AlgorithmTeachingPlatform POST_BUILD
        COMMAND "${QT_DEPLOY_EXECUTABLE}"
            --no-compiler-runtime
            $<$<CONFIG:Debug>:--debug>
            $<$<NOT:$<CONFIG:Debug>>:--release>
            "$<TARGET_FILE:AlgorithmTeachingPlatform>"
        COMMENT "部署 Qt 运行库"
    )
endif()
