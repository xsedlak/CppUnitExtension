#ifndef READER_H
#define READER_H

#include <string>
#include <vector>
#include <map>
#include <stdio.h>

namespace cppunitmsext {

#ifdef DEBUG_READER
#define R_DEBUG_ENTER(file, function) \
	cout << "################################################################################\n"; \
	cout <<  "ENTERING : " << file << " | "  << function << endl;\
	cout << "################################################################################\n";
#define R_DEBUG(var, val)    cout << "  R|" << var << "\"" << val << "\"" << endl;
#define NR_DEBUG(var, val)    cout << " NR|" << var << "\"" << val << "\"" << endl;
#define R_DEBUG_WITHOUT_ENDL(var, val)    cout << "  R|" << var << val;
#define NR_DEBUG_WITHOUT_ENDL(var, val)    cout << " NR|" << var << val;
#define R_DEBUG_LEAVE(file, function) \
	cout << "################################################################################\n"; \
	cout << "LEAVING : " << file << " | " << function << endl;\
	cout << "################################################################################\n";
#define R_ERR_DEBUG(var,val) cerr << << var << " | " << val  << endl;
#else
#define R_DEBUG_ENTER(file, function)
#define R_DEBUG(var, val)
#define NR_DEBUG(var, val)
#define R_DEBUG_WITHOUT_ENDL(var, val)
#define NR_DEBUG_WITHOUT_ENDL(var, val)
#define R_DEBUG_LEAVE(file, function)
#define R_ERR_DEBUG(var,val)
#endif

typedef struct TFileInfo
{
	FILE *file_handle;
	std::string file_name;
	FILE *output_file_handle;
	std::string output_file_name;
	int m_line;
} TFileInfo;


const int TFR_BUFFER_SIZE=65535;

enum commands {
	CM_NO_COMMAND=0,
	CM_REPEAT,
	CM_INFO,
	CM_COMMENT,
	CM_ECHO,
	CM_ECHO_TRC,
	CM_CALL,
	CM_ORDER,
	CM_ORDER_KEEP_ENTRY,
	CM_ORDER_KEEP,
	CM_ORDER_DELETE_LAST,
	CM_ORDER_DELETE_ALL,
	CM_IFDEF,
	CM_IFNDEF,
	CM_ELSE,
	CM_ENDIF,
	CM_INDEX,
	CM_INDEX_INC,
	CM_INDEX_RESET,
	CM_INT,
	CM_STRING,
	CM_STRING_HEX,
	CM_TIMER_TYPE,
	CM_BOOL,
	CM_SKIP_START,
	CM_SKIP_END,
	CM_INCLUDE,
	CM_MARK,
	CM_SLEEP,
	CM_STRING_ARRAY,
	CM_BATCH_END,
	CM_BATCH_SIM,
	CM_BATCH_SIM_END,
	CM_SLEEP_SIM,
	CM_DEFINE,
	CM_EXEC,
	CM_CLASS,
	CM_GET_VALUE,
	CM_NEM_NOTIFICATION,
	CM_NEM_NOTIFICATION_SIMPLE,
	CM_EVENT_NOTIFICATION,
	CM_EVENT_COMMUNICATION_BACKUP_STATUS,
	CM_EVENT_LOW_PRIO_TASK,
	CM_EVENT_MESSAGE_TYPE,
	CM_NEMLIB_CHANGE_CONFIG,
	CM_NEMLIB_CHANGE_CONFIG_END,
	CM_ADD_PARAM,
	CM_CHANGE_PARAM,
	CM_DEL_PARAM,
	CM_DEL_TABLE,
	CM_CHANGE_TABLE,
	CM_ADD_TABLE,
	CM_CHANGE_STORED_VALUE,
	CM_OVL_LCL_CTX_UPDATE,
	CM_OVL_VOL_CTX_READ,
	CM_OVL_GET_LOAD_INFO,
	CM_PERSISTENCE_COMMIT,
	CM_PERSISTENCE_ROLLBACK,
	CM_PERSISTENCE_SET_PREPARE_COMMIT,
	CM_LAST_COMMAND
};


extern std::string commands[];

class CStringTokenizer
{
	private:

