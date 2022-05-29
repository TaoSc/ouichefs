#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../ioctl.h"

#define FN "/wish/test_part4"

char buf[128];
char out_buf[128];

// Nécessite qu'une partition ouichefs soit montée dans le dossier /wish
int main(int argc, char *argv[])
{
    int fd = open(FN, O_RDWR | O_CREAT | O_EXCL, 0666);

    // Rev 1:
    strcpy(buf, "really old.\n");
    write(fd, buf, strlen(buf));

    read(fd, out_buf, 4);
    printf("Rev 1: %s\n", out_buf);

    // Rev 2:
    strcpy(buf, "old.\n");
    write(fd, buf, strlen(buf));

    read(fd, out_buf, 4);
    printf("Rev 2: %s\n", out_buf);

    // Rev 3:
    strcpy(buf, "new.\n");
    write(fd, buf, strlen(buf));

    read(fd, out_buf, 4);
    printf("Rev 3: %s\n", out_buf);

    // Rollback to Rev 2:
    ioctl(fd, CHANGE_VER, -1);

    read(fd, out_buf, 4);
    printf("Rollback to Rev 2: %s\n", out_buf);

    // Delete Rev 3:
    ioctl(fd, NEW_LATEST);

    // Rev 3 (new branch):
    strcpy(buf, "brand new.\n");
    write(fd, buf, strlen(buf));

    read(fd, out_buf, 10);
    printf("Rev 3 (new branch): %s\n", out_buf);

    return 0;
}
