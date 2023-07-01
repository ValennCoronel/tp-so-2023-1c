#include "peticiones_cpu.h"



t_dictionary* recurso_bloqueado;
sem_t esperar_proceso_ejecutando;


void manejar_seg_fault(int socket_cliente){
	// si se escribe en un segmento inválido
	//finalizo el proceso

	//recibo el contexto de cpu, a pesar de que no lo uso para evitar errores de codigo de operación
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	//TODO pedir_finalizar_las_estructuras_de_memoria();

	enviar_mensaje("SEG_FAULT !!", proceso_ejecutando->socket_server_id, FINALIZAR_PROCESO);

	log_info(logger, "FInaliza el proceso %d - Motivo: SEG_FAULT", proceso_ejecutando->PID);

	contexto_ejecucion_destroy(contexto);
	destroy_proceso_ejecutando();
	poner_a_ejecutar_otro_proceso();
}


void* simulacion_io(void* arg){
	t_argumentos_simular_io* argumentos = (t_argumentos_simular_io* ) arg;
	int tiempo_io = argumentos->tiempo_io;
	int grado_max_multiprogramacion = argumentos->grado_max_multiprogramacion;
	int tiempo_actual = 0;

	t_pcb* proceso_en_IO = proceso_ejecutando;

	// despierta el semáforo para poner a ejecutar otro proceso
	sem_post(&esperar_proceso_ejecutando);

	//espera activa mientras se ejecuta otro en cpu
	t_temporal* temporal_dormido = temporal_create();

	while(tiempo_io!=tiempo_actual)
	{

	tiempo_actual = temporal_gettime(temporal_dormido);

	}

	temporal_stop(temporal_dormido);
	temporal_destroy(temporal_dormido);

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_en_IO->PID, "BLOC","READY");

	pasar_a_ready(proceso_en_IO,grado_max_multiprogramacion);

	return NULL;
}



void bloquear_proceso_IO(int socket_cliente,int grado_max_multiprogramacion){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);
	pthread_t hilo_simulacion;

	sem_init(&esperar_proceso_ejecutando, 0, 0);

	int tiempo_io = atoi(instruccion->parametros[0]);
	t_argumentos_simular_io* argumentos = malloc(sizeof(t_argumentos_simular_io));
	argumentos->tiempo_io=tiempo_io;
	argumentos->grado_max_multiprogramacion=grado_max_multiprogramacion;
	pthread_create(&hilo_simulacion, NULL, simulacion_io, (void *) argumentos);

	pthread_detach(hilo_simulacion);

	//se calcula la rafaga anterior para el algoritmo hrrn
	proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
	temporal_stop(rafaga_proceso_ejecutando);
	temporal_destroy(rafaga_proceso_ejecutando);
	rafaga_proceso_ejecutando = NULL;

	sem_wait(&esperar_proceso_ejecutando);

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", contexto->pid, "EXEC","BLOC");

	log_info(logger, "PID: %d - Ejecuta IO: %d", contexto->pid,tiempo_io);


	poner_a_ejecutar_otro_proceso();

	//destruyo el contexto de ejecucion
	contexto_ejecucion_destroy(contexto);
}


void apropiar_recursos(int socket_cliente, char** recursos, int* recurso_disponible, int cantidad_de_recursos){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);

	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);

	int indice_recurso = obtener_indice_recurso(recursos, instruccion->parametros[0]);

	// si no existe el recurso finaliza
	if(indice_recurso == -1){

		//TODO pedir_finalizar_las_estructuras_de_memoria();
		enviar_mensaje("INVALID_RESOURCE !!", proceso_ejecutando->socket_server_id, FINALIZAR_PROCESO);
		log_info(logger, "FInaliza el proceso %d - Motivo: INVALID_RESOURCE", proceso_ejecutando->PID);

		log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","EXIT");


		contexto_ejecucion_destroy(contexto);
		destroy_proceso_ejecutando();
		poner_a_ejecutar_otro_proceso();

		return;
	}

	if(recurso_disponible[indice_recurso] <= 0){
		bloquear_proceso_por_recurso(proceso_ejecutando, recursos[indice_recurso]);
		poner_a_ejecutar_otro_proceso();
	} else {
		recurso_disponible[indice_recurso] -= 1;
	}

	char* recursos_disponibles_string = listar_recursos_disponibles(recurso_disponible, cantidad_de_recursos);
	log_info(logger, "PID: %d - Wait: %s - Instancias: [%s]", contexto->pid,recursos[indice_recurso], recursos_disponibles_string);

	free(recursos_disponibles_string);

	// continua ejecutandose el mismo proceso
	enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cliente);

	//destruyo el contexto de ejecucion
	contexto_ejecucion_destroy(contexto);
}

