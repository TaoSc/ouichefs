#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "../src/ioctl.h"

#define FN "/wish/test_part4"

char buf[128];
char out_buf[128];

// Nécessite qu'une partition ouichefs soit montée dans le dossier /wish
int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Format: exo3 ino nb_version\n.");
        return -1;
    }

	int fd_file = open(FN, O_RDWR | O_CREAT | O_EXCL, 0666);

	struct stat file_stat;
	fstat(fd_file, &file_stat);

	struct ioctl_request i = {.ino = file_stat.st_ino, .nb_version = -1};
	sprintf(buf, "/dev/%s", IOCTL_NAME);
	int fd_ioctl = open(buf, O_RDWR);
	if (fd_ioctl == -1) {
		printf("open : %s\n", strerror(errno));
		return -1;
	}

	// Rev 1:
	strcpy(buf, "really old.\n");
	write(fd_file, buf, strlen(buf));

	read(fd_file, out_buf, 4);
	printf("Rev 1: %s\n", out_buf);

	// Rev 2:
	strcpy(buf, "old.\n");
	write(fd_file, buf, strlen(buf));

	read(fd_file, out_buf, 4);
	printf("Rev 2: %s\n", out_buf);

	// Rev 3:
	strcpy(buf, "new.\n");
	write(fd_file, buf, strlen(buf));

	read(fd_file, out_buf, 4);
	printf("Rev 3: %s\n", out_buf);

	// Rollback to Rev 2:
	ioctl(fd_ioctl, CHANGE_VER, -1);

	read(fd_file, out_buf, 4);
	printf("Rollback to Rev 2: %s\n", out_buf);

	// Delete Rev 3:
	ioctl(fd_ioctl, NEW_LATEST, &i);

	// Rev 3 (new branch):
	strcpy(buf, "brand new.\n");
	write(fd_file, buf, strlen(buf));

	read(fd_file, out_buf, 10);
	printf("Rev 3 (new branch): %s\n", out_buf);

	return 0;
}
