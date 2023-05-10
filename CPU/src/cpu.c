#include "cpu.h"

int main(void){

	//Declaracion variables para config
	char* retardo_instruccion;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;
	char* tam_max_segmento;

	//Declaracion variables para test de conexion
	int conexion_memoria;

	//Iniciar logger y config

	logger = iniciar_logger();
	t_config* config = iniciar_config();


	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");

		terminar_programa(conexion_memoria, logger, config);
	}

	//Levantar datos de config a variables

	retardo_instruccion = config_get_int_value(config, "RETARDO_INSTRUCCION");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	tam_max_segmento = config_get_int_value(config, "TAM_MAX_SEGMENTO");

	// retardo_instruccion = config_get_string_value(config, "RETARDO_INSTRUCCION");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	//tam_max_segmento = config_get_string_value(config, "TAM_MAX_SEGMENTO");



	//Testeo de carga de variables
	//if(!ip_memoria || !puerto_memoria || !puerto_escucha){
		//log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'RETARDO_INSTRUCCION', 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA', 'TAM_MAX_SEGMENTO'");

		//terminar_programa(conexion_memoria, logger, config);
	//}

	//Realizo la conexion con memoria
	int result_conexion_memoria = conectar_modulo(conexion_memoria, ip_memoria, puerto_memoria);

	//Testeo el resultado de la conexion
	if(result_conexion_memoria == -1){
		log_error(logger, "La CPU no se pudo conectar con el modulo Memoria !!");

		terminar_programa(conexion_memoria, logger, config);
	}

	log_info(logger, "La CPU se conecto con el modulo Memoria correctamente");



	//Escucho conexiones del Kernel
	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "CPU listo para recibir peticiones del Kernel");

	manejar_peticiones_kernel(logger, server_fd);


	terminar_programa(conexion_memoria, logger, config);


} //FIN DEL MAIN




//Funciones de inicio de Config y Logger
t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("cpu.config");

	return nueva_config;
}



//Funcion para finalizar el programa
void terminar_programa(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
}


//Funcion para crear conexion entre modulos
int conectar_modulo(int conexion, char* ip, char* puerto){

	conexion = crear_conexion(ip, puerto);

	//enviar handshake
	enviar_mensaje("OK", conexion, HANDSHAKE);

	int size;
	char* buffer = recibir_buffer(&size, conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;
}


//Funcion para manejo de peticiones del kernel
void manejar_peticiones_kernel(t_log* logger, int server_fd){

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

				case CODIGO_OPERACION_RECIBIDO_POR_KERNEL:

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

void Codigo_recibido_por_Kernel (int cliente_fd){


	//se levanta el pcb
	t_contexto_ejec* nuevo_pcb = recibir_paquete_pcb (cliente_fd);




}


