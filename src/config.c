#include "config.h"

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

void init_config()
{
  if (!cfg) return;

  cfg->title      = "Git Repositories";
  cfg->path       = "./nougat.cfg";
  cfg->style_path = "./style.css";
  cfg->index_path = "./index.html";
  cfg->icon_path  = "./icon.ico";
}

int parse_config()
{
  check_config_path();

  config_setting_t *category_section;
  config_setting_t *repository_section;

  config_repo_category *rc;
  repository *r;

  config_init(&raw_config);

  if (!config_read_file(&raw_config, cfg->path))
  {
    config_destroy(&raw_config);
    return 1;
  }

  /* Assign all of the top level general config options. If any of these aren't
   * found in the given config, it defaults to the values in init_config(). */
  config_lookup_string(&raw_config, "title", &cfg->title);
  config_lookup_string(&raw_config, "style", &cfg->style_path);
  config_lookup_string(&raw_config, "index", &cfg->index_path);
  config_lookup_string(&raw_config, "icon",  &cfg->icon_path);

  if ((category_section = config_lookup(&raw_config, "categories")) != NULL)
  {
    unsigned int ci = config_setting_length(category_section);
    cfg->repo_category = calloc(ci, sizeof(config_repo_category*));

    for (unsigned int i = 0; i < ci; i++)
    {
      rc = calloc(1, sizeof(config_repo_category));
      cfg->repo_category[i] = rc;
      /* repo_category = */
      /*   (config_repo_category*) malloc(sizeof(config_repo_category)); */

      config_setting_t *current_category =
        config_setting_get_elem(category_section, i);

      /* Set the values for the config_repo_category struct */
      config_setting_lookup_string(current_category, "name", &rc->name);

      if ((repository_section =
           config_setting_lookup(current_category, "repositories")) != NULL)
      {
        unsigned int ri = config_setting_length(repository_section);
        cfg->repo_category[i]->repos = calloc(ri, sizeof(repository*));

        for (unsigned int j = 0; j < ri; j++)
        {
          r = calloc(1, sizeof(repository));
          rc->repos[j] = r;

          config_setting_t *current_repo =
            config_setting_get_elem(repository_section, j);

          /* Set the values for the config_repo struct */
          config_setting_lookup_string(current_repo, "path", &r->path);

          rc->repo_count = j;
        }
      }

      cfg->category_count = i;
    }
  }

  /* fprintf(stderr, "Repo path: %s\n", cfg->repo_category[1]->repos[0]->path); */
  /* printf("Category name: %s\n", cfg->repo_category[0]->name); */

  return 0;
}
