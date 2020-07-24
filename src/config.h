#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"

/* Technically this puts a hard limit on the number of categories the user can
 * create. However, it seems very unlikely that they will ever reach anywhere
 * near 128. If they do, this value can be raised easily without consequence. */
#define MAX_CATEGORIES 64

/* We can add more options the the repo in the future if we want. For exmaple,
 * we could override the description, owner, etc. in the config. */
typedef struct config_repo {
  const char *path;
} config_repo;

typedef struct config_repo_category {
  const char *name;
  struct config_repo *repo;
} config_repo_category;

typedef struct config {
  const char *title;
  const char *path;
  const char *style_path;
  const char *icon_path;
  struct config_repo_category *repo_category[MAX_CATEGORIES];
} config;

//probably want to change the size of config_js

char config_js[200];

config *cfg;

int parse_config();

#endif
