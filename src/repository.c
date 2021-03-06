#include "repository.h"

static void initialize(repository *repo);
static commit_info *init_commit_info(repository *repo, const git_oid *id);
static int get_commit_info(commit_info *ci, repository *repo);
static void free_commit_info(commit_info *ci);
static void free_delta_info(delta_info *di);
static void write_page_header(FILE *fp, repository *repo, const char *root_path,
                              const char *repo_path);
static void write_commit_info(FILE *fp, commit_info *ci);
static void write_commit_diff(FILE *fp, commit_info *ci);
static void write_log(FILE *fp[], repository *repo);
static void write_log_line(FILE *fp, commit_info *ci);
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

  if (ci->deltas)
    for (unsigned int i = 0; i < ci->ndeltas; i++)
      free_delta_info(ci->deltas[i]);

  free(ci->deltas);
  git_diff_free(ci->diff);
  git_tree_free(ci->commit_tree);
  git_tree_free(ci->parent_tree);
  git_commit_free(ci->commit);
  git_commit_free(ci->parent);
  free(ci);
}

void free_delta_info(delta_info *di)
{
  if (!di) return;

  git_patch_free(di->patch);
  free(di);
}

commit_info *init_commit_info(repository *repo, const git_oid *id)
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
  ci->body = git_commit_body(ci->commit);

  return ci;
err:
  return NULL;
}

int get_commit_info(commit_info *ci, repository *repo)
{
  delta_info *di;
  git_diff_options opts;
  git_diff_find_options fopts;
  const git_diff_delta *delta;
  const git_diff_hunk *hunk;
  const git_diff_line *line;
  git_patch *patch = NULL;
  size_t ndeltas, nhunks, nhunklines;
  unsigned int i;

  /* Get the tree object for the current commit hash */
  if (git_tree_lookup(&(ci->commit_tree),
                        repo->repo, git_commit_tree_id(ci->commit))) goto err;

  if (!git_commit_parent(&(ci->parent), ci->commit, 0))
    if (git_tree_lookup(&(ci->parent_tree),
                          repo->repo, git_commit_tree_id(ci->parent)))
    {
      ci->parent = NULL;
      ci->parent_tree = NULL;
    }

  git_diff_init_options(&opts, GIT_DIFF_OPTIONS_VERSION);
  opts.flags |= GIT_DIFF_DISABLE_PATHSPEC_MATCH |
                GIT_DIFF_IGNORE_SUBMODULES      |
                GIT_DIFF_INCLUDE_TYPECHANGE;

  if (git_diff_tree_to_tree(&(ci->diff), repo->repo,
                              ci->parent_tree, ci->commit_tree, &opts)) goto err;

  if (git_diff_find_init_options(&fopts, GIT_DIFF_FIND_OPTIONS_VERSION))
    goto err;

  fopts.flags |= GIT_DIFF_FIND_RENAMES |
                 GIT_DIFF_FIND_COPIES  |
                 GIT_DIFF_FIND_EXACT_MATCH_ONLY;
  if (git_diff_find_similar(ci->diff, &fopts)) goto err;

  ndeltas = git_diff_num_deltas(ci->diff);
  ci->deltas = calloc(ndeltas, sizeof(delta_info*));

  for (i = 0; i < ndeltas; i++) {
    if (git_patch_from_diff(&patch, ci->diff, i)) goto err;

    di = calloc(1, sizeof(delta_info));
    di->patch = patch;
    ci->deltas[i] = di;

    delta = git_patch_get_delta(patch);

    /* skip stats for binary data */
    if (delta->flags & GIT_DIFF_FLAG_BINARY) continue;

    nhunks = git_patch_num_hunks(patch);
    for (unsigned int j = 0; j < nhunks; j++)
    {
      if (git_patch_get_hunk(&hunk, &nhunklines, patch, j)) break;
      for (unsigned int k = 0; ; k++) {
        if (git_patch_get_line_in_hunk(&line, patch, j, k)) break;
        if (line->old_lineno == -1)
        {
          di->add_count++;
          ci->add_count++;
        }
        else if (line->new_lineno == -1)
        {
          di->del_count++;
          ci->del_count++;
        }
      }
    }
  }
  ci->ndeltas = i;
  ci->file_count = i;

  return 0;
err:
  git_diff_free(ci->diff);
  git_tree_free(ci->commit_tree);
  git_tree_free(ci->parent_tree);
  git_commit_free(ci->parent);
  if (ci->deltas)
    for (unsigned int i = 0; i < ci->ndeltas; i++)
      free_delta_info(ci->deltas[i]);
  free(ci->deltas);

  return -1;
}

