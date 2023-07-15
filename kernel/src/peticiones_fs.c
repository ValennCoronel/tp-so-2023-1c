
#include "peticiones_fs.h"

//-------------------------------- Peticiones FS ------------------------------------------------------------

void f_open(){
	t_contexto_ejec* contexto = recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion = list_get(contexto->lista_instrucciones, contexto->program_counter-2); //obtengo instruccion a ejecutar

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->program_counter = contexto->program_counter;
	sem_post(&m_proceso_ejecutando);

	char* nombre_archivo = strdup(instruccion->parametros[0]);

	log_info(logger, "PID: %d - Abrir Archivo: %s", contexto->pid, nombre_archivo);

	//busco si el archivo esta en la tabla global de archivos
	if(dictionary_has_key(tabla_global_de_archivos_abiertos, nombre_archivo)){
		//Enviar a la cola de bloqueados esperando la apertura del archivo
		t_tabla_global_de_archivos_abiertos* tabla = dictionary_get(tabla_global_de_archivos_abiertos, nombre_archivo);
		tabla->open ++;

		sem_wait(&m_proceso_ejecutando);
		proceso_ejecutando->tabla_archivos_abiertos_del_proceso = malloc(sizeof(t_tabla_de_archivos_por_proceso));
		proceso_ejecutando->tabla_archivos_abiertos_del_proceso->file = nombre_archivo;
		proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero = 0;
		sem_post(&m_proceso_ejecutando);

		enviar_cola_archivos_bloqueados(instruccion);

	}else{
		//(si no esta en la tabla) enviar a FS peticion para que verifique
		enviar_peticion_fs(ABRIR_ARCHIVO, instruccion);

		op_code cod_op = recibir_operacion(socket_fs);

		if(cod_op == ABRIR_ARCHIVO){
			char* mensaje = recibir_mensaje(socket_fs);

			if(strcmp(mensaje,"OK") == 0){

				abrir_archivo(instruccion);

			}else if(strcmp(mensaje,"ERROR") == 0){
				log_info(logger, "La FCB no se encuentra en el FileSystem, procediendo a crear archivo");
				enviar_peticion_fs(CREAR_ARCHIVO, instruccion);

				op_code cod_op_2 = recibir_operacion(socket_fs);

				if(cod_op_2 == CREAR_ARCHIVO){
					char* mensaje = recibir_mensaje(socket_fs);
					if(strcmp(mensaje,"OK") == 0){
						//Abrimos el archivo
						abrir_archivo(instruccion);
					}
				}
			}
		}

		// continua con el mismo proceso
		enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
	}

}

//Cierra la instancia de un archivo abierto, si ya no hay mas procesos solicitando el archivo lo saca de la tabla global, sino reduce el contador de archivos abiertos
void f_close(){
	t_contexto_ejec* contexto = recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-2); //obtengo instruccion a ejecutar

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->program_counter = contexto->program_counter;
	sem_post(&m_proceso_ejecutando);

	char* nombre_archivo = instruccion->parametros[0];
	t_tabla_global_de_archivos_abiertos* tabla = dictionary_get(tabla_global_de_archivos_abiertos, nombre_archivo);

	//se borra la entrada de la tabla de archivos por porceso en cualquier caso
	sem_wait(&m_proceso_ejecutando);
	free(proceso_ejecutando->tabla_archivos_abiertos_del_proceso->file);
	free(proceso_ejecutando->tabla_archivos_abiertos_del_proceso);
	proceso_ejecutando->tabla_archivos_abiertos_del_proceso = NULL;
	sem_post(&m_proceso_ejecutando);

	log_info(logger, "PID: %d - Cerrar Archivo: %s", contexto->pid, nombre_archivo);

	if(tabla->open == 1){
		//se borra la entrada de la tabla global de archivos abiertos

		t_tabla_global_de_archivos_abiertos* tabla = dictionary_remove(tabla_global_de_archivos_abiertos, nombre_archivo);
		free(tabla->file);
		free(tabla);

	}else{
		//Reduzco la cantidad de abiertos y avanzo al proximo proceso
		tabla->open --;

		t_queue* cola_proceso_a_desbloquear = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo);
		t_pcb* pcb_a_desbloquear = queue_pop(cola_proceso_a_desbloquear);


		log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_a_desbloquear->PID, "BLOC","READY");

		pasar_a_ready(pcb_a_desbloquear, grado_max_multiprogramacion);
	}

	// en ambos casos continua con el mismo proceso
	enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
}

