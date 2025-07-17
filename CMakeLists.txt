cmake_minimum_required(VERSION 3.24)
project(generic_host_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(GH_BUILD_TESTS "Build Catch2 tests" OFF)

file(GLOB_RECURSE GH_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
)

add_library(generic_host STATIC ${GH_SOURCES})
target_include_directories(generic_host PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

find_package(Boost REQUIRED CONFIG COMPONENTS asio system)
find_package(spdlog REQUIRED)

target_link_libraries(generic_host
        PUBLIC Boost::asio Boost::system spdlog::spdlog_header_only
)

if(WIN32)
    target_link_libraries(generic_host PRIVATE ws2_32)
endif()

# -- Example executable -------------------
add_executable(log_indexer_example examples/log_indexer.cpp)
target_link_libraries(log_indexer_example PRIVATE generic_host)

# -- Tests -----------
if(GH_BUILD_TESTS)
    enable_testing()
    find_package(Catch2 REQUIRED)
    file(GLOB_RECURSE GH_TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
    add_executable(generic_host_tests ${GH_TEST_SOURCES})
    target_link_libraries(generic_host_tests PRIVATE generic_host Catch2::Catch2WithMain)
    add_test(NAME generic_host_tests COMMAND generic_host_tests)
endif()

# -- Install & CPack -------------------------------
include(GNUInstallDirs)
install(TARGETS generic_host
        EXPORT generic_hostTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT generic_hostTargets
        NAMESPACE generic_host::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/generic_host)

include(CPack)
