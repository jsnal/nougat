#include "index.h"
#include "constants.h"
#include "utils.h"
#include "config.h"

static void usage(char *binary);
static void clean();
static int parse_cmdline(int argc, char *argv[]);

static char *config_path;
static repository *current_repo;

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
  char cwd[PATH_MAX];

  D fprintf(stderr, __PG_NAME__": Warning DEBUG is on\n");

  if (parse_cmdline(argc, argv) != 0) return 1;

  cfg = malloc(sizeof(config));
  init_config();

  if (config_path) cfg->path = config_path;
  if (parse_config() != 0)
  {
    fprintf(stderr, __PG_NAME__": Unable to parse %s\n", cfg->path);
    return 1;
  }

  if (git_libgit2_init() != 1) return 1;

  /* Get the current working path so we can return to it later */
  if (getcwd(cwd, sizeof(cwd)) != NULL) cfg->working_path = cwd;
  else
    fprintf(stderr, __PG_NAME__": Problems getting the current working path");

  FILE *index_fp = fopen(cfg->index_path, "w");

  for (unsigned int i = 0; i <= cfg->category_count; i++)
  {

    for (unsigned int j = 0; j <= cfg->repo_category[i]->repo_count; j++)
    {
      /* TODO: Add tilda HOME expansion */
      /* repo->path = cfg->repo_category[i]->repos[j]->path; */
      current_repo = cfg->repo_category[i]->repos[j];

      /* Test if the given path exists */
      if (!realpath(current_repo->path, buf))
      {
        D fprintf(stderr, __PG_NAME__": %s is not a valid path\n",
                  current_repo->path);
        continue;
      }

      /* Test that it is a git project and store it if it is */
      if (git_repository_open_ext(&current_repo->repo, current_repo->path,
            GIT_REPOSITORY_OPEN_NO_SEARCH, NULL))
      {
        D fprintf(stderr, __PG_NAME__": %s is not a valid git repository\n",
            current_repo->path);
        continue;
      }

      /* If fopen failed, notify the user but don't fail the whole program */
      if (index_fp) index_repo(index_fp, current_repo, i, j);
      else          fprintf(stderr, "Unable to open path %s\n", cfg->index_path);

      /* Write all of the individual repo information for the current repo */
      write_repo(current_repo);

      /* Free the repository object everytime or else they will build up */
      git_repository_free(current_repo->repo);
    }
  }

  if (index_fp) fclose(index_fp);

  clean(repo);
  return 0;
}
