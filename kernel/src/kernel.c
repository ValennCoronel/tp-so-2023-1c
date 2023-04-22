#include "kernel.h"

int main(void){

	//Declaraciones varias:

	char* ip_memoria;
	char* puerto_memoria;
	char* ip_filesystem;
	char* puerto_filesystem;
	char* ip_cpu;
	char* puerto_cpu;
	char* puerto_escucha;
	char* algoritmo_planificacion;
	char* estimacion_inicial;
	char* hrrn_alfa;
	char* grado_max_multiprogramacion;
	t_list* recursos;
	t_list* instancias_recursos;
	int conexion_memoria;
	int conexion_filesystem;
	int conexion_cpu;

	// Iniciar archivos de log y configuracion:

	t_config* config = iniciar_config();
	logger = iniciar_logger();

	// Verificacion de creacion archivo config
	if(config == NULL){
		log_error(logger, "No fue posible iniciar el archivo de configuracion !!");
		terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);
	}

	// Carga de datos de config en variable y archivo
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

	ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
	puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");

	ip_cpu = config_get_string_value(config, "IP_CPU");
	puerto_cpu = config_get_string_value(config, "PUERTO_CPU");

	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

	algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	estimacion_inicial = config_get_string_value(config, "ESTIMACION_INICIAL");
	hrrn_alfa = config_get_string_value(config, "HRRN_ALFA");
	grado_max_multiprogramacion = config_get_string_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	recursos = config_get_string_value(config, "RECURSOS");
	instancias_recursos = config_get_string_value(config, "INSTANCIAS_RECURSOS");


	// Control archivo configuracion
	if(!ip_memoria || !puerto_memoria || !ip_filesystem || !puerto_filesystem || !ip_cpu || !puerto_cpu || !puerto_escucha){
			log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA'");

			terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);
		}


	//Declaracion de enteros para probar la conexion
	int result_conexion_memoria = conectar_modulo(conexion_memoria, ip_memoria, puerto_memoria);
	int result_conexion_filesystem = conectar_modulo(conexion_filesystem, ip_filesystem, puerto_filesystem);
	int result_conexion_cpu = conectar_modulo(conexion_cpu, ip_cpu, puerto_cpu);


	//Prueba de conexion

	if(result_conexion_memoria == -1){
			log_error(logger, "No se pudo conectar con el modulo Memoria !!");

			terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);

		}
	if(result_conexion_filesystem == -1){
			log_error(logger, "No se pudo conectar con el modulo filesystem !!");

			terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);

		}
	if(result_conexion_cpu == -1){
			log_error(logger, "No se pudo conectar con el modulo CPU !!");

			terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);

		}

	log_info(logger, "El KerneL se conecto con el modulo Memoria correctamente");
	log_info(logger, "El KerneL se conecto con el modulo Filesystem correctamente");
	log_info(logger, "El KerneL se conecto con el modulo CPU correctamente");



	//Iniciar escucha de consola

	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "El Kernel esta listo para recibir instrucciones de la consola");

	manejar_peticiones_consola(logger, server_fd);


	terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);



} // Fin del MAIN


// DECLARACION DE FUNCIONES

//Iniciar archivo de log y de config

t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("kernel.log", "Kernel", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("kernel.config");

	return nueva_config;
}


//Finalizar el programa

 void terminar_programa(int conexion, int conexion2, int conexion3, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
	close(conexion2);
	close(conexion3);
}


// CREAR CONEXIONES --------------------------------------------------------------------

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


// Menejo de peticiones de consola

void manejar_peticiones_consola(t_log* logger, int server_fd){

	int cliente_fd = esperar_cliente(server_fd);

	while (1) {
			int cod_op = recibir_operacion(cliente_fd);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case HANDSHAKE:
					recibir_handshake(cliente_fd);
					break;
				case -1:
					log_error(logger, "El cliente se desconecto. Terminando servidor");
					return;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}

	return ;
}
