#include "index.h"

static void write_table_header(FILE *fp);
static void write_table_row(FILE *fp);
static void write_table_end(FILE *fp);

void write_table_header(FILE *fp)
{
  /* Replace from the config file */
  fprintf(fp, "<table>\n<td><span class=\"desc\">%s", "TITLE!");

  fputs("</span></td></tr><tr><td></td><td>\n"
      "</td></tr>\n</table>\n<hr/>\n<div id=\"content\">\n"
      "<table id=\"index\"><thead>\n"
      "<tr><td><b>Name</b></td><td><b>Description</b></td><td><b>Owner</b></td>"
      "<td><b>Last commit</b></td></tr>"
      "</thead><tbody>\n", fp);
}

void write_table_row(FILE *fp)
{
  git_commit *commit = NULL;
  const git_signature *author;
  git_revwalk *w = NULL;
  git_oid id;
  char *stripped_name = NULL, *p;

  git_revwalk_new(&w, repo->repo);
  git_revwalk_push_head(w);
  git_revwalk_simplify_first_parent(w);

  if (git_revwalk_next(&id, w) ||
      git_commit_lookup(&commit, repo->repo, &id)) goto err;

  author = git_commit_author(commit);

  /* strip .git suffix */
  if (!(stripped_name = strdup(repo->name)))
    err(1, "strdup");
  if ((p = strrchr(stripped_name, '.')))
    if (!strcmp(p, ".git"))
      *p = '\0';

  repo->stripped_name = stripped_name;

  fputs("<tr><td><a href=\"", fp);
  xml_encode(fp, repo->stripped_name, strlen(repo->stripped_name));
  fputs("/log.html\">", fp);
  xml_encode(fp, repo->stripped_name, strlen(repo->stripped_name));
  fputs("</a></td><td>", fp);
  xml_encode(fp, repo->desc, strlen(repo->desc));
  fputs("</td><td>", fp);
  xml_encode(fp, repo->owner, strlen(repo->owner));
  fputs("</td><td>", fp);

  if (author) format_git_time_short(fp, &(author->when));
  fputs("</td></tr>", fp);
  git_commit_free(commit);

err:
  git_revwalk_free(w);
  free(stripped_name);
}

void write_table_end(FILE *fp)
{
  fputs("</tbody>\n</table>\n</div>\n</body>\n</html>\n", fp);
}

void index_repo(FILE *fp, int ci, int ri)
{
  if (!repo) return;
  D fprintf(stderr, __PG_NAME__": Indexing %s\n", repo->path);

  FILE *read_fp;
  char path[PATH_MAX];

  if (ci == 0 && ri == 0)
  {
    write_header(fp, "My Title");
    write_table_header(fp);
  }

  /* use directory name as name */
  if ((repo->name = strrchr(buf, '/')))
    repo->name++;
  else
    repo->name = "";

  /* read description or .git/description */
  join_path(path, sizeof(path), repo->path, "description");
  if (!(read_fp = fopen(path, "r"))) {
    join_path(path, sizeof(path), repo->path, ".git/description");
    read_fp = fopen(path, "r");
  }
  repo->desc[0] = '\0';
  if (read_fp) {
    if (!fgets(repo->desc, sizeof(repo->desc), read_fp))
      repo->desc[0] = '\0';
    fclose(read_fp);
  }

  /* read owner or .git/owner */
  join_path(path, sizeof(path), repo->path, "owner");
  if (!(read_fp = fopen(path, "r"))) {
    join_path(path, sizeof(path), repo->path, ".git/owner");
    read_fp = fopen(path, "r");
  }
  repo->owner[0] = '\0';
  if (read_fp) {
    if (!fgets(repo->owner, sizeof(repo->owner), read_fp))
      repo->owner[0] = '\0';
    repo->owner[strcspn(repo->owner, "\n")] = '\0';
    fclose(read_fp);
  }

  write_table_row(fp);

  if (ci == cfg->category_count && ri == cfg->repo_category[ci]->repo_count)
    write_table_end(fp);
}