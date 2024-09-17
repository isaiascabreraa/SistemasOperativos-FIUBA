#define FUSE_USE_VERSION 30
#define MAX_CONTENIDO 100

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "types.h"

char fs_file[MAX_PATH] = "file.fisopfs";
struct block data_blocks[DATA_BLOCKS];


// Funciones auxiliares:

// Pre: -
// Post: Busca un bloque libre (el primero que encuentre le sirve).
struct block *
find_free_block()
{
	for (int i = 0; i < DATA_BLOCKS; i++) {
		if (data_blocks[i].estado == LIBRE) {
			return &data_blocks[i];
		}
	}
	return NULL;  // No se encontró ningún bloque libre
}

// Pre: -
// Post: Busca un bloque que tenga un PATH determinado.
struct block *
find_block_for_path(const char *path)
{
	for (int i = 0; i < DATA_BLOCKS; i++) {
		if (strcmp(path, data_blocks[i].path) == 0) {
			return &data_blocks[i];
		}
	}
	return NULL;  // No se encontró ningún bloque con ese path
}


static void *
fisopfs_init()
{
	printf("[debug] fisopfs_init\n");

	FILE *file = fopen(fs_file, "r");

	if (!file) {
		struct block root;
		root.estado = OCUPADO;
		root.type = DIR;
		root.mode = __S_IFDIR | 0755;
		root.size = MAX_CONTENT;
		root.uid = getuid();
		root.gid = getgid();
		root.ultimo_acceso = time(NULL);
		root.ultima_modificacion = time(NULL);
		strcpy(root.path, "/");
		memset(root.contenido, 0, sizeof(root.contenido));
		data_blocks[0] = root;


		for (int i = 1; i < DATA_BLOCKS; i++) {
			struct block bloque = { 0 };
			data_blocks[i] = bloque;
		}

	} else {
		int n = fread(&data_blocks, sizeof(data_blocks), 1, file);

		if (n != 1) {
			fprintf(stderr,
			        "[debug] Error init: %s\n",
			        strerror(errno));
			return NULL;
		}
		fclose(file);
	}
	return NULL;
}

static int
fisops_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	struct block *bloque = find_block_for_path(path);

	if (bloque->type == FIL) {
		bloque->estado = LIBRE;
		memset(bloque, 0, sizeof(struct block));
	} else {
		return -EISDIR;
	}

	return 0;
}


static int
fisops_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	// Encontrar el bloque correspondiente al directorio
	struct block *bloque = find_block_for_path(path);
	if (!bloque) {
		fprintf(stderr, "[debug] Error rmdir: no se encontró el directorio %s\n", path);
		return -ENOENT;
	}

	// Chequear si el bloque es un directorio
	if (bloque->type != DIR) {
		fprintf(stderr,
		        "[debug] Error rmdir: %s no es un directorio\n",
		        path);
		return -ENOTDIR;
	}


	int contador = 0;

	for (int i = 0; i < DATA_BLOCKS; i++) {
		if (strstr(data_blocks[i].path, path) != NULL) {
			contador++;
		}
	}

	if (contador == 1) {
		// Marcar el bloque como libre y limpiarlo
		bloque->estado = LIBRE;
		memset(bloque, 0, sizeof(struct block));

	} else {
		fprintf(stderr,
		        "[debug] Error rmdir: el directorio %s no está vacío\n",
		        path);
		return -ENOTEMPTY;
	}

	contador = 0;

	return 0;
}


static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct block *bloque = find_block_for_path(path);
	if (!bloque) {
		printf("[debug] no se encontro el bloque (%s)\n", path);
		return -ENOENT;
	}

	if (offset < bloque->size) {
		if (offset + size > bloque->size)
			size = bloque->size - offset;
		memcpy(buffer, bloque->contenido + offset, size);
	} else
		size = 0;

	return size;
}


// Pre: -
// Post: Guarda todo el data_blocks en fs_file para que persista en memoria.
void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");

	FILE *file = fopen(fs_file, "w");
	if (!file) {
		fprintf(stderr,
		        "[debug] Error saving filesystem: %s\n",
		        strerror(errno));
		return;
	}
	fwrite(&data_blocks, sizeof(data_blocks), 1, file);
	fflush(file);
	fclose(file);
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	// Busca un bloque de datos libre
	struct block *new_block = find_free_block();
	if (new_block == NULL) {
		return -ENOSPC;  // No hay espacio en el dispositivo
	}

	new_block->estado = OCUPADO;
	new_block->type = DIR;
	new_block->mode = __S_IFDIR | mode;
	new_block->size = MAX_CONTENT;
	new_block->uid = getuid();
	new_block->gid = getgid();
	new_block->ultimo_acceso = time(NULL);
	new_block->ultima_modificacion = time(NULL);
	strcpy(new_block->path, path);
	memset(new_block->contenido, 0, sizeof(new_block->contenido));


	return 0;
}


