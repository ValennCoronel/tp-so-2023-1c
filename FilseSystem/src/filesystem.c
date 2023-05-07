#include "filesystem.h"

int main(void){

	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;
	char* path_superbloque;
	char* path_bitmap;
	char* path_bloques;
	char* path_fcb;
	double retardo_acceso_bloque;
	int conexion;
	FILE* bitmap;

	logger = iniciar_logger();
	t_config* config = iniciar_config();

	if(config == NULL){
		log_error(logger, "No se pudo crear el archivo de configuración !!");

		terminar_programa(conexion, logger, config, bitmap);
	}

	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	path_superbloque = config_get_string_value(config, "PATH_SUPERBLOQUE");
	path_bitmap =config_get_string_value(config, "PATH_BITMAP");
	path_bloques =config_get_string_value(config, "PATH_BLOQUES");
	path_fcb = config_get_string_value(config, "PATH_FCB");
	retardo_acceso_bloque = config_get_double_value(config, "RETARDO_ACCESO_BLOQUE");

	if(!ip_memoria || !puerto_memoria || !puerto_escucha || !path_superbloque || !path_bitmap || !path_bloques || !path_fcb || !retardo_acceso_bloque){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA', 'PATH_SUPERBLOQUE', 'PATH_BITMAP', 'PATH_BLOQUES', 'PATH_FCB', 'RETARDO_ACCESO_BLOQUE' ");

		terminar_programa(conexion, logger, config, bitmap);
	}

	int result_conexion_memoria = conectar_con_memoria(conexion, ip_memoria, puerto_memoria);


	if(result_conexion_memoria == -1){
		log_error(logger, "El File System no se pudo conectar con el modulo Memoria !!");

		terminar_programa(conexion, logger, config, bitmap);

	}

	log_info(logger, "El File System se conecto con el modulo Memoria correctamente");

	//TODO DESCOMENTAR ESTO DESPUES Levanto el bitmap
	//bitmap = fopen(path_bitmap, "w");
	//administrar_fcb(path_fcb);

	//escucho conexiones del Kernel
	int server_fd = iniciar_servidor(puerto_escucha);

	log_info(logger, "File System listo para recibir peticiones del Kernel");

	manejar_peticiones_kernel(logger, server_fd);


	terminar_programa(conexion, logger, config, bitmap);
}

t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("fileSystem.log", "FileSystem", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("filesystem.config");

	return nueva_config;
}


void terminar_programa(int conexion, t_log* logger, t_config* config, FILE* bitmap){
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
	fclose(bitmap);
}

int conectar_con_memoria(int conexion, char* ip, char* puerto){

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

void administrar_fcb(char* path_fcb){
	FILE* archivo_FCB = fopen(path_fcb, 'r');

	// si no existe lo creo
	if(archivo_FCB == NULL){
		fclose(archivo_FCB);
		archivo_FCB = fopen(path_fcb, 'w');
	}

	//TODO iniciar estructuras del FCB

	fclose(archivo_FCB);
}