void desalojar_recursos(int cliente_fd,char** recursos, int* recurso_disponible, int grado_max_multiprogramacion, int cantidad_de_recursos){

	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(cliente_fd);

		t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1);

		int indice_recurso = obtener_indice_recurso(recursos, instruccion->parametros[0]);

		// si no existe el recurso finaliza
		if(indice_recurso == -1){

			//TODO pedir_finalizar_las_estructuras_de_memoria();
			enviar_mensaje("INVALID_RESOURCE !!", proceso_ejecutando->socket_server_id, FINALIZAR_PROCESO);
			log_info(logger, "FInaliza el proceso %d - Motivo: INVALID_RESOURCE", proceso_ejecutando->PID);
			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","EXIT");

			contexto_ejecucion_destroy(contexto);
			destroy_proceso_ejecutando();
			poner_a_ejecutar_otro_proceso();
			return;
		}

		recurso_disponible[indice_recurso] += 1;

		t_queue* cola_bloqueados= (t_queue*) dictionary_get(recurso_bloqueado,recursos[indice_recurso]);

		t_pcb* proceso_desbloqueado = queue_pop(cola_bloqueados);

		if(proceso_desbloqueado!=NULL){

			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_desbloqueado->PID, "BLOC","READY");

			pasar_a_ready(proceso_desbloqueado,grado_max_multiprogramacion);
		}


		char* recursos_disponibles_string = listar_recursos_disponibles(recurso_disponible, cantidad_de_recursos);
		log_info(logger, "PID: %d - Signal: %s - Instancias: [%s]", contexto->pid,recursos[indice_recurso], recursos_disponibles_string);

		free(recursos_disponibles_string);
		//continua ejecutandose el mismo proceso
		enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, cliente_fd);

		//destruyo el contexto de ejecucion
		contexto_ejecucion_destroy(contexto);

}


void desalojar_proceso(int socket_cliente,int grado_max_multiprogramacion){
	//deserializa el contexto de ejecucion
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);


	//devolver proceso a la cola de ready
	//calcula la ráfaga anterior y lo guarda en el pcb para el hrrn
	// si es fifo no lo usa
	proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
	temporal_stop(rafaga_proceso_ejecutando);
	temporal_destroy(rafaga_proceso_ejecutando);
	rafaga_proceso_ejecutando = NULL;

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","READY");


	pasar_a_ready(proceso_ejecutando,grado_max_multiprogramacion);
	poner_a_ejecutar_otro_proceso();

	//destruyo el contexto de ejecucion
	contexto_ejecucion_destroy(contexto);
}


void bloquear_proceso_por_recurso(t_pcb* proceso_a_bloquear, char* nombre_recurso){

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","BLOC");


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
		//free(pcb_proceso);
		// dar aviso al módulo Memoria para que éste libere sus estructuras.
		// Una vez hecho esto, se dará aviso a la Consola de la finalización del proceso.

	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cliente);
	t_tabla_de_segmento *tabla = proceso_ejecutando->tabla_segmentos;

	log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","EXIT");


	//agrego en el paquete, serializo y envio
	t_paquete *paquete = crear_paquete(FINALIZAR_PROCESO_MEMORIA);
	agregar_a_paquete_sin_agregar_tamanio(paquete,&(tabla->cantidad_segmentos),sizeof(uint32_t));
	agregar_a_paquete_sin_agregar_tamanio(paquete,&(tabla->pid),sizeof(uint32_t));

	// tabla->cantidad_segmento == list_size(tabla->segmentos) porque guardan el mismo valor
	for(int i =0; i< tabla->cantidad_segmentos ; i++){
		t_segmento* segmento_N = list_get(tabla->segmentos, i);

		agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento_N->id_segmento), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento_N->direccion_base), sizeof(uint32_t));
		agregar_a_paquete_sin_agregar_tamanio(paquete, &(segmento_N->tamano), sizeof(uint32_t));
	}

	enviar_paquete(paquete,socket_memoria);

	//libero
	eliminar_paquete(paquete);

	//Enviar mensaje a Consola informando que finalizo el proceso

	enviar_mensaje("Proceso finalizado", proceso_ejecutando->socket_server_id, FINALIZAR_PROCESO);

	log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", proceso_ejecutando->PID);

	destroy_proceso_ejecutando();
	contexto_ejecucion_destroy(contexto);
	return;
}



void create_segment(){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 1);

	t_segmento_parametro* peticion_segmento = (t_segmento_parametro*) malloc(sizeof(t_segmento_parametro));

	peticion_segmento->id_segmento = atoi(instruccion_peticion->parametros[0]);
	peticion_segmento->tamano_segmento = atoi(instruccion_peticion->parametros[1]);

	// serializo segmento_parámetro

	t_paquete* paquete = crear_paquete(CREAR_SEGMENTO);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(peticion_segmento->id_segmento), sizeof(uint32_t));
	agregar_a_paquete_sin_agregar_tamanio(paquete, &(peticion_segmento->tamano_segmento), sizeof(uint32_t));
	agregar_a_paquete_sin_agregar_tamanio(paquete, &(contexto->pid), sizeof(int));

	// enviar paquete serializado
	enviar_paquete(paquete,socket_memoria);

	log_info(logger, "PID: %d - Crear Segmento - Id: %d - Tamaño: %d", contexto->pid, peticion_segmento->id_segmento, peticion_segmento->tamano_segmento);

	// escuha y maneja en cualquiera de los 3 posibles casos de la respuesta de memoria
	escuchar_respuesta_memoria(contexto, peticion_segmento);

	contexto_ejecucion_destroy(contexto);
}

void manejar_escucha_out_of_memory(){
	// si no hay espacio disponible, ya sea contiguo o no contiguo,
	//finalizo el proceso

	//recibo el texto, a pesar de que no lo uso para evitar errores de codigo de operación
	int size;
	void *  buffer = recibir_buffer(&size, socket_memoria);
	char* mensaje = malloc(size);
	memcpy(mensaje, buffer,size);

	//TODO pedir_finalizar_las_estructuras_de_memoria();
	enviar_mensaje("OUT_OF_MEMORY !!", proceso_ejecutando->socket_server_id, FINALIZAR_PROCESO);

	log_info(logger, "FInaliza el proceso %d - Motivo: OUT_OF_MEMORY", proceso_ejecutando->PID);

	destroy_proceso_ejecutando();
	poner_a_ejecutar_otro_proceso();
}

void manejar_escucha_crear_segmento(t_contexto_ejec* contexto, t_segmento_parametro* peticion_segmento){
	// en caso de que pudo crear el segmento,
	// se recibe la dirección base de memoria
	uint32_t direccion_base;
	int size;
	void *  buffer = recibir_buffer(&size, socket_memoria);
	memcpy(&direccion_base, buffer, sizeof(uint32_t));

	// creo el nuevo segmento
	t_segmento* segmento_nuevo = malloc(sizeof(t_segmento));

	segmento_nuevo->id_segmento = peticion_segmento->id_segmento;
	segmento_nuevo->tamano = peticion_segmento->tamano_segmento;
	segmento_nuevo->direccion_base = direccion_base;


	// lo agrego en al tabla de segmentos, pero no lo indexo por el id del segmento
	list_add(proceso_ejecutando->tabla_segmentos->segmentos, segmento_nuevo);


	// actualizo la cantidad de segmentos de la tabla
	proceso_ejecutando->tabla_segmentos->cantidad_segmentos += 1;

	// continuo con las siguientes instrucciones del proceso
	enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
}

