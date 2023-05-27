#include "kernel.h"

int main(void){

	//Declaraciones de variables para config:

	char* ip_memoria;
	char* puerto_memoria;
	char* ip_filesystem;
	char* puerto_filesystem;
	char* ip_cpu;
	char* puerto_cpu;
	char* puerto_escucha;
	char* algoritmo_planificacion;
	int estimacion_inicial;
	double hrrn_alfa;
	int grado_max_multiprogramacion;
	char** recursos;
	char** instancias_recursos;

	// Variables de testeeo de conexion
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
	estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	hrrn_alfa = config_get_double_value(config, "HRRN_ALFA");
	grado_max_multiprogramacion = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	recursos = config_get_array_value(config, "RECURSOS");
	instancias_recursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");

	// Control archivo configuracion
	if(!ip_memoria || !puerto_memoria || !ip_filesystem || !puerto_filesystem || !ip_cpu || !puerto_cpu || !puerto_escucha){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA'");

		terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);
	}

	// Realizar las conexiones y probarlas

	int result_conexion_memoria = conectar_modulo(&conexion_memoria, ip_memoria, puerto_memoria);
	if(result_conexion_memoria == -1){
		log_error(logger, "No se pudo conectar con el modulo Memoria !!");

		terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);
	}
	log_info(logger, "El KerneL se conecto con el modulo Memoria correctamente");



	int result_conexion_filesystem = conectar_modulo(&conexion_filesystem, ip_filesystem, puerto_filesystem);
	if(result_conexion_filesystem == -1){
		log_error(logger, "No se pudo conectar con el modulo filesystem !!");

		terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);
	}
	log_info(logger, "El KerneL se conecto con el modulo Filesystem correctamente");


	int result_conexion_cpu = conectar_modulo(&conexion_cpu, ip_cpu, puerto_cpu);
	if(result_conexion_cpu == -1){
		log_error(logger, "No se pudo conectar con el modulo CPU !!");

		terminar_programa(conexion_memoria, conexion_filesystem, conexion_cpu, logger, config);
	}
	log_info(logger, "El KerneL se conecto con el modulo CPU correctamente");


	//Iniciar escucha de consola

	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "El Kernel esta listo para recibir instrucciones de la consola");


	inicializar_colas_y_semaforos();

	//inicio hilos
	pthread_t thread_consolas, thread_planificador_largo_plazo, thread_planificador_corto_plazo;

	manejar_peticiones_cosola_args* args_consolas = malloc(sizeof(manejar_peticiones_cosola_args));
	args_consolas->estimacion_inicial = estimacion_inicial;
	args_consolas->server_fd = server_fd;

	pthread_create(&thread_consolas, NULL, manejar_peticiones_consola, args_consolas);


	//inicio planificador largo plazo

	planificar_nuevos_procesos_largo_plazo_args* args_planificador_largo_plazo = malloc(sizeof(planificar_nuevos_procesos_largo_plazo_args));
	args_planificador_largo_plazo->grado_max_multiprogramacion = grado_max_multiprogramacion;
	args_planificador_largo_plazo->conexion_memoria = conexion_memoria;

	pthread_create(&thread_planificador_largo_plazo, NULL, planificar_nuevos_procesos_largo_plazo, args_planificador_largo_plazo);

	//inicio planificador corto plazo
	planificar_corto_plazo_args* args_planificador_corto_plazo = malloc(sizeof(planificar_corto_plazo_args));
	args_planificador_corto_plazo->algoritmo_planificacion = malloc( strlen(algoritmo_planificacion) *sizeof(char));
	args_planificador_corto_plazo->algoritmo_planificacion = algoritmo_planificacion;
	args_planificador_corto_plazo->hrrn_alfa = hrrn_alfa;
	args_planificador_corto_plazo->socket_cpu = conexion_cpu;

	pthread_create(&thread_planificador_corto_plazo, NULL, planificar_corto_plazo, args_planificador_corto_plazo);


	//Cerrando ejecucion de hilos luego de terminado
	pthread_detach(thread_consolas);
	pthread_detach(thread_planificador_largo_plazo);
	pthread_detach(thread_planificador_corto_plazo);


	escuchar_peticiones_cpu(conexion_cpu);

	free(args_consolas);
	free(args_planificador_largo_plazo);
	free(args_planificador_corto_plazo->algoritmo_planificacion);
	free(args_planificador_corto_plazo);

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

int conectar_modulo(int* conexion, char* ip, char* puerto){

	*conexion = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", *conexion, HANDSHAKE);

	int size;
	char* buffer = recibir_buffer(&size, *conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;
}


// Menejo de peticiones de consola



void *manejar_peticiones_consola(void *arg){
	manejar_peticiones_cosola_args* args = (manejar_peticiones_cosola_args*) arg;

	int server_fd = args->server_fd;
	int estimacion_inicial = args->estimacion_inicial;


		while(1){
			pthread_t thread;
			int consola_fd = esperar_cliente(server_fd);

			atender_cliente_args* args_cliente = malloc(sizeof(atender_cliente_args));
			args_cliente->consola_fd = consola_fd;
			args_cliente->estimacion_inicial = estimacion_inicial;

			pthread_create(&thread, NULL, atender_cliente, args_cliente);

			pthread_detach(thread);
		}

		free(args);

		return NULL;
}

void *atender_cliente(void *arg){

	atender_cliente_args* args = (atender_cliente_args*) arg;

	int cliente_fd = args->consola_fd;
	int estimacion_inicial = args->estimacion_inicial;
	printf("%d", cliente_fd);
	while(1){
		int cod_op = recibir_operacion(cliente_fd);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case HANDSHAKE:
					recibir_handshake(cliente_fd);
					break;
				case INSTRUCCIONES:
					recibir_instrucciones(cliente_fd, estimacion_inicial);
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

void recibir_instrucciones(int socket_cliente, int estimacion_inicial){

	t_list* instrucciones = (t_list*) recibir_paquete_instrucciones(socket_cliente);

	//se levanta el pcb
	t_pcb* pcb_proceso = malloc(sizeof(t_pcb));

	pcb_proceso->PID = rand() % 10000;
	pcb_proceso->instrucciones = instrucciones;
	pcb_proceso->program_counter = 1;
	pcb_proceso->estimado_proxima_rafaga = estimacion_inicial;
	pcb_proceso->tiempo_llegada_rady = 0;

	log_info(logger, "Se crea el proceso %d en NEW", pcb_proceso->PID);

	agregar_cola_new(pcb_proceso);

}



//Peticiones CPU

void *escuchar_peticiones_cpu(int cliente_fd){

	while(1){
		int cod_op = recibir_operacion(cliente_fd);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case HANDSHAKE:
					manejar_handshake_del_cliente(cliente_fd);
					break;
				case FINALIZAR_PROCESO:
					finalizar_proceso(cliente_fd);
					//llamar hilo planificar_corto_plazo para poner a ejecutar al siguiente proceso
					break;
				case BLOQUEAR_PROCESO:
					bloquear_proceso(cliente_fd);
					//llamar hilo planificar_corto_plazo para poner a ejecutar al siguiente proceso
					break;
				case PETICION_KERNEL:
					manejar_peticion_al_kernel(cliente_fd);

					break;
				case DESALOJAR_PROCESO:
					desalojar_proceso(cliente_fd);
					//llamar hilo planificar_corto_plazo para poner a ejecutar al siguiente proceso
					break;
				case -1:
					log_error(logger, "La CPU se desconecto. Terminando servidor");
					return NULL;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata, la operacion es: %d",cod_op );
					break;
			}
	}

	return NULL;
}

