#include "peticiones_cpu.h"


void finalizar_proceso(int socket_cliente){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);
	//crear estrutura para el contexto de ejecucion
	// liberar todos los recursos que tenga asignados (aca se usa el free)
	//free(instrucciones);
	// free(pcb_proceso);
	// dar aviso al módulo Memoria para que éste libere sus estructuras.
	// Una vez hecho esto, se dará aviso a la Consola de la finalización del proceso.
}



void bloquear_proceso_IO(int socket_cliente){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);
	pthread_t hilo_simulacion;

	int tiempo_io = atoi(instruccion->parametros[0]);

	pthread_create(&hilo_simulacion, NULL, simulacion_io,tiempo_io);

	pthread_detach(hilo_simulacion);

}

void* simulacion_io(void* arg){
	int tiempo_io = (int )arg;
	int tiempo_actual = 0;

	t_temporal* temporal_dormido = temporal_create();

	while(tiempo_io!=tiempo_actual)
	{

	tiempo_actual = temporal_gettime(temporal_dormido);

	}

	temporal_stop(temporal_dormido);
	temporal_destroy(temporal_dormido);
}

void apropiar_recursos(int socket_cliente, char** recursos, int* recurso_disponible){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);

	if(!existe_recurso(recursos)){

	 	 //llamar a finalizar proceso UwU ♥♥♥

		return;
	}

	//recurso_disponible
	//TODO
	/*apropiarRecursos() : A la hora de recibir de la CPU un Contexto
			de Ejecución desplazado por WAIT, el Kernel deberá verificar
			primero que exista el recurso solicitado y en caso de que exista
			restarle 1 a la cantidad de instancias del mismo. En caso de que el
			número sea estrictamente menor a 0, el proceso que realizó WAIT se
			bloqueará en la cola de bloqueados correspondiente al recurso.
			Si no existe se debe enviar el proceso a EXIT
	*/

}

bool existe_recurso(t_instruccion* instruccion, char** recursos){

	if (string_array_size(recursos)==0)

return false;
}

void desalojar_recursos(cliente_fd, recursos, recurso_disponible){

}
void manejar_peticion_al_kernel(int socket_cliente){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	// verificar que tipo de peticion es
	//manejar cada peticion según corresponda
}

void desalojar_proceso(int socket_cliente,char** recursos, char** instancias_recursos){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);
	//crear estrutura para el contexto de ejecucion

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);
	//TODO

	//devolver proceso a la cola de ready
}