void f_seek(int cliente_fd){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 2);

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->program_counter = contexto->program_counter;
	sem_post(&m_proceso_ejecutando);

	char* nombre_archivo = instruccion_peticion->parametros[0];
	int posicion = atoi(instruccion_peticion->parametros[1]);

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero = posicion;
	sem_post(&m_proceso_ejecutando);

	log_info(logger, "PID: %d - Actualizar puntero Archivo: %s - Puntero %d", contexto->pid, nombre_archivo, posicion);

	// continua con el mismo proceso
	enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
}

void truncar_archivo(){
	t_contexto_ejec* contexto = recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 2);

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->program_counter = contexto->program_counter;
	sem_post(&m_proceso_ejecutando);

	char* nombre_archivo = instruccion_peticion->parametros[0];
	char* tamanio_a_truncar_string = instruccion_peticion->parametros[1];

	log_info(logger, "PID: %d - Archivo: %s - Tamaño: %s", contexto->pid, nombre_archivo, tamanio_a_truncar_string);

	//crear una bloquear proceso general UwU
	enviar_peticion_fs(TRUNCAR_ARCHIVO,instruccion_peticion);

	//bloquear proceso
	enviar_cola_archivos_bloqueados(instruccion_peticion);

	op_code cod_op = recibir_operacion(socket_fs);

	if(cod_op == TRUNCAR_ARCHIVO){
		char* mensaje = recibir_mensaje(socket_fs);
		if(strcmp(mensaje,"OK") == 0){
			//desbloquear tras recibir "OK"
			t_queue* cola_proceso_a_desbloquear = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo);
			t_pcb* pcb_a_desbloquear = queue_pop(cola_proceso_a_desbloquear);


			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_a_desbloquear->PID, "BLOC","READY");

			pasar_a_ready(pcb_a_desbloquear, grado_max_multiprogramacion);

			//continua con el mismo proceso
			enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
		}
	}

}

void leer_archivo(){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 2);

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->program_counter = contexto->program_counter;
	sem_post(&m_proceso_ejecutando);

	char* nombre_archivo = instruccion_peticion->parametros[0];
	sem_wait(&m_proceso_ejecutando);
	int puntero = proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero;
	sem_post(&m_proceso_ejecutando);
	char* direccion_fisica = instruccion_peticion->parametros[1];
	char* bytes_a_leer_string = instruccion_peticion->parametros[2];

	log_info(logger, "PID: %d - Leer Archivo: %s - Puntero %d - Dirección Memoria %s - Tamaño %s", contexto->pid, nombre_archivo, puntero, direccion_fisica, bytes_a_leer_string);

	sem_wait(&m_proceso_ejecutando);
	enviar_peticion_puntero_fs(LEER_ARCHIVO,instruccion_peticion, proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero, contexto->pid);
	sem_post(&m_proceso_ejecutando);

	enviar_cola_archivos_bloqueados(instruccion_peticion);

	op_code cod_op = recibir_operacion(socket_fs);

	if(cod_op == LEER_ARCHIVO){
		char* mensaje = recibir_mensaje(socket_fs);
		if(strcmp(mensaje,"OK") == 0){
			//desbloquear tras recibir "OK"
			t_queue* cola_proceso_a_desbloquear = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo);
			t_pcb* pcb_a_desbloquear = queue_pop(cola_proceso_a_desbloquear);


			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_a_desbloquear->PID, "BLOC","READY");

			pasar_a_ready(pcb_a_desbloquear, grado_max_multiprogramacion);

			//continua con el mismo proceso
			enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
		}
	}

}

void escribir_archivo(){
	t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
	t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 2);

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->program_counter = contexto->program_counter;
	sem_post(&m_proceso_ejecutando);

	char* nombre_archivo = instruccion_peticion->parametros[0];
	sem_wait(&m_proceso_ejecutando);
	int puntero = proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero;
	sem_post(&m_proceso_ejecutando);
	char* direccion_fisica = instruccion_peticion->parametros[1];
	char* bytes_a_escribir_string = instruccion_peticion->parametros[2];

	log_info(logger, "PID: %d - Escribir Archivo: %s - Puntero %d - Dirección Memoria %s - Tamaño %s", contexto->pid, nombre_archivo, puntero, direccion_fisica, bytes_a_escribir_string);

	sem_wait(&m_proceso_ejecutando);
	enviar_peticion_puntero_fs(ESCRIBIR_ARCHIVO,instruccion_peticion, proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero, contexto->pid);
	sem_post(&m_proceso_ejecutando);

	enviar_cola_archivos_bloqueados(instruccion_peticion);

	op_code cod_op = recibir_operacion(socket_fs);

	if(cod_op == ESCRIBIR_ARCHIVO){
		char* mensaje = recibir_mensaje(socket_fs);
		if(strcmp(mensaje,"OK") == 0){
			//desbloquear tras recibir "OK"
			t_queue* cola_proceso_a_desbloquear = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo);
			t_pcb* pcb_a_desbloquear = queue_pop(cola_proceso_a_desbloquear);


			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb_a_desbloquear->PID, "BLOC","READY");

			pasar_a_ready(pcb_a_desbloquear, grado_max_multiprogramacion);

			//continua con el mismo proceso
			enviar_contexto_de_ejecucion_a(contexto, PETICION_CPU, socket_cpu);
		}
	}
}