void write_commit_info(FILE *fp, commit_info *ci)
{
  fputs("<table id=\"commit-info\">\n<tbody>\n", fp);

  /* Commit author and time authored */
  if (ci->author)
  {
    fputs("<tr><td>author</td><td>", fp);
    xml_encode(fp, ci->author->name, strlen(ci->author->name));
    fputs("&lt;<a href=\"mailto:", fp);
    xml_encode(fp, ci->author->email, strlen(ci->author->email));
    fputs("\">", fp);
    xml_encode(fp, ci->author->email, strlen(ci->author->email));
    fputs("</a>&gt;</td><td>", fp);
    format_git_time(fp, FULL, &(ci->author->when));
    fputs("</td></tr>\n", fp);
  }

  /* Committer and time committed */
  if (ci->committer)
  {
    fputs("<tr><td>committer</td><td>", fp);
    xml_encode(fp, ci->committer->name, strlen(ci->committer->name));
    fputs("&lt;<a href=\"mailto:", fp);
    xml_encode(fp, ci->committer->email, strlen(ci->committer->email));
    fputs("\">", fp);
    xml_encode(fp, ci->committer->email, strlen(ci->committer->email));
    fputs("</a>&gt;</td><td>", fp);
    format_git_time(fp, FULL, &(ci->committer->when));
    fputs("</td></tr>\n", fp);
  }

  /* Current commit hash */
  if (ci->hash)
  {
    fputs("<tr><td>commit</td><td>", fp);
    fprintf(fp, "<a href=\"../commit/%1$s.html\">%1$s</a></td></tr>\n",
            ci->hash);
  }

  /* Parent commit hash */
  if (ci->parent_hash)
  {
    fputs("<tr><td>parent</td><td>", fp);
    fprintf(fp, "<a href=\"../commit/%1$s.html\">%1$s</a></td></tr>\n",
            ci->parent_hash);
  }

  fputs("</table></tbody>", fp);

  if (ci->body)
  {
    fprintf(fp, "<div id=\"commit-summary\">%s</div>\n", ci->summary);
    fprintf(fp, "<div id=\"commit-body\">%s</div>\n", ci->body);
  }

  write_commit_diff(fp, ci);
}

void write_commit_diff(FILE *fp, commit_info *ci)
{
	const git_diff_delta *delta;
	const git_diff_hunk *hunk;
	const git_diff_line *line;
	git_patch *patch;
	size_t nhunks, nhunklines, changed, add, del, total, j, k;
	char linestr[80], new_hash[8], old_hash[8];
  const char *status;
	int c;

  if (!ci->deltas) return;

	if (ci->file_count > 1000   ||
	    ci->ndeltas   > 1000    ||
	    ci->add_count  > 100000 ||
	    ci->del_count  > 100000)
  {
		fputs("Diff is too large, output suppressed.\n", fp);
		return;
	}

  fputs("<div id=\"diff-header\">Diffstat</div>\n<table id=\"diff-table\">\n",
        fp);
  for (unsigned int i = 0; i < ci->ndeltas; i++)
  {
    delta = git_patch_get_delta(ci->deltas[i]->patch);

    switch (delta->status)
    {
      case GIT_DELTA_ADDED:      status = "Added";      break;
      case GIT_DELTA_COPIED:     status = "Copied";     break;
      case GIT_DELTA_DELETED:    status = "Deleted";    break;
      case GIT_DELTA_MODIFIED:   status = "Modified";   break;
      case GIT_DELTA_RENAMED:    status = "Renamed";    break;
      case GIT_DELTA_TYPECHANGE: status = "Typechange"; break;
      default:                   status = "Unknown";    break;
    }

    fprintf(fp, "<tr><td>%s</td>", status);

    fprintf(fp, "<td><a href=\"#h%d\">", i);
    xml_encode(fp, delta->old_file.path, strlen(delta->old_file.path));
    if (strcmp(delta->old_file.path, delta->new_file.path)) {
      fputs(" -&gt; ", fp);
      xml_encode(fp, delta->new_file.path, strlen(delta->new_file.path));
    }
    fputs("</a></td></tr>\n", fp);

    /* TODO: add diffstate graph here (table within a table) */
  }

  fprintf(fp, "</table>\n<div id=\"summary\">%zu file%s changed, %zu insertion%s, "
              "%zu deletion%s</div>\n",
              ci->file_count, ci->file_count == 1 ? "" : "s",
              ci->add_count,  ci->add_count  == 1 ? "" : "s",
              ci->del_count,  ci->del_count  == 1 ? "" : "s");

  /* TODO: Optimize this so there aren't two of the same exact loops */
  for (unsigned int i = 0; i < ci->ndeltas; i++)
  {
    /* Need to delcare this verbosely so git_patch_num_hunks works later */
    patch = ci->deltas[i]->patch;
    delta = git_patch_get_delta(patch);
    git_oid_tostr(new_hash, sizeof(new_hash), &delta->new_file.id);
    git_oid_tostr(old_hash, sizeof(old_hash), &delta->old_file.id);

    fputs("<table class=\"head\">\n<tr><td>diff --git a/", fp);
    xml_encode(fp, delta->old_file.path, strlen(delta->old_file.path));
    fputs(" b/", fp);
    xml_encode(fp, delta->old_file.path, strlen(delta->new_file.path));
    fprintf(fp, "</td></tr>\n");
    fprintf(fp, "<tr><td>index %s..%s</td></tr>\n</table>\n", old_hash, new_hash);

    /* Check binary data */
    if (delta->flags & GIT_DIFF_FLAG_BINARY) {
      fputs("Binary files differ.\n", fp);
      continue;
    }

    nhunks = git_patch_num_hunks(patch);
    for (unsigned int j = 0; j < nhunks; j++)
    {
      if (git_patch_get_hunk(&hunk, &nhunklines, patch, j))
        break;

      for (unsigned int k = 0; ; k++)
      {
        if (git_patch_get_line_in_hunk(&line, patch, j, k))
          break;

        xml_encode(fp, line->content, line->content_len);
      }
    }
  }
}