		size_t m_curr_pos;
		size_t m_max_pos;
		std::string m_str;
		std::string m_delims;
		bool m_ret_tok;

		void init(const char *str, const char *delimiters, bool returnTokens = false);

	public:

		void ltrim(std::string &s);

		void rtrim(std::string &s);

		void trim(std::string &s);

		void st_skip_delimiters();

		CStringTokenizer(const char *str, const char *delimiters, bool returnTokens = false);

		CStringTokenizer(const std::string &str, const std::string &delimiters, bool returnTokens = false);

		CStringTokenizer(const char *str);

		CStringTokenizer(const std::string &str);

		CStringTokenizer();

		void st_new_string(const char *str);

		void st_new_string(const std::string &str);

		bool st_has_more_tokens();

		std::string st_next_token();

		std::string st_next_token(std::string &delims);

		std::string st_next_token(const char *delims);

		std::string st_next_all();

		std::string st_next_all(std::string &delims);

		std::string st_next_all(const char *delims);

		int st_count_tokens();

		std::string st_change_delims(std::string delims);

		virtual ~CStringTokenizer(){};

}; // class CStringTokenizer


class CFileReader: public CStringTokenizer
{

	private:

		static CFileReader *m_pinstance;

		CFileReader();

		void* fr_return_command_class_ptr();

		char** fr_return_command_string_array();

		void fr_change_param(std::vector<std::string> &config, std::string section, std::string param_name, std::string new_param, bool add_new_param);

		void fr_delete_param(std::vector<std::string> &config, std::string section, std::string param_name, bool table);

		void fr_macro_add_or_change(std::string macro_name, std::string value);

		bool fr_macro_name_is_defined(std::string macro_name);

		void fr_macro_name_replace_by_value(std::string &line);

		int fr_conditional_command_find_pair();

		bool fr_conditional_command_find_endif();

		void fr_macro_add_index();

	public:

		std::string m_token;
		char m_tmp_conf[L_tmpnam];
		bool m_param_bool;
		std::string m_param_string;
		int m_param_int;
		void * m_param_ptr;
		int m_line;
		int m_line_next_command;
		int m_current_file;
		TFileInfo m_files[100];
		char m_buffer [TFR_BUFFER_SIZE];
		bool m_buffer_valid;
		int m_index_counter;
		bool m_conditional_command;
		bool m_not_conditional_command;
		int m_count_conditional_command;
		char** m_param_string_array;
		bool m_test_next_command;
		std::map <std::string, std::string>  m_define_map;
		std::map <std::string,std::string>::iterator  m_define_iterator;

		virtual ~CFileReader();

		static CFileReader * fr_instance();

		void fr_delete_instance();

		void fr_init(FILE *input_file, const char* input_file_name);

		void fr_read_dns_msg();

		bool fr_find_next_command (bool test_commands=true);

		int  fr_return_command_integer();

		void fr_return_command_string();

		bool fr_read_bool();

		bool fr_read_only_bool();

		bool fr_test_bool();

		int fr_read_int();

		int fr_read_only_int();

		bool fr_test_int();

		std::string fr_read_string();

		std::string fr_read_string_hex();

		std::string fr_read_only_string();

		bool fr_test_string();

		int fr_return_command();

		void *fr_read_class_ptr();

		std::string fr_inquire_next_command();

		char** fr_read_string_array();

		void fr_free_string_array(char** array);

		bool fr_print_error(const char* msg, ...);

		void fr_print_macros(FILE* file_name);

		int fr_hex2bin(const char* hexastr, void* binastr, size_t hexastrlen);

		void fr_parse_parameters_get_value(bool replace_get_value);

		std::string fr_next_parameter(std::string& fce_parameters);

		void fr_remove_pair_string(std::string& str, std::string str_rem_beg, std::string str_rem_end);

};

}

#endif // READER_H
