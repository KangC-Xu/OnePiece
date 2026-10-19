add_library(algorithm_contracts INTERFACE)

target_include_directories(algorithm_contracts
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(algorithm_contracts
    INTERFACE
        Qt6::Core
        Qt6::Gui
)
