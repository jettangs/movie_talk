#include "tools.h"
#include "../cJSON/cJSON.h"
#include <stdio.h>
#include <Windows.h>

char* GetSuffix(char *str) 
{

	int i = 0;
	for (int j = 0; j < strlen(str); j++)
	{
		if (str[j] == '.') i = 1;
	}
	if (!i) return NULL;

	char t[1000];
	strcpy_s(t, str);

	char *buf = NULL, *s = NULL, *p = NULL;
	s = strtok_s(t, ".", &buf);
	while (s) {
		p = s;
		s = strtok_s(NULL, ".", &buf);
	}
	return p;
}

char *GetContentType(char *str)
{
	char *t = NULL;
	if (!str) return NULL;
	if (!_stricmp(str, "html")) {
		t = "text/html";
	}
	else if (!_stricmp(str, "js")) {
		t = "application/javascript";
	}
	else if (!_stricmp(str, "css")) {
		t = "text/css";
	}
	else if (!_stricmp(str, "json")) {
		t = "application/json";
	}
	else {
		t = "text/plain";
	}
	return t;
}

#define LINE_LEN 1000

char *CombineString(char *b, FILE *in, char *s) {
	fgets(b, LINE_LEN, in); //°üÀ¨\r\n
	int len = strlen(b);
	if (!s)
		s = "\0";
	int q = strlen(s);
	int m = len - 2;
	for (int i = 0; i < q; i++) {
		b[m + i] = s[i];
	}

	char *p = &b[m + q];
	char u[LINE_LEN];
	memset(u, 0, LINE_LEN);
	fgets(u, LINE_LEN, in);
	do {
		if (u[0] == '\r' && u[1] == '\n')
			continue;
		for (int i = 0; i < strlen(u); i++) 
		{
			if (u[i] == '\r' && u[i + 1] == '\n')
				break;
			*p = u[i];
			p++;
		}
		while (*s) 
		{
			*p = *s;
			p++;
			s++;
		}
		s -= q;
	} while (fgets(u, LINE_LEN, in));
	p -= q;
	while (q)
	{
		*p = 0;
		p++;
		q--;
	}
	*(p + 1) = 0;
	return b;
}