// -------- UTILS ----------

void enviar_cola_archivos_bloqueados(t_instruccion* instruccion){
	char* nombre_archivo = instruccion->parametros[0];

	//Si no existe el archivo en el diccionario, lo creo. Si existe agrego el elemento a la cola
	if(!dictionary_has_key(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo)){
		//loggeo
		sem_wait(&m_proceso_ejecutando);
		log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","BLOC");

		//antes de bloquearse, se calcula las ráfagas anteriores para el hrrn, si es fifo no lo usa
		proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
		sem_post(&m_proceso_ejecutando);
		temporal_stop(rafaga_proceso_ejecutando);
		temporal_destroy(rafaga_proceso_ejecutando);
		rafaga_proceso_ejecutando = NULL;

		//creo la cola de bloqueados y le cargo el proceso actual
		t_queue* cola_bloqueados = queue_create();
		sem_wait(&m_proceso_ejecutando);
		queue_push(cola_bloqueados, proceso_ejecutando);
		sem_post(&m_proceso_ejecutando);
		//agrego la cola al diccionario
		dictionary_put(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo, cola_bloqueados);

		//ejecuto el proceso siguiente en la cola
		poner_a_ejecutar_otro_proceso();


	}else{
		sem_wait(&m_proceso_ejecutando);
		log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","BLOC");


		//antes de bloquearse, se calcula las ráfagas anteriores para el hrrn, si es fifo no lo usa
		proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
		sem_post(&m_proceso_ejecutando);
		temporal_stop(rafaga_proceso_ejecutando);
		temporal_destroy(rafaga_proceso_ejecutando);
		rafaga_proceso_ejecutando = NULL;


		//Creo una cola, le asigno la cola del diccionario y la remuevo
		t_queue* cola_bloqueados = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, nombre_archivo);
		//cargo el proceso a bloquear en la cola
		sem_wait(&m_proceso_ejecutando);
		queue_push(cola_bloqueados, proceso_ejecutando);
		sem_post(&m_proceso_ejecutando);
		//ejecuto el proceso siguiente en la cola
		poner_a_ejecutar_otro_proceso();
	}

}

void abrir_archivo(t_instruccion* instruccion){

	char* nombre_archivo = instruccion->parametros[0];

	//Cargo estructuras restantes
	t_tabla_global_de_archivos_abiertos* archivo = malloc(sizeof(t_tabla_global_de_archivos_abiertos));
	t_fcb* fcb = dictionary_get(fcb_por_archivo, nombre_archivo);
	archivo->fileDescriptor = fcb->puntero_directo;
	archivo->file = nombre_archivo;
	archivo->open = 1;

	sem_wait(&m_proceso_ejecutando);
	proceso_ejecutando->tabla_archivos_abiertos_del_proceso = malloc(sizeof(t_tabla_de_archivos_por_proceso));
	proceso_ejecutando->tabla_archivos_abiertos_del_proceso->file = strdup(nombre_archivo) ;
	proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero = 0;
	sem_post(&m_proceso_ejecutando);
	//Cargo la FCB en el diccionario
	dictionary_put(tabla_global_de_archivos_abiertos, nombre_archivo, archivo);
}

//Envia la peticion con el codigo de operacion y la instruccion a realizar al file system
void enviar_peticion_fs(op_code code,t_instruccion* instruccion ){
	t_paquete* paquete = crear_paquete(code);

		agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

		agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
		agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
		enviar_paquete(paquete, socket_fs);


}

void enviar_peticion_puntero_fs(op_code code,t_instruccion* instruccion,int puntero, int pid){

	t_paquete* paquete = crear_paquete(code);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &pid, sizeof(int));

	agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

	agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
	agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
	agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);

	agregar_a_paquete_sin_agregar_tamanio(paquete, &puntero, sizeof(int));

	enviar_paquete(paquete, socket_fs);

}
