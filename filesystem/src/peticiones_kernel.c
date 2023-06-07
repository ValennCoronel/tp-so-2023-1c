#include "peticiones_kernel.h"


// TODO responder con mensajes genéricos
void abrir_archivo(int socket_kernel){
/*
 * Esta operación consistirá en verificar que exista el FCB correspondiente al
 * archivo y en caso de que exista deberá devolver un OK, caso contrario, deberá informar
 * que el archivo no existe.
 * */
	//enviar_mensaje("OK",socket_kernel);
	//enviar_mensaje("ERROR, NO EXISTE ESE ARCHIVO",socket_kernel);
}
void crear_archivo(int socket_kernel){
/*
 * Para esta operación se deberá crear un archivo FCB correspondiente al nuevo archivo, con tamaño 0 y
 * sin bloques asociados.
Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.
 *
 * */
	//enviar_mensaje("OK",socket_kernel);
}
void truncar_archivo(int socket_kernel){
/*
 *
 * Al momento de truncar un archivo, pueden ocurrir 2 situaciones:
	- Ampliar el tamaño del archivo: Al momento de ampliar el tamaño del archivo deberá actualizar
		el tamaño del archivo en el FCB y se le deberán asignar tantos bloques como sea necesario para poder
		direccionar el nuevo tamaño.
	- Reducir el tamaño del archivo: Se deberá asignar el nuevo tamaño del archivo en el FCB y
		se deberán marcar como libres todos los bloques que ya no sean necesarios para direccionar el
		tamaño del archivo (descartando desde el final del archivo hacia el principio).
 * */
}
void leer_archivo(int socket_kernel, int socket_memoria){
/*
 * Esta operación deberá leer la información correspondiente de los bloques a partir del puntero
 * y el tamaño recibidos. Esta información se deberá enviar a la Memoria para ser escrita a partir
 * de la dirección física recibida por parámetro y esperar su finalización para poder confirmar el éxito
 * de la operación al Kernel.
 *
 * */
}

void escribir_archivo(int socket_kernel, int socket_memoria){
/*
 * Se deberá solicitar a la Memoria la información que se encuentra a partir de la dirección física y
 *  escribirlo en los bloques correspondientes del archivo a partir del puntero recibido.
El tamaño de la información a leer de la memoria y a escribir en los bloques también deberá recibirse
por parámetro desde el Kernel.
 * */
}

/*
 * Persistencia
Todas las operaciones que se realicen sobre los FCBs, Bitmap y Bloques deberán mantenerse actualizadas
en disco a medida que ocurren.
En caso de utilizar la función mmap(), se recomienda investigar el uso de la función msync() para tal fin
 * */
