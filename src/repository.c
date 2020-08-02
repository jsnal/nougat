#include "repository.h"

static void initialize(repository *repo);
static commit_info *get_commit_info(repository *repo, const git_oid *id);
static void free_commit_info(commit_info *ci);
static void write_page_header(FILE *fp, repository *repo, const char *relpath);
static void write_log(FILE *fp[], repository *repo);
static void write_log_line(FILE *fp, repository *repo);
static void write_log_header(FILE *fp);

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

void free_commit_info(commit_info *ci)
{
  if (!ci) return;

  git_diff_free(ci->diff);
  git_tree_free(ci->commit_tree);
  git_tree_free(ci->parent_tree);
  git_commit_free(ci->commit);
  git_commit_free(ci->parent);
  free(ci);
}

commit_info *get_commit_info(repository *repo, const git_oid *id)
{
  commit_info *ci = calloc(1, sizeof(commit_info));

  if (git_commit_lookup(&(ci->commit), repo->repo, id)) goto err;
  ci->id = id;

  git_oid_tostr(ci->hash, sizeof(ci->hash), git_commit_id(ci->commit));
  git_oid_tostr(ci->parent_hash, sizeof(ci->parent_hash),
                git_commit_parent_id(ci->commit, 0));

  ci->author = git_commit_author(ci->commit);
  ci->committer = git_commit_committer(ci->commit);
  ci->summary = git_commit_summary(ci->commit);
  ci->message = git_commit_message(ci->commit);

  return ci;
err:
  return NULL;
}

void write_log(FILE *fp[], repository *repo)
{
  commit_info *ci;
  git_revwalk *w = NULL;
  git_oid id;
  char path[PATH_MAX], hash[GIT_OID_HEXSZ + 1];
  FILE *cfp;

  /* Write a log header on all the passed in files */
  for (unsigned int i = 0; i < MAX_FOPEN; i++) write_log_header(fp[i]);

  git_revwalk_new(&w, repo->repo);
  git_revwalk_push(w, head);
  git_revwalk_simplify_first_parent(w);

  while (!git_revwalk_next(&id, w))
  {
    git_oid_tostr(hash, sizeof(hash), &id);
    snprintf(path, sizeof(path), "commit/%s.html", hash);

    /* If the commit hash has already been created, skip to the next hash */
    if (!access(path, F_OK)) continue;

    if (!(ci = get_commit_info(repo, &id))) break;

    free_commit_info(ci);
  }

  git_revwalk_free(w);
}

static void write_log_line(FILE *fp, repository *repo)
{}

void write_log_header(FILE *fp)
{
  fputs("<table id=\"log\">\n<thead>\n<tr><td>Age</td><td>Commit Message</td>"
        "<td>Author</td><td>Files</td><td>Lines</td></tr>\n</thead>\n<tbody>\n",
        fp);
}

void write_page_header(FILE *fp, repository *repo, const char *relpath)
{
  write_header(fp, relpath);

  /* TODO: Put the logo to the left of this table-row */
  fputs("<table id=\"header\">\n<tbody>\n<tr><td>"
        "<a href=\"../index.html\">index</a> : ", fp);
  fprintf(fp, "%s<td></tr>\n<tr><td>%s</td></tr>\n", repo->name, repo->desc);
  /* TODO: Add the clone URL */

  fputs("</tbody>\n</table>\n<table id=\"nav\">\n<tbody>\n", fp);
  fprintf(fp, "<tr><td>"
              "\n<a href=\"#\">about</a>"
              "\n<a href=\"summary.html\">summary</a>"
              "\n<a href=\"refs.html\">refs</a>"
              "\n<a href=\"log.html\">log</a>"
              "\n<a href=\"tree.html\">tree</a>"
              "\n</td></tr>");
  fputs("\n</tbody>\n</table>\n", fp);
}

int write_repo(repository *repo)
{
  FILE *fp[MAX_FOPEN];

  if (!git_revparse_single(&obj, repo->repo, "HEAD")) head = git_object_id(obj);
  git_object_free(obj);

  initialize(repo);

  /* Create and change directory into the current repo */
  mkdir(repo->name, 0700);
  chdir(repo->name);

  /* Write the summary file */
  fp[0] = fopen("summary.html", "w");
  write_page_header(fp[0], repo, "../");

  /* Write the log file */
  fp[1] = fopen("log.html", "w");
  write_page_header(fp[1], repo, "../");
  write_log(fp, repo);

  fclose(fp[1]);

  /* write refs */

  /* Close the summary file */
  write_footer(fp[0]);
  fclose(fp[0]);

  /* Return back to the execution location */
  chdir(cfg->working_path);

  return 0;
}
