#-------------------------------------------------------------------------------
# Generate runtime header
#-------------------------------------------------------------------------------

set(RUNTIME_HEADERS
    ${CMAKE_SOURCE_DIR}/src/pe_vm_consts.h
    ${CMAKE_SOURCE_DIR}/src/runtime_header.c
)

set(GENERATED_H ${CMAKE_BINARY_DIR}/src/generated_header.h)

add_custom_command(
    OUTPUT ${GENERATED_H}
    COMMAND ${CMAKE_COMMAND}
        -D OUTPUT_FILE=${GENERATED_H}
        -D INPUT_FILES="${RUNTIME_HEADERS}"
        -P ${CMAKE_SOURCE_DIR}/src/GenerateHeader.cmake
    DEPENDS ${RUNTIME_HEADERS}
)

add_custom_target(generate_header DEPENDS ${GENERATED_H})
add_dependencies(morphoit generate_header)

target_include_directories(morphoit PRIVATE ${CMAKE_BINARY_DIR}/src)

#-------------------------------------------------------------------------------
# Add target sources
#-------------------------------------------------------------------------------

target_sources(morphoit
    PRIVATE
        main.cpp
        value.cpp
        pe_vm.cpp
        runtime.cpp
        userfn.cpp
)