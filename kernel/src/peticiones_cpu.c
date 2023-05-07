#include "peticiones_cpu.h"

void finalizar_proceso(int socket_cliente){
	t_list* contexto_ejecucion = recibir_paquete(socket_cliente);
	//crear estrutura para el contexto de ejecucion
	// liberar todos los recursos que tenga asignados (aca se usa el free)
	//free(instrucciones);
	// free(pcb_proceso);

	// dar aviso al módulo Memoria para que éste libere sus estructuras.
	// Una vez hecho esto, se dará aviso a la Consola de la finalización del proceso.
}

void bloquear_proceso(int socket_cliente){

	//verificar si es una IO o WAIT
	// si es una IO obtener el tiempo de espeja junto con el contexto de ejecución
	// simular I/O
	// si es un WAIT obtener el recurso  junto con el contexto de ejecución
	// simular espera de recursos
}

void manejar_peticion_al_kernel(int socket_cliente){

	// verificar que tipo de peticion es
	//manejar cada peticion según corresponda
}

void desalojar_proceso(int socket_cliente){
	t_list* contexto_ejecucion = recibir_paquete(socket_cliente);
	//crear estrutura para el contexto de ejecucion

	//devolver proceso a la cola de ready
}
