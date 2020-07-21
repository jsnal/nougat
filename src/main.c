#include "utils.h"

static void usage(char *binary);

void usage(char *binary)
{
  /* TODO: Write a proper usage output */
  fprintf(stdout, "%s: Usage\n", binary);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  D fprintf(stderr, __PG_NAME__": Warning DEBUG is on\n");

  if (argc <= 1) usage(argv[0]);

  create_header();

  return 0;
}
