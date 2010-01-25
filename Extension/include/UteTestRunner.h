#ifndef UTRTESTRUNNER_H
#define UTRTESTRUNNER_H

#include <cppunit/TestRunner.h>
#include <fstream>
#include <iostream>
#include <list>
#include <sys/time.h>

namespace ute {

	class CTestClass {
		public:
			CTestClass();
			CTestClass(std::string name_class, void *handle);
			virtual ~CTestClass();
			virtual bool tc_test_stored_classes(const std::string &csCurrentMethod){ return 0;};

			std::string m_name_class; //!The name of tested class
			void *m_handle;           //!The instance of tested class
	};

	typedef std::list  <CTestClass*> TestClassContainer;  //!The container for tested classes
	typedef TestClassContainer::iterator IterTestClassContainer;  //!The iterator of list testing classes

	class UteTestRunner: public CPPUNIT_NS::TestRunner
	{
		private:
			static UteTestRunner* m_instance; //!The pointer to singleton
			//DELETE
			FILE* m_inputfile; 
			FILE* m_outputfile; //! The file pointer to output file
			FILE* m_redirect_stdout;  //! The file pointer to writing output, user can redirect standard output to other file stream (<*.trc>, etc...) 
			//DELETE
			void utr_parse_cmd (int argc, char* argv[]);
			void utr_print_options();
			void utr_next_step(void);
		public:
			UteTestRunner();
			virtual ~UteTestRunner();
			std::ifstream m_input;
			std::ofstream m_output; 
			std::ofstream m_redirect; 
			static	UteTestRunner* utr_get_instance();
			int m_event_number;   //! The event number of CALL
			bool m_finish;

			virtual void run( CPPUNIT_NS::TestResult &controller, int argc, char* argv[]);
			virtual void addTest( CPPUNIT_NS::Test *test )
			{
				std::cerr<<"UteTestRunner::addTest() unsupported"<<std::endl;
			}

			//		type: input, output, stdout_redirect
			void utr_open_file(const char* file_name, const char* mode, std::string type_file);
			void utr_close_file(std::string type_file);  //!It closes input file
			void utr_print_file_position(FILE* file, std::string type_file);
			void utr_set_file(FILE* file, std::string type);
			FILE* utr_get_file(std::string type_file);               //!It returns file pointer to input file 

			int utr_fprintf (FILE* file, const char * format, ...);
			int utr_vfprintf (FILE* file, const char * format, va_list ap);

			void utr_increase_sim_time(int delta_sec, int delta_u_sec);
			const struct ::timeval* utr_get_simulated_time(void);
			time_t utr_restore_simulated_time(char* date_and_time);

			void utr_store_test_class(CTestClass* TestClass);
			void utr_print_stored_classes(FILE* file_name);
			void utr_delete_test_classes();
			TestClassContainer TestContainer; //!The container for tested classes


	}; // class UteTestRunner

	static inline UteTestRunner* utr() { return UteTestRunner::utr_get_instance(); }



} // namespace ute

#endif // UTRTESTRUNNER_H
