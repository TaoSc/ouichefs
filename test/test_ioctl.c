#include "../ioctl.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
int main(){
    int i = 1;
    //char buf[20];
    //sprintf(buf,"/dev/%d",ioctl_name);
    int fd = open("/wish/test_file1",O_RDWR);
    if(fd == -1){
        printf("failed open\n");
        return 1;
    }
    printf("%d\n",ioctl(fd,CHANGE_VER,i));
    close(fd);
    return 0;
}