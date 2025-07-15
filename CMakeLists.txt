cmake_minimum_required(VERSION 3.24)
project(generic_host_cpp VERSION 0.1.0 LANGUAGES CXX)

# -- Enable C++20, position-independent everywhere ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -- vcpkg toolchain integration (should work with GitHub Actions) --
#   Make sure VCPKG_ROOT is in the environment or pass
#   -DCMAKE_TOOLCHAIN_FILE=$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
#   when configuring locally.

# -- Options -------------------------------------------------------
option(GH_BUILD_TESTS "Build Catch2 tests" OFF)

# -- Library target ------------------------------------------------
add_library(generic_host STATIC
        src/HostBuilder.cpp
        src/ConsoleLogger.cpp
)
target_include_directories(generic_host PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
find_package(Boost REQUIRED COMPONENTS asio system)
find_package(spdlog REQUIRED)

target_link_libraries(generic_host
        PUBLIC
        Boost::asio
        Boost::system
        spdlog::spdlog_header_only
)

# -----------------------------------------------------------------
add_executable(log_indexer_example examples/log_indexer.cpp)
target_link_libraries(log_indexer_example PRIVATE generic_host)

# -- Tests --------------------------------------------------------
if(GH_BUILD_TESTS)
    find_package(Catch2 REQUIRED)
    add_executable(generic_host_tests tests/host_tests.cpp)
    target_link_libraries(generic_host_tests PRIVATE generic_host Catch2::Catch2WithMain)
    add_test(NAME generic_host_tests COMMAND generic_host_tests)
endif()

# -- Install rules (for vcpkg too!) -----------------
include(GNUInstallDirs)
install(TARGETS generic_host
        EXPORT generic_hostTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT generic_hostTargets
        NAMESPACE generic_host::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/generic_host)

# -- CPack for source/binary tarballs ------------------------------
include(CPack)
