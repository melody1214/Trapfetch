#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <linux/unistd.h>       /* for _syscallX macros/related stuff */
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>
#include <fcntl.h>


extern bool startup_child(int argc, char **argv);
extern bool trace(void);

long get_uptime()
{
    struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
    }
    return s_info.uptime;
}

int main(int argc, char *argv[]) {
  FILE *fd = fopen("/proc/uptime", "r");
  char *uptime = (char *)malloc(16);

  if (argc < 2) {
    printf("Usage: <prefetcher> <target app>\n");
    exit(EXIT_FAILURE);
  }

  if (!startup_child(argc, argv)) {
    perror("startup_child");
    return -1;
  }

  fgets(uptime, 16, fd);
  printf("prefetcher uptime: %s\n", uptime);  

  while (trace())
    ;
  
  printf("\nprefetcher: every tracees have been terminated normally\n");
  
  return 0;
}