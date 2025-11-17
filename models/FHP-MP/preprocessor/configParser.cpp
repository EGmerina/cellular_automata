#include "configParser.h"
using namespace std;

int get_parm_offset(char * str,int nom)	{
	while((str[nom]!=',')&&(str[nom]!=';')&&(str[nom]!='\0')) {
		nom++;
	}
	if((str[nom]==';')||(str[nom]=='\0')) {
		return 0;
	}
	return(++nom);
}
int conv_string_to_int(char *str,int *save_buffer)
{
	int nom = 0;
	int count = 0;

	/*save first elem. from the string*/
	if(sscanf(str,"%d,",&save_buffer[count])>0) {
		count++;
	}

	while ((nom=get_parm_offset(str,nom))!=0) {
		sscanf(str+nom,"%d,",&save_buffer[count++]);
	}

	return count;
}

int conv_string_to_double(char *str,double *save_buffer)
{
	int nom = 0;
	int count = 0;

	if(sscanf(str,"%lf,",&save_buffer[count])>0) {
		count++;
	}

	while ((nom=get_parm_offset(str,nom))!=0) {
		sscanf(str+nom,"%lf,",&save_buffer[count++]);
	}

	return count;
}

int conv_string_to_range(char *str,int *save_buffer)
{
	int nom = 0;
	int count = 0;

	if(sscanf(str,"%d,",&save_buffer[count])>0) {
		count++;
	}

	while ((nom=get_parm_offset(str,nom))!=0) {
		sscanf(str+nom,"%d,",&save_buffer[count++]);
	}

	return count;
}

int read_sec_parm(char *filename,const char *sect_name,const char *parm_name,char *save_buffer,size_t save_size) {
	FILE *cfg_file;
	char Order[4000];
	char buf[4000];
	int num_char=0;

	memset(Order,' ',sizeof(Order));

	cfg_file=fopen(filename,"r");
	if(cfg_file==NULL) {
		cerr << "Error opening config file";
		return -1;
	}

	while(fgets(buf,sizeof(buf),cfg_file)!=NULL) {
		int i=0;
		char ch=buf[i];

		while((ch==' ')||(ch=='\t')) {
			ch=buf[++i];
		}

		if ((ch=='#')||(ch==';')||(ch=='\n')) {
			continue;
		}

		if(sscanf(buf+i,"[%s]",Order)!=0){
			continue;
		}

		if (strncmp(sect_name,Order,strlen(sect_name))!=0){
			continue;
		}
		if (strncmp(parm_name,buf+i,strlen(parm_name))!=0){
			continue;
		}
		int j=i;
		ch=buf[j];

		while((ch!='\t')&&(ch!=';')&&(ch!='#')&&(ch!='\n')) {
			ch=buf[++j];
		}
		num_char=j-(i+strlen(parm_name)+sizeof('='));
		if(num_char>=save_size) {
				num_char=-1;break;
		}
		strncpy(save_buffer,&buf[i+strlen(parm_name)+sizeof('=')],num_char);
		save_buffer[num_char]=0;

		break;
	}

	fclose(cfg_file);
	return num_char;
}

int getIntParamByName(char *filename,const char *sect_name,const char *parm_name) {
	char paramCharValue[10];
	int paramIntValue[10];
	read_sec_parm(filename, sect_name, parm_name, paramCharValue, sizeof(paramCharValue));
	conv_string_to_int(paramCharValue, &paramIntValue[0]);
	return paramIntValue[0];
}

double getDoubleParamByName(char *filename,const char *sect_name,const char *parm_name) {
	char paramCharValue[10];
	double paramIntValue[10];
	read_sec_parm(filename, sect_name, parm_name, paramCharValue, sizeof(paramCharValue));
	conv_string_to_double(paramCharValue, &paramIntValue[0]);
	return paramIntValue[0];
}


void getCharParamByName(char *filename,const char *sect_name,const char *parm_name, int* save_array) {
	char paramCharValue[10];
	read_sec_parm(filename, sect_name, parm_name, paramCharValue, sizeof(paramCharValue));
	conv_string_to_range(paramCharValue, save_array);
}

void getFileByName(char *filename,const char *sect_name,const char *parm_name, char* save_array) {
	read_sec_parm(filename, sect_name, parm_name, save_array, sizeof(char*) * FILE_PATH_LENGHT);
}
