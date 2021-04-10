#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int variable = 1024;
    void *ptr = (void *)&variable;
    char *string;
    
    //sprintf(&string, (char *)&ptr);
    sprintf(string, "%p", ptr);
    printf("%s\n", string);

    return 0;
}