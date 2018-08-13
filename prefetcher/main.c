
void startup_child(int argc, char **argv) {
    pid_t tracee;

    int wait_status;

    if ((tracee = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // tracee
    if (tracee == 0) {
        raise(SIGSTOP);

        char *dirc = strncup(argv[1], strlen(argv[1]));
        char *dname = dirname(dirc);

        setenv("LD_LIBRARY_PATH", dname, 1);
        chdir(dname);

        free(dirc);

        if (execv(argv[1], &argv[1]) < 0) {
            perror("execv");
            exit(EXIT_FAILURE);
        }
    }

    // tracer
    if ((tracee = waitpid(-1, &wait_status, WSTOPPED)) < 0) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    
}

int main(int argc, char *argv[]) {
    if (!startup_child(argc, argv)) {
        perror("startup_child");
        return -1;
    }
}