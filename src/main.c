#include "index.h"
#include "constants.h"
#include "utils.h"
#include "config.h"

static void usage(char *binary);
static void clean();
static int parse_cmdline(int argc, char *argv[]);

static char *config_path;

void clean()
{
  git_libgit2_shutdown();
  free(repo);

  for (unsigned int i = 0; i <= cfg->category_count; i++)
  {
    for (unsigned int j = 0; j <= cfg->repo_category[i]->repo_count; j++)
      free(cfg->repo_category[i]->repos[j]);
    free(cfg->repo_category[i]->repos);
    free(cfg->repo_category[i]);
  }

  free(cfg->repo_category);
  free(cfg);
  config_destroy(&raw_config);
}


int parse_cmdline(int argc, char *argv[])
{
  static int c;

  while ((c = getopt(argc, argv, "c:")) != -1)
    switch (c)
    {
      case 'c': config_path = optarg; break;
      default:  usage(argv[0]); return 1;
    }

  return 0;
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

  if (parse_cmdline(argc, argv) != 0) return 1;

  cfg = malloc(sizeof(config));
  init_config();

  if (config_path) cfg->path = config_path;
  if (parse_config() != 0)
  {
    fprintf(stderr, "Unable to parse %s\n", cfg->path);
    return 1;
  }

  if (git_libgit2_init() != 1) return 1;

  repo = malloc(sizeof(repository));
  FILE *index_fp = fopen(cfg->index_path, "w");

  for (unsigned int i = 0; i <= cfg->category_count; i++)
  {

    for (unsigned int j = 0; j <= cfg->repo_category[i]->repo_count; j++)
    {
      /* TODO: Add tilda HOME expansion */
      repo->path = cfg->repo_category[i]->repos[j]->path;

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

      /* If fopen failed, notify the user but don't fail the whole program */
      if (index_fp) index_repo(index_fp, i, j);
      else          fprintf(stderr, "Unable to open path %s\n", cfg->index_path);

      /* Free the repository object everytime or else they will build up */
      git_repository_free(repo->repo);
    }
  }

  if (index_fp) fclose(index_fp);

  clean(repo);
  return 0;
}
