#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>


int mkpath(const char *s, mode_t mode){
        char *q, *r = NULL, *path = NULL, *up = NULL;
        int rv;

        rv = -1;
        if (strcmp(s, ".") == 0 || strcmp(s, "/") == 0)
                return (0);

        if ((path = strdup(s)) == NULL)
                exit(1);
     
        if ((q = strdup(s)) == NULL)
                exit(1);

        if ((r = dirname(q)) == NULL)
                goto out;
        
        if ((up = strdup(r)) == NULL)
                exit(1);

        if ((mkpath(up, mode) == -1) && (errno != EEXIST))
                goto out;

        if ((mkdir(path, mode) == -1) && (errno != EEXIST))
                rv = -1;
        else
                rv = 0;

out:
        if (up != NULL)
                free(up);
        free(q);
        free(path);
        return (rv);
}

int main(int argc, char *argv[]) {
    if (mkpath(argv[1], 0775) < 0) {
        perror("mkpath");
        return -1;
    }

    return 0;
}
