#-------------------------------------------------------------------------------
# Morpho-it/CMakeLists.txt
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

project(morphoit)

#-------------------------------------------------------------------------------
# Build BuildIt
#-------------------------------------------------------------------------------

set(BUILDIT_PATH "${CMAKE_SOURCE_DIR}/buildit")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -Wfatal-errors")

include(ExternalProject)

# Build the buildit subproject using make
ExternalProject_Add(buildit
    SOURCE_DIR ${BUILDIT_PATH}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND $(MAKE) -C ${BUILDIT_PATH}
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
)


# Query buildit for compile flags
execute_process(
    COMMAND make --no-print-directory -C ${BUILDIT_PATH} compile-flags
    OUTPUT_VARIABLE BUILDIT_COMPILE_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Query buildit for linker flags
execute_process(
    COMMAND make --no-print-directory -C ${BUILDIT_PATH} linker-flags
    OUTPUT_VARIABLE BUILDIT_LINKER_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#-------------------------------------------------------------------------------
# Add BuildIt library
#-------------------------------------------------------------------------------

add_library(buildit_lib STATIC IMPORTED GLOBAL)

set_target_properties(buildit_lib PROPERTIES
    IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/buildit/build/libbuildit.a
)

add_dependencies(buildit_lib buildit)

#-------------------------------------------------------------------------------
# Build morphoit
#-------------------------------------------------------------------------------

add_executable(morphoit "")

add_dependencies(morphoit buildit_lib)

#-------------------------------------------------------------------------------
# Include morpho and buildit
#-------------------------------------------------------------------------------

separate_arguments(BUILDIT_COMPILE_FLAGS)
target_compile_options(morphoit PRIVATE ${BUILDIT_COMPILE_FLAGS})

separate_arguments(BUILDIT_LINKER_FLAGS)
target_link_options(morphoit PRIVATE ${BUILDIT_LINKER_FLAGS})

# Locate the morpho.h header file and store in MORPHO_HEADER
find_file(MORPHO_HEADER
          morpho.h 
          HINTS 
          /usr/local/include/
          /usr/local/include/morpho
        #   /opt/homebrew/include/
        #   /opt/homebrew/include/morpho
        #   /home/linuxbrew/.linuxbrew/include/
        #   /home/linuxbrew/.linuxbrew/include/morpho
          "C:/Program Files/Morpho/include/morpho"
          )

message(STATUS "Found morpho.h at ${MORPHO_HEADER}")

# Identify folder that morpho.h is located in from MORPHO_HEADER and store in MORPHO_INCLUDE
get_filename_component(MORPHO_INCLUDE ${MORPHO_HEADER} DIRECTORY)

# Add morpho headers to MORPHO_INCLUDE
target_include_directories(morphoit PUBLIC ${MORPHO_INCLUDE})
message(STATUS "Including directory ${MORPHO_INCLUDE}")

# Add morpho headers in subfolders to MORPHO_INCLUDE
file(GLOB morpho_subdirectories LIST_DIRECTORIES true ${MORPHO_INCLUDE}/*)
foreach(dir ${morpho_subdirectories})
    IF(IS_DIRECTORY ${dir})
        target_include_directories(morphoit PUBLIC ${dir})
        message(STATUS "Including directory ${dir}")
    ELSE()
        CONTINUE()
    ENDIF()
endforeach()

# Locate libmorpho
find_library(MORPHO_LIBRARY
    NAMES morpho libmorpho
    HINTS
    "C:/Program Files/Morpho/lib/" 
)

target_link_libraries(morphoit ${MORPHO_LIBRARY} dl buildit_lib)

add_subdirectory(src)