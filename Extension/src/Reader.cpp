#include "Reader.h"
#include "UteTestRunner.h"

using namespace std;

namespace ute {

	const int MAX_CAT_TYPES=10;

	string commands[]={
		"",
		"$ECHO",
		"$CALL",
		"$INT",
		"$STRING",
		"$BOOL",
		"$SLEEP",
		"$SLEEP_SIM",
		"$EXEC",
		""
	};

	CStringTokenizer::CStringTokenizer(const char *str, const char *delimiters, bool returnTokens)
	{
		init(str, delimiters, returnTokens);
	}

	CStringTokenizer::CStringTokenizer(const string &str, const string &delimiters, bool returnTokens)
	{
		init(str.c_str(), delimiters.c_str(), returnTokens);
	}

	CStringTokenizer::CStringTokenizer(const char *str)
	{
		init(str, "\t ", false);
	}

	CStringTokenizer::CStringTokenizer(const string &str)
	{
		init(str.c_str(), "\t ", false);
	}

	CStringTokenizer::CStringTokenizer()
	{
		init("", "\t ", false);
	}

	void CStringTokenizer::st_new_string(const char *str)
	{
		init(str, m_delims.c_str(), m_ret_tok);
	}

	void CStringTokenizer::st_new_string(const string &str)
	{
		st_new_string(str.c_str());
	}

	bool CStringTokenizer::st_has_more_tokens()
	{
		st_skip_delimiters();
		return (m_curr_pos < m_max_pos);
	}

	string CStringTokenizer::st_next_token()
	{
		st_skip_delimiters();

		if (m_curr_pos >= m_max_pos)
		{
			R_DEBUG("No such element, all elements were skipped as delimiters!","");
			return "";
		}

		size_t start = m_curr_pos;
		while ((m_curr_pos < m_max_pos) && ((size_t)m_delims.find(m_str.at((size_t)m_curr_pos)) == string::npos))
		{
			m_curr_pos++;
		}
		if (m_ret_tok && (start == m_curr_pos) && (m_delims.find(m_str.at((size_t)m_curr_pos)) != string::npos))
		{
			m_curr_pos++;
		}
		return m_str.substr((size_t)start, (size_t)(m_curr_pos - start));
	}

	string CStringTokenizer::st_next_token(string &delims)
	{
		return st_next_token(delims.c_str());
	}

	string CStringTokenizer::st_next_token(const char *delims)
	{
		m_delims = delims;
		return st_next_token();
	}

	string CStringTokenizer::st_next_all()
	{
		st_skip_delimiters();

		if (m_curr_pos >= m_max_pos)
		{
			return "";
		}

		size_t start = m_curr_pos;
		m_curr_pos = m_max_pos;
		return m_str.substr((size_t)start);
	}

	string CStringTokenizer::st_next_all(string &delims)
	{
		return st_next_all(delims.c_str());
	}

	string CStringTokenizer::st_next_all(const char *delims)
	{
		m_delims = delims;
		return st_next_all();
	}


	int CStringTokenizer::st_count_tokens()
	{
		int count = 0;
		size_t currpos = m_curr_pos;
		R_DEBUG("m_delims(token delimeters): \t", "\""<< m_delims << "\"");

		while (currpos < m_max_pos)
		{
			while (!m_ret_tok && (currpos < m_max_pos) && (m_delims.find(m_str.at((size_t)currpos)) != string::npos))
			{
				currpos++;
			}

			if (currpos >= m_max_pos)
			{
				break;
			}

			size_t start = currpos;
			while ((currpos < m_max_pos) && (m_delims.find(m_str.at((size_t)currpos)) == string::npos))
			{
				currpos++;
			}
			if (m_ret_tok && (start == currpos) && (m_delims.find(m_str.at((size_t)currpos)) != string::npos))
			{
				currpos++;
			}
			count++;
		}
		return count;
	}


