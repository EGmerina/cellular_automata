#ifndef CONFIGPARSER_H_INCLUDED
#define CONFIGPARSER_H_INCLUDED
#define FILE_PATH_LENGHT 100

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cstring>
int get_parm_offset(char * str,int nom);
int conv_string_to_int(char *str,int *save_buffer);
int conv_string_to_range(char *str,int *save_buffer);
int read_sec_parm(char *filename,const char *sect_name,const char *parm_name,char *save_buffer,size_t save_size);
int getIntParamByName(char *filename,const char *sect_name,const char *parm_name);
void getCharParamByName(char *filename,const char *sect_name,const char *parm_name, int* save_array);
void getFileByName(char *filename,const char *sect_name,const char *parm_name, char* save_array);
double getDoubleParamByName(char *filename,const char *sect_name,const char *parm_name);
#endif // CONFIGPARSER_H_INCLUDED
