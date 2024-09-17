# shell

### Búsqueda en $PATH

¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (exec(3)) se utilizan para ejecutar un nuevo programa en un proceso hijo.

Diferencias:

Nombre y argumentos:
execve(2): Recibe el nombre del archivo ejecutable y una matriz de argumentos como argumentos.
exec(3): Existen varias funciones en esta familia (execl, execlp, execle, execv, execvp, execvpe) que proporcionan diferentes formas de pasar los argumentos al nuevo programa, pero todas ellas ocultan la necesidad de pasar explícitamente un arreglo de punteros de caracteres como argumentos.

Entorno:
execve(2): Permite especificar el entorno del nuevo programa a través de un arreglo de punteros de caracteres.
exec(3): Usa el entorno actual del proceso.

Error Handling:
execve(2): Devuelve -1 en caso de error y establece errno para indicar el tipo específico de error.
exec(3): La mayoría de las funciones en esta familia no devuelven un valor si tienen éxito; solo retornan si hay un error, estableciendo errno en caso de fallo.

Uso de PATH:
execve(2): Requiere especificar la ruta completa del archivo ejecutable si no está en la variable de entorno PATH.
exec(3): Puede buscar el archivo ejecutable en las rutas especificadas en la variable de entorno PATH.

¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Sí, la llamada a exec(3) en un programa puede fallar por diversas razones. Algunas de las razones comunes por las cuales exec(3) podría fallar incluyen:
Archivo no encontrado: Si el archivo especificado para ejecutar no se encuentra en la ruta especificada o no tiene los permisos adecuados para ser ejecutado, exec(3) fallará.
Error de formato o versión del archivo ejecutable: Si el archivo ejecutable especificado tiene un formato incorrecto o no es compatible con la versión del sistema operativo, la llamada a exec(3) puede fallar.
Error de memoria insuficiente: Si el sistema operativo no puede asignar suficiente memoria para cargar el programa ejecutable, la llamada a exec(3) también puede fallar.


### Procesos en segundo plano

Mecanismo utilizado para implementar procesos en segundo plano:
Se crea un proceso el cual tiene su propio ID de proceso (PID).
La shell espera a que el programa termine su ejecución antes de devolver el control al usuario (ejecucion en primer plano).
Se ejecuta el proceso en segundo plano. Para ello se agrega el carácter & al final del comando en la línea de comandos. Esto le indica a la shell que inicie el proceso en segundo plano y devuelva el control al usuario de inmediato, permitiéndole continuar con otras tareas en la misma terminal.
La shell le asigna  un nuevo process group al proceso para controlarlo de manera independiente del proceso principal de la terminal.
Mientras se ejecuta en segundo plano, la shell sigue estando disponible para recibir más comandos..

### Flujo estándar

La expresión 2>&1 es un operador de redirección que se utiliza para redirigir la salida estándar de error (stderr) a la misma ubicación que la salida estándar (stdout).

Forma general de ‘2>&1’:
2. Representa el descriptor de archivo para la salida estándar de error (stderr).

>. Es el operador de redirección que se utiliza para redirigir la salida de un flujo de datos a otro destino, como un archivo o otro descriptor de archivo.

&1. Representa el descriptor de archivo para la salida estándar (stdout).

Salida de cat out.txt en el ejemplo:

![Cat 1 .jpg](Images/Cat%201%20.jpg)

salida de cat out.txt invirtiendo el orden de las redirecciones:


![Cat 2.jpg](Images/Cat%202.jpg)

se puede ver que no cambió en nada la salida del cat.

se compara con el comportamiento en bash:

![Cat 3.jpg](Images/Cat%203.jpg)
![Cat 4.jpg](Images/Cat%204.jpg)

Se puede ver como en el primer caso coincide con la salida de nuestra shell mientras que en el segundo caso esto no sucede

### Tuberías múltiples

El exit code reportado por la shell en el contexto de un pipe depende del comportamiento de los comandos individuales involucrados en el mismo y de cómo se gestionan los errores en cada uno de ellos.
Si todos los comandos en el pipe se ejecutan exitosamente (es decir, todos devuelven  0), entonces el exit code reportado por la shell será 0, indicando que todo el pipe se ejecutó correctamente. En cambio , si uno de los comandos en el pipe falla (es decir, devuelve un código de salida distinto de 0), la shell aún ejecutará los comandos restantes en la tubería. Sin embargo, el exit code reportado por la shell en este caso será el código de salida del último comando que falló en el pipe.

![Comando que falla 1.jpg](Images/Comando%20que%20falla%201.jpg)
![comando que falla 2.jpg](Images/comando%20que%20falla%202.jpg)

Se puede ver como comparando el comportamiento de nuestra implementacion con el de bash, el resultado es el mismo.


### Variables de entorno temporarias

¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario configurar las variables de entorno después de llamar a fork(2) porque cualquier modificación en las variables de entorno antes de fork(2) afectaría tanto al proceso padre como al hijo. Si se modifican las variables de entorno después de fork(2), solo afectarán al proceso hijo, lo que es el comportamiento deseado para variables temporales específicas de un proceso en segundo plano.

En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3). ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

El comportamiento no sería el mismo que al usar setenv(3) individualmente para cada variable de entorno. Cuando se pasan las variables de entorno en un arreglo como tercer argumento de las funciones de la familia exec(3), el nuevo proceso creado por él utilizara exactamente esas variables de entorno y no “heredará” ninguna otra variable de entorno del proceso padre. Por otro lado, cuando se utilizar setenv(3) para cada variable de entorno antes del exec(3), el proceso hijo hereda todas las variables de entorno del proceso padre y se le añaden las nuevas definidas con setenv(3).

### Pseudo-variables

$! - PID del último proceso en segundo plano:
Propósito: Esta variable contiene el ID de proceso (PID) del último proceso ejecutado en segundo plano. Es útil cuando se necesita saber el PID de un proceso en segundo plano para realizar operaciones como detenerlo, revisar su estado, o redirigir señales a él.
Ejemplo de uso:

![$!.jpg](Images/%24%21.jpg)

$$ - PID del proceso actual:
Propósito: Contiene el PID del proceso actual en ejecución. Esto es útil en scripts o programas para identificar de manera única el proceso en sí mismo o para generar nombres de archivos temporales únicos.
Ejemplo de uso:

![$$.jpg](Images/%24%24.jpg)

$0 - Nombre del script o comando:
Propósito: Esta variable contiene el nombre del script o comando que se está ejecutando actualmente. Es útil para mostrar información sobre el script o comando en sí mismo dentro de un script, especialmente cuando se ejecutan múltiples scripts o comandos en una secuencia.
Ejemplo de uso:

![$0.jpg](Images/%2F%240.jpg)

### Comandos built-in
¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in?

pwd podría implementarse sin necesidad de ser un built-in porque simplemente imprime la ruta del directorio actual de trabajo. Esta implementación puede realizarse utilizando la función getcwd(3) en cualquier proceso, sin necesidad de ser un comando agregado en la shell. Sin embargo, al hacerlo built-in se ejecutaría en el mismo proceso de la shell, lo que significa que no habría necesidad de crear un nuevo proceso para ejecutar getcwd(3). Esto hace que sea más eficiente en términos de uso de recursos y tiempos de ejecución.


### Segundo plano avanzado

El uso de señales es necesario porque estas proporcionan un mecanismo para notificar, controlar y comunicarse con los procesos en background.
Con la señal SIGCHLD por ejemplo podemos notificar cuando un proceso en segundo plano terminó, lo que permite que la shell haga acciones según sea necesario con esa información.
