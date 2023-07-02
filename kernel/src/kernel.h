/*
 * kernel.h
 *
 *  Created on: Apr 14, 2023
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/node.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>
#include "planificador_largo_plazo.h"
#include "planificador_corto_plazo.h"
#include "peticiones_cpu.h"
#include "peticiones_fs.h"

extern int socket_cpu;
extern int socket_kernel;
extern int socket_memoria;
extern int socket_fs;
extern int grado_max_multiprogramacion;



typedef struct {
	int server_fd;
	int estimacion_inicial;
}manejar_peticiones_cosola_args;

typedef struct {
	int consola_fd;
	int estimacion_inicial;
}atender_cliente_args;


t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(t_log* logger, t_config* config);
int conectar_modulo(int *conexion, char* ip, char* puerto);

int conectar_memoria(char* ip, char* puerto);
int conectar_fs(char* ip, char* puerto);
int conectar_cpu(char* ip, char* puerto);

void *manejar_peticiones_consola(void *arg);
void *atender_cliente(void *args);
void recibir_instrucciones(int socket_cliente, int estimacion_inicial);

void *escuchar_peticiones_cpu(int cliente_fd,char** recursos,char** instancias_recursos, int grado_max_multiprogramacion, int conexion_memoria);



#endif /* FILESYSTEM_H_ */
