#include "../ioctl.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
int main(){
    char *argp = "1";
    char buf[20];
    sprintf(buf,"/dev/%s",ioctl_name);
    int fd = open(buf,O_RDWR);
    printf("%d\n",ioctl(fd,change_version,argp));
}