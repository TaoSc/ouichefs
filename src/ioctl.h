#include <asm-generic/ioctl.h>

#define IOCTL_NAME  "ouichefs_ioctl"
#define CHANGE_VER  _IOWR('0', 1, int32_t) 
#define NEW_LATEST  _IOWR('1', 2, int32_t) 

struct ioctl_request {
	int ino;
	int nb_version;
};
