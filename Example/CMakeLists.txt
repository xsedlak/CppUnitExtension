include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit ${CPPUNITEXTENSION_SOURCE_DIR}/CppUnitExtension)
link_directories (${CPPUNITEXTENSION_BINARY_DIR}/CppUnit ${CPPUNITEXTENSION_BINARY_DIR}/CppUnitExtension)
add_executable (CppUnitExtensionExample main.cpp)
target_link_libraries (CppUnitExtensionExample CppUnit CppUnitExtension)
