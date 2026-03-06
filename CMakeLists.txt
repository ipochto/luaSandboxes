cmake_minimum_required(VERSION 3.30)
project(luaSandboxes LANGUAGES CXX)
set(luaSandboxes_lib luaSandboxes)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(WARNING "CMAKE_BUILD_TYPE is not set! Defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_PROFILING "Enable Profiling Flags" OFF)
message(STATUS "Profiling enabled: ${ENABLE_PROFILING}")

option(BUILD_TESTING "Enable Testing" OFF)
message(STATUS "Testing enabled: ${BUILD_TESTING}")

option(ENABLE_COVERAGE "Enable Coverage Flags" OFF)
message(STATUS "Coverage enabled: ${ENABLE_COVERAGE}")

include(ThirdPartyDependencies)

set(sources
    src/lua/runtime.cpp
    src/lua/utils.cpp
)
set(headers
    src/lua/runtime.hpp
    src/lua/sol2.hpp
    src/lua/utils.hpp

    src/utils/enum_set.hpp
    src/utils/filesystem.hpp
    src/utils/log.hpp
    src/utils/optional_ref.hpp
)

set(include_dirs
    SYSTEM
    src
)

set(libraries
    lua::lua51
    sol2::sol2
)

add_library(${luaSandboxes_lib} STATIC)
target_sources(${luaSandboxes_lib} PRIVATE
    ${sources} 
    ${headers}
)
target_include_directories(${luaSandboxes_lib} PUBLIC ${include_dirs})

target_compile_features(${luaSandboxes_lib} PRIVATE cxx_std_20)
target_link_libraries(${luaSandboxes_lib} PUBLIC ${libraries})

set(is_gcc "$<CXX_COMPILER_ID:GNU>")
set(is_clang "$<CXX_COMPILER_ID:Clang>")
set(is_msvc "$<OR:$<CXX_COMPILER_ID:MSVC>,$<AND:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>")

target_compile_definitions(${luaSandboxes_lib} PUBLIC
    $<$<CONFIG:Debug>:DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<BOOL:${ENABLE_PROFILING}>:PROFILE_BUILD>
)

target_compile_options(${luaSandboxes_lib} PRIVATE 
    # Linux
    $<$<AND:$<PLATFORM_ID:Linux>,$<CONFIG:Debug>>:
        -fno-inline
        -fstack-protector-strong
        -fno-omit-frame-pointer
        -Wall -Wextra -Wpedantic -Wreorder
        -Wwrite-strings
        $<${is_gcc}: -ggdb>
        $<${is_clang}: -glldb>
    >
    $<$<AND:$<BOOL:${ENABLE_PROFILING}>,$<PLATFORM_ID:Linux>>:
        -fno-omit-frame-pointer
    >
    $<$<AND:$<BOOL:${ENABLE_COVERAGE}>,$<PLATFORM_ID:Linux>>:
        $<${is_gcc}: --coverage>
        $<${is_clang}: -fprofile-instr-generate -fcoverage-mapping>
    >

    # Windows
    $<$<AND:$<PLATFORM_ID:Windows>,$<CONFIG:Debug>,${is_msvc}>:
        /Oy-
        /W4
        /permissive-
        /Zc:strictStrings
    >
    $<$<AND:$<BOOL:${ENABLE_PROFILING}>,${is_msvc}>:
        /Oy-
        /Gh
        /GH
    >
    $<$<CXX_COMPILER_ID:MSVC>:/MP>    
)
target_link_options(${luaSandboxes_lib} PRIVATE
    $<$<AND:$<PLATFORM_ID:Linux>,$<CONFIG:Debug>>:
        -rdynamic
    >
    $<$<AND:$<BOOL:${ENABLE_PROFILING}>,${is_msvc}>:
        /PROFILE
    >
    $<$<AND:$<BOOL:${ENABLE_COVERAGE}>,$<PLATFORM_ID:Linux>>:
        $<${is_gcc}: --coverage>
        $<${is_clang}: -fprofile-instr-generate>
    >
)
add_library(luaSandboxes::luaSandboxes ALIAS ${luaSandboxes_lib})

include(Tests)
