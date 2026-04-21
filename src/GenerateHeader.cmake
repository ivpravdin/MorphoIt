# Cross-platform CMake script to generate embedded header file
# Usage: cmake -D OUTPUT_FILE=<path> -D INPUT_FILES=<files> -P GenerateHeader.cmake

# Ensure output directory exists
get_filename_component(OUTPUT_DIR ${OUTPUT_FILE} DIRECTORY)
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Start generating the header
file(WRITE ${OUTPUT_FILE} "#pragma once\n")
file(APPEND ${OUTPUT_FILE} "// Auto-generated header file - do not edit\n")
file(APPEND ${OUTPUT_FILE} "constexpr char header[] = {\n")

# Convert INPUT_FILES string to list (space-separated from CMake command line)
string(REPLACE " " ";" INPUT_FILE_LIST "${INPUT_FILES}")

foreach(input_file ${INPUT_FILE_LIST})
    if(NOT EXISTS ${input_file})
        message(FATAL_ERROR "Input file not found: ${input_file}")
    endif()

    # Read file contents in hex format
    file(READ ${input_file} FILE_CONTENT HEX)

    # Convert each pair of hex digits to a byte literal
    string(LENGTH ${FILE_CONTENT} FILE_LENGTH)
    math(EXPR FILE_LENGTH_PAIRS "${FILE_LENGTH} / 2")

    set(idx 0)
    set(byte_count 0)
    while(idx LESS FILE_LENGTH_PAIRS)
        math(EXPR byte_offset "${idx} * 2")
        string(SUBSTRING ${FILE_CONTENT} ${byte_offset} 2 byte_hex)
        file(APPEND ${OUTPUT_FILE} "0x${byte_hex}, ")
        math(EXPR idx "${idx} + 1")
        math(EXPR byte_count "${byte_count} + 1")

        # Add newline every 16 bytes for readability
        math(EXPR line_pos "${byte_count} % 16")
        if(line_pos EQUAL 0)
            file(APPEND ${OUTPUT_FILE} "\n")
        endif()
    endwhile()

    # Add newline byte (0x0a)
    file(APPEND ${OUTPUT_FILE} "0x0a, ")
    math(EXPR byte_count "${byte_count} + 1")
    math(EXPR line_pos "${byte_count} % 16")
    if(line_pos EQUAL 0)
        file(APPEND ${OUTPUT_FILE} "\n")
    endif()
endforeach()

# Terminate the array with null terminator
file(APPEND ${OUTPUT_FILE} "0x00\n")
file(APPEND ${OUTPUT_FILE} "};\n")

message(STATUS "Generated header file: ${OUTPUT_FILE}")
