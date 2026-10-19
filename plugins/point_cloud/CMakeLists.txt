add_library(point_cloud_plugin MODULE
    point_cloud_plugin.cpp
    point_cloud_plugin.h
    point_cloud_plugin_entry.cpp
)

set_target_properties(point_cloud_plugin PROPERTIES
    OUTPUT_NAME "point_cloud_plugin"
    RUNTIME_OUTPUT_DIRECTORY "${ATP_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ATP_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${ATP_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${ATP_PLUGIN_DIR}"
)

target_include_directories(point_cloud_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(point_cloud_plugin
    PRIVATE
        Qt6::Core
        Qt6::Gui
        algorithm_contracts
        algorithm_shared
)
