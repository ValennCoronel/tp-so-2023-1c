#include "memoria.h"

int main(void){

	//Declaracion variables para config
	char* puerto_escucha;
	char* tam_memoria;
	char* tam_segmento_0;
	char* cant_segmentos;
	char* retardo_memoria;
	char* retardo_compactacion;
	char* algoritmo_asignacion;

	//Iniciar logger y config

	logger = iniciar_logger();
	t_config* config = iniciar_config();


	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");

		terminar_programa(logger, config);
	}

	//Levantar datos de config a variables
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	tam_memoria = config_get_string_value(config, "TAM_MEMORIA");
	tam_segmento_0 = config_get_string_value(config, "TAM_SEGMENTO_0");
	cant_segmentos = config_get_string_value(config, "CANT_SEGMENTOS");
	retardo_memoria = config_get_string_value(config, "RETARDO_MEMORIA");
	retardo_compactacion = config_get_string_value(config, "RETARDO_COMPACTACION");
	algoritmo_asignacion = config_get_string_value(config, "ALGORITMO_ASIGNACION");


	//Testeo de carga de variables
	if(!puerto_escucha || !tam_memoria || !tam_segmento_0 || !cant_segmentos || !retardo_memoria || !retardo_compactacion || !algoritmo_asignacion){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'puerto_escucha', 'tam_memoria', 'tam_segmento_0', 'cant_segmentos', 'retardo_memoria', 'retardo_compactacion', 'algoritmo_asignacion'");

		terminar_programa(logger, config);
	}

	//Escucho conexiones del Kernel, CPU y File System
	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "Memoria lista para recibir peticiones");

	manejar_peticiones(server_fd);



} //FIN DEL MAIN


//Funciones de inicio de Config y Logger
t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("memoria.log", "Memoria", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("memoria.config");

	return nueva_config;
}



//Funcion para finalizar el programa
void terminar_programa(t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
}


//Funcion para crear conexion entre modulos
int conectar_modulo(int conexion, char* ip, char* puerto){

	conexion = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", conexion);

	int size;
	char* buffer = recibir_buffer(&size, conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;
}


//Funcion para manejo de peticiones
void manejar_peticiones(int server_fd ){

	while(1){
		pthread_t thread;
		int cliente_fd = esperar_cliente(server_fd);

		pthread_create(&thread, NULL, atender_cliente, cliente_fd);

		pthread_detach(thread);
	}

}

void* atender_cliente(void *args){
	int cliente_fd = (int) args;

	while(1){
		int cod_op = recibir_operacion(cliente_fd);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case HANDSHAKE:
					recibir_handshake(cliente_fd);
					break;
				case NUEVO_PROCESO_MEMORIA:
					crear_nuevo_proceso(cliente_fd);
					break;
				case -1:
					log_error(logger, "El cliente se desconecto. Terminando servidor");
					return NULL;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
	}

	return NULL;
}

void crear_nuevo_proceso(int socket_cliente){
	//TODO crear estructuras administrativas y enviar tabla de segmentos a Kernell
}

void acceder_espacio_ususario(int socket_kernel){
	enviar_mensaje("OK",socket_kernel);
}
