#include "utils.h"
#include "config.h"

static void usage(char *binary);
static int parse_cmdline(int argc, char *argv[]);

static char *config_path;

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
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{

  D fprintf(stderr, __PG_NAME__": Warning DEBUG is on\n");

  if (parse_cmdline(argc, argv) != 0) return 1;

  cfg = (config*) malloc(sizeof(config));
  if (config_path) cfg->path = config_path;
  else cfg->path = "./config.json";

  if (parse_config() != 0) return 1;

  create_header();

  free(cfg);
  return 0;
}
