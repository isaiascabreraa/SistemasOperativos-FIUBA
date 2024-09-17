#include <sys/types.h>
#include <time.h>


#define MAX_CONTENT 4096
#define DATA_BLOCKS 200
#define MAX_PATH 100
#define DIR 1
#define FIL 0
#define LIBRE 0
#define OCUPADO 1


struct block {
	char contenido[MAX_CONTENT];
	char path[MAX_PATH];
	int type; /* File type */
	int estado;
	mode_t mode;                /* Permissions */
	uid_t uid;                  /* User ID of owner */
	gid_t gid;                  /* Group ID of owner */
	off_t size;                 /* Total size, in bytes */
	time_t ultimo_acceso;       /* Time of last access */
	time_t ultima_modificacion; /* Time of last modification */
};
