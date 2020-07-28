#ifndef INDEX_H
#define INDEX_H

#include <time.h>

#include "utils.h"
#include "config.h"
#include "constants.h"
#include "repository.h"

void index_repo(FILE *fp, repository *repo, int current, int total);

#endif
