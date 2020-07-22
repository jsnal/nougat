#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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
void xml_encode(FILE *fp, const char *s, size_t len);
void joinpath(char *buf, size_t bufsiz, const char *path, const char *path2);

void writeheader(FILE *fp);
void writefooter(FILE *fp);
int writelog(FILE *fp);

#endif
