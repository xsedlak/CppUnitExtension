#include "UteTestRunner.h"
#include "Reader.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

using namespace std;

namespace ute {

	/******************************************************************************/
	//!Definition for CTestClass
	/******************************************************************************/

	CTestClass::CTestClass():
		m_name_class(""),
		m_handle(0)
	{
	}

	CTestClass::~CTestClass()
	{
		m_name_class = "";
		m_handle = 0;
	}

	CTestClass::CTestClass(string name_class, void *handle)
	{
		m_name_class = name_class;
		m_handle = handle;
	}

	/******************************************************************************/
	//!Simulated time
	/******************************************************************************/
	static struct ::timeval simulated_time;

	void UteTestRunner::utr_increase_sim_time(int delta_sec, int delta_u_sec)
	{
		simulated_time.tv_sec += delta_sec;
		simulated_time.tv_usec += delta_u_sec;
	}

	const struct ::timeval* UteTestRunner::utr_get_simulated_time(void)
	{
		return &simulated_time;
	}

	time_t UteTestRunner::utr_restore_simulated_time(char* date_and_time)
	{
		time_t retval=(time_t)-1;
		static struct tm tms;

		if (sscanf(date_and_time,"%d.%d.%d %d:%d:%d",
					/* date */ &tms.tm_mday,&tms.tm_mon,&tms.tm_year,
					/* time */ &tms.tm_hour,&tms.tm_min,&tms.tm_sec)!=6)
		{
			return retval;
		}
		tms.tm_isdst=-1;
		tms.tm_year-=1900;
		tms.tm_mon-=1;
		retval=mktime(&tms);
		return retval;
	}

	UteTestRunner* UteTestRunner::m_instance = 0;

	UteTestRunner::UteTestRunner():
		m_inputfile(0),
		m_outputfile(0),
		m_redirect_stdout(stdout),
		m_event_number(0),
		m_finish(0)
	{
		m_instance = this;
	}

	UteTestRunner::~UteTestRunner()
	{
		utr_close_file("input");
		utr_close_file("output");
		utr_close_file("redirect");
	}

	UteTestRunner* UteTestRunner::utr_get_instance()
	{
		if (m_instance == 0)
			m_instance = new UteTestRunner();
		return m_instance;
	}

	void UteTestRunner::utr_next_step(void)
	{
		switch (fr()->fr_return_command())
		{
			case CM_NO_COMMAND:
				//		//UTR_TRACE_DETAILED("Event=CM_NO_COMMAND {{{{");
				m_finish = true;
				break;
			case CM_ECHO:
				//UTR_TRACE_DETAILED("Event=CM_ECHO/CM_INFO {{{{");
				//!String that user wants to show on the screen
				utr_fprintf(utr_get_file("redirect"),"%s\n",fr()->m_param_string.c_str());
				break;
				/*
					 case CM_COMMENT:
				//UTR_TRACE_DETAILED("Event=CM_COMMENT {{{{");
				//!String that will be written to output_file
				m_output << fr()->m_param_string << endl;
				break;
				*/
			case CM_SLEEP:
				//UTR_TRACE_DETAILED("Event=CM_SLEEP {{{{");
				// sleep for m_param_int seconds
				if (fr()->m_param_int <= 0) 
				{
					utr_fprintf(utr_get_file("redirect"),"The number after $SLEEP must be a positive value in seconds; so I'm not gonna sleep\n");
				} else
				{
					utr_fprintf(utr_get_file("redirect"),"$SLEEP reached in the file: %s [%d]\n\tUTE Runer will wait for %d seconds before next command!\n",fr()->m_files[fr()->m_current_file].file_name.c_str(), fr()->m_line, fr()->m_param_int);
					sleep(fr()->m_param_int);
				}
				break;
			case CM_SLEEP_SIM:
				//UTR_TRACE_DETAILED("Event=CM_SLEEP_SIM {{{{");
				if (fr()->m_param_int <= 0) 
				{
					utr_fprintf(utr_get_file("redirect"),"The number after $SLEEP_SIM must be a positive value in seconds; so I'm not gonna simulate sleep\n");
				} else utr_increase_sim_time(fr()->m_param_int, 0);
				break;
			case CM_EXEC: // $EXEC "command args ... for sh"[,STD][,OUT][,TRC]
				//UTR_TRACE_DETAILED("Event=CM_EXEC {{{{");
				{
					bool write_to_std=false;
					bool write_to_out=false;
					bool write_to_trc=false;
					string command;
					if (fr()->fr_read_exec_command((const char*)fr()->m_buffer, command, write_to_std, write_to_out, write_to_trc))
					{
						//export environment variable UTR_RAND=rand()
						//if (getenv("UTR_RAND")) utr_fprintf(utr_get_file("redirect"), "Warning: testdriver shall overwrite environment variable UTR_RAND!\n");;
						char buf[12];
						snprintf(buf,12,"%010d",(rand() << 16) ^ rand());
						setenv("UTR_RAND",buf,1);

						FILE* p=popen(command.c_str(),"r");
						if (p)
						{
							char buf[1024];
							size_t len;
							string res;
							// read all from pipe
							while ( (len=fread(buf,1,sizeof(buf)-1,p))>0 )
							{
								if (res.empty())
									res="$EXEC output: \n\t";
								char *b=buf;
								while (static_cast<size_t>(b-buf)<len)
								{
									if (*b=='\0')
										*b=' ';
									if (*b=='\n')
										res+="\n\t";
									//									res+="\n$EXEC output: ";
									else
										res+=*b;
									b++;
								}
							}
							// put data read to output or stdout
							if (write_to_out)
							{
								utr_fprintf(utr_get_file("output"),"%s\n", res.c_str());
							}
							res+="\n";
							if (write_to_std) utr_fprintf(utr_get_file("redirect"), "%s", res.c_str());
							int retval=pclose(p);
							if (retval)
							{
								utr_fprintf(utr_get_file("redirect"), "ERROR: Program \"%s\" exited with code=%d (full code from pclose()=%d)\n", command.c_str(), (retval >> 8), retval);
								if (retval<0)
									utr_fprintf(utr_get_file("redirect"), "(errno=%d\"%s\") in the file:  %s [%d]\n", errno, strerror(errno),  fr()->m_files[fr()->m_current_file].file_name.c_str(), fr()->m_line);
								exit(1);
							}
						} else
						{
							utr_fprintf(utr_get_file("redirect"), "ERROR: Unable to exec \"%s\", errno=%d, %s in the file: %s [%d]\n", command.c_str(), errno, strerror(errno), fr()->m_files[fr()->m_current_file].file_name.c_str(), fr()->m_line);
							exit(1);
						}
					} else
					{
						utr_fprintf(utr_get_file("redirect"), "ERROR: Unable to process $EXEC command in the file: %s [%d]\n", fr()->m_files[fr()->m_current_file].file_name.c_str(), fr()->m_line);
						exit(1);
					}
				}
				break;
			case CM_CALL:
				//UTR_TRACE_DETAILED("Event=CM_CALL {{{{");
				CTestClass* pCurrentClass=0;
				bool ret_tc_test_container = false;
				//UTR_DEBUG_ENTER("TestDriver.cxx", utr_get_inputfile(), "next_step", "CALL");

				//!Call of the function at first, put the number of event to global variable, so all dummies can access it
				m_event_number = fr()->m_param_int;

				//!Try to find the actual class among the classes passed to TestDriver
				//!Debug trace
				size_t i = fr()->m_param_string.find_first_of ("::",0);
				if (i == string::npos)
				{
					utr_fprintf(utr_get_file("redirect"),"Class and method must be delimited by '::'. ERROR in directive: %s\n", fr()->m_param_string.c_str());
					exit (1);
				}

				string sCurrentClass = fr()->m_param_string.substr(0,i).c_str();
				string sCurrentMethod = fr()->m_param_string.c_str()+i+2;

				for ( IterTestClassContainer it=TestContainer.begin(); it!=TestContainer.end(); it++) {

					if ((*it)->m_name_class == sCurrentClass)
					{
						pCurrentClass = (*it);
					}
				}
				if (pCurrentClass == 0 )
				{
					utr_fprintf(utr_get_file("redirect"),"\nERROR: Unexpected class name in $CALL directive: %s\n", fr()->m_param_string.c_str());
					exit(1);
				}

				//UTR_DEBUG("Current class:    " << sCurrentClass);
				//UTR_DEBUG("Current method:   " << sCurrentMethod);
				//UTR_DEBUG("detail_output:    " << detail_output);

				else //UTR_DEBUG("You haven't registered plugin for starting sapi transaction!\n");
				ret_tc_test_container=pCurrentClass->tc_test_stored_classes(sCurrentMethod);
				//UTR_DEBUG("ret_tc_test_container:\t"<< ret_tc_test_container);

				//UTR_DEBUG_LEAVE("TestDriver.cxx", next_step, "END_CALL");
		} // switch (fr()->fr_return_command())
	}

