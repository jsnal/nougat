#ifndef INDEX_H
#define INDEX_H

#include <err.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"
#include "constants.h"

void index_repo(FILE *fp, repository *repo, int current, int total);

#endif