	string CStringTokenizer::st_change_delims(string delims)
	{
		m_delims = delims;
		return m_delims;
	}


	void CStringTokenizer::init(const char *str, const char *delimiters, bool returnTokens)
	{
		m_str = str;
		m_delims = delimiters;
		m_curr_pos = 0;
		m_max_pos = m_str.length();
		m_ret_tok = returnTokens;
	}


	void CStringTokenizer::ltrim(string &s)
	{
		std::string::size_type p = 0;
		while (p < s.length() && ((s[p] <= ' ') || (s[p] == ','))) p++;
		if (p > 0) s.erase(0, p);
	}


	void CStringTokenizer::rtrim(string &s)
	{
		size_t x;
		while (((x = s.length()) > 0) && ((s[x - 1] <= ' ')||(s[x - 1] == ',')))
		{
			s.erase(x - 1, 1);
		}
	}


	void CStringTokenizer::trim(string &s)
	{
		ltrim(s);
		rtrim(s);
	}


	void CStringTokenizer::st_skip_delimiters()
	{
		while (!m_ret_tok && (m_curr_pos < m_max_pos) && (m_delims.find(m_str.at(m_curr_pos)) != string::npos))
		{
			m_curr_pos++;
		}
	}


	CFileReader *CFileReader::m_pinstance = NULL;

	CFileReader::CFileReader():
		m_index_counter(0),
		m_param_string_array(0),
		m_test_next_command(0)
	{
		R_DEBUG_ENTER("Reader.cxx","CFileReader constructor!");
		m_tmp_conf[0] = '\0';
		//fr_macro_add_index();
	}

	CFileReader *CFileReader::fr_instance()
	{
		if (m_pinstance == NULL)
		{
			m_pinstance = new CFileReader;
		}
		return m_pinstance;
	}


	CFileReader::~CFileReader()
	{
		R_DEBUG_LEAVE("Reader.cxx","CFileReader destructor!");
		if ((m_tmp_conf[0] != '\0'))
		{
			remove(m_tmp_conf);
		}
		remove("temp_in.tmp");
		m_define_map.clear();
	}


	void CFileReader::fr_delete_instance()
	{
		if (m_pinstance != NULL)
		{
			delete m_pinstance;
		}
	}


	void CFileReader::fr_init(FILE *input_file, const char* input_file_name)
	{
		R_DEBUG_ENTER("Reader.cxx","fr_init");
		//		utr()->utr_set_file(input_file, "input");
		m_line=0;
		m_line_next_command=0;
		m_index_counter=0;
		m_conditional_command=0;
		m_not_conditional_command=0;
		m_count_conditional_command=0;
		m_test_next_command=0;
		m_current_file = 0;
		m_files[m_current_file].file_handle = utr()->utr_get_file("input");
		m_files[m_current_file].file_name = input_file_name;
		m_files[m_current_file].output_file_handle = utr()->utr_get_file("output");
		m_files[m_current_file].output_file_name = "main output file";
		m_files[m_current_file].m_line = 0;
		m_define_map.clear();
		R_DEBUG_LEAVE("Reader.cxx","fr_init");
	}


