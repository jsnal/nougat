#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


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
