
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


	void f_open(){
		t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
		t_instruccion* instruccion = list_get(contexto->lista_instrucciones,contexto->program_counter-1); //obtengo instruccion a ejecutar

		//busco si la instruccion esta en la tabla global de archivos

		bool buscar_archivo(void* element){
			tabla_global_de_archivos_abiertos* aux = (tabla_global_de_archivos_abiertos*)element;
			return aux->file == instruccion->parametros[0];
		}

		tabla_global_de_archivos_abiertos* resultado = list_find(tabla_gaa, buscar_archivo);

		if(resultado != NULL){
			//Enviar a la cola de bloqueados esperando la apertura del archivo

		}else{
			//(si no esta en la tabla) enviar a FS peticion para que verifique, lo cree y lo añada a ambas tablas
			enviar_peticion_fs(ABRIR_ARCHIVO, instruccion);
		}
	}


	//Cierra la instancia de un archivo abierto, si ya no hay mas procesos solicitando el archivo lo saca de la tabla global, sino reduce el contador de archivos abiertos
	void f_close(int cliente_fd){

	}

	void f_seek(int cliente_fd){

	}



	void enviar_cola_archivos_bloqueados(t_instruccion instruccion){


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

			enviar_peticion_fs(LEER_ARCHIVO,instruccion_peticion);

		}


	void escribir_archivo(){
			t_contexto_ejec* contexto = (t_contexto_ejec*) recibir_contexto_de_ejecucion(socket_cpu);
			t_instruccion* instruccion_peticion = (t_instruccion*) list_get(contexto->lista_instrucciones, contexto->program_counter - 1);

			enviar_peticion_fs(ESCRIBIR_ARCHIVO,instruccion_peticion);
		}

