// FHP miscellaneous
// Version 2.0
//

#define MAX_STR 10

int str2int(char *str)			// Converts string to integer
{
	int i,m=0,n;
	for(i=0;str[i];i++)
	{
		if((i>MAX_STR)||(str[i]<'0')||(str[i]>'9'))
			return -1;
		n=m;
		m*=10;
		m+=str[i]-'0';
		if((m/10)!=n)
			return -1;
	}
	return m;
}

int rang(int v)
{
  int r=!!v;
  while(v&=(v-1))
    r++;
  return r;
}
