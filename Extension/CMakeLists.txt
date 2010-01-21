include_directories (${CPPUNITEXTENSION_SOURCE_DIR}/CppUnit/include ${CPPUNITEXTENSION_SOURCE_DIR}/Extension/include)
add_library (Reader src/Reader.cpp)
add_library (Writer src/Writer.cpp)
add_library (UteTestRunner src/UteTestRunner.cpp)
