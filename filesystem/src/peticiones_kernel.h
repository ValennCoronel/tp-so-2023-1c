#ifndef PETICIONES_KERNEL_H_
#define PETICIONES_KERNEL_H_

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>
#include "filesystem.h"
#include <math.h>
typedef struct {
	int puntero;
	t_instruccion* instruccion;
} t_instruccion_y_puntero;


//void abrir_archivo(); //TODO
void crear_archivo(int socket_kernel);
void truncar_archivo(int socket_kernel, int socket_memoria,FILE* bloques,t_superbloque* superbloque);
void leer_archivo(int socket_kernel, int socket_memoria, FILE* bloques,int tamanio_bloque);
void escribir_archivo(int socket_kernel,int socket_memoria, FILE* bloques,int tamanio_bloque);
t_fcb* iniciar_fcb(t_config* config);
t_fcb* crear_fcb(t_config* config, t_instruccion* instruccion, char* path);
t_instruccion_y_puntero* recibir_instruccion_y_puntero_kernel(int socket_kernel);
t_instruccion* recibir_instruccion(int socket_cliente);

#endif /* PETICIONES_KERNEL_H_ */
