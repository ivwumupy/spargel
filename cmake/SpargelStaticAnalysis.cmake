if (SPARGEL_ENABLE_CLANG_TIDY)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/sources.txt
        COMMAND # extract source files
            jq -r '.[].file' ${CMAKE_BINARY_DIR}/compile_commands.json | sort | uniq > ${CMAKE_BINARY_DIR}/sources.txt
        COMMAND # do not include sources in build directory
            sed -i '/build/d' ${CMAKE_BINARY_DIR}/sources.txt
    )

    add_custom_target(
        clang_tidy
        COMMAND
            clang-tidy
                --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy
                --export-fixes=${CMAKE_BINARY_DIR}/clang-tidy-fixes.yml
                -p ${CMAKE_BINARY_DIR} @${CMAKE_BINARY_DIR}/sources.txt
                -header-filter=.*
        DEPENDS
            ${CMAKE_BINARY_DIR}/sources.txt
    )

endif ()
