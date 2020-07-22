#ifndef CONST_H
#define CONST_H

#include <git2.h>

char buf[PATH_MAX];

typedef struct repository {
  git_repository *repo;
  const char *path;
  const char *name;
  char owner[255];
  char desc[255];
} repository;

repository *repo;

#endif
