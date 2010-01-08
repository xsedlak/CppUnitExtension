#include "CueTestRunner.h"

namespace cue {

	CueTestRunner::CueTestRunner()
	{
	}


	CueTestRunner::~CueTestRunner()
	{
	}


	void CueTestRunner::run( CPPUNIT_NS::TestResult &controller, const std::string &testPath )
	{
		// testPath is ignored, otherwise we should keep up what's in TestRunner.cpp
		// TestPath path = m_suite->resolveTestPath( testPath );
	}

}
