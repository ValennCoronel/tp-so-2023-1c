

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
#include<commons/collections/dictionary.h>

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>


typedef struct {
	int grado_max_multiprogramacion;
	int conexion_memoria;
	char* algoritmo_planificacion;
}planificar_nuevos_procesos_largo_plazo_args;


extern t_dictionary* recurso_bloqueado;

extern t_dictionary* colas_de_procesos_bloqueados_para_cada_archivo;

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_pcb* proceso_ejecutando;
extern t_temporal* rafaga_proceso_ejecutando;

extern sem_t m_cola_ready;
extern sem_t m_cola_new;
extern sem_t consumidor;
extern sem_t m_proceso_ejecutando;

void inicializar_colas_y_semaforos();
void *planificar_nuevos_procesos_largo_plazo(void *arg);
void agregar_proceso_a_ready(int conexion_memoria,  char* algoritmo_planificacion);
int puede_ir_a_ready(int grado_max_multiprogramacion);
void agregar_cola_new(t_pcb* pcb_proceso);

t_tabla_de_segmento* obtener_tabla_segmentos(int conexion_memoria, int pid);
char* listar_pids_cola_ready(void);
void pasar_a_ready(t_pcb* proceso_bloqueado,int grado_max_multiprogramacion);

#endif /* SRC_PLANIFICADOR_LARGO_PLAZO_H_ */
