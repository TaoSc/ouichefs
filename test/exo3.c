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
// Et que l'inode passé en paramètre ait subi plusieurs modifications
int main(int argc, char *argv[])
{
	if (argc != 2)
		printf("Format: exo3 ino nb_revs");

	int ino = atoi(argv[0]);
	int nb_revs = atoi(argv[1]);

	struct ioctl_request i = {.ino = ino, .nb_version = nb_revs};
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
