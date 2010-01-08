include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit/include)
link_directories (${CPPUNITEXTENSION_BINARY_DIR}/CppUnit)
add_executable (example1-classiccppunit Fraction.cpp FractionTest.cpp main.cpp)
target_link_libraries (example1-classiccppunit CppUnit)
