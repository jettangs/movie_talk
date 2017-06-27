#include "srt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_LEN 1000
typedef unsigned int u32;

void CompressSrt(FILE *in, FILE *out, int *c) 
{
	char u[LINE_LEN];
	memset(u, 0, LINE_LEN);

	int i = 0, n = 1;
	while (fgets(u, LINE_LEN, in)) {
		
		if (u[0] == '\r' && u[1] == '\n') {
			n++;
			if (n == *c) {
				while (fgets(u, LINE_LEN, in)) {
					if (u[0] == '\r' && u[1] == '\n') {
						long ps = ftell(in);
						ps -= 2;
						fseek(in, ps, SEEK_SET);
						break;
					}
				}
				c++;
				continue;
			}
		}

		fwrite(u, sizeof(char), strlen(u), out);
		
		i++;
	}
}