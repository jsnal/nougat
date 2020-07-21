#include "utils.h"

const char *relpath = "";
char *name = "";
char description[255] = "Repositories";
git_repository *repo;


void create_header()
{
  fprintf(stdout, "Hello Header!\n");
}

void xmlencode(FILE *fp, const char *s, size_t len)
 {
 	size_t i;

	for (i = 0; *s && i < len; s++, i++) {
		switch(*s) {
			case '<':  fputs("&lt;",   fp); break;
 			case '>':  fputs("&gt;",   fp); break;
 			case '\'': fputs("&#39;" , fp); break;
			case '&':  fputs("&amp;",  fp); break;
 			case '"':  fputs("&quot;", fp); break;
 			default:   fputc(*s, fp);
 		}
	}
}

void writeheader(FILE *fp) 
{
	fputs("<!DOCTYPE html>\n"
	     "<html>\n<head>\n"
	     "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
	     "<title>", fp);
	     
	     xmlencode(fp, description, strlen(description));
	     fprintf(fp, "</title>\n<link rel=\"icon\" type=\"image/png\" href=\"%sfavicon.png\" />\n", 		relpath);
	     fprintf(fp, "<link rel=\"stylesheet\" type=\"text/css\" href=\"%sstyle.css\" />\n", relpath);
	     fputs("</head>\n<body>\n", fp);
	     
	     fprintf(fp, "<table>\n<tr><td><img src=\"%slogo.png\" alt=\"\" width=\"32\" height=\"32\" /></td>\n"
	     "<td><span class=\"desc\">", relpath);
	     
	     xmlencode(fp, description, strlen(description));
	     fputs("</span></td></tr><tr><td></td><td>\n"
	     "</td></tr>\n</table>\n<hr/>\n<div id=\"content\">\n"
	     "<table id=\"index\"><thead>\n"
	     "<tr><td><b>Name</b></td><td><b>Description</b></td><td><b>Owner</b></td>"
	     "<td><b>Last commit</b></td></tr>"
	     "</thead><tbody>\n", fp);
}

void writefooter(FILE *fp)
{
	fputs("</tbody>\n</table>\n</div>\n</body>\n</html>\n", fp);
}


int writelog(FILE *fp)
{
	git_commit *commit = NULL;
	const git_signature *author;
	git_revwalk *w = NULL;
	git_oid id;
	char *stripped_name = NULL, *p;
	int ret = 0;
	
	git_revwalk_new(&w, repo);
	git_revwalk_push_head(w);
	git_revwalk_simplify_first_parent(w);
	
	if (git_revwalk_next(&id, w) ||
     	    git_commit_lookup(&commit, repo, &id)) {
     		ret = -1;
     		goto err;
     	}
     
     	author = git_commit_author(commit);
     
     	/* strip .git suffix */
     	if (!(stripped_name = strdup(name)))
     		err(1, "strdup");
     	if ((p = strrchr(stripped_name, '.')))
     		if (!strcmp(p, ".git"))
     			*p = '\0';
     
    fputs("<tr><td><a href=\"", fp);
    xmlencode(fp, stripped_name, strlen(stripped_name));
    fputs("/log.html\">", fp);
    xmlencode(fp, stripped_name, strlen(stripped_name));
    fputs("</a></td><td>", fp);
    xmlencode(fp, description, strlen(description));
    fputs("</td><td>", fp);
    xmlencode(fp, owner, strlen(owner));
    fputs("</td><td>", fp);
    if (author)
    printtimeshort(fp, &(author->when));
    fputs("</td></tr>", fp);     
    git_commit_free(commit);
    err:
    git_revwalk_free(w);
    free(stripped_name);
     
    return ret;
	
	
}




