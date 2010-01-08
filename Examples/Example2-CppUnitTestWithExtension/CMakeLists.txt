include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit/include ${CPPUNITEXTENSION_SOURCE_DIR}/Extension/include)
link_directories (${CPPUNITEXTENSION_BINARY_DIR}/CppUnit ${CPPUNITEXTENSION_SOURCE_DIR}/Extension)
add_executable (example2-cppunitwithextension main.cpp)
target_link_libraries (example2-cppunitwithextension CppUnit Extension)
