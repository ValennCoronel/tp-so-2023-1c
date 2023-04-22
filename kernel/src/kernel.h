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

#include "utils_cliente_kernel.h"
#include "utils_server_kernel.h"

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(int conexion, int conexion2, int conexion3, t_log* logger, t_config* config);
int conectar_con_memoria(int conexion, char* ip, char* puerto);
void manejar_peticiones_kernel(t_log* logger, int server_fd);

#endif /* FILESYSTEM_H_ */
