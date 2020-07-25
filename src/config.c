#include "config.h"

static void check_config_path();
static char *read_config();

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

char *read_config()
{
  FILE *config;
  char *buffer = NULL;
  size_t len;
  ssize_t bytes_read;

  if (!(config = fopen(cfg->path, "r"))) return NULL;

  bytes_read = getdelim(&buffer, &len, '\0', config);
  fclose(config);

  if (bytes_read == -1) return NULL;
  return buffer;
}

int parse_config()
{
  check_config_path();

  const char *str;
  config_t config;
  config_setting_t *setting;

  config_init(&config);

  if (!config_read_file(&config, cfg->path))
  {
    config_destroy(&config);
    return 1;
  }

  if (config_lookup_string(&config, "title", &str))
  {
    fprintf(stderr, "Config Contents %s\n", str);
  }

  /* if ((setting = config_lookup(&config, "hours")) != NULL) */
  /* { */
  /*   unsigned int count config_setting_length(setting); */
  /*  */
  /*   for (unsigned int i = 0; i < count; i++) */
  /*   { */
  /*     const_setting_t *crepo = config_setting_get_elem(setting, i); */
  /*   } */
  /* } */

  fprintf(stderr, "Config %s\n", cfg->path);

  config_destroy(&config);
  return 0;
}
