#include "index.h"
#include "constants.h"

static void usage(char *binary);
static void clean();

void clean()
{
  git_repository_free(repo->repo);
  git_libgit2_shutdown();

  free(repo);
}

void usage(char *binary)
{
  /* TODO: Write a proper usage output */
  fprintf(stdout, "%s: Usage\n", binary);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  D fprintf(stderr, __PG_NAME__": Warning DEBUG is on\n");

  if (argc <= 1) usage(argv[0]);

  git_libgit2_init();

  repo = (repository*) malloc(sizeof(repository));
  FILE *index_fp = fopen("./index.html", "w");

  for (int i = 1; i < argc; i++)
  {
    repo->path = argv[i];

    /* Test if the given path exists */
    if (!realpath(repo->path, buf))
    {
      D fprintf(stderr, __PG_NAME__": %s is not a valid path\n", repo->path);
      continue;
    }

    /* Test that it is a git project and store it if it is */
    if (git_repository_open_ext(&repo->repo, repo->path,
        GIT_REPOSITORY_OPEN_NO_SEARCH, NULL))
    {
      D fprintf(stderr, __PG_NAME__": %s is not a valid git repository\n",
                repo->path);
      continue;
    }

    /* If fopen failed, just default to printing to stdout */
    if (index_fp) index_repo(index_fp, repo, i, argc);
    else          index_repo(stdout, repo, i, argc);
  }

  if (index_fp) fclose(index_fp);

  clean();
  return 0;
}
