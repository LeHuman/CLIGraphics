cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----

# Note: update this to your new project's name and version
project(
    CLIGraphics
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

set(USE_CCACHE ON)
include(cmake/CPM.cmake)
include(cmake/tools.cmake)

# macro(CPMAddPackageST)
#     CPMAddPackage(${ARGV})
#     FILE(GLOB "${NAME}_sources" "${${${NAME}_SOURCE_DIR}}}/*.c" "${${${NAME}_SOURCE_DIR}}}/*.h" "${${${NAME}_SOURCE_DIR}}}/*.cpp" "${${${NAME}_SOURCE_DIR}}}/*.hpp")
#     source_group(TREE "${${${NAME}_SOURCE_DIR}}" FILES "${${${NAME}_sources}}")
# endmacro()

CPMAddPackage(
    NAME fmt
    GIT_TAG 9.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
)
# source_group(TREE ${fmt_SOURCE_DIR} FILES ${fmt_sources})

CPMAddPackage(
    NAME manymouse
    GIT_TAG main
    GITHUB_REPOSITORY icculus/manymouse
)
if (manymouse_ADDED)
    FILE(GLOB manymouse_sources "${manymouse_SOURCE_DIR}/*.c" "${manymouse_SOURCE_DIR}/*.h")
    add_library(manymouse STATIC ${manymouse_sources})
    # source_group(TREE ${manymouse_SOURCE_DIR} FILES ${manymouse_sources})

    target_include_directories(manymouse
        PUBLIC
        $<BUILD_INTERFACE:${manymouse_SOURCE_DIR}>
    )
endif()

CPMAddPackage(
    NAME glm
    GIT_TAG 1.0.1 # or any specific version you need
    GIT_REPOSITORY https://github.com/g-truc/glm.git
)
# source_group(TREE ${glm_SOURCE_DIR} FILES ${glm_sources})

CPMAddPackage(
    NAME openstl
    GIT_TAG v1.1.1
    GIT_REPOSITORY https://github.com/Innoptech/OpenSTL.git
)
# source_group(TREE ${openstl_SOURCE_DIR} FILES ${openstl_sources})

CPMAddPackage(
    GITHUB_REPOSITORY jarro2783/cxxopts
    VERSION 3.0.0
    OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)
# source_group(TREE ${cxxopts_SOURCE_DIR} FILES ${cxxopts_sources})

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE)
# add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
add_executable(${PROJECT_NAME} ${headers} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers} ${sources})

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt manymouse glm cxxopts openstl::core)

target_include_directories(
    ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# add_subdirectory(standalone)

set(DIRECTORY_TO_COPY ${CMAKE_SOURCE_DIR}/models)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DIRECTORY_TO_COPY}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/models)