	void UteTestRunner::run( CPPUNIT_NS::TestResult &controller, int argc, char* argv[])
	{
		// testPath is ignored, otherwise we should keep up what's in TestRunner.cpp
		// TestPath path = m_suite->resolveTestPath( testPath );
		utr_parse_cmd(argc, argv);
		do
		{
			utr_next_step();
		}
		while (!(m_finish));
		m_output << "Test Started" << endl;

	}

	void UteTestRunner::utr_parse_cmd (int argc, char* argv[])
	{
		string output("");
		string input("");
		string redirect("");
		int retval;

		while(1)
		{
			retval = getopt(argc, argv, ":i:o:r:");
			if(retval == -1)
			{
				break; // all option parsed
			}
			switch(retval)
			{
				case 'i': 
					input = optarg; 
					//					cout << "[UTR] -i: " << input << " (input file)" << endl;
					break; //in file
				case 'o': 
					output = optarg; 
					//					cout << "[UTR] -o: " << output  << " (output file)" << endl;
					break; //out file
				case 'r': 
					redirect = optarg; 
					//					cout << "[UTR] -r: sets redirecting output from stdout to " <<  redirect <<  endl;
					break;
				case ':':
					fprintf(stderr, "ERROR: Option \"-%c\" requires an value\n", optopt);
					utr_print_options();
					exit(1);
					break;
				case '?':
					fprintf(stderr, "ERROR: Unrecognized option: \"-%c\"\n", optopt);
					utr_print_options();
					exit(1);
			}
		}

		//!Open files
		if (""!=input)
		{	
			utr_open_file(input.c_str(), "r", "input");
			cout << "INPUT FILE:\t\t" << input << endl;
		}

		if (""!=output)
		{
			utr_open_file(output.c_str(), "w+","output");
			cout << "OUTPUT FILE:\t\t" << output << endl;
		}

		if (""!=redirect)
		{
			cout << "REDIRECT FILE:\t\t" << redirect << endl;
			utr_open_file(redirect.c_str(), "w+","redirect");
		}
		cout << endl;
		//CHI
		fr()->fr_init(utr_get_file("input"), input.c_str());
	}


	void UteTestRunner::utr_open_file(const char* file_name, const char* mode,std::string type_file)
	{
		if (type_file.compare("input") == 0)
		{
			//CHI			m_input.open(file_name, ifstream::in);
			if (0 == ( m_inputfile = (FILE*) fopen (file_name, mode)))
			{
				utr_fprintf(m_redirect_stdout,"ERROR: Can't open input file: '%s'!\n",file_name);
				exit(1);
			}
		} else if (type_file.compare("output") == 0)
		{
			//CHI			m_output.open(file_name);
			if (0 == (m_outputfile = (FILE*) fopen (file_name, mode)))
			{
				utr_fprintf(m_redirect_stdout,"ERROR while opening output file: '%s'!\n",file_name);
				exit(1);
			}
		} else if (type_file.compare("redirect") == 0)
		{
			//			m_redirect.open(file_name);
			if (0 == (m_redirect_stdout = (FILE*) fopen (file_name, mode)))
			{
				m_redirect_stdout=stdout;
				utr_fprintf(m_redirect_stdout,"ERROR while opening redirect_stdout file: '%s'!\n", file_name);
				exit(1);
			}
		}
		//		cout << "[UTR] utr_open_file: " << type_file << endl;
	}