void write_log(FILE *fp[], repository *repo)
{
  commit_info *ci;
  git_revwalk *w = NULL;
  git_oid id;
  char path[PATH_MAX], hash[GIT_OID_HEXSZ + 1];
  FILE *hash_fp;


  /* Write a log header on all the passed in files */
  for (unsigned int i = 0; i < MAX_FOPEN; i++) write_log_header(fp[i]);

  /* Make the directory to store all of the commit files */
  mkdir("commit", 0700);

  git_revwalk_new(&w, repo->repo);
  git_revwalk_push(w, head);
  git_revwalk_simplify_first_parent(w);

  /* while (!git_revwalk_next(&id, w)) */
  for (unsigned int i = 0; !git_revwalk_next(&id, w); i++)
  {
    git_oid_tostr(hash, sizeof(hash), &id);
    snprintf(path, sizeof(path), "commit/%s.html", hash);

    if (!(ci = init_commit_info(repo, &id))) break;
    if (get_commit_info(ci, repo) == -1) goto err;

    for (unsigned int j = 0; j < MAX_FOPEN; j++)
    {
      /* Print only the 10 most recent commits on the summary page */
      if (j == 0 && i > 10) continue;
      write_log_line(fp[j], ci);
    }

    /* If the commit hash has already been created, skip to the next hash */
    if (!access(path, F_OK)) goto err;

    printf("creating the %s\n", path);
    hash_fp = fopen(path, "w");
    write_page_header(hash_fp, repo, "../../", "../");
    write_commit_info(hash_fp, ci);
    write_footer(hash_fp);
    fclose(hash_fp);

err:
    free_commit_info(ci);
  }

  git_revwalk_free(w);
}

void write_log_line(FILE *fp, commit_info *ci)
{
  fputs("<tr><td>\n", fp);

  /* Age */
  if (ci->author)
    format_git_time(fp, SHORT, &(ci->author->when));
  fputs("</td><td>", fp);

  /* Commit message */
  if (ci->summary)
  {
    fprintf(fp, "<a href=\"commit/%s.html\">", ci->hash);
    xml_encode(fp, ci->summary, strlen(ci->summary));
    fputs("</a>", fp);
  }
  fputs("</td><td>", fp);

  /* Author Name */
  if (ci->author)
    xml_encode(fp, ci->author->name, strlen(ci->author->name));
  fputs("</td>", fp);

  /* Files changed */
  fprintf(fp, "<td>%zu</td>", ci->file_count);

  /* File delta */
  fprintf(fp, "<td>"
               "<span class=\"add\">+%zu</span>/<span class=\"del\">-%zu</span>"
               "</td>\n", ci->add_count, ci->del_count);
}

void write_log_header(FILE *fp)
{
  fputs("<table id=\"log\">\n<thead>\n<tr><td>Age</td><td>Commit Message</td>"
        "<td>Author</td><td>Files</td><td>Lines</td></tr>\n</thead>\n<tbody>\n",
        fp);
}

/* TODO: See if there is a better way of handling the relpaths */
void write_page_header(FILE *fp, repository *repo, const char *root_path,
                       const char *repo_path)
{
  write_header(fp, root_path);

  /* TODO: Put the logo to the left of this table-row */
  fprintf(fp, "<table id=\"header\">\n<tbody>\n<tr><td>"
              "<a href=\"%sindex.html\">index</a> : ", root_path);
  fprintf(fp, "%s<td></tr>\n<tr><td>%s</td></tr>\n", repo->name, repo->desc);
  /* TODO: Add the clone URL */

  fputs("</tbody>\n</table>\n<table id=\"nav\">\n<tbody>\n", fp);
  fprintf(fp, "<tr><td>"
              "\n<a href=\"%1$s#\">about</a>"
              "\n<a href=\"%1$ssummary.html\">summary</a>"
              "\n<a href=\"%1$srefs.html\">refs</a>"
              "\n<a href=\"%1$slog.html\">log</a>"
              "\n<a href=\"%1$stree.html\">tree</a>"
              "\n</td></tr>", repo_path);
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
  write_page_header(fp[0], repo, "../", "");

  /* Write the log file */
  fp[1] = fopen("log.html", "w");
  write_page_header(fp[1], repo, "../", "");
  write_log(fp, repo);
  write_footer(fp[1]);
  fclose(fp[1]);

  /* write refs */
  fp[1] = fopen("refs.html", "w");
  write_page_header(fp[1], repo, "../", "");
  /* write_log(fp, repo); */
  write_footer(fp[1]);
  fclose(fp[1]);

  /* Close the summary file */
  write_footer(fp[0]);
  fclose(fp[0]);

  /* Return back to the execution location */
  chdir(cfg->working_path);

  return 0;
}
