#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

void *_do_threads() {
    int i;

    for (i = 0; i < 10; i++) {
        printf("[tid %d]: %d\n", gettid(), i);
    }

    pthread_exit(NULL);
    return NULL;
}


int do_threads() {
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&thread, &attr, &_do_threads, NULL) != 0) {
        printf("pthread_create error\n");
        return -1;
    }

    printf("return to do_threads tid: %d\n", gettid());
    return 0;
        
}

int main(int argc, char *argv[]) {
    printf("first thread id: %d\n", gettid());

    if (do_threads() < 0) {
        printf("error\n");
        return -1;
    }

    sleep(2);
    printf("return to main tid: %d\n", gettid());
    return 0;
}