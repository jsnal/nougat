#include "index.h"




void joinpath(char *buf, size_t bufsiz, const char *path, const char *path2)
{
	int r;
	
	r = snprintf(buf, bufsiz, "%s%s%s",
	path, path[0] && path[strlen(path) - 1] != '/' ? "/" : "", path2);
	
	if (r < 0 || (size_t)r >= bufsiz)
		errx(1, "path truncated: '%s%s%s'",
		path, path[0] && path[strlen(path) - 1] != '/' ? "/" : "", path2);
}

int initialize(char *argv[], int argc)
{
FILE *fp;
    	char path[PATH_MAX], repodirabs[PATH_MAX + 1];
    	const char *repodir;
    	int i, ret = 0;
    
    	if (argc < 2) {
    		fprintf(stderr, "%s [repodir...]\n", argv[0]);
    		return 1;
    	}
    
    	git_libgit2_init();
    
    #ifdef __OpenBSD__
    	for (i = 1; i < argc; i++)
    		if (unveil(argv[i], "r") == -1)
    			err(1, "unveil: %s", argv[i]);
   
    	if (pledge("stdio rpath", NULL) == -1)
    		err(1, "pledge");
    #endif
    
    	writeheader(stdout);
    
     	for (i = 1; i < argc; i++) {
     		repodir = argv[i];
     		if (!realpath(repodir, repodirabs))
    			err(1, "realpath");
     
     		if (git_repository_open_ext(&repo, repodir,
     		    GIT_REPOSITORY_OPEN_NO_SEARCH, NULL)) {
     			fprintf(stderr, "%s: cannot open repository\n", argv[0]);
     			ret = 1;
     			continue;
     		}
     
     		/* use directory name as name */
     		if ((name = strrchr(repodirabs, '/')))
    			name++;
     		else
     			name = "";
     
     		/* read description or .git/description */
     		joinpath(path, sizeof(path), repodir, "description");
     		if (!(fp = fopen(path, "r"))) {
     			joinpath(path, sizeof(path), repodir, ".git/description");
     			fp = fopen(path, "r");
     		}
     		description[0] = '\0';
     		if (fp) {
     			if (!fgets(description, sizeof(description), fp))
     				description[0] = '\0';
     			fclose(fp);
     		}
     
     		/* read owner or .git/owner */
     		joinpath(path, sizeof(path), repodir, "owner");
     		if (!(fp = fopen(path, "r"))) {
     			joinpath(path, sizeof(path), repodir, ".git/owner");
     			fp = fopen(path, "r");
     		}
     		owner[0] = '\0';
     		if (fp) {
     			if (!fgets(owner, sizeof(owner), fp))
     				owner[0] = '\0';
     			owner[strcspn(owner, "\n")] = '\0';
    			fclose(fp);
    	}
    	writelog(stdout);
    }
    writefooter(stdout);
        /* cleanup */
    git_repository_free(repo);
    git_libgit2_shutdown();
    
            return ret;
}
