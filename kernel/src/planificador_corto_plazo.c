#include "planificador_corto_plazo.h"

void *planificar_corto_plazo(void *arg){
	planificar_corto_plazo_args* args = (planificar_corto_plazo_args*) arg;

	//FIFO O HRRN
	char *algoritmo_planificacion = args->algoritmo_planificacion;
	double hrrn_alpha = args->hrrn_alfa;
	int socket_cpu = args->socket_cpu;

	if(strcmp(algoritmo_planificacion, "FIFO") == 0){
		planificar_corto_plazo_fifo(socket_cpu);
	} else {
		planificar_corto_plazo_hrrn(hrrn_alpha, socket_cpu);
	}

	return NULL;
}

void planificar_corto_plazo_fifo(int socket_cpu){
	//planificar los procesos con fifo

	sem_wait(&consumidor);
	t_pcb *proceso_a_ejecutar = queue_pop(cola_ready);
	sem_post(&productor);

	//envio proceso a cpu

	t_paquete* paquete = crear_paquete(PROCESAR_INSTRUCCIONES);
	agregar_a_paquete(paquete, proceso_a_ejecutar, sizeof(proceso_a_ejecutar));//TODO ver si esta bien
	enviar_paquete(paquete, socket_cpu);

	eliminar_paquete(paquete);

}

void planificar_corto_plazo_hrrn(double hrrn_alpha, int socket_cpu){
	//planificar los procesos con hrrn

}
