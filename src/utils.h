#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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

void write_header(FILE *fp, const char *title);
void write_footer(FILE *fp);
void xml_encode(FILE *fp, const char *s, size_t len);
void join_path(char *buf, size_t bufsiz, const char *path, const char *path2);
void format_git_time_short(FILE *fp, const git_time *gtime);

#endif
