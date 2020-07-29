#include "repository.h"

static void initialize(repository *repo);

static git_object *obj = NULL;
static const git_oid *head = NULL;
static char *possible_licenses[] = {
  "HEAD:LICENSE",
  "HEAD:LICENSE.md",
  "HEAD:COPYING"
};
static char *possible_readmes[] = {
  "HEAD:README",
  "HEAD:README.md"
};

/* Initialize the repo struct with all of the metadata that can be retrived
 * without going through the repository commit tree */
void initialize(repository *repo)
{
  /* Set the name to the directory if the name wasn't set by index.c */
  if (!repo->name)
  {
    if ((repo->name = strrchr(buf, '/')))
      repo->name++;
    else
      repo->name = "";
  }

  /* Find the correct README */
  for (unsigned int i = 0;
      i < sizeof(possible_licenses) / sizeof(*possible_licenses) && !repo->license;
      i++)
  {
    if (!git_revparse_single(&obj, repo->repo, possible_licenses[i]) &&
        git_object_type(obj) == GIT_OBJ_BLOB)
      repo->license = possible_licenses[i] + strlen("HEAD:");
    git_object_free(obj);
  }

  /* Find the correct LICENSE */
  for (unsigned int i = 0;
      i < sizeof(possible_readmes) / sizeof(*possible_readmes) && !repo->readme;
      i++)
  {
    if (!git_revparse_single(&obj, repo->repo, possible_readmes[i]) &&
        git_object_type(obj) == GIT_OBJ_BLOB)
      repo->readme = possible_readmes[i] + strlen("HEAD:");
    git_object_free(obj);
  }

  /* Test if there are any submodules */
  if (!git_revparse_single(&obj, repo->repo, "HEAD:.gitmodules") &&
      git_object_type(obj) == GIT_OBJ_BLOB)
    repo->submodules = ".gitmodules";
  git_object_free(obj);
}

int write_repo(repository *repo)
{
  if (!git_revparse_single(&obj, repo->repo, "HEAD")) head = git_object_id(obj);
  git_object_free(obj);

  initialize(repo);

  /* Create and change directory into the current repo */
  mkdir(repo->name, 0700);
  chdir(repo->name);

  FILE *fp;
  fp = fopen("log.html", "w");
  fclose(fp);

  printf("working path: %s\n", cfg->working_path);
  printf("license: %s\n", repo->license);
  printf("readme: %s\n", repo->readme);
  printf("modules: %s\n", repo->submodules);

  /* Return back to the execution location */
  chdir(cfg->working_path);

  return 0;
}
