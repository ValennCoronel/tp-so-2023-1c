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
#include <pthread.h>

#include "utils_server.h"
#include "planificador_largo_plazo.h"

void finalizar_proceso(int socket_cliente);
void bloquear_proceso(int socket_cliente);
void manejar_peticion_al_kernel(int socket_cliente);
void desalojar_proceso(int socket_cliente);
void apropiar_recursos(int socket_cliente, char** recursos, int* recurso_disponible);
void desalojar_recursos(int cliente_fd,char** recursos, int* recurso_disponible);
void bloquear_proceso_IO(int socket_cliente);
int obtener_indice_recurso(char** recursos, char* recurso_a_buscar);
void bloquear_proceso_por_recurso(t_pcb* proceso_a_bloquear, char* nombre_recurso);
void poner_a_ejecutar_otro_proceso();

#endif /* SRC_PETICIONES_CPU_H_ */
