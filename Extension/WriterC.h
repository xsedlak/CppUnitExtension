#ifndef WRITER_H
#define WRITER_H

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus 
extern "C" 
{
#endif  // __cplusplus 

typedef enum
{  
  ARG_INT               =0, 
  ARG_INT32,
  ARG_UINT32,
  ARG_UINT8,
  ARG_UINT8_PTR,
  ARG_UINT8_PTR_HEXA,
  ARG_UINT8_PTR_HEXA_COMBI,
  ARG_PTR,
  ARG_CHAR_PTR,
  ARG_CHAR_PTR_ARRAY,    
  ARG_TIME_T,
  ARG_BOOL,
  ARG_USER,
  ARG_VAR,
  ARG_UINT32_HEXA,
  ARG_UINT8_HEXA,
  ARG_INVISIBLE_PTR,
  ARG_UINT_LIST,
  ARG_CHAR,
  ARG_UINT8_PTR_HEXA_ENHANCED,
  ARG_UINT8_PTR_HEXA_NO_NEWLINE,
  ARG_LAST
} DummyArgType;

int dummy_trace_c (const char * event, int number_of_arguments, ...);

extern FILE * traceoutputfile;
extern int copy_output_to_screen;
extern int copy_trace_to_screen;

int my_fprintf (FILE* file, const char * format, ...);
int my_vfprintf (FILE* file, const char * format, va_list ap);

#ifdef __cplusplus 
}
#endif  // __cplusplus 

#endif  // WRITER_H
