#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include "repository.h"
#include <libconfig.h>

/* The category struct uses the repository struct defined in repository.c. */
typedef struct config_repo_category {
  const char *name;
  unsigned int repo_count;
  struct repository **repos;
} config_repo_category;

/* TODO: Create an option to turn off categories completely */
typedef struct config {
  const char *title;
  const char *path;
  const char *style_path;
  const char *index_path;
  const char *icon_path;
  unsigned int category_count;
  struct config_repo_category **repo_category;
} config;

config *cfg;
config_t raw_config;

int parse_config();
void init_config();

#endif
