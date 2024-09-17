# fisop-fs

# Estructuras y Constantes utilizadas

Se utilizó la siguiente estructura para la realización del file system:

struct block{
	char contenido [MAX_CONTENT];	// Block content
	char path [MAX_PATH];		// Block path
	int type;           			// Block type 
	int estado;				// Block status
	mode_t mode;    	 		// Permissions
	uid_t 	uid;     				// User ID of owner 
	gid_t 	gid;     				// Group ID of owner 
	off_t 	size;    				// Total size, in bytes 
	time_t ultimo_acceso;  		// Time of last access 
	time_t ultima_modificacion;  		// Time of last modification 
};

utilizada tanto para archivos como para directorios

#define MAX_CONTENT 4096		// Contenido de cada bloque de datos
#define DATA_BLOCKS 200			// Cantidad de bloques de datos
#define MAX_PATH 100			// Largo del path 
#define DIR 1					// Representa que el bloque es un directorio
#define FIL 0					// Representa que el bloque es un archivo
#define LIBRE 0				// Representa que el bloque está libre
#define OCUPADO 1				// Representa que el bloque está ocupado

# Búsqueda de archivos y directorios
Para la búsqueda de archivos y directorios se implementaron dos funciones.
Por un lado, una búsqueda general mediante el path del bloque de datos: 


struct block *
find_block_for_path(const char* path)
{
	for (int i = 0; i < DATA_BLOCKS; i++) {
		if (strcmp(path, data_blocks[i].path) == 0) {
			return &data_blocks[i];
		}
	}
	return NULL;  // No se encontró ningún bloque con ese path
}

Se recorre el arreglo donde se almacenan los bloques de datos con el objetivo de encontrar uno de ellos que coincida con el path pasado por parámetro. 
La otra función es muy similar a la anterior con la diferencia de que devuelve el primer bloque libre que encuentra en el arreglo de bloque de datos:
struct block* find_free_block() {
   for (int i = 0; i < DATA_BLOCKS; i++) {
       if (data_blocks[i].estado == LIBRE) {
           return &data_blocks[i];
       }
   }
   return NULL;  // No se encontró ningún bloque libre
}


# Persistencia
La persistencia en disco es lo que permite que nuestro sistema de archivos mantenga sus datos y metadatos al desmontar y montar nuevamente. Al desmontar, se persiste toda la información necesaria.
La persistencia podríamos decir que consiste en dos procesos: uno de escritura(que ocurre en flush y en destroy) y otro de lectura (en init).
En nuestra implementación del sistema de archivos la persistencia solo anda cuando se lo ejecuta en foreground con el parámetro -f.


# PRUEBAS 

Como primer paso, debemos compilar nuestro código con make
Luego, debemos montar el filesystem en una carpeta. Para eso, dentro de la carpeta que contiene nuestro proyecto, creamos una carpeta ‘prueba’.
En la carpeta que contiene nuestro filesystem, abrimos una terminal y montamos el filesystem en la carpeta ‘prueba’ con el siguiente comando:
$ ./fisopfs -f prueba/ 
Por último, en otra terminal abierta en la carpeta que contiene nuestro filesystem, ejecutamos las pruebas con los siguiente comandos: 




Creación de un directorio en prueba
$ mkdir prueba
$ cd prueba
$ mkdir directory 
$ ls prueba
directory




Creación de un archivo en prueba
$ mkdir prueba
$ cd prueba
$ touch file.txt 
$ ls prueba
file.txt




Lectura del archivo file.txt con el texto “Hola, soy file.txt”
$ cat file.txt
Hola, soy file.txt




Redireccion de salida estandar
$ echo "hola, soy file.txt" > file.txt
$ cat file.txt
$ hola, soy file.txt




Eliminación de un archivo
$ touch file.txt hola.txt
$ ls
$ file.txt hola.txt
$ unlink file.txt
$ ls
hola.txt





Eliminación de un directorio
$ mkdir directory1 directory2
$ ls
$ directory1 directory2
$ rmdir directory2
$ ls
directory1


