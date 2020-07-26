#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include <libconfig.h>

/* Technically this puts a hard limit on the number of categories the user can
 * create. However, it seems very unlikely that they will ever reach anywhere
 * near 64. If they do, this value can be raised easily without consequence. The
 * same thing is true for the MAX_REPOSITORIES. */
#define MAX_CATEGORIES 64
#define MAX_REPOSITORIES 256

/* We can add more options the the repo in the future if we want. For exmaple,
 * we could override the description, owner, etc. in the config. */
typedef struct config_repo {
  const char *path;
} config_repo;

typedef struct config_repo_category {
  const char *name;
  unsigned int repo_count;
  struct config_repo *repo[MAX_REPOSITORIES];
} config_repo_category;

typedef struct config {
  const char *title;
  const char *path;
  const char *style_path;
  const char *icon_path;
  unsigned int category_count;
  struct config_repo_category *repo_category[MAX_CATEGORIES];
} config;

config *cfg;
config_t raw_config;

int parse_config();
void init_config();

#endif
