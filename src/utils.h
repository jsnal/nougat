#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <err.h>
#include <unistd.h>
#include <ctype.h>
#include <wordexp.h>
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


/* Get the length of an array */
#define LENGTH(X) (sizeof(X) / sizeof(X[0]))

/* The maximum number of files that can be open at one time */
#define MAX_FOPEN 2

enum time_format { SHORT, LONG, FULL };

void write_header(FILE *fp, const char *relpath);
void write_footer(FILE *fp);
void xml_encode(FILE *fp, const char *s, size_t len);
void join_path(char *buf, size_t bufsiz, const char *path, const char *path2);
void format_git_time_short(FILE *fp, const git_time *gtime);
void format_git_time(FILE *fp, enum time_format tf, const git_time *gtime);

char buf[PATH_MAX];

#endif
