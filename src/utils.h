#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <git2.h>


/* The offical name of the program used in logs and other output */
#define __PG_NAME__ "nougat"

/*
 * Used to log things if the binary is built with debug on. For example:
 *
 * D fprintf(stderr, "Failed to do something\n");
 */
#ifndef DEBUG
#define D if (true)
#else
#define D if (false)
#endif

void create_header();
void writeheader(FILE *fp);
void writefooter(FILE *fp);
void xmlencode(FILE *fp, const char *s, size_t len);
int writelog(FILE *fp);

git_repository *repo;
const char *relpath;
char description[255];
char *name;
char owner[255];


#endif
