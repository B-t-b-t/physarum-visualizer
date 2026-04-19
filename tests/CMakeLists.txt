add_executable(PhysarumTests
    unit/test_parameter_parser.cpp
    ${CMAKE_SOURCE_DIR}/source/utility/parameter_parser.cpp
)
target_link_libraries(PhysarumTests PRIVATE Catch2::Catch2WithMain)

target_include_directories(PhysarumTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/source
    ${imgui_SOURCE_DIR}
)

set_target_properties(PhysarumTests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
)

include(Catch)
include(CTest)
catch_discover_tests(PhysarumTests)