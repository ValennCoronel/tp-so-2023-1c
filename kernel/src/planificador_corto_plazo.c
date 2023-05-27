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
	sem_wait(&m_cola_ready);
	t_pcb *proceso_a_ejecutar = queue_pop(cola_ready);
	sem_post(&m_cola_ready);

	//creo el contexto de ejecucion

	t_contexto_ejec* contexto_ejecucion = malloc(sizeof(t_contexto_ejec));
	contexto_ejecucion->lista_instrucciones = proceso_a_ejecutar->instrucciones;
	contexto_ejecucion->program_counter = proceso_a_ejecutar->program_counter;
	contexto_ejecucion->tamanio_lista = list_size(proceso_a_ejecutar->instrucciones);

	//Lo envio proceso a CPU

	t_paquete* paquete = crear_paquete(PROCESAR_INSTRUCCIONES);

	agregar_a_paquete_sin_agregar_tamanio(paquete, contexto_ejecucion->tamanio_lista, sizeof(int));

	for(int i =0; i<contexto_ejecucion->tamanio_lista; i++){
		t_instruccion* instruccion = list_get(contexto_ejecucion->lista_instrucciones, i);


		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght);

		agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);

	}

	agregar_a_paquete_sin_agregar_tamanio(paquete, contexto_ejecucion->program_counter, sizeof(int));


	enviar_paquete(paquete, socket_cpu);

	eliminar_paquete(paquete);
}

void planificar_corto_plazo_hrrn(double hrrn_alpha, int socket_cpu){
	//planificar los procesos con hrrn

}
