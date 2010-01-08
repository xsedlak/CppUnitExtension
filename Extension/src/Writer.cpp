#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#include "TestDriver.h"
#include "DummyTrace.h"
#include "TdTrace.h"

#ifdef IMS_MULTI_THREAD
#include <pthread.h>
#endif

#define BUFFER_SIZE 128000

FILE * traceoutputfile = stdout;

#ifdef IMS_MULTI_THREAD
	pthread_mutex_t s_dummy_trace_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

using namespace testdriver;
typedef char*(*array)[];

int dummy_trace_aux (const char * event, int number_of_arguments, va_list& arg_list)
{
	CTestDriver* singleton_test_driver = CTestDriver::td_get_instance();
#ifdef ENABLE_QUANTIFY

	if (quantify_active) quantify_stop_recording_data();
#endif
	DummyArgType argument;
	char * arg_char_ptr;
	va_list passed_arg_list;
	int i, j;
	bool last_was_char;

	int arg_int;
	char arg_char;
	unsigned int arg_uint;
	unsigned int* arg_uint_list;
	time_t arg_time_t;


	array arg_char_ptr_array;
	char * arg_format;
	void * arg_ptr;
	time_t ltime;
	int length;

	/* Get UNIX-style time and display as number and string. */
	time( &ltime);


	if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "================================================================================\n"))
		return 1;

	if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "Event: %s\n", event))
		return 1;

	/* please use this: printf( "UNIX time and date:\t\t\t%s", ctime( &ltime ) );*/
	if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "Time: %s", ctime (&ltime)))
		return 1;

	if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "================================================================================\n"))
		return 1;
	fflush(0);

	D_ERR_DEBUG_DELIMETER("EVENT:								" << event << "\nnumber_of_arguments: "<< number_of_arguments);
	for (i=0; i<number_of_arguments; i++)
	{

		argument = va_arg (arg_list, DummyArgType);
		D_ERR_DEBUG("argument["<< i <<"]: "<< argument);

		char *decomp_msg = NULL;


		switch (argument)
		{
		case ARG_CHAR:
			arg_char =	(char) va_arg (arg_list, char);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%c\n", arg_char)) return 1;
			break;
		case ARG_INT:
			arg_int =  (int) va_arg (arg_list, int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%d\n", arg_int)) return 1;
			break;
		case ARG_BOOL:
			arg_int =  (int) va_arg (arg_list, int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%s\n", arg_int?"True":"False")) return 1;
			break;
		case ARG_INT32:
			arg_int =  (int) va_arg (arg_list, int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%ld\n", arg_int)) return 1;
			break;
		case ARG_UINT32:
			arg_uint =	(unsigned int) va_arg (arg_list, unsigned int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%ld\n", arg_uint)) return 1;
			break;
		case ARG_UINT8:
			arg_int =  (int) va_arg (arg_list, int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%c\n", arg_int)) return 1;
			break;
		case ARG_UINT32_HEXA:
			arg_uint =	(unsigned int) va_arg (arg_list, unsigned int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%8.8lX\n", arg_uint)) return 1;
			break;
		case ARG_UINT8_HEXA:
			arg_int =  (int) va_arg (arg_list, int);
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%2.2X\n", arg_int)) return 1;
			break;
		case ARG_UINT8_PTR:
			arg_int =  (int) va_arg (arg_list, int);
			arg_char_ptr =	va_arg (arg_list, char *);
			for (j=0; j<arg_int; j++)
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%c", arg_char_ptr[j])) return 1;
			}
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			break;
		case ARG_UINT8_PTR_HEXA:
			arg_int =  (int) va_arg (arg_list, int);
			arg_char_ptr =	va_arg (arg_list, char *);
			if (NULL==arg_char_ptr)
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
			}
			else
			{
				for (j=0; j<arg_int; j++)
				{

					if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), " 0x%.2X", (unsigned char)arg_char_ptr[j])) return 1;
				}
			}
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			break;
		case ARG_UINT8_PTR_HEXA_COMBI:
			arg_int =  (int) va_arg (arg_list, int);
			last_was_char = false;

			if (td()->m_pSigCompPlugin != 0)
			{
					TD_DEBUG("SigComp dependences | case ARG_UINT8_PTR_HEXA_COMBI");



					arg_char_ptr = td()->m_pSigCompPlugin->bscp_sigcomp_msg_decompress(arg_list, arg_int);

					if ((strlen(arg_char_ptr) == ((size_t)arg_int - 1))||(size_t)(strlen(arg_char_ptr) == (size_t)arg_int))  arg_int = (int)strlen(arg_char_ptr);
					else arg_int--;



			}
			else arg_char_ptr =  va_arg (arg_list, char *);




			if (NULL==arg_char_ptr)
			{
					if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
			}
			else
			{
					for (j=0; j<arg_int; j++)
					{

							if ( ((31 < (unsigned char)arg_char_ptr[j]) && ( 127 > (unsigned char)arg_char_ptr[j])) ||
									 ( ((10 == (unsigned char)arg_char_ptr[j]) || (13 == (unsigned char)arg_char_ptr[j])) &&
										 (0 < j) &&
										 ( ((31 < (unsigned char)arg_char_ptr[j-1]) && ( 127 > (unsigned char)arg_char_ptr[j-1])) ||
											 (10 == (unsigned char)arg_char_ptr[j-1]) || (13 == (unsigned char)arg_char_ptr[j-1])) ) )
							{

									if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%c", arg_char_ptr[j])) return 1;
									last_was_char = true;
							}
							else
							{


									if (last_was_char)
									{
											if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), " ")) return 1;
											last_was_char = false;
									}




									if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "0x%.2X ", (unsigned char)arg_char_ptr[j])) return 1;
							}
					}
			}
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			if (decomp_msg != NULL) free(decomp_msg);
			break;
		case ARG_UINT8_PTR_HEXA_NO_NEWLINE:
			arg_int =  (int) va_arg (arg_list, int);
			last_was_char = false;

			if (td()->m_pSigCompPlugin != 0)
			{
					TD_DEBUG("SigComp dependences | case ARG_UINT8_PTR_HEXA_COMBI");



					arg_char_ptr = td()->m_pSigCompPlugin->bscp_sigcomp_msg_decompress(arg_list, arg_int);

					if ((strlen(arg_char_ptr) == ((size_t)arg_int - 1))||(size_t)(strlen(arg_char_ptr) == (size_t)arg_int))  arg_int = (int)strlen(arg_char_ptr);
					else arg_int--;



			}
			else arg_char_ptr =  va_arg (arg_list, char *);




			if (NULL==arg_char_ptr)
			{
					if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
			}
			else
			{
					for (j=0; j<arg_int; j++)
					{

			if ( ((31 < (unsigned char)arg_char_ptr[j]) && ( 127 > (unsigned char)arg_char_ptr[j])))
							{

									if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%c", arg_char_ptr[j])) return 1;
									last_was_char = true;
							}
							else
							{


									if (last_was_char)
									{
											if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), " ")) return 1;
											last_was_char = false;
									}




									if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "0x%.2X ", (unsigned char)arg_char_ptr[j])) return 1;
							}
					}
			}
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			if (decomp_msg != NULL) free(decomp_msg);
			break;
		case ARG_UINT8_PTR_HEXA_ENHANCED:
		{
				arg_int =  (int) va_arg (arg_list, int);
				arg_char_ptr =	va_arg (arg_list, char *);
				if (NULL == arg_char_ptr)
				{
						if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
				}
				else
				{
							char row[100];
						int  i,j;
						for (i = 0; i < arg_int;)
							{
								memset(row, ' ', 80);

										for (j = 0; j < 16 && i < arg_int; i++, j++)
								{
											unsigned char c = 0xff & arg_char_ptr[i];
											sprintf(&row[j*3+7], "%02X ", c);
											row[j+58] = isgraph(c) ? c : '.';
										}
										sprintf(row, "%4d:", i);


										row[5]		 = ' ';
										row[3*j+7] = ' ';

										if (arg_int != i)
										{
											row[58+j]  = '\n';
										}
										row[59+j]  = '\0';
										if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%s", row)) return 1;
						}
						if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
				}
		}
		break;
		case ARG_PTR:
			arg_ptr =  va_arg (arg_list, void *);
			if (NULL!=arg_ptr)
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%p\n", arg_ptr)) return 1;
			}
			else
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
			}
			break;
		case ARG_CHAR_PTR:
			arg_char_ptr =	va_arg (arg_list, char*);
			if (NULL==arg_char_ptr)
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
			}
			else
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%s\n", arg_char_ptr)) return 1;
			}
			break;
		case ARG_CHAR_PTR_ARRAY:
			arg_char_ptr_array =	va_arg (arg_list, array);
			if (NULL == arg_char_ptr_array)
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "NULL\n")) return 1;
				break;
			}
			j=0;
			while (NULL != (*arg_char_ptr_array)[j])
			{
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%s\n", (*arg_char_ptr_array)[j])) return 1;
				j++;
			}
			break;
		case ARG_TIME_T:
			arg_time_t =	va_arg (arg_list, time_t);

			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "%s", ctime(&arg_time_t))) return 1;
			break;
		case ARG_USER:



			if (i<number_of_arguments-1)
			{
				arg_format =	va_arg (arg_list, char*);
				arg_ptr =  va_arg (arg_list, void*);

				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), arg_format, arg_ptr)) return 1;
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			}
			else
			{
				arg_format =	va_arg (arg_list, char*);
				if (0==my_vfprintf (singleton_test_driver->td_get_outputfile(), arg_format, arg_list)) return 1;
				if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			}
			break;
		case ARG_VAR:
			arg_format =	va_arg (arg_list, char*);
			passed_arg_list = va_arg (arg_list, va_list);
			if (0==my_vfprintf (singleton_test_driver->td_get_outputfile(), arg_format, passed_arg_list)) return 1;
			if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "\n")) return 1;
			break;
		case ARG_INVISIBLE_PTR:
			arg_ptr =  va_arg (arg_list, void *);
			break;
		default:


				IterDummyArgumentContainer IterTypeContainer = singleton_test_driver->TypeContainer.find(argument);



				if ((IterTypeContainer != singleton_test_driver->TypeContainer.end()) && (argument != -1))
				{


						ostrstream ostring;

						my_fprintf (singleton_test_driver->td_get_outputfile(),"%s", (IterTypeContainer->second)->da_dummy(ostring, arg_list).str());
						ostring.freeze(0);
				}
				else {
						my_fprintf (singleton_test_driver->td_get_outputfile(), "ERROR: Unsupported type to trace!\nEvent: %s\n\
												Number of arguments: %d\nType of argument: %d\nSpecified argument count is probably higher then \
												actual number of passed arguments\n", event, number_of_arguments, argument);
						my_fprintf(td()->td_get_redirect_stdout(), "%sEVENT: \"%s\"\n", TABLE_SEPARATOR, event);
						my_fprintf(td()->td_get_redirect_stdout(), "NUMBER of argument: \"%d\"\n",number_of_arguments);
						my_fprintf(td()->td_get_redirect_stdout(), "TYPE of argument: %s\n%s", argument, TABLE_SEPARATOR);
						exit (1);
				}
		break;
		}

		if (0==my_fprintf (singleton_test_driver->td_get_outputfile(), "--------------------------------------------------------------------------------\n")) return 1;
		fflush(0);
	}



