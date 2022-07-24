set(DOXYGEN_IN ${CMAKE_CURRENT_LIST_DIR}/Doxyfile)
set(DOXYGEN_OUT ${PROJECT_SOURCE_DIR}/build/docs/Doxyfile)

configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

add_custom_target(docs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating documentation..."
)

set(symlink ${CMAKE_CURRENT_LIST_DIR}/index.html)
set(target ${CMAKE_CURRENT_LIST_DIR}/html/index.html)

add_custom_target(index ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${target} ${symlink}
)
