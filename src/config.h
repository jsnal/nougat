#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include <libconfig.h>

/* We can add more options the the repo in the future if we want. For exmaple,
 * we could override the description, owner, etc. in the config. */
typedef struct config_repo {
  const char *path;
} config_repo;

typedef struct config_repo_category {
  const char *name;
  unsigned int repo_count;
  struct config_repo **repos;
} config_repo_category;

typedef struct config {
  const char *title;
  const char *path;
  const char *style_path;
  const char *icon_path;
  unsigned int category_count;
  struct config_repo_category **repo_category;
} config;

config *cfg;
config_t raw_config;

int parse_config();
void init_config();

#endif
