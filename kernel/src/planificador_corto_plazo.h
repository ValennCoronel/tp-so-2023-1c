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
#include "utils_cliente.h"
#include "utils_server.h"
#include "planificador_largo_plazo.h"


typedef struct {
	char* algoritmo_planificacion;
	double hrrn_alfa;
}planificar_corto_plazo_args;

void *planificar_corto_plazo(void *arg);
void planificar_corto_plazo_fifo();
void planificar_corto_plazo_hrrn(double hrrn_alpha);

#endif /* SRC_PLANIFICADOR_CORTO_PLAZO_H_ */
