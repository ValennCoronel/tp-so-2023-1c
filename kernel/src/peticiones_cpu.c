#include "peticiones_cpu.h"

t_dictionary* recurso_bloqueado;

//GENERAL: en algún momento hay que calcular la ráfaga anterior
//TODO para agregar un proceso a ready se puede usar agregar_proceso_a_ready(1); del planificador a largo plazo

void* simulacion_io(void* arg){
	t_argumentos_simular_io* argumentos = (t_argumentos_simular_io* ) arg;
	int tiempo_io = argumentos->tiempo_io;
	int grado_max_multiprogramacion = argumentos->grado_max_multiprogramacion;
	int tiempo_actual = 0;


	//espera activa mientras se ejecuta otro en cpu
	t_pcb* proceso_en_IO = proceso_ejecutando;
	t_temporal* temporal_dormido = temporal_create();


	while(tiempo_io!=tiempo_actual)
	{

	tiempo_actual = temporal_gettime(temporal_dormido);

	}

	temporal_stop(temporal_dormido);
	temporal_destroy(temporal_dormido);

	pasar_a_ready(proceso_en_IO,grado_max_multiprogramacion);

	return NULL;
}



void bloquear_proceso_IO(int socket_cliente,int grado_max_multiprogramacion){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);
	pthread_t hilo_simulacion;

	int tiempo_io = atoi(instruccion->parametros[0]);
	t_argumentos_simular_io* argumentos = malloc(sizeof(t_argumentos_simular_io));
	argumentos->tiempo_io=tiempo_io;
	argumentos->grado_max_multiprogramacion=grado_max_multiprogramacion;
	pthread_create(&hilo_simulacion, NULL, simulacion_io, (void *) argumentos);

	pthread_detach(hilo_simulacion);

	proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
	temporal_stop(rafaga_proceso_ejecutando);
	temporal_destroy(rafaga_proceso_ejecutando);
	rafaga_proceso_ejecutando = NULL;

	poner_a_ejecutar_otro_proceso();


}


void apropiar_recursos(int socket_cliente, char** recursos, int* recurso_disponible){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);

	int indice_recurso = obtener_indice_recurso(recursos, instruccion->parametros[0]);

	// si no existe el recurso finaliza
	if(indice_recurso == -1){

	 	 //llamar a finalizarProceso
		return;
	}

	if(recurso_disponible[indice_recurso] <= 0){
		bloquear_proceso_por_recurso(proceso_ejecutando, recursos[indice_recurso]);
		poner_a_ejecutar_otro_proceso();
	} else {
		recurso_disponible[indice_recurso] -= 1;
	}
	// avisa a consola y continua ejecutandose el mismo proceso
	enviar_contexto_de_ejecucion_a(contexto, PROCESAR_INSTRUCCIONES, socket_cliente);

	//TODO destroy contexto_ejecucion
}

void desalojar_recursos(int cliente_fd,char** recursos, int* recurso_disponible, int grado_max_multiprogramacion){

	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(cliente_fd);

		t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);

		int indice_recurso = obtener_indice_recurso(recursos, instruccion->parametros[0]);

		// si no existe el recurso finaliza
		if(indice_recurso == -1){

		 	 //llamar a finalizarProceso

			return;
		}

		recurso_disponible[indice_recurso] += 1;

		t_queue* cola_bloqueados= (t_queue*) dictionary_get(recurso_bloqueado,recursos[indice_recurso]);

		t_pcb* proceso_desbloqueado = queue_pop(cola_bloqueados);

		if(proceso_desbloqueado!=NULL){

			pasar_a_ready(proceso_desbloqueado,grado_max_multiprogramacion);
		}

		// avisa a consola y continua ejecutandose el mismo proceso
		enviar_contexto_de_ejecucion_a(contexto, PROCESAR_INSTRUCCIONES, cliente_fd);


}

//para manejar otras peticiones de cpu a kernel como crear un segmento de memoria o alguna de file system
void manejar_peticion_al_kernel(int socket_cliente){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	// verificar que tipo de peticion es
	//manejar cada peticion según corresponda
}

