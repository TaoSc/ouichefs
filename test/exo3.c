#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "../src/ioctl.h"

// Nécessite qu'une partition ouichefs soit montée dans le dossier /wish
int main()
{
    struct ioctl_request i = {.ino = 5, .nb_version = 1};
    char buf[20];
    sprintf(buf, "/dev/%s", IOCTL_NAME);
    int fd = open(buf, O_RDWR);
    if (fd == -1) {
        printf("open : %s\n", strerror(errno));
        return -1;
    }
    int ret = ioctl(fd, CHANGE_VER, &i);
    if (ret) {
        printf("ioctl : %s\n", strerror(errno));
        return -1;
    }
    close(fd);
    
    return 0;
}
