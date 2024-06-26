#include "kernel.h"


int socket_cpu;
int socket_kernel;
int socket_memoria;
int socket_fs;
int grado_max_multiprogramacion;

//Tablas del FS
t_dictionary* tabla_global_de_archivos_abiertos;

int main(void){

	//Declaraciones de variables para config:

	char* ip_memoria;
	char* puerto_memoria;
	char* ip_filesystem;
	char* puerto_filesystem;
	char* ip_cpu;
	char* puerto_cpu;
	char* puerto_escucha;	
	int estimacion_inicial;
	double hrrn_alfa;

	char** recursos;
	char** instancias_recursos;



	// Iniciar archivos de log y configuracion:

	t_config* config = iniciar_config();
	logger = iniciar_logger();



	// Verificacion de creacion archivo config
	if(config == NULL){
		log_error(logger, "No fue posible iniciar el archivo de configuracion !!");

		terminar_programa(logger, config);
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

		terminar_programa(logger, config);
	}

	// Realizar las conexiones y probarlas

	int result_conexion_memoria = conectar_memoria(ip_memoria, puerto_memoria);
	if(result_conexion_memoria  == -1){
		log_error(logger, "No se pudo conectar con el modulo Memoria !!");

		terminar_programa(logger, config);
	}
	log_info(logger, "El KerneL se conecto con el modulo Memoria correctamente");



	int result_conexion_filesystem = conectar_fs(ip_filesystem, puerto_filesystem);
	if(result_conexion_filesystem == -1){
		log_error(logger, "No se pudo conectar con el modulo filesystem !!");

		terminar_programa(logger, config);
	}
	log_info(logger, "El KerneL se conecto con el modulo Filesystem correctamente");


	int result_conexion_cpu = conectar_cpu(ip_cpu, puerto_cpu);
	if(result_conexion_cpu == -1){
		log_error(logger, "No se pudo conectar con el modulo CPU !!");

		terminar_programa(logger, config);
	}
	log_info(logger, "El KerneL se conecto con el modulo CPU correctamente");


	//Iniciar escucha de consola

	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "El Kernel esta listo para recibir instrucciones de la consola");


	proceso_ejecutando=NULL;
	inicializar_colas_y_semaforos();
	recurso_bloqueado = dictionary_create();
	colas_de_procesos_bloqueados_para_cada_archivo = dictionary_create(); //diccionario con key=archivo y elementos=procesos bloqueados



	void iterador_recursos(char* nombre_recurso){
		t_queue* cola_bloqueados = queue_create();

		dictionary_put(recurso_bloqueado,nombre_recurso,cola_bloqueados);
	}
	string_iterate_lines(recursos,iterador_recursos);

	tabla_global_de_archivos_abiertos = dictionary_create();

	//inicio hilos

	//inicio escucha de peticones de consolas

	pthread_t thread_consolas, thread_planificador_largo_plazo, thread_planificador_corto_plazo;

	manejar_peticiones_cosola_args* args_consolas = malloc(sizeof(manejar_peticiones_cosola_args));
	args_consolas->estimacion_inicial = estimacion_inicial;
	args_consolas->server_fd = server_fd;

	pthread_create(&thread_consolas, NULL, manejar_peticiones_consola, args_consolas);


	//inicio planificador largo plazo

	planificar_nuevos_procesos_largo_plazo_args* args_planificador_largo_plazo = malloc(sizeof(planificar_nuevos_procesos_largo_plazo_args));
	args_planificador_largo_plazo->grado_max_multiprogramacion = grado_max_multiprogramacion;
	args_planificador_largo_plazo->conexion_memoria = socket_memoria;
	args_planificador_largo_plazo->algoritmo_planificacion = algoritmo_planificacion;

	pthread_create(&thread_planificador_largo_plazo, NULL, planificar_nuevos_procesos_largo_plazo, args_planificador_largo_plazo);

	//inicio planificador corto plazo
	planificar_corto_plazo_args* args_planificador_corto_plazo = malloc(sizeof(planificar_corto_plazo_args));
	args_planificador_corto_plazo->algoritmo_planificacion = algoritmo_planificacion;
	args_planificador_corto_plazo->hrrn_alfa = hrrn_alfa;
	args_planificador_corto_plazo->socket_cpu = socket_cpu;

	pthread_create(&thread_planificador_corto_plazo, NULL, planificar_corto_plazo, args_planificador_corto_plazo);


	//Cerrando ejecucion de hilos luego de terminado
	pthread_detach(thread_consolas);
	pthread_detach(thread_planificador_largo_plazo);
	pthread_detach(thread_planificador_corto_plazo);

	//escucho las peticiones de CPU
	escuchar_peticiones_cpu(socket_cpu, recursos, instancias_recursos,grado_max_multiprogramacion,socket_memoria);

	free(args_consolas);
	free(args_planificador_largo_plazo);
	//free(args_planificador_corto_plazo->algoritmo_planificacion);
	free(args_planificador_corto_plazo);

	terminar_programa(logger, config);
} // Fin del MAIN


// DECLARACION DE FUNCIONES ====================================================================================================

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

 void terminar_programa(t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	close(socket_cpu);
	close(socket_fs);
	close(socket_memoria);
}


