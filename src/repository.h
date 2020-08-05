#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "utils.h"
#include "config.h"
#include <sys/stat.h>

typedef struct repository {
  git_repository *repo;
  const char *path;
  const char *name;
  const char *stripped_name;
  const char *readme;
  const char *license;
  const char *submodules;
  char clone_url[255];
  char owner[255];
  char desc[255];

  /* reference info */
  struct git_reference *ref;
} repository;

typedef struct commit_info {
  const git_oid *id;
  char hash[GIT_OID_HEXSZ + 1];
  char parent_hash[GIT_OID_HEXSZ + 1];

  const git_signature *author;
  const git_signature *committer;
  const char          *summary;
  const char          *body;

  git_diff   *diff;
  git_commit *commit;
  git_commit *parent;
  git_tree   *commit_tree;
  git_tree   *parent_tree;

  size_t add_count;
  size_t del_count;
  size_t file_count;

  struct delta_info **deltas;
  size_t ndeltas;
} commit_info;

typedef struct delta_info {
  git_patch *patch;
  size_t add_count;
  size_t del_count;
} delta_info;

repository *repo;

int write_repo(repository *repo);

#endif
