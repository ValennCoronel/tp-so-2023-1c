/*
 * filesystem.h
 *
 *  Created on: Apr 14, 2023
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>

#include "utils_server.h"
#include "utils_cliente.h"

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(t_log*, t_config*);
int conectar_con_memoria(int conexion, char* ip, char* puerto);
void manejar_peticiones_kernel(int server_fd);
void atender_cliente(void *args);

#endif /* CPU_H */
