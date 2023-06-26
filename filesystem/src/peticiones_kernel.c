#include "peticiones_kernel.h"
#include "filesystem.h"


void abrir_archivo(int socket_kernel){
/*

Esta operación consistirá en verificar que exista el FCB correspondiente al
archivo y en caso de que exista deberá devolver un OK, caso contrario, deberá informar
que el archivo no existe.
*///enviar_mensaje("OK",socket_kernel);//enviar_mensaje("ERROR, NO EXISTE ESE ARCHIVO",socket_kernel);
}

void crear_archivo(int socket_kernel){
/*

Para esta operación se deberá crear un archivo FCB correspondiente al nuevo archivo, con tamaño 0 y
sin bloques asociados.
Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.
 *

*///enviar_mensaje("OK",socket_kernel);
}
void truncar_archivo(int socket_kernel, int socket_memoria){

	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion();

    t_fcb* peticion_truncado = (t_fcb*) malloc(sizeof(t_fcb));

            peticion_truncado->nombre_archivo = instruccion_peticion->parametros[0];
            peticion_truncado->tamanio_archivo = instruccion_peticion->parametros[1];


            enviar_mensaje("OK", socket_kernel, TRUNCAR_ARCHIVO);

}

void leer_archivo(int socket_kernel, int socket_memoria, FILE* bloques){


	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion();



	enviar_peticion_memoria(WRITE_MEMORY,instruccion_peticion);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op != WRITE_MEMORY){
		return;
	}

	int size;
	void *  buffer = recibir_buffer(&size, socket_memoria);

	t_fcb* fcb = dictionary_get(fcb_por_archivo,instruccion_peticion->parametros[0]);


}
/*
 Acceso a espacio de usuario
Tanto CPU como File System pueden, dada una dirección física, solicitar accesos
al espacio de usuario de Memoria. El módulo deberá realizar lo siguiente:

****1Ante un pedido de lectura, devolver el valor que se encuentra en la posición pedida.
****2Ante un pedido de escritura, escribir lo indicado en la posición pedida y responder un mensaje de ‘OK’.

Para simular la realidad y la velocidad de los accesos a Memoria,
cada acceso al espacio de usuario tendrá un tiempo de espera en milisegundos
definido por archivo de configuración.


 */

void enviar_peticion_memoria(op_code code,t_instruccion* instruccion ){
	t_paquete* paquete = crear_paquete(code);

		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

		agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
		enviar_paquete(paquete, socket_memoria);
}


/*

typedef struct {
	char* file;
	char permiso;
	int puntero;

}tabla_de_archivos_por_proceso;


 typedef struct {
	int fileDescriptor;
	char* file;
	int open;


}tabla_global_de_archivos_abiertos;

typedef struct {
	char* nombre_archivo;
	int tamanio_archivo;
	uint32_t puntero_directo;
	uint32_t puntero_indirecto;
} t_fcb;

t_dictionary* fcb_por_archivo

 * */

void escribir_archivo(int socket_kernel,int socket_memoria, FILE* bloques){

	t_instruccion* instruccion_peticion = (t_instruccion*) recibir_instruccion();

	enviar_peticion_memoria(READ_MEMORY,instruccion_peticion);

	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op != READ_MEMORY){
		return;
	}

	int size;
	void *  buffer = recibir_buffer(&size, socket_memoria);

	t_fcb* fcb = dictionary_get(fcb_por_archivo,instruccion_peticion->parametros[0]);

}



t_instruccion* recibir_instruccion(){

	int size;
	void *  buffer = recibir_buffer(&size, socket_kernel);
	int desplazamiento=0;

	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	while (desplazamiento<size){


				memcpy(&(instruccion->opcode_lenght), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->opcode = malloc(instruccion->opcode_lenght);
				memcpy(instruccion->opcode, buffer+desplazamiento, instruccion->opcode_lenght);
				desplazamiento+=instruccion->opcode_lenght;

				memcpy(&(instruccion->parametro1_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[0] = malloc(instruccion->parametro1_lenght);
				memcpy(instruccion->parametros[0], buffer + desplazamiento, instruccion->parametro1_lenght);
				desplazamiento += instruccion->parametro1_lenght;

				memcpy(&(instruccion->parametro2_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[1] = malloc(instruccion->parametro2_lenght);
				memcpy(instruccion->parametros[1], buffer + desplazamiento, instruccion->parametro2_lenght);
				desplazamiento += instruccion->parametro2_lenght;

				memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
				memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
				desplazamiento += instruccion->parametro3_lenght;
	}

	return instruccion;
}
/*
 * Persistencia
Todas las operaciones que se realicen sobre los FCBs, Bitmap y Bloques deberán mantenerse actualizadas
en disco a medida que ocurren.
En caso de utilizar la función mmap(), se recomienda investigar el uso de la función msync() para tal fin
 * */
