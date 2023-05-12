

#ifndef SRC_PLANIFICADOR_LARGO_PLAZO_H_
#define SRC_PLANIFICADOR_LARGO_PLAZO_H_

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
#include <commons/temporal.h>
#include<semaphore.h>


#include "utils_cliente.h"
#include "utils_server.h"


typedef struct {
	int grado_max_multiprogramacion;
	int conexion_memoria;
}planificar_nuevos_procesos_largo_plazo_args;

typedef	struct {
	int id;
	int direccion_base;
	int tamano;
} segmento;

typedef struct {
	int archivo_id;
	int puntero;
} archivo;

typedef struct {
    char AX[4];   // Registro de 4 bytes
    char BX[4];   // Registro de 4 bytes
    char CX[4];   // Registro de 4 bytes
    char DX[4];   // Registro de 4 bytes
    char EAX[8];  // Registro de 8 bytes
    char EBX[8];  // Registro de 8 bytes
    char ECX[8];  // Registro de 8 bytes
    char EDX[8];  // Registro de 8 bytes
    char RAX[16]; // Registro de 16 bytes
    char RBX[16]; // Registro de 16 bytes
    char RCX[16]; // Registro de 16 bytes
    char RDX[16]; // Registro de 16 bytes
} registros_CPU;

typedef struct
{
	int PID;
	t_list* instrucciones;
	int program_counter;

	registros_CPU* registros_CPU;

	double estimado_proxima_rafaga;
	int64_t tiempo_llegada_rady;

	t_list* tabla_segmentos;
	t_list* tabla_archivos;

	t_temporal* temporal;
} t_pcb;

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern sem_t productor;
extern sem_t consumidor;

void inicializar_colas_y_semaforos();
void *planificar_nuevos_procesos_largo_plazo(void *arg);
void agregar_proceso_a_ready(int conexion_memoria);
int puede_ir_a_ready(int grado_max_multiprogramacion);
void agregar_cola_new(t_pcb* pcb_proceso);

t_list* obtener_tabla_segmentos(int conexion_memoria);
char* listar_pids_cola_ready(void);

#endif /* SRC_PLANIFICADOR_LARGO_PLAZO_H_ */