	bool CFileReader::fr_find_next_command(bool test_commands)
	{
		char *tmpbuf = m_buffer;
		int buf_rest = TFR_BUFFER_SIZE;
		m_line_next_command = 0;

		while (1)
		{
			bool stop=false;
			char* ret_fgets = fgets (tmpbuf, buf_rest, utr()->utr_get_file("input"));
			//				cout << "[  R] LINE: " << ret_fgets << endl;
			if (ret_fgets == NULL)
			{
				R_DEBUG("EOF reached in current file:" <<  m_files[m_current_file].file_name.c_str() <<  " [" << m_line << "] ","");
				stop=true;
				return false;
				break;
			} else
			{
				R_DEBUG_WITHOUT_ENDL(m_files[m_current_file].file_name <<" [" << m_line << "]: ", tmpbuf);
				m_line++;
				m_line_next_command++;
			}
			size_t len=strlen(tmpbuf);
			for (size_t i=0;i<len;i++)
			{
				if (tmpbuf[i]=='#')
				{
					break;
				}
				if (!isalpha(tmpbuf[i]) && tmpbuf[i]!='\r' && tmpbuf[i]!='\n')
				{
					stop=true;
					break;
				}
			}
			if (stop)
				break;
		}
		st_new_string(m_buffer);
		m_token=st_next_token(":,#( \t");
		//			cout << "[  R] m_token: " << m_token << endl;
		if (m_token!="")
		{
			if (m_token[0]=='$')
			{ /* new command has been found */
				for (size_t i=0; i<m_token.length(); i++)
					m_token[i]=toupper (m_token[i]);
				trim (m_token);
				return true;
			}
			if (test_commands&&((m_token[0]>'#')&&(m_token[0]!='/')&&(m_token[0]!='*')))
			{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Bad syntax in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "     TD reads line which contains: \"%s\"\n", m_token.c_str());
				exit (1);
			}
		}
		return true;
	}


	bool CFileReader::fr_conditional_command_find_endif()
	{
		while (m_token.compare("$ENDIF") != 0)
		{
			if (!fr_find_next_command(false))
			{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: The command $ENDIF isn't found!\n");
				return false;
			}
			if (m_token.compare("$ELSE") == 0)
			{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: The command $ENDIF isn't found!\n");
				return false;
			}
		}
		m_count_conditional_command--;
		return true;
	}


	int CFileReader::fr_conditional_command_find_pair()
	{
		bool pair_command  = 0;
		int i = 0;
		do {
			if (i++ > 100)
			{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "The command $IFDEF hasn't pair command \"$ELSE\" or \"$ENDIF\"!\n");
				exit(1);
			}
			fr_find_next_command(false);
			if (m_token.compare("$ELSE") == 0)
			{
				pair_command = 1;
			}
			if (m_token.compare("$ENDIF") == 0)
			{
				m_count_conditional_command--;
				pair_command = 1;
			}
		} while (!pair_command);
		return pair_command;
	}


	int CFileReader::fr_return_command()
	{
		size_t i, j;
		string in_file_name, out_file_name, res_file_name, temp_in_file_name("temp_in.tmp");
		//    FILE *resfile = NULL, * temp_in_file = NULL;
		//    bool output_present, result_present;
		string def, def_value, def_search("${"), def_search_dollar("$"), cur_line, conditional_command;
		//    char tmp_buffer [TFR_BUFFER_SIZE];

		//    BEGIN: 
		for (;;)
			if (fr_find_next_command())
			{
				for (i=CM_NO_COMMAND; i<CM_LAST_COMMAND; i++) {
					////CHI					cout << "[  R] m_token x commands["<< i <<"]: \"" << m_token << "\" x \"" << commands[i] << "\"" << endl;
					if (m_token==commands[i])
					{
						////CHI						cout << "[  R] i:" << i << endl;
						switch (i)
						{
							case CM_INT:
								m_param_int=fr_return_command_integer();
								break;
							case CM_STRING:
							case CM_ECHO:
								fr_return_command_string();
								break;
							case CM_SLEEP:
							case CM_SLEEP_SIM:
								m_param_int=fr_return_command_integer();
								break;
							case CM_EXEC:
								break;
								/*
									 case CM_INCLUDE:
									 m_token=st_next_token();
									 trim (m_token);

									 in_file_name = m_token;


									 m_token=st_next_token();
									 trim (m_token);

									 out_file_name = m_token;


									 m_token=st_next_token();
									 trim (m_token);
									 res_file_name = m_token;


									 if(!m_define_map.empty())
									 {
									 temp_in_file = fopen(temp_in_file_name.c_str(),"w");

									 utr()->utr_open_file(in_file_name.c_str(), "r","input");



									 for (;;)
									 {
									 if(fgets(tmp_buffer,TFR_BUFFER_SIZE,utr()->utr_get_file("input"))==NULL) break;
									 cur_line = tmp_buffer;
									 if( cur_line.find(def_search) != string::npos)
									 {
									 fr_macro_name_replace_by_value(cur_line);
									 } else
									 {
									 if ((cur_line[0] != '#' ) && (cur_line.find_first_of(def_search_dollar, 1) != string::npos))
									 {
									 fr_print_macros(stdout);
									 utr()->utr_fprintf(utr()->utr_get_file("redirect"), "FILE: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
									 utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Bad format name of macro in the line: %s\n", cur_line.c_str());
									 utr()->utr_fprintf(utr()->utr_get_file("redirect"), "\nEXAMPLE: \n$DEFINE NAME dave.lister\n$DEFINE DOMAIN red.dwarf.umts\n$DEFINE USER1 ${NAME}@${DOMAIN}\n$STRING ${USER}\n");
									 exit (1);
									 }
									 }
									 fprintf(temp_in_file,cur_line.c_str());
									 }
									 fclose(temp_in_file);
									 utr()->utr_close_file("input");

									 utr()->utr_open_file(temp_in_file_name.c_str(), "r", "input");
									 } else
									 {

									 utr()->utr_open_file(in_file_name.c_str(), "r", "input");
									 }
									 if (out_file_name.compare("") != 0 && res_file_name.compare("") != 0)
									 {
									 output_present = true;
									 result_present = true;
									 } else if (out_file_name.compare("") != 0)
									 {
									 if (out_file_name.size() >= 4 && out_file_name.compare(out_file_name.size()-4, 4, ".sim") == 0)
									 {
									 output_present = false;
									 result_present = true;
									 res_file_name = out_file_name;
									 out_file_name = "";
									 } else
									 {
									 output_present = true;
								result_present = false;
						}
						} else
						{
							output_present = false;
							result_present = false;
						}
						if (output_present)
						{
							string local_mode("w");
							fflush (utr()->utr_get_file("output"));

							utr()->utr_open_outputfile(out_file_name.c_str(), "w");

							if (utr()->utr_get_file("output") == NULL)
							{
								utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR while opening output file: %s\n", out_file_name.c_str());
								exit(1);
							}




							if ((utr()->m_sign_mark == '-') ||
									(utr()->m_number_mark == 0) ||
									(utr()->m_number_mark <= utr()->m_counter_mark))
								m_output << "Test Started", 0 << endl;
						}
						if (result_present)
						{

							resfile = (FILE*) fopen (res_file_name.c_str(), "r");


							if (resfile == NULL)
							{
								utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR while opening result file: %s\n", res_file_name.c_str());
								exit(1);
							}
						}


						m_files[m_current_file].m_line = m_line;



						m_current_file++;
						m_files[m_current_file].file_handle = utr()->utr_get_file("input");
						m_files[m_current_file].file_name = in_file_name;
						m_files[m_current_file].output_file_handle = utr()->utr_get_file("output");
						m_files[m_current_file].output_file_name = out_file_name;


						m_line = 0;

						if (resfile != NULL)
						{
							ResultFileReader.m_file = NULL;
							InitResultFileReader(resfile, res_file_name.c_str());
						}
						result_files[m_current_file] = ResultFileReader;


						goto BEGIN;
						break;
						*/
							case CM_CALL:
							m_param_int=fr_return_command_integer();
							fr_return_command_string();
							//							cout << "[  R] m_param_string: " << m_param_string << endl;
							break;
							case CM_BOOL:
							m_token=st_next_token();
							trim (m_token);
							for (j=0; j<m_token.length(); j++)
								m_token[j]=toupper (m_token[j]);
							if (m_token=="TRUE") m_param_bool=true;
							else if (m_token=="FALSE") m_param_bool=false;
							else
							{
								utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Invalid boolean value in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
								exit (1);
							}
							break;
							/*							case CM_SKIP_END:
															utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Unexpected $SKIP_END without preceeding $SKIP_START in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
															exit (1);
															*/
						}// end of switch
						return i;
					}// if (m_token==commands[i])
					if (i==CM_LAST_COMMAND)
					{
						utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Unknown directive (\"%s...\") in the file: %s [%d]\n", m_token.substr(0,30).c_str(), m_files[m_current_file].file_name.c_str(), m_line);
						exit (1);
					}
				}
			} else
			{
				if (m_count_conditional_command > 0 )
					utr()->utr_fprintf(utr()->utr_get_file("redirect"), "WARNING: The count of $ENDIF isn't correct with considering cout of $IFDEF or $IFNDEF! Is missing %d $ENDIF!\n", m_count_conditional_command);
				return (CM_NO_COMMAND);
			}
	}

	int CFileReader::fr_return_command_integer()
	{
		int  retval;
		size_t i;
		m_token=st_next_token(":,#( \t");
		trim (m_token);
		if (m_token=="")
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Missing value after $INT in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
			exit (1);
		}
		retval=0;
		bool negative;
		if ('-' == m_token[0])
		{
			negative = true;
			i = 1;
		} else
		{
			negative = false;
			i = 0;
		};

		for (; i<m_token.length(); i++)
		{
			if ((m_token[i]<'0') || (m_token[i]>'9'))
			{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Not a number: %s in the file: %s [%d]\n", m_token.c_str(), m_files[m_current_file].file_name.c_str(), m_line);
				exit (1);
			}
			else
				retval=retval*10+m_token[i]-'0';
		}
		return ( !negative ? retval: -retval );
	}


	void CFileReader::fr_return_command_string()
	{
		m_param_string=st_next_all("");
		trim (m_param_string);
		//  if (m_param_string.find("${") != string::npos)
		//    fr_macro_name_replace_by_value(m_param_string);
		if (m_param_string.find("$GET_VALUE") != string::npos)
		{

			fr_parse_parameters_get_value(1);
		}
	}

	void* CFileReader::fr_return_command_class_ptr()
	{
		m_param_string=st_next_all();
		trim (m_param_string);
		m_param_ptr=NULL;
		/*
			 if (utr()->TestContainer.empty())
			 {
			 printf ("ERROR: List of registered classes is empty!\nYou should register class: %s\n", m_param_string.c_str());
			 exit (1);
			 } else
			 {
			 for ( IterTestClassContainer it = utr()->TestContainer.begin(); it != utr()->TestContainer.end(); it++)
			 {
			 if ((*it)->m_name_class == m_param_string)
			 {
			 m_param_ptr = (void*) (*it);
			 }
			 }
			 printf ("The reference to last element of a list classes is: %p\n", m_param_ptr);
			 }
			 */
		return m_param_ptr;
	}

	int CFileReader::fr_read_int()
	{
		if (fr_return_command()!=CM_INT)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Integer expected in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
			exit (1);
		}
		return m_param_int;
	}

	int CFileReader::fr_read_only_int()
	{
		m_param_int=fr_return_command_integer();
		R_DEBUG("m_param_int: ", m_param_int);
		return m_param_int;
	}

	bool CFileReader::fr_test_int()
	{
		if (fr_find_next_command())
		{
			if (m_token=="$INT") return 1;
		}
		return 0;
		/*

			 if ( fr_inquire_next_command() == "$INT" )
			 return 1;
			 return 0;
			 */
	}

	string CFileReader::fr_read_string()
	{
		if (fr_return_command()!=CM_STRING)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: String expected in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
			exit (1);
		}
		return m_param_string;
	}

	string CFileReader::fr_read_string_hex()
	{
		/*	if (fr_return_command()!=CM_STRING_HEX)
				{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Hexadecimal string expected in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
				exit (1);
				}
				return m_param_string;
				*/
		return "";
	}

	string CFileReader::fr_read_only_string()
	{
		m_param_string=st_next_all();
		trim (m_param_string);
		return m_param_string;
	}

	bool CFileReader::fr_test_string()
	{
		if (fr_find_next_command())
		{
			if (m_token=="$STRING") return 1;
		}
		return 0;
		/*

			 if ( fr_inquire_next_command() == "$STRING" )
			 return 1;
			 return 0;
			 */
	}

	void* CFileReader::fr_read_class_ptr()
	{
		/*	if (fr_return_command()!=CM_CLASS)
				{
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Class name expected in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
				exit (1);
				}
				*/
		return m_param_ptr;
	}
	/******************************************************************************/
	//!read $EXEC
	/******************************************************************************/

	bool CFileReader::fr_read_exec_command( const char* line, string& command, bool& write_to_std, bool& write_to_out, bool& write_to_trc )
	{
		bool retval=true;
		char* buf=strdup(line);
		char* b=buf;
		write_to_out=false;

		int state=0;
		const char* p=0;
		while (*b)
		{
			if (state==0)
			{
				if (*b==' ' || *b=='\t') { b++; continue; }
				if (strncmp(b,"$EXEC",5))
				{
					utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Missing '$EXEC' as commad identifier (internal TD error)\n");
					retval=false;
					break;
				}
				b+=5;
				state=1;
				continue;
			} else if(state==1)
			{
				if (*b==' ' || *b=='\t') { b++; continue; }
				if (*b!='"')
				{
					utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Missing '\"' at beginning of the command\n");
					retval=false;
					break;
				}
				b++;
				p=b;
				state=2;
				continue;
			} else if (state==2)
			{
				if (*b=='"')
				{
					command.assign(p,static_cast<size_t>(b-p));
					b++;
					state=3;
					continue;
				}
				if (*b=='\\' && ( *(b+1)=='"' || *(b+1)=='\\' ) )
					memmove(b,b+1,strlen(b)); // =strlen(b+1)+1
				b++;
			} else if (state==3)
			{
				if (*b==' ' || *b=='\t' || *b=='\n') { b++; continue; }
				if (*b!=',')
				{
					utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Missing separator ',' of command and STD/OUT/TRC \"%c\"\n", *b);
					retval=false;
					break;
				}
				b++;
				state=4;
			} else // state=4
			{
				if (*b==' ' || *b=='\t') { b++; continue; }
				if (!strncmp(b,"STD",3)) { b+=3; write_to_std=true; state=3; continue; }
				if (!strncmp(b,"OUT",3)) { b+=3; write_to_out=true; state=3; continue; }
				if (!strncmp(b,"TRC",3)) { b+=3; write_to_trc=true; state=3; continue; }
				utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Use only STD,OUT or TRC as last arguments of $EXEC\n");
				retval=false;
				break;
			}
		}
		if (state==2)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: Missing '\"' at end of the command\n");
			retval=false;
		}

		free(buf);
		return retval;
	} // read_exec_command

	char** CFileReader::fr_read_string_array()
	{
		/*		if (fr_return_command()!=CM_STRING_ARRAY)
					{
					utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: The command $STRING_ARRAY is expected in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
					exit (1);
					}
					*/
		return m_param_string_array;
	}

	char** CFileReader::fr_return_command_string_array()
	{
		char** array = NULL;
		st_change_delims(" ");
		int num_tokens = st_count_tokens();
		int i;
		R_DEBUG("num_tokens (count of token on the line): ", num_tokens);

		if (num_tokens == 0)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: No strings after $STRING_ARRAY in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
			exit (1);
		}
		array = (char **) malloc((size_t)(num_tokens+1)*sizeof(char *));

		for (i = 0; i < num_tokens; i++)
		{
			m_token = st_next_token(" ");
			trim(m_token);
			R_DEBUG("m_token(token on the line): ", m_token);

			if (m_token == "") break;
			array[i] = (char *) malloc((m_token.length()+1)*sizeof(char));
			strcpy(array[i], m_token.c_str());
		}

		array[i] = NULL;
		return array;

	}


	void CFileReader::fr_free_string_array(char** array)
	{
		if (array != NULL)
		{
			int i = 0;
			while (array[i] != NULL)
			{
				free(array[i]);
				i++;
			}
			free(array);
		}
	}


	int CFileReader::fr_hex2bin(const char* hexastr, void* binastr, size_t hexastrlen)
	{

		unsigned char  *sig_p;
		const char  *str_p, *str_e;
		unsigned char  high, low;
		//  int len, cnt;
		int cnt;


		cnt = 0;
		sig_p = (unsigned char *)binastr;
		str_e = hexastr + hexastrlen;

		for (str_p = hexastr; str_p < str_e; str_p += 2)
		{

			high = *str_p;

			if ((high >= '0') && (high <= '9')) high -= '0';
			else if ((high >= 'A') && (high <= 'F')) high = high - 'A' + 10;
			else if ((high >= 'a') && (high <= 'f')) high = high - 'a' + 10;
			else
			{
				printf("ERROR: Improper character in $STRING_HEX on position: %d at line: %d\n", 2*cnt+1, m_line);
				exit(1);
			}

			low = *(str_p + 1);

			if ((low >= '0') && (low <= '9')) low -= '0';
			else if ((low >= 'A') && (low <= 'F')) low = low - 'A' + 10;
			else if ((low >= 'a') && (low <= 'f')) low = low - 'a' + 10;
			else
			{
				printf("ERROR: Improper character in $STRING_HEX on position: %d at line: %d\n", 2*cnt+2, m_line);
				exit(1);
			}

			*sig_p++ = 16 * high + low;
			cnt++;
		}
		return cnt;
	}


	bool CFileReader::fr_read_bool()
	{
		if (fr_return_command()!=CM_BOOL)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: The command $BOOL is expected in the file: %s [%d]\n", m_files[m_current_file].file_name.c_str(), m_line);
			exit (1);
		}
		return m_param_bool;
	}

	bool CFileReader::fr_read_only_bool()
	{
		m_token=st_next_token(":,#( \t");
		trim (m_token);
		R_DEBUG("m_param_bool: ", m_param_bool);
		if (m_token == "1")
			return true;
		else
			return false;
	}

	bool CFileReader::fr_test_bool()
	{

		if ( fr_inquire_next_command() == "$BOOL" )
			return 1;
		return 0;
	}

	string CFileReader::fr_inquire_next_command()
	{
		/*
			 CTestDriver * pTD = CTestDriver::utr_get_instance();
			 fpos_t pos, npos;

			 m_test_next_command = 1;


			 fgetpos(pTD->utr_get_file("input"),&pos);


			 fr_find_next_command();

			 m_line = m_line - m_line_next_command;


			 fgetpos(pTD->utr_get_file("input"),&npos);


			 fseek(pTD->utr_get_file("input"), -(npos - pos),SEEK_CUR);

			 m_test_next_command = 0;
			 */
		return m_token;
	}

	bool CFileReader::fr_print_error(const char* msg, ...)
	{
		printf("ERROR: %s [%d]", m_files[m_current_file].file_name.c_str(),m_line);

		va_list args;
		va_start(args, msg);
		vprintf(msg, args);
		va_end(args);
		return false;
	}

	void CFileReader::fr_remove_pair_string(string& str, string str_rem_beg, string str_rem_end)
	{
		size_t bracket_beg, bracket_end;
		if ((bracket_beg = str.find(str_rem_beg)) == string::npos)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: The substring \"%s\" is'n contains in the string \"%s\" for pair!\n", str_rem_beg.c_str(), str.c_str());
			exit(1);
		}
		if ((bracket_end = str.find_last_of(str_rem_end)) == string::npos)
		{
			utr()->utr_fprintf(utr()->utr_get_file("redirect"), "ERROR: The substring \"%s\" is'n contains in the string \"%s\" for pair!\n", str_rem_end.c_str(), str.c_str());
			exit(1);
		}
		str  = str.substr(bracket_beg+1, bracket_end-(bracket_beg+1));
	}

	string CFileReader::fr_next_parameter(string& fce_parameters)
	{
		size_t comma = fce_parameters.find(",");
		string parameter = fce_parameters.substr(0, comma);
		trim(parameter);
		fce_parameters = fce_parameters.substr(comma+1);
		return parameter;
	}

	void CFileReader::fr_parse_parameters_get_value(bool replace_get_value)
	{
		//  size_t i=0;
		/*
			 string parameters[6];
			 string function = "$GET_VALUE" + fr()->m_param_string;
			 string sub = fr()->m_param_string;
			 size_t fce_beg = fr()->m_param_string.find("$GET_VALUE");
			 size_t fce_end = fr()->m_param_string.find(")");
			 stringstream oss;

			 if (sub.find("(") != string::npos)  fr_remove_pair_string(sub,"(",")");

			 while (sub.find(",") != string::npos)
			 {
			 parameters[i] = fr_next_parameter(sub);
			 i++;
			 }
			 trim(sub);
			 parameters[i]=sub;

			 if (parameters[1].find("'") != string::npos) fr_remove_pair_string(parameters[1],"'","'");


			 int call = atoi(parameters[0].c_str());
			 const char* event_name = parameters[1].c_str();
			 int order = atoi(parameters[2].c_str());
			 bool remove_event = (bool) atoi(parameters[3].c_str());
			 int index_value = atoi(parameters[5].c_str());

			 if (parameters[4] == "INT" )
			 if (replace_get_value)
			 {
			 oss <<  pf->pf_get_int(index_value) << endl;
			 fr()->m_param_string.replace(fce_beg, fr()->m_param_string.length() - fce_beg, oss.str());
			 }
			 else
			 m_output << "The account to stored pair functions by $GET_VALUE(... << endl",
			 2,
			 ARG_CHAR_PTR, function.c_str(),
			 ARG_INT, pf->pf_get_int(index_value)
			 );
			 if (parameters[4] == "CHARP" )
			 if (replace_get_value)
			 {
			 oss << pf->pf_get_charp(index_value) << endl;
			 fr()->m_param_string.replace(fce_beg, fr()->m_param_string.length() - fce_beg, oss.str());
			 }
			 else
			 m_output << "The account to stored pair functions by $GET_VALUE(... << endl: ",
			 2,
			 ARG_CHAR_PTR, function.c_str(),
			 ARG_CHAR_PTR, pf->pf_get_charp(index_value)
			 );

			 if (parameters[4] == "VOID" )
			 if (replace_get_value)
			 {
			 oss <<  pf->pf_get_voidp_free(index_value) << endl;
			 fr()->m_param_string.replace(fce_beg, fr()->m_param_string.length() - fce_beg, oss.str());
			 }
			 else
			 m_output << "The account to stored pair functions by $GET_VALUE(... << endl: ",
			 2,
			 ARG_CHAR_PTR, function.c_str(),
			 ARG_PTR, pf->pf_get_voidp_free(index_value)
			 );
			 if (parameters[4] == "VOIDP" )
			 if (replace_get_value)
			 {
			 oss <<  pf->pf_get_voidp(index_value) << endl;
			 fr()->m_param_string.replace(fce_beg, fr()->m_param_string.length() - fce_beg, oss.str());
			 }
			 else
		m_output << "The account to stored pair functions by $GET_VALUE(... << endl: ",
						 2,
						 ARG_CHAR_PTR, function.c_str(),
						 ARG_PTR, pf->pf_get_voidp(index_value)
							 );
		cerr << "VOID " << pf->pf_get_voidp() << endl;
		*/
	}

} // namespace ute