static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir(%s)\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);


	struct block *bloque = find_block_for_path(path);
	if (!bloque) {
		printf("[debug] no se encontro bloque de path (%s)\n", path);
		return -ENOENT;
	}

	bloque->ultimo_acceso = time(NULL);

	for (int i = 0; i < MAX_CONTENIDO; i++) {
		if (data_blocks[i].estado != LIBRE) {
			char *ret = strrchr(data_blocks[i].path,
			                    '/');  // nombre con / al principio

			if (strlen(ret) == 1) {  // si es root continuo
				continue;
			}

			char dir[strlen(data_blocks[i].path) - strlen(ret)];

			memcpy(dir,
			       data_blocks[i].path,
			       strlen(data_blocks[i].path) - strlen(ret) +
			               1);  // copio el dir del path

			dir[strlen(data_blocks[i].path) - strlen(ret)] =
			        '\0';  // cambio el / final por '\0'

			if (strlen(dir) == 0) {
				if (strcmp("/", bloque->path) == 0)
					filler(buffer, ret + 1, NULL, 0);

			} else {
				if (strcmp(dir, bloque->path) == 0)
					filler(buffer, ret + 1, NULL, 0);
			}
		}
	}

	return 0;
}


static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	struct block *bloque;

	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;

	} else {
		bloque = find_block_for_path(path);
		if (!bloque) {
			printf("[debug] no se encontro el bloque (%s)\n", path);
			return -ENOENT;
		}
		st->st_mode = bloque->mode;
		st->st_size = bloque->size;
		st->st_uid = bloque->uid;
		st->st_gid = bloque->gid;
		st->st_atime = bloque->ultimo_acceso;
		st->st_mtime = bloque->ultima_modificacion;
		st->st_nlink = 1;
	}

	return 0;
}

static int
fisops_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);

	// Busca un bloque de datos libre
	struct block *new_block = find_free_block();
	if (new_block == NULL) {
		return -ENOSPC;  // No hay espacio en el dispositivo
	}

	// Inicializa el nuevo bloque
	strcpy(new_block->path, path);
	new_block->type = FIL;
	new_block->estado = OCUPADO;
	new_block->mode = mode;
	new_block->uid = getuid();
	new_block->gid = getgid();
	new_block->size = 0;
	memset(new_block->contenido, 0, sizeof(new_block->contenido));
	new_block->ultimo_acceso = time(NULL);
	new_block->ultima_modificacion = time(NULL);

	return 0;
}

static int
fisops_write(const char *path,
             const char *buf,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s\n", path);
	// Encuentra el bloque de datos para este archivo
	struct block *block = find_block_for_path(path);
	if (block == NULL) {
		int res = fisops_create(path, 33204, fi);
		if (res < 0) {
			return res;
		}
		block = find_block_for_path(path);
	}

	// Verifica que el tamaño de la escritura no exceda el tamaño máximo del bloque
	if (offset + size > MAX_CONTENT) {
		return -EFBIG;  // El archivo es demasiado grande
	}

	// Escribe los datos en el bloque
	memcpy(block->contenido + offset, buf, size);
	block->size = offset + size;
	block->ultima_modificacion = time(NULL);

	return size;
}

static int
fisops_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	// Encuentra el bloque de datos para este archivo
	struct block *block = find_block_for_path(path);
	if (block == NULL) {
		return -ENOENT;  // No existe el archivo o directorio
	}

	// Verifica que el tamaño no exceda el tamaño máximo del bloque
	if (size > MAX_CONTENT) {
		return -EFBIG;  // El archivo es demasiado grande
	}
	block->size = size;
	block->ultima_modificacion = time(NULL);

	return 0;
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush(%s)\n", path);
	fisopfs_destroy(NULL);
	return 0;
}


static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);

	// Encuentra el bloque de datos para este archivo
	struct block *block = find_block_for_path(path);
	if (block == NULL) {
		return -ENOENT;  // No existe el archivo o directorio
	}

	// Actualiza los tiempos de acceso y modificación
	block->ultimo_acceso = ts[0].tv_sec;
	block->ultima_modificacion = ts[1].tv_sec;

	return 0;
}


static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.read = fisopfs_read,
	.create = fisops_create,
	.write = fisops_write,
	.truncate = fisops_truncate,
	.unlink = fisops_unlink,
	.rmdir = fisops_rmdir,
	.destroy = fisopfs_destroy,
	.utimens = fisopfs_utimens,
	.mkdir = fisopfs_mkdir,
	.readdir = fisopfs_readdir,
	.init = fisopfs_init,
	.flush = fisopfs_flush,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