void manejar_escucha_compactar(t_contexto_ejec* contexto, t_segmento_parametro* peticion_segmento){

	//recibo el texto, a pesar de que no lo uso para evitar errores de codigo de operación
	int size;
	void *  buffer = recibir_buffer(&size, socket_memoria);
	char* mensaje = malloc(size);
	memcpy(mensaje, buffer,size);

	// se solicita compactar la memoria
	enviar_mensaje("Compacta!!",socket_memoria, COMPACTAR_MEMORIA);

	// luego se recibe las tablas de segmentos actualizadas
	t_list* tablas_de_segmentos_actualizadas = (t_list*) recibir_tablas_de_segmentos();

	//y actualizar la tabla de segmentos de cada proceso en la cola de ready
	acutalizar_tablas_de_procesos(tablas_de_segmentos_actualizadas);

	// luego solicita a memoria otra vez de crear el segmento
	t_paquete* paquete = crear_paquete(CREAR_SEGMENTO);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &(peticion_segmento->id_segmento), sizeof(uint32_t));
	agregar_a_paquete_sin_agregar_tamanio(paquete, &(peticion_segmento->tamano_segmento), sizeof(uint32_t));

	// enviar paquete serializado
	enviar_paquete(paquete, socket_memoria);

	// de forma recursiva escucho y manejo las peticiones de memoria
	// 	la recursión se rompe en un out_of_memory o con el segmento creado
	escuchar_respuesta_memoria(contexto, peticion_segmento);
}

// escucha las respuestas de memoria pero es solo luego de intentar de crear el segmento
void escuchar_respuesta_memoria(t_contexto_ejec* contexto, t_segmento_parametro* peticion_segmento){
			int cod_op = recibir_operacion(socket_memoria);

			switch (cod_op) {
				case OUT_OF_MEMORY:
					manejar_escucha_out_of_memory();
					break;
				case CREAR_SEGMENTO:
					manejar_escucha_crear_segmento(contexto, peticion_segmento);
					break;
				case COMPACTAR_MEMORIA:
					manejar_escucha_compactar(contexto, peticion_segmento);
					break;
				case -1:
					log_error(logger, "Memoria se desconecto. Terminando servidor");
					return ;
					break;
				default:
					log_warning(logger,"Memoria Operacion desconocida. No quieras meter la pata!!" );
					break;
			}
}


// acutaliza las tablas de segmentos de todos los procesos que están en ready
// 	incluido el del proceso ejecutandose actualmente en CPU
void acutalizar_tablas_de_procesos(t_list* tablas_de_segmentos_actualizadas){

	// el -1 es porque descuento el proceso ejecutando de la lista
	int tamanio_tablas = list_size(tablas_de_segmentos_actualizadas) -1;

	sem_wait(&m_cola_ready);
	int tamanio_cola_ready = queue_size(cola_ready);
	sem_post(&m_cola_ready);

	if(tamanio_cola_ready > 0){
		// actualizo las tablas de cada proceso de la cola new
		for(int i = 0; i< tamanio_tablas ; i++){
			sem_wait(&m_cola_ready);
			t_pcb* proceso_N = (t_pcb*) list_get(cola_ready->elements,i);
			sem_post(&m_cola_ready);


			actualizar_tabla_del_proceso(tablas_de_segmentos_actualizadas, proceso_N);
		}
	}


	// actualizo la tabla del proceso ejecutando actualmente
	actualizar_tabla_del_proceso(tablas_de_segmentos_actualizadas, proceso_ejecutando);

}

