#include "repository.h"

static void initialize(repository *repo);
static void write_summary(FILE *fp, repository *repo);
static void write_page_header(FILE *fp, repository *repo, const char *relpath);

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

void write_page_header(FILE *fp, repository *repo, const char *relpath)
{
  /* TODO: Put the logo to the left of this table-row */
  fputs("<table id=\"header\">\n<tbody>\n<tr><td>"
        "<a href=\"../index.html\">index</a> : ", fp);
  fprintf(fp, "%s<td></tr>\n<tr><td>%s</td></tr>\n", repo->name, repo->desc);
  /* TODO: Add the clone URL */

  fputs("</tbody>\n</table>\n<table id=\"nav\">\n<tbody>\n", fp);
  fprintf(fp, "<tr><td>"
              "\n<a href=\"#\">about</a>"
              "\n<a href=\"%1$ssummary.html\">summary</a>"
              "\n<a href=\"%1$srefs.html\">refs</a>"
              "\n<a href=\"%1$slog.html\">log</a>"
              "\n<a href=\"%1$stree.html\">tree</a>"
              "\n</td></tr>", relpath);
  fputs("\n</tbody>\n</table>\n", fp);
}

void write_summary(FILE *fp, repository *repo)
{
  write_header(fp, "../");
  write_page_header(fp, repo, "../");
  write_footer(fp);
}

int write_repo(repository *repo)
{
  FILE *fp;

  if (!git_revparse_single(&obj, repo->repo, "HEAD")) head = git_object_id(obj);
  git_object_free(obj);

  initialize(repo);

  /* Create and change directory into the current repo */
  mkdir(repo->name, 0700);
  chdir(repo->name);

  /* Write the summary file */
  fp = fopen("summary.html", "w");
  write_summary(fp, repo);
  fclose(fp);

  /* Return back to the execution location */
  chdir(cfg->working_path);

  return 0;
}
