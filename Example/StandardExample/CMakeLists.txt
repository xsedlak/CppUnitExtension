add_subdirectory (StandardExample)

include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit/include ${CPPUNITEXTENSION_SOURCE_DIR}/Extension/include)
link_directories (${CPPUNITEXTENSION_BINARY_DIR}/CppUnit ${CPPUNITEXTENSION_BINARY_DIR}/Extension)
add_executable (example main.cpp StandardExample.cpp)
target_link_libraries (example CppUnit Extension)
