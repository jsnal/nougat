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
  git_repository_free(repo->repo);
  git_libgit2_shutdown();

  free(repo);
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

  cfg = (config*) malloc(sizeof(config));
  init_config();

  if (config_path) cfg->path = config_path;
  if (parse_config() != 0) return 1;

  fprintf(stderr, "count %d\n", cfg->category_count);
  fprintf(stderr, "After parse name %s\n", cfg->repo_category[1]->name);
  fprintf(stderr, "After parse name %s\n", cfg->title);

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
