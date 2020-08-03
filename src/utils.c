#include "utils.h"
#include "config.h"

void join_path(char *buf, size_t bufsiz, const char *path, const char *path2)
{
  int r;

  r = snprintf(buf, bufsiz, "%s%s%s",
      path, path[0] && path[strlen(path) - 1] != '/' ? "/" : "", path2);

  if (r < 0 || (size_t)r >= bufsiz)
    err(1, "path truncated: '%s%s%s'",
        path, path[0] && path[strlen(path) - 1] != '/' ? "/" : "", path2);
}

void format_git_time_short(FILE *fp, const git_time *gtime)
{
  struct tm *intm;
  time_t t;
  char out[32];

  t = (time_t) gtime->time;
  if (!(intm = gmtime(&t))) return;

  strftime(out, sizeof(out), "%Y-%m-%d", intm);
  fputs(out, fp);
}

void xml_encode(FILE *fp, const char *s, size_t len)
{
  size_t i;

  for (i = 0; *s && i < len; s++, i++)
  {
    switch(*s)
    {
      case '<':  fputs("&lt;",   fp); break;
      case '>':  fputs("&gt;",   fp); break;
      case '\'': fputs("&#39;" , fp); break;
      case '&':  fputs("&amp;",  fp); break;
      case '"':  fputs("&quot;", fp); break;
      default:   fputc(*s, fp);
    }
  }
}

void write_header(FILE *fp, const char *relpath)
{
  fputs("<!DOCTYPE html>\n"
      "<html>\n<head>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
      "<title>", fp);

  /* TODO: Add this line for favicon support */
  /* fprintf(fp, "<link rel=\"icon\" type=\"image/png\" href=\"%sfavicon.png\" />\n",relpath); */

  xml_encode(fp, cfg->title, strlen(cfg->title));
  fprintf(fp, "</title>\n");
  fprintf(fp, "<link rel=\"stylesheet\" type=\"text/css\" href=\"%s%s\" />\n", relpath, cfg->style_path);
  fprintf(fp, "<link href='http://fonts.googleapis.com/css?family=Lato&subset=latin,latin-ext' rel='stylesheet' type='text/css'>");
  fputs("</head>\n<body>\n", fp);
}

void write_footer(FILE *fp)
{
  fputs("</div>\n</body>\n</html>\n", fp);
}
