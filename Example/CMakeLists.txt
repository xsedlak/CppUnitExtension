include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit/include)
link_directories (${CPPUNITEXTENSION_BINARY_DIR}/CppUnit)
add_executable (standardExample main.cpp)
target_link_libraries (standardExample CppUnit)
