
#include "peticiones_fs.h"

//-------------------------------- Peticiones FS ------------------------------------------------------------

//Envia la peticion con el codigo de operacion y la instruccion a realizar al file system
	void enviar_peticion_fs(op_code code,t_instruccion* instruccion ){
		t_paquete* paquete = crear_paquete(code);

			agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

			agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
			agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
			agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
			enviar_paquete(paquete, socket_fs);


	}

	void enviar_peticion_puntero_fs(op_code code,t_instruccion* instruccion,int puntero){

		t_paquete* paquete = crear_paquete(code);

					agregar_a_paquete(paquete, instruccion->opcode, sizeof(char)*instruccion->opcode_lenght );

					agregar_a_paquete(paquete, instruccion->parametros[0], instruccion->parametro1_lenght);
					agregar_a_paquete(paquete, instruccion->parametros[1], instruccion->parametro2_lenght);
					agregar_a_paquete(paquete, instruccion->parametros[2], instruccion->parametro3_lenght);
					agregar_a_paquete_sin_agregar_tamanio(paquete, &puntero, sizeof(int));

					enviar_paquete(paquete, socket_fs);

	}

	void f_open(){
		t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
		t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1); //obtengo instruccion a ejecutar

		//busco si el archivo esta en la tabla global de archivos

		if(dictionary_has_key(tabla_global_de_archivos_abiertos, instruccion->parametros[0])){
			//Enviar a la cola de bloqueados esperando la apertura del archivo
			t_tabla_global_de_archivos_abiertos* tabla = dictionary_remove(tabla_global_de_archivos_abiertos, instruccion->parametros[0]);
			tabla->open ++;
			dictionary_put(tabla_global_de_archivos_abiertos, instruccion->parametros[0], tabla);
			enviar_cola_archivos_bloqueados(instruccion);
		}else{
			//(si no esta en la tabla) enviar a FS peticion para que verifique
			enviar_peticion_fs(ABRIR_ARCHIVO, instruccion);
			char* mensaje = recibir_mensaje(socket_fs);
			if(strcmp(mensaje,"OK") == 0){
				abrir_archivo(instruccion);
			}else if(strcmp(mensaje,"ERROR") == 0){
				log_info(logger, "La FCB no se encuentra en el FileSystem, procediendo a crear archivo");
				enviar_peticion_fs(CREAR_ARCHIVO, instruccion);
				char* mensaje = recibir_mensaje(socket_fs);
				if(strcmp(mensaje,"OK") == 0){
					//Abrimos el archivo
					t_fcb* fcb = dictionary_get(fcb_por_archivo, instruccion->parametros[0]);
					fcb->archivo = fopen(instruccion->parametros[0], "r+");
					t_tabla_global_de_archivos_abiertos* archivo = malloc(sizeof(t_tabla_global_de_archivos_abiertos));
					archivo->fileDescriptor = fcb->puntero_directo;
					archivo->file = instruccion->parametros[0];
					archivo->open = 1;
					dictionary_put(tabla_global_de_archivos_abiertos, instruccion->parametros[0], archivo);
				}
			}
		}
	}

	void abrir_archivo(t_instruccion* instruccion){
		//Cargo estructuras restantes
		t_tabla_global_de_archivos_abiertos* archivo = malloc(sizeof(t_tabla_global_de_archivos_abiertos));
		t_fcb* fcb = dictionary_get(fcb_por_archivo, instruccion->parametros[0]);
		archivo->fileDescriptor = fcb->puntero_directo;
		archivo->file = instruccion->parametros[0];
		archivo->open = 1;

		//abro el archivo y lo agrego a la FCB
		FILE* archivo_abierto = fopen(instruccion->parametros[0], "r+");
		fcb->archivo = archivo_abierto;

		//Cargo la FCB en el diccionario
		dictionary_put(tabla_global_de_archivos_abiertos, instruccion->parametros[0], archivo);
	};

	//Cierra la instancia de un archivo abierto, si ya no hay mas procesos solicitando el archivo lo saca de la tabla global, sino reduce el contador de archivos abiertos
	void f_close(){
		t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
		t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1); //obtengo instruccion a ejecutar

		t_tabla_global_de_archivos_abiertos* tabla = dictionary_get(tabla_global_de_archivos_abiertos, instruccion->parametros[0]);

		if(tabla->open == 1){
			//Cierro el archivo y libero la tabla
			t_fcb* archivo = dictionary_get(fcb_por_archivo, instruccion->parametros[0]);
			fclose(archivo->archivo);
			//fclose();
			dictionary_remove(tabla_global_de_archivos_abiertos, instruccion->parametros[0]);
		}else{
			//Reduzco la cantidad de abiertos y avanzo al proximo proceso
			tabla->open --;
			t_fcb* archivo = dictionary_get(fcb_por_archivo, instruccion->parametros[0]);
			fclose(archivo->archivo);
			t_queue* proceso_a_desbloquear = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, instruccion->parametros[0]);
			t_pcb* pcb_a_desbloquear = queue_pop(proceso_a_desbloquear);
			pasar_a_ready(pcb_a_desbloquear, grado_max_multiprogramacion);
		}
	}

	void f_seek(int cliente_fd){


	}



	void enviar_cola_archivos_bloqueados(t_instruccion* instruccion){

		//Si no existe el archivo en el diccionario, lo creo. Si existe agrego el elemento a la cola
		if(!dictionary_has_key(colas_de_procesos_bloqueados_para_cada_archivo, instruccion->parametros[0])){
			//loggeo
			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","BLOC");

			//antes de bloquearse, se calcula las ráfagas anteriores para el hrrn, si es fifo no lo usa
			proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
			temporal_stop(rafaga_proceso_ejecutando);
			temporal_destroy(rafaga_proceso_ejecutando);
			rafaga_proceso_ejecutando = NULL;

			//creo la cola de bloqueados y le cargo el proceso actual
			t_queue* cola_bloqueados = queue_create();
			queue_push(cola_bloqueados, proceso_ejecutando);
			//agrego la cola al diccionario
			dictionary_put(colas_de_procesos_bloqueados_para_cada_archivo, instruccion->parametros[0], cola_bloqueados);
			//ejecuto el proceso siguiente en la cola
			poner_a_ejecutar_otro_proceso();


		}else{
			log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_ejecutando->PID, "EXEC","BLOC");


			//antes de bloquearse, se calcula las ráfagas anteriores para el hrrn, si es fifo no lo usa
			proceso_ejecutando->rafaga_anterior = temporal_gettime(rafaga_proceso_ejecutando);
			temporal_stop(rafaga_proceso_ejecutando);
			temporal_destroy(rafaga_proceso_ejecutando);
			rafaga_proceso_ejecutando = NULL;


			//Creo una cola, le asigno la cola del diccionario y la remuevo
			t_queue* cola_bloqueados = dictionary_get(colas_de_procesos_bloqueados_para_cada_archivo, instruccion->parametros[0]);
			//cargo el proceso a bloquear en la cola
			queue_push(cola_bloqueados, proceso_ejecutando);

			//ejecuto el proceso siguiente en la cola
			poner_a_ejecutar_otro_proceso();
		}

	}




	void truncar_archivo(){

			t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
			t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 1);

			//bloquear proceso
			//crear una bloquear proceso general UwU
			enviar_peticion_fs(TRUNCAR_ARCHIVO,instruccion_peticion);

			//desbloquear tras recibir "OK"
		}


	void leer_archivo(){

			t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
			t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 1);



			enviar_peticion_puntero_fs(LEER_ARCHIVO,instruccion_peticion, proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero);

		}


	void escribir_archivo(){
			t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
			t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 1);



			enviar_peticion_puntero_fs(ESCRIBIR_ARCHIVO,instruccion_peticion, proceso_ejecutando->tabla_archivos_abiertos_del_proceso->puntero);
		}

