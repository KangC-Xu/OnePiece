add_library(traditional_vision_plugin MODULE
    traditional_vision_plugin.cpp
    traditional_vision_plugin.h
    traditional_vision_plugin_entry.cpp
)

set_target_properties(traditional_vision_plugin PROPERTIES
    OUTPUT_NAME "traditional_vision_plugin"
    RUNTIME_OUTPUT_DIRECTORY "${ATP_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ATP_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${ATP_PLUGIN_DIR}"
)

target_include_directories(traditional_vision_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(traditional_vision_plugin
    PRIVATE
        Qt6::Core
        Qt6::Gui
        algorithm_contracts
        algorithm_shared
)
