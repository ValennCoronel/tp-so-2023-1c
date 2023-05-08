#include "consola.h"

int main(void){

	//Declaracion variables para config
	char* ip_kernel;
	char* puerto_kernel;

	//Declaracion variables para test de conexion
	int conexion_kernel;

	//Iniciar logger y config

	logger = iniciar_logger();
	t_config* config = iniciar_config();


	//Testeo config
	if(config == NULL){
		log_error(logger, "No se pudo iniciar el archivo de configuración !!");

		terminar_programa(conexion_kernel, logger, config);
	}

	//Levantar datos de config a variables
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");


	//Testeo de carga de variables
	if(!ip_kernel || !puerto_kernel ){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'PUERTO_KERNEL', 'IP_KERNEL");

		terminar_programa(conexion_kernel, logger, config);
	}

	//Realizo la conexion con memoria
	int result_conexion = conectar_modulo(conexion_kernel, ip_kernel, puerto_kernel);

	//Testeo el resultado de la conexion
	if(result_conexion == -1){
		log_error(logger, "Consola no se pudo conectar con el modulo Kernel !!");

		terminar_programa(conexion_kernel, logger, config);

	}

	log_info(logger, "La Consola se conecto con el modulo Kernel correctamente");



	terminar_programa(conexion_kernel, logger, config);

} //FIN DEL MAIN




//Funciones de inicio de Config y Logger
t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("consola.log", "CPU", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("consola.config");

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
	enviar_mensaje("OK", conexion);

	int size;
	char* buffer = recibir_buffer(&size, conexion);

	if(strcmp(buffer, "ERROR") == 0 || strcmp(buffer, "") == 0){
		return -1;
	}

	return 0;

