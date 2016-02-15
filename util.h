#include <stdio.h>

// prints to stderr than exits with code 1
static inline
void err(const char * const msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

static inline
FILE *write_file(const char * const filename)
{
	if(filename == NULL) return NULL;

	FILE *fp = fopen(filename, "w");
	if(fp == NULL)	return NULL;

	return fp;
}