void desalojar_proceso(int socket_cliente,int grado_max_multiprogramacion){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);
	//crear estrutura para el contexto de ejecucion

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);
	//TODO

	//devolver proceso a la cola de ready
	//calcula la ráfaga anterior y lo guarda en el pcb para el hrrn
	// si es fifo no lo usa
	proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
	temporal_stop(rafaga_proceso_ejecutando);
	temporal_destroy(rafaga_proceso_ejecutando);
	rafaga_proceso_ejecutando = NULL;

	pasar_a_ready(proceso_ejecutando,grado_max_multiprogramacion);
	poner_a_ejecutar_otro_proceso();

	//

}


void bloquear_proceso_por_recurso(t_pcb* proceso_a_bloquear, char* nombre_recurso){

	//antes de bloquearse, se calcula las ráfagas anteriores para el hrrn, si es fifo no lo usa
	proceso_a_bloquear->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
	temporal_stop(rafaga_proceso_ejecutando);
	temporal_destroy(rafaga_proceso_ejecutando);
	rafaga_proceso_ejecutando = NULL;

	t_queue* cola_bloqueados=dictionary_get(recurso_bloqueado,nombre_recurso);

	queue_push(cola_bloqueados,proceso_a_bloquear);

}


// si no lo encuentra devuelve -1
int obtener_indice_recurso(char** recursos, char* recurso_a_buscar){

	int indice_recurso = 0;
	int tamanio_recursos = string_array_size(recursos);

	if(string_array_is_empty(recursos)){
		return -1;
	}

	while(indice_recurso < tamanio_recursos && strcmp(recurso_a_buscar, recursos[indice_recurso]) != 0 ){
		indice_recurso++;
	}
	if(indice_recurso == tamanio_recursos){
		return -1;
	}

	return indice_recurso;
}

void poner_a_ejecutar_otro_proceso(){
	if(rafaga_proceso_ejecutando != NULL){
		temporal_stop(rafaga_proceso_ejecutando);
		temporal_destroy(rafaga_proceso_ejecutando);
	}

	proceso_ejecutando = NULL;
	sem_post(&consumidor);
}


void finalizarProceso(int socket_cliente, int socket_memoria){
	//crear estrutura para el contexto de ejecucion
		// liberar todos los recursos que tenga asignados (aca se usa el free)
		//free(instrucciones);
		// free(pcb_proceso);
		// dar aviso al módulo Memoria para que éste libere sus estructuras.
		// Una vez hecho esto, se dará aviso a la Consola de la finalización del proceso.

	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	void destructor_instrucciones (void* arg){
		t_instruccion* inst = (t_instruccion*) arg;

		instruccion_destroy(&inst);
	}
	void destructor_tabla_archivos (void* arg){}
	void destructor_tabla_segmentos (void* arg){}

	//Liberar PCB del proceso actual
	list_destroy_and_destroy_elements(proceso_ejecutando->instrucciones, destructor_instrucciones);
	free(proceso_ejecutando->instrucciones);


	registro_cpu_destroy(&(proceso_ejecutando->registros_CPU));

	list_destroy_and_destroy_elements(proceso_ejecutando->tabla_archivos, destructor_tabla_archivos);
	free(proceso_ejecutando->tabla_archivos);
	list_destroy_and_destroy_elements(proceso_ejecutando->tabla_segmentos,destructor_tabla_segmentos);
	free(proceso_ejecutando->tabla_segmentos);
	temporal_destroy(proceso_ejecutando->temporal_ready);
	free(proceso_ejecutando->temporal_ready);
	temporal_destroy(proceso_ejecutando->temporal_ultimo_desalojo);

	//Enviar datos necesarios a memoria para liberarla

	//TODO decomentar y completar cuando este implementada la funcion en memoria (hasta entonces envio mensaje de prueba)
	// t_buffer buffer;
	// enviar_a_memoria(TERMINAR_PROCESO, socket_memoria, buffer);

	enviar_mensaje("Mensaje de prueba para desalojar memoria", socket_memoria, MENSAJE);
	//Enviar mensaje a Consola informando que finalizo el proceso
	enviar_mensaje("Proceso actual finalizado", proceso_ejecutando->socket_server_id, MENSAJE);
	free(proceso_ejecutando);
	//TODO contexto_destroy esta creada pero tiene temitas
	poner_a_ejecutar_otro_proceso();

}


//Funcion que envia un paquete a memoria con un codigo de operacion
void *enviar_a_memoria(op_code codigo, int socket_memoria, t_buffer buffer){
	t_paquete* paquete;
	paquete = crear_paquete(codigo);

	//Rellenar y serializar contenido del paquete

}

