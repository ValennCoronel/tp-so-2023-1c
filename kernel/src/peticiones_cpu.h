/*
 * peticiones_cpu.h
 *
 *  Created on: May 2, 2023
 *      Author: utnso
 */

#ifndef SRC_PETICIONES_CPU_H_
#define SRC_PETICIONES_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/string.h>
#include <commons/collections/node.h>
#include <commons/collections/list.h>

#include "utils_server.h"

void finalizar_proceso(int socket_cliente);
void bloquear_proceso(int socket_cliente);
void manejar_peticion_al_kernel(int socket_cliente);
void desalojar_proceso(int socket_cliente);

#endif /* SRC_PETICIONES_CPU_H_ */
