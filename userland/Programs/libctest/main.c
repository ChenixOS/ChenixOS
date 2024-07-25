#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc,char** argv) {
    printf("Hello Libc \n");
    printf("Args=%s\n",argv[0]);
    setenv("PATH","1",1);
    printf("PATH=%s\n",getenv("PATH"));

    time_t t;
    time(&t);
    printf("%d %s",(unsigned int)t,ctime(&t));
}