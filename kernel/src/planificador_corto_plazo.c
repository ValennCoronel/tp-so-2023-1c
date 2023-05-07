#include "planificador_corto_plazo.h"

void *planificar_corto_plazo(void *arg){
	planificar_corto_plazo_args* args = (planificar_corto_plazo_args*) arg;

	//FIFO O HRRN
	char *algoritmo_planificacion = args->algoritmo_planificacion;
	double hrrn_alpha = args->hrrn_alfa;

	if(strcmp(algoritmo_planificacion, "FIFO") == 0){
		planificar_corto_plazo_fifo();
	} else {
		planificar_corto_plazo_hrrn(hrrn_alpha);
	}

	return NULL;
}

void planificar_corto_plazo_fifo(){
	//planificar los procesos con fifo

	sem_wait(&consumidor);
	t_pcb *proceso_a_ejecutar = queue_pop(cola_ready);
	sem_post(&productor);
	//enviar proceso a cpu

}

void planificar_corto_plazo_hrrn(double hrrn_alpha){
	//planificar los procesos con hrrn

}