// dada una lista de tablas de segmentos y un pcb, actualiza la tabla de segmentos del pcb con la tabla
// 		que se encuentra en la lista de tablas que se recibe por parámetros
void actualizar_tabla_del_proceso(t_list* tablas_de_segmentos_actualizadas, t_pcb* proceso_a_actualizar){

	int cantidad_de_tablas_actualizadas = list_size(tablas_de_segmentos_actualizadas);

	if(cantidad_de_tablas_actualizadas == 0){
		return;
	}

	// busco la tabla de segmentos del proceso en base a su PID
	void *_encontrar_tabla_del_proceso(void*tabla_1, void* tabla_2){
		t_tabla_de_segmento* tabla_actual = (t_tabla_de_segmento*) tabla_1;
		t_tabla_de_segmento* tabla_siguiente = (t_tabla_de_segmento*) tabla_2;

		if(tabla_actual->pid == proceso_a_actualizar->PID){
			return tabla_actual;
		}

		return tabla_siguiente;
	}

	// si no encuentra la tabla de segmentos del proceso, devuelve la última tabla de la lista
	t_tabla_de_segmento* tabla_actualizada = list_fold1(tablas_de_segmentos_actualizadas, _encontrar_tabla_del_proceso);

	// si no lo encuentra no la actualiza y la deja como estaba
	if(tabla_actualizada->pid == proceso_a_actualizar->PID){
		if(proceso_a_actualizar->tabla_segmentos != NULL){
			// libero la tabla anterior
			destroy_tabla_de_segmentos(proceso_a_actualizar->tabla_segmentos);
		}


		// seteo con la nueva tabla actualizada
		proceso_a_actualizar->tabla_segmentos = tabla_actualizada;
	}
}

t_list* recibir_tablas_de_segmentos(){
	int size;
	void* buffer = recibir_buffer(&size, socket_memoria);

	t_list* tablas_de_segmentos = list_create();
	int desplazamiento = 0;

	while(desplazamiento < size){
		int lista_length = 0;
		memcpy(&lista_length, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		for(int i = 0; i< lista_length; i++){
			t_tabla_de_segmento* tabla_de_segmento = malloc(sizeof(t_tabla_de_segmento));


			memcpy(&(tabla_de_segmento->cantidad_segmentos), buffer+desplazamiento, sizeof(uint32_t));
			desplazamiento+= sizeof(uint32_t);

			memcpy(&(tabla_de_segmento->pid), buffer+desplazamiento, sizeof(uint32_t));
			desplazamiento+= sizeof(uint32_t);

			int tamano_segmentos;
			memcpy(&tamano_segmentos, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);

			for(int j =0; j<tamano_segmentos; j++){
				t_segmento* segmento = malloc(sizeof(t_segmento*));

				memcpy(&(segmento->id_segmento), buffer + desplazamiento, sizeof(uint32_t));
				desplazamiento+= sizeof(uint32_t);

				memcpy(&(segmento->direccion_base), buffer + desplazamiento, sizeof(uint32_t));
				desplazamiento+= sizeof(uint32_t);

				memcpy(&(segmento->tamano), buffer + desplazamiento, sizeof(uint32_t));
				desplazamiento+= sizeof(uint32_t);

				list_add_in_index(tabla_de_segmento->segmentos,segmento->id_segmento ,segmento);
			}

			list_add(tablas_de_segmentos, tabla_de_segmento);
		}
	}

	free(buffer);
	return tablas_de_segmentos;
}


void delete_segment(){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 1);

	// busco el segmento a borrar por id de la tabla de segmentos del proceso ejecutando
	t_segmento_parametro* segmento_a_eliminar = malloc(sizeof(t_segmento_parametro));
	segmento_a_eliminar->id_segmento = atoi(instruccion_peticion->parametros[0]);

	t_paquete* paquete = crear_paquete(ELIMINAR_SEGMENTO);

	//serializo el tamaño del segmento y el id del segmento
	agregar_a_paquete_sin_agregar_tamanio(paquete,&(segmento_a_eliminar->id_segmento),sizeof(uint32_t) );

	//se envía a memoria
	enviar_paquete(paquete, socket_memoria);


	log_info(logger, "PID: %d - Eliminar Segmento - Id Segmento: %d", contexto->pid, segmento_a_eliminar->id_segmento);


	// espero a la respuesta de memoria
	int cod_op = recibir_operacion(socket_memoria);

	if(cod_op == ELIMINAR_SEGMENTO){
		// se recibe la tabla de segmentos actualizada del proceso ejecutando
		t_tabla_de_segmento* tabla_de_segmentos_actualizada = (t_tabla_de_segmento*) recibir_tabla_de_segmentos();

		//y actualizar la tabla de segmentos del proceso ejecutando

		// libero la tabla anterior
		destroy_tabla_de_segmentos(proceso_ejecutando->tabla_segmentos);

		// seteo con la nueva tabla actualizada
		proceso_ejecutando->tabla_segmentos = tabla_de_segmentos_actualizada;
	}

	// continuo con las siguientes instrucciones del proceso
	enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
}

