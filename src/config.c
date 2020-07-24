#include "jsmn.h"
#include "config.h"
/* jsmn_parser parser; */
/* jsmntok_t tokens[10]; */
/*  */
/* jsmn_init(&parser); */
/*  */
/* //change up the token variables when editing the structs */
/* jsmn_parse(&parser, config_js, strlen(js), tokens, 10); */

static void check_config_path();

void check_config_path()
{
  FILE *tmp;

  if (!(tmp = fopen(cfg->path, "r+")))
  {
    fprintf(stderr, __PG_NAME__": Unable to open config %s\n", cfg->path);
    exit(EXIT_FAILURE);
  }
  else
    fclose(tmp);
}

int parse_config()
{
  check_config_path();

  fprintf(stderr, "Config %s\n", cfg->path);
  return 0;
}
