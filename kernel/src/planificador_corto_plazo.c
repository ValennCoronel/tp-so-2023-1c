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

	sem_wait(&consumidor);
	sem_wait(&m_cola_ready);
	if(queue_size(cola_ready) == 0) {
		sem_post(&consumidor);
		sem_post(&m_cola_ready);
		return;
	}
	t_pcb *proceso_a_ejecutar = queue_pop(cola_ready);
	sem_post(&m_cola_ready);

	//frena y elimina el temporal innecesario
	temporal_stop(proceso_a_ejecutar->temporal_ultimo_desalojo);
	temporal_destroy(proceso_a_ejecutar->temporal_ultimo_desalojo);

	//creo el contexto de ejecucion

	t_contexto_ejec* contexto_ejecucion = malloc(sizeof(t_contexto_ejec));
	contexto_ejecucion->lista_instrucciones = proceso_a_ejecutar->instrucciones;
	contexto_ejecucion->program_counter = proceso_a_ejecutar->program_counter;
	contexto_ejecucion->tamanio_lista = list_size(proceso_a_ejecutar->instrucciones);

	proceso_ejecutando = proceso_a_ejecutar;

	//inicio cronometro para contar las rafagas del proceco a ejecutar
	rafaga_proceso_ejecutando = temporal_create();

	//Lo envio proceso a CPU
	enviar_contexto_de_ejecucion_a(contexto_ejecucion, PROCESAR_INSTRUCCIONES, socket_cpu);


	//contexto_ejecucion_destroy(&contexto_ejecucion);
}

void planificar_corto_plazo_hrrn(double hrrn_alpha, int socket_cpu){

	sem_wait(&consumidor);
	sem_wait(&m_cola_ready);
	if(queue_size(cola_ready) == 0) {
		sem_post(&consumidor);
		sem_post(&m_cola_ready);
		return;
	}

	void _calcular_proxima_rafaga_estimada_cada_proceso(t_pcb* pcb_proceso){
		int64_t tiempo_de_espera = calcular_tiempo_de_espera(pcb_proceso);

		double estimado_proxima_rafaga = estimar_proxima_rafaga_proceso(hrrn_alpha, pcb_proceso->rafaga_anterior , pcb_proceso->estimado_proxima_rafaga);

		double prioridad = calcular_prioridad_con_hrrn(tiempo_de_espera, estimado_proxima_rafaga);

		pcb_proceso->estimado_proxima_rafaga = estimado_proxima_rafaga;
		pcb_proceso->prioridad = prioridad;
	}

	list_iterate(cola_ready->elements, (void *) _calcular_proxima_rafaga_estimada_cada_proceso);


	reordenar_cola_ready_hrrn();

	t_pcb *proceso_a_ejecutar = queue_pop(cola_ready);
	sem_post(&m_cola_ready);

	t_contexto_ejec* contexto_ejecucion = malloc(sizeof(t_contexto_ejec));
	contexto_ejecucion->lista_instrucciones = proceso_a_ejecutar->instrucciones;
	contexto_ejecucion->program_counter = proceso_a_ejecutar->program_counter;
	contexto_ejecucion->tamanio_lista = list_size(proceso_a_ejecutar->instrucciones);
	contexto_ejecucion->registros_CPU = proceso_a_ejecutar->registros_CPU;

	proceso_ejecutando = proceso_a_ejecutar;

	//inicio cronometro para contar las rafagas del proceco a ejecutar
	rafaga_proceso_ejecutando = temporal_create();

	enviar_contexto_de_ejecucion_a(contexto_ejecucion, PROCESAR_INSTRUCCIONES, socket_cpu);

	//contexto_ejecucion_destroy(&contexto_ejecucion);
}

void reordenar_cola_ready_hrrn(){
	// reodena de mayor a menor para que al hacer pop, saque al de mayor proridad
	// cuando hace pop saca al primer elemento de la lista
	bool __proceso_mayor_prioridad(t_pcb* pcb_proceso1, t_pcb* pcb_proceso2){
		return pcb_proceso1->prioridad > pcb_proceso2->prioridad;
	}

	list_sort(cola_ready->elements, (void *) __proceso_mayor_prioridad);
}

int64_t calcular_tiempo_de_espera(t_pcb* pcb_proceso){

	int64_t tiempo_espera = temporal_gettime(pcb_proceso->temporal_ultimo_desalojo);

	temporal_stop(pcb_proceso->temporal_ultimo_desalojo);
	temporal_destroy(pcb_proceso->temporal_ultimo_desalojo);

	return tiempo_espera;
}

double estimar_proxima_rafaga_proceso(double hrrn_alpha, int64_t anterior_rafaga, int64_t anterior_estimado ){
	return hrrn_alpha * anterior_rafaga + (1 - hrrn_alpha) * anterior_estimado;
}

double calcular_prioridad_con_hrrn(int64_t tiempo_de_espera, double tiempo_proxima_rafaga ){
	return (tiempo_de_espera + tiempo_proxima_rafaga) / tiempo_proxima_rafaga ;
}

void enviar_contexto_de_ejecucion_a(t_contexto_ejec* proceso_a_ejecutar, op_code opcode, int socket_cliente){

	t_paquete* paquete = crear_paquete(opcode);

		agregar_a_paquete_sin_agregar_tamanio(paquete, (void *) proceso_a_ejecutar->tamanio_lista, sizeof(int));

		for(int i =0; i<proceso_a_ejecutar->tamanio_lista; i++){
			t_instruccion* instruccion = list_get(proceso_a_ejecutar->lista_instrucciones, i);

			agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght);

			agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
			agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
			agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);

		}

		agregar_a_paquete_sin_agregar_tamanio(paquete, (void *) proceso_a_ejecutar->program_counter, sizeof(int));

		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->AX, sizeof(char)*4);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->BX, sizeof(char)*4);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->CX, sizeof(char)*4);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->DX, sizeof(char)*4);

		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->EAX, sizeof(char)*8);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->EBX, sizeof(char)*8);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->ECX, sizeof(char)*8);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->EDX, sizeof(char)*8);

		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RAX, sizeof(char)*16);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RBX, sizeof(char)*16);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RCX, sizeof(char)*16);
		agregar_a_paquete(paquete,  proceso_a_ejecutar->registros_CPU->RDX, sizeof(char)*16);


		enviar_paquete(paquete, socket_cliente);

		eliminar_paquete(paquete);
}
