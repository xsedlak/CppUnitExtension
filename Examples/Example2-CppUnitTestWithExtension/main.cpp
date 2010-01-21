#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <TestClass.h>
#include <UteTestRunner.h>

using namespace std;
using namespace ute;

int main (int argc, char* argv[])
{
  // informs test-listener about testresults
  CPPUNIT_NS :: TestResult testresult;

  // register listener for collecting the test-results
  CPPUNIT_NS :: TestResultCollector collectedresults;
  //	testresult.addListener (&collectedresults);

  // register listener for per-test progress output
  	CPPUNIT_NS :: BriefTestProgressListener progress;
  	testresult.addListener (&progress);

  // insert test-suite at test-runner by registry
  // (cppunit testrunner is replaced by new one UteTestRunner which will execute tests in diff. way)
  ute::UteTestRunner testrunner;
  utr()->utr_store_test_class(new TestClass("BaseTest",NULL));
  testrunner.run (testresult, argc, argv);

  // output results in compiler-format
  CPPUNIT_NS :: CompilerOutputter compileroutputter (&collectedresults, std::cerr);
//  compileroutputter.write ();

  // return 0 if tests were successful
  return collectedresults.wasSuccessful () ? 0 : 1;
}

// vim: ts=2 sw=2 et