// CREAR CONEXIONES --------------------------------------------------------------------

int conectar_memoria(char* ip, char* puerto){

	socket_memoria = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", socket_memoria, HANDSHAKE);

	op_code cod_op = recibir_operacion(socket_memoria);
	if(cod_op != HANDSHAKE){
		return -1;
	}

	int size;
	char* buffer = recibir_buffer(&size, socket_memoria);


	if(strcmp(buffer, "OK") != 0){
		return -1;
	}

	return 0;
}

int conectar_fs(char* ip, char* puerto){

	socket_fs = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", socket_fs, HANDSHAKE);

	op_code cod_op = recibir_operacion(socket_fs);

	if(cod_op != HANDSHAKE){
		return -1;
	}

	int size;
	char* buffer = recibir_buffer(&size, socket_fs);


	if(strcmp(buffer, "OK") != 0){
		return -1;
	}

	return 0;
}

int conectar_cpu(char* ip, char* puerto){

	socket_cpu = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", socket_cpu, HANDSHAKE);

	op_code cod_op = recibir_operacion(socket_cpu);

	if(cod_op != HANDSHAKE){
		return -1;
	}

	int size;
	char* buffer = recibir_buffer(&size, socket_cpu);


	if(strcmp(buffer, "OK") != 0){
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
					log_error(logger, "Consola El cliente se desconecto. Terminando servidor");

					return NULL;
				default:
					log_warning(logger,"Consola Operacion desconocida. No quieras meter la pata");
					break;
			}
	}

	free(args);

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
	pcb_proceso->socket_server_id = socket_cliente;
	pcb_proceso->tabla_archivos_abiertos_del_proceso = NULL;



	//este malloc para evitar el sementation fault en el envio del contexto de ejecución a cpu
	pcb_proceso->registros_CPU = malloc(sizeof(registros_CPU));

	strcpy(pcb_proceso->registros_CPU->AX, "");
	strcpy(pcb_proceso->registros_CPU->BX, "");
	strcpy(pcb_proceso->registros_CPU->CX, "");
	strcpy(pcb_proceso->registros_CPU->DX, "");


	strcpy(pcb_proceso->registros_CPU->EAX, "");
	strcpy(pcb_proceso->registros_CPU->EBX, "");
	strcpy(pcb_proceso->registros_CPU->ECX, "");
	strcpy(pcb_proceso->registros_CPU->EDX, "");

	strcpy(pcb_proceso->registros_CPU->RAX, "");
	strcpy(pcb_proceso->registros_CPU->RBX, "");
	strcpy(pcb_proceso->registros_CPU->RCX, "");
	strcpy(pcb_proceso->registros_CPU->RDX, "");


	agregar_cola_new(pcb_proceso);

}



// ---------------------------- Peticiones CPU ----------------------------------------------

void *escuchar_peticiones_cpu(int cliente_fd,char** recursos,char** instancias_recursos,int grado_max_multiprogramacion,int conexion_memoria){


	int cantidad_de_recursos = string_array_size(instancias_recursos);
	int* recursos_disponibles = malloc(sizeof(int)*cantidad_de_recursos);

	if(cantidad_de_recursos!=0){
		for(int i = 0; i< cantidad_de_recursos; i++ ){
			recursos_disponibles[i] = atoi(instancias_recursos[i]);
		}
	}


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
					finalizarProceso(cliente_fd,conexion_memoria);
					break;
				case BLOQUEAR_PROCESO:
					bloquear_proceso_IO(cliente_fd,grado_max_multiprogramacion);
					break;
				case APROPIAR_RECURSOS:
					apropiar_recursos(cliente_fd, recursos, recursos_disponibles, cantidad_de_recursos);
					break;
				case DESALOJAR_RECURSOS:
					desalojar_recursos(cliente_fd, recursos, recursos_disponibles,grado_max_multiprogramacion, cantidad_de_recursos);
					break;
				case DESALOJAR_PROCESO:
					desalojar_proceso(cliente_fd,grado_max_multiprogramacion);
					break;
				case CREAR_SEGMENTO:
					//enviar_a_memoria leer word
					create_segment();
					break;
				case ELIMINAR_SEGMENTO:
					//enviar_a_memoria leer word
					delete_segment();
					break;
				case ABRIR_ARCHIVO:
					f_open();
					break;
				case CERRAR_ARCHIVO:
					f_close();
					break;
				case TRUNCAR_ARCHIVO:
					truncar_archivo();
					break;
				case APUNTAR_ARCHIVO:
					f_seek(cliente_fd);
					break;
				case LEER_ARCHIVO:
					leer_archivo();
					break;
				case ESCRIBIR_ARCHIVO:
					escribir_archivo();
					break;
				case CREAR_ARCHIVO:
					enviar_mensaje("Creando archivo nuevo", socket_fs, CREAR_ARCHIVO);
					break;
				case SEG_FAULT:
					manejar_seg_fault(cliente_fd);
					break;
				case -1:
					log_error(logger, "La CPU se desconecto. Terminando servidor ");
					free(recursos_disponibles);
					return NULL;
				default:
					log_warning(logger,"CPU Operacion desconocida. No quieras meter la pata. cod_op: %d", cod_op );
					break;
			}
	}

	return NULL;
}