	void UteTestRunner::utr_close_file(std::string type_file)
	{
		if (type_file.compare("input") == 0)
		{
			if (m_inputfile != 0)
			{
				fclose(m_inputfile);
				m_inputfile = 0;
			}
		} else if (type_file.compare("output") == 0)
		{
			if (m_outputfile != 0)
			{
				fclose(m_outputfile);
				m_outputfile = 0;
			}
		} else if (type_file.compare("redirect") == 0)
		{
			if (m_redirect_stdout != 0)
			{
				fclose(m_redirect_stdout);
				m_redirect_stdout = 0;
			}
		}
		//		cout << "[UTR] utr_close_file: " << type_file << endl;
	}

	void UteTestRunner::utr_print_file_position(FILE* file, std::string type_file)
	{
		if (type_file.compare("input") == 0)
		{
			utr_fprintf(file, "INPUT FILE: %s [%d]\n", fr()->m_files[fr()->m_current_file].file_name.c_str(), fr()->m_line);
		}
	}

	void UteTestRunner::utr_print_options()
	{
		printf ("\nNAME\n");
		printf ("   testprogram - interface between Unit Test Environment and CppUnit (Unit Test Framework)\n");
		printf ("\nSYNOPSIS\n");
		printf ("   testprogram -i INPUT_FILE [-o OUTPUT_FILE] [-r REDIRECT_STDOUT]\n"); 
		printf ("\n\nDESCRIPTION\n");
		printf ("   Unit Test Envinronment Test Runner is using for testing functions\n");
		printf ("\n\nOPTIONS\n");
		printf ("   -i INPUT_FILE name of the file containing your testcase(s) - file with suffix in\n");
		printf ("   -o OUTPUT_FILE name of the file where dummies write their results - file with suffix out\n");
		printf ("   -r REDIRECT_STDOUT name of the file containing lines which should be written on the stdout by UteTestRunner\n");
		printf ("\n");
	}

	FILE* UteTestRunner::utr_get_file(std::string type_file)
	{
		if (type_file.compare("input") == 0)
			return m_inputfile;
		//CHI			return m_input;
		else if (type_file.compare("output") == 0)
			return m_outputfile;
		//CHI			return m_output;
		else if (type_file.compare("redirect") == 0)
			return m_redirect_stdout;
		//CHI			return m_redirect_stdout;
		else
			cout << "The type of file is not defined: \"" << type_file << "\"" << endl;
		return 0;
	}

	int UteTestRunner::utr_fprintf (FILE* file, const char * format, ...)
	{

		va_list arg_list;
		int ret_value = 0;
		if( NULL == file)
		{
			return ret_value;
		}

		va_start (arg_list, format);
		ret_value = utr_vfprintf (file, format, arg_list);  
		va_end (arg_list);

		return ret_value;
	}

	int UteTestRunner::utr_vfprintf (FILE* file, const char * format, va_list ap)
	{
		int ret_value = 0;

		if( NULL == file)
		{
			return ret_value;
		}

		ret_value = vfprintf (file, format, ap);

		/*  if (  ((copy_output_to_screen) && (file!=traceoutputfile))
				||((copy_trace_to_screen) && (file==traceoutputfile)))
				{    
				vfprintf (stdout, format, ap);
				}    
				*/
		return ret_value;
	}

	void UteTestRunner::utr_store_test_class(CTestClass* TestClass)
	{
		TestContainer.push_back(TestClass);
	}

	void UteTestRunner::utr_print_stored_classes(FILE* file_name)
	{
		int i = 0;
		if (TestContainer.empty())
			fprintf(file_name, "TD: Weren't registered any classes!\n"); 
		else
		{
			fprintf(file_name,"TD: Registered CLASSES for testing:| %-5s | %-30s | %-20s |\n","ORDER","NAME","INSTANCE");
			//CHI		td_print_table(file_name,"TD: Registered CLASSES for testing:","| %-5s | %-30s | %-20s |\n","ORDER","NAME","INSTANCE");
			for (IterTestClassContainer it = TestContainer.begin(); it != TestContainer.end(); it++)
				fprintf(file_name, "| %-5d | %-30s | %-20p |\n", i++, (*it)->m_name_class.c_str(), (*it)->m_handle);
			//CHI			td_print_table_line(file_name,"| %-5d | %-30s | %-20p |\n", i++, (*it)->m_name_class.c_str(), (*it)->m_handle);
		}
	}

	void UteTestRunner::utr_delete_test_classes()
	{
		for (IterTestClassContainer it = TestContainer.begin(); it != TestContainer.end(); it++)
			delete *it;
	}

}
