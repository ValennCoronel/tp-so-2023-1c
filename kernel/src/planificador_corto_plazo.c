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

/*
TODO deserializar esto del otro lado
 * */
void planificar_corto_plazo_fifo(int socket_cpu){
	//planificar los procesos con fifo

	sem_wait(&consumidor);
	t_pcb *proceso_a_ejecutar = queue_pop(cola_ready);
	sem_post(&productor);

	//creo el contexto de ejecucion

	//TODO ARREGLAR ESTO O VER SI FUNCIONA ANTES DE COMMITEAR
	/*
	t_contexto_ejec* contexto_ejecucion = malloc(sizeof(t_contexto_ejec));
	contexto_ejecucion->lista_instrucciones = proceso_a_ejecutar->instrucciones;
	contexto_ejecucion->program_counter = proceso_a_ejecutar->program_counter;
	contexto_ejecucion->tamanio_lista = list_size(proceso_a_ejecutar->instrucciones);
	contexto_ejecucion->registros_CPU = proceso_a_ejecutar->registros_CPU;

	//Lo envio proceso a CPU

	t_paquete* paquete = crear_paquete(PROCESAR_INSTRUCCIONES);

	agregar_a_paquete(paquete, contexto_ejecucion->tamanio_lista, sizeof(int));

	for(int i =0; i<contexto_ejecucion->tamanio_lista; i++){
		t_instruccion* instruccion = list_get(contexto_ejecucion->lista_instrucciones, i);

		agregar_a_paquete(paquete, instruccion->opcode_lenght, sizeof(int));
		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght+1);
		agregar_a_paquete(paquete, instruccion->parametro1_lenght, sizeof(int));
		agregar_a_paquete(paquete, instruccion->parametro2_lenght, sizeof(int));
		agregar_a_paquete(paquete, instruccion->parametro3_lenght, sizeof(int));


			//Parametros (valores)
		for(int i = 0; i< 3; i++){
			agregar_a_paquete(paquete, instruccion->parametros[i], strlen(instruccion->parametros[i])+1);
		}
	}

	agregar_a_paquete(paquete, contexto_ejecucion->program_counter, sizeof(int));

	//cargo los registros de CPU
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->AX,sizeof(char)*4);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->BX,sizeof(char)*4);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->CX,sizeof(char)*4);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->DX,sizeof(char)*4);

	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->EAX,sizeof(char)*8);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->EBX,sizeof(char)*8);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->ECX,sizeof(char)*8);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->EDX,sizeof(char)*8);

	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->RAX,sizeof(char)*16);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->RBX,sizeof(char)*16);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->RCX,sizeof(char)*16);
	agregar_a_paquete(paquete, contexto_ejecucion->registros_CPU->RDX,sizeof(char)*16);



	enviar_paquete(paquete, socket_cpu);

	eliminar_paquete(paquete);
	*/
}

void planificar_corto_plazo_hrrn(double hrrn_alpha, int socket_cpu){
	//planificar los procesos con hrrn

}
