include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit/include)
include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/Extension/include ./)
link_directories (${CPPUNITEXTENSION_SOURCE_DIR}/Extension)
link_directories (${CPPUNITEXTENSION_BINARY_DIR}/CppUnit)
add_executable (../exec/example2-cppunitwithextension main.cpp TestClass.cpp)
target_link_libraries (../exec/example2-cppunitwithextension UteTestRunner Reader Writer CppUnit)

