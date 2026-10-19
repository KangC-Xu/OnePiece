add_library(deep_learning_plugin MODULE
    deep_learning_plugin.cpp
    deep_learning_plugin.h
    deep_learning_plugin_entry.cpp
)

set_target_properties(deep_learning_plugin PROPERTIES
    OUTPUT_NAME "deep_learning_plugin"
    RUNTIME_OUTPUT_DIRECTORY "${ATP_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ATP_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${ATP_PLUGIN_DIR}"
)

target_include_directories(deep_learning_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ATP_ONNXRUNTIME_INCLUDE_DIR}
)

target_link_libraries(deep_learning_plugin
    PRIVATE
        Qt6::Core
        Qt6::Gui
        algorithm_contracts
        algorithm_shared
        ${ATP_ONNXRUNTIME_LIB}
)

if (ATP_ONNXRUNTIME_ENABLED)
    target_compile_definitions(deep_learning_plugin PRIVATE ATP_ONNXRUNTIME_ENABLED=1)
endif()
