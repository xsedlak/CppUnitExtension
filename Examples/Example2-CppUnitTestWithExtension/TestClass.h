#ifndef TESTCLASS_H
#define TESTCLASS_H

#include "UteTestRunner.h"

namespace ute {

	class TestClass: public CTestClass {	
		public:
			TestClass(std::string sClassName,void *tc) : CTestClass(sClassName, tc) {}
			virtual bool tc_test_stored_classes(const std::string &csCurrentMethod);
	};

} // namespace ute

#endif // TESTCLASS_H