void compactar_memoria()
{

}

t_tabla_de_segmento* recibir_tabla_de_segmentos(){
	t_tabla_de_segmento* tabla_de_segmento = malloc(sizeof(t_tabla_de_segmento));
	int size;
	int desplazamiento = 0;
	void* buffer =  recibir_buffer(&size, socket_memoria);

	while(desplazamiento<size){
		memcpy(&(tabla_de_segmento->cantidad_segmentos), buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+= sizeof(uint32_t);

		memcpy(&(tabla_de_segmento->pid), buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento+= sizeof(uint32_t);

		int tamano_segmentos;
		memcpy(&tamano_segmentos, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		for(int j =0; j<tamano_segmentos; j++){
			t_segmento* segmento = malloc(sizeof(t_segmento*));

			memcpy(&(segmento->id_segmento), buffer + desplazamiento, sizeof(uint32_t));
			desplazamiento+= sizeof(uint32_t);

			memcpy(&(segmento->direccion_base), buffer + desplazamiento, sizeof(uint32_t));
			desplazamiento+= sizeof(uint32_t);

			memcpy(&(segmento->tamano), buffer + desplazamiento, sizeof(uint32_t));
			desplazamiento+= sizeof(uint32_t);

			list_add_in_index(tabla_de_segmento->segmentos,segmento->id_segmento ,segmento);
		}
	}

	free(buffer);
	return tabla_de_segmento;

}


void destroy_proceso_ejecutando(){

	void destructor_instrucciones (void* arg){
		t_instruccion* inst = (t_instruccion*) arg;

		instruccion_destroy(inst);
	}
		//TODO finalizar el free de estas estructuras cuando se definan
		void destructor_tabla_archivos (void* arg){}


		//Liberar PCB del proceso actual
		list_destroy_and_destroy_elements(proceso_ejecutando->instrucciones, destructor_instrucciones);

		registro_cpu_destroy(proceso_ejecutando->registros_CPU);

		//list_destroy_and_destroy_elements(proceso_ejecutando->tabla_archivos_abiertos_del_proceso, destructor_tabla_archivos);

		destroy_tabla_de_segmentos(proceso_ejecutando->tabla_segmentos);


		if(proceso_ejecutando->temporal_ready != NULL){
			temporal_destroy(proceso_ejecutando->temporal_ready);
		}


		if(proceso_ejecutando->temporal_ultimo_desalojo != NULL){
			temporal_destroy(proceso_ejecutando->temporal_ultimo_desalojo);
		}

		free(proceso_ejecutando);
		proceso_ejecutando = NULL;
}

char* listar_recursos_disponibles(int* recursos_disponibles, int cantidad_de_recursos){

		char** recursos_disponibles_string_array = string_array_new();
		char* recursos_disponibles_string = string_new();


		for(int i =0; i< cantidad_de_recursos; i++){
			int diponibilidad_recurso_n = recursos_disponibles[i];

			string_array_push(&recursos_disponibles_string_array, string_itoa(diponibilidad_recurso_n));
		}


		void crear_string(char *recurso_disponible_string){
		    string_append(&recursos_disponibles_string, recurso_disponible_string);
		    string_append(&recursos_disponibles_string, ",");
		}

		string_iterate_lines(recursos_disponibles_string_array,crear_string);

		string_array_destroy(recursos_disponibles_string_array);

		return recursos_disponibles_string ;
}

