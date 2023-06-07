/*
 * planificador_corto_plazo.h
 *
 *  Created on: May 2, 2023
 *      Author: utnso
 */

#ifndef SRC_PLANIFICADOR_CORTO_PLAZO_H_
#define SRC_PLANIFICADOR_CORTO_PLAZO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>

#include "planificador_largo_plazo.h"


typedef struct {
	char* algoritmo_planificacion;
	double hrrn_alfa;
	int socket_cpu;
}planificar_corto_plazo_args;



void *planificar_corto_plazo(void *arg);
void planificar_corto_plazo_fifo(int socket_cpu);
void planificar_corto_plazo_hrrn(double hrrn_alpha, int socket_cpu);
void enviar_contexto_de_ejecucion_a(t_contexto_ejec* proceso_a_ejecutar, op_code opcode, int socket_cliente);

double calcular_prioridad_con_hrrn(int64_t tiempo_de_espera, double tiempo_proxima_rafaga );
double estimar_proxima_rafaga_proceso(double hrrn_alpha, int64_t anterior_rafaga, int64_t anterior_estimado );
int64_t calcular_tiempo_de_espera(t_pcb* pcb_proceso);
void reordenar_cola_ready_hrrn();


#endif /* SRC_PLANIFICADOR_CORTO_PLAZO_H_ */
