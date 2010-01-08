#ifndef CUETESTRUNNER_H
#define CUETESTRUNNER_H

#include <cppunit/TestRunner.h>
#include <iostream>

namespace cue {

	class CueTestRunner: public CPPUNIT_NS::TestRunner
	{

		public:

			CueTestRunner();

			virtual ~CueTestRunner();

			virtual void run( CPPUNIT_NS::TestResult &controller, const std::string &testPath = "" );

			virtual void addTest( CPPUNIT_NS::Test *test )
			{
				std::cerr<<"CueTestRunner::addTest() unsupported"<<std::endl;
			}

	}; // class CueTestRunner

} // namespace cue

#endif // CUETESTRUNNER_H
