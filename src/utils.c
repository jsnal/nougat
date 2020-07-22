#include "utils.h"

void joinpath(char *buf, size_t bufsiz, const char *path, const char *path2)
{
  int r;

  r = snprintf(buf, bufsiz, "%s%s%s",
      path, path[0] && path[strlen(path) - 1] != '/' ? "/" : "", path2);

  if (r < 0 || (size_t)r >= bufsiz)
    errx(1, "path truncated: '%s%s%s'",
        path, path[0] && path[strlen(path) - 1] != '/' ? "/" : "", path2);
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

void write_header(FILE *fp, const char *title)
{
  fputs("<!DOCTYPE html>\n"
      "<html>\n<head>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
      "<title>", fp);

  /* TODO: Add this line for favicon support */
  /* fprintf(fp, "<link rel=\"icon\" type=\"image/png\" href=\"%sfavicon.png\" />\n",relpath); */

  xml_encode(fp, title, strlen(title));
  fprintf(fp, "</title>\n");
  fprintf(fp, "<link rel=\"stylesheet\" type=\"text/css\" href=\"%sstyle.css\" />\n", "./");
  fputs("</head>\n<body>\n", fp);
}

void writefooter(FILE *fp)
{
  fputs("</tbody>\n</table>\n</div>\n</body>\n</html>\n", fp);
}