#ifdef ENABLE_QUANTIFY

	if (quantify_active) quantify_start_recording_data();
#endif
	return 0;
}

int dummy_trace (const char * event, int number_of_arguments, ...)
{
#ifdef IMS_MULTI_THREAD
	pthread_mutex_lock(&s_dummy_trace_lock);
#endif
	va_list arg_list;
	va_start(arg_list,number_of_arguments);
	int retval=dummy_trace_aux(event,number_of_arguments,arg_list);
	va_end(arg_list);
#ifdef IMS_MULTI_THREAD
	pthread_mutex_unlock(&s_dummy_trace_lock);
#endif
	return retval;
}

#ifdef __cplusplus
extern "C"
{
#endif

int copy_output_to_screen = 0;
int copy_trace_to_screen = 0;


int my_fprintf (FILE* file, const char * format, ...)
{
	va_list arg_list;
	int ret_value = 0;
	if( NULL == file)
	{
		return ret_value;
	}

	va_start (arg_list, format);
	ret_value = my_vfprintf (file, format, arg_list);
	va_end (arg_list);

	return ret_value;
}

int my_vfprintf (FILE* file, const char * format, va_list ap)
{
	int ret_value = 0;

	if( NULL == file)
	{
		return ret_value;
	}

	ret_value = vfprintf (file, format, ap);




	if (	((copy_output_to_screen) && (file!=traceoutputfile))
			||((copy_trace_to_screen) && (file==traceoutputfile)))
	{
		vfprintf (stdout, format, ap);
	}

	return ret_value;
}

int dummy_trace_c (const char * event, int number_of_arguments, ...)
{
#ifdef IMS_MULTI_THREAD
	pthread_mutex_lock(&s_dummy_trace_lock);
#endif
	va_list arg_list;
	va_start(arg_list,number_of_arguments);
	int retval=dummy_trace_aux(event,number_of_arguments,arg_list);
	va_end(arg_list);
#ifdef IMS_MULTI_THREAD
	pthread_mutex_unlock(&s_dummy_trace_lock);
#endif
	return retval;
}

#ifdef __cplusplus
}

#endif

// vim: ts=4 sw=4 tw=0 noet
