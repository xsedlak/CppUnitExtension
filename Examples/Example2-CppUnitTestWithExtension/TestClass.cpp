#include "Reader.h"
#include "TestClass.h"

using namespace ute;
using namespace std;

bool TestClass::tc_test_stored_classes(const string &csCurrentMethod)
{
	if (0 == csCurrentMethod.compare ("test01"))
	{
		string local_param_string = fr()->fr_read_string();		 
		int local_param_int = fr()->fr_read_int();		 
		bool local_param_bool = fr()->fr_read_bool();		 
		utr()->utr_fprintf(utr()->utr_get_file("output"),"test01(%s,%d,%d)\n",
				local_param_string.c_str(),
				local_param_int,
				local_param_bool);
		return true;  
	} else if (0 == csCurrentMethod.compare ("test02"))
	{
		int order_of_event = fr()->fr_read_int();
		string event_name = fr()->fr_read_string();
		utr()->utr_fprintf(utr()->utr_get_file("output"),"test02(%s,%d)\n",
				event_name.c_str(),
				order_of_event);
		return true;
	}
	return false;
}
