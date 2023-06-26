#include "filesystem.h"


int socket_cpu;
int socket_kernel;
int socket_memoria;
int socket_fs;
t_dictionary fcb_por_archivo;

int main(void){

	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;
	char* path_superbloque;
	char* path_bitmap;
	char* path_bloques;
	char* path_fcb;
	double retardo_acceso_bloque;
	FILE* bitmap;
	FILE* bloques;
	t_fcb* fcb;
	t_superbloque* superbloque;

	logger = iniciar_logger();
	t_config* config = iniciar_config();

	if(config == NULL){
		log_error(logger, "No se pudo crear el archivo de configuración !!");

		terminar_programa(socket_memoria, logger, config, bitmap, bloques);
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

		terminar_programa(socket_memoria, logger, config, bitmap, bloques);
	}

	int result_conexion_memoria = conectar_con_memoria(ip_memoria, puerto_memoria);


	if(result_conexion_memoria == -1){
		log_error(logger, "El File System no se pudo conectar con el modulo Memoria !!");

		terminar_programa(socket_memoria, logger, config, bitmap, bloques);

	}

	log_info(logger, "El File System se conecto con el modulo Memoria correctamente");


	// levanta los archivos binarios que neceesita y inicializa las estructuras administrativas necesarias

//TODO IMPORTANTISIMO ESTO DEBE SER ARREGLADO Y FCB DEBE INICIARSE EN la de fopen
	//de aca ↧ ↧ ↧ ↧ ↧ ↧ ↧ ↧ ↧
	fcb = iniciar_fcb(path_fcb);
	fcb_por_archivo = dictionary_create();
	dictionary_put(fcb_por_archivo, fcb->nombre_archivo, fcb);

	if(fcb == NULL){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo del FCB: 'NOMBRE_ARCHIVO', 'TAMANIO_ARCHIVO', 'PUNTERO_DIRECTO', 'PUNTERO_INDIRECTO' ");

		terminar_programa(socket_memoria, logger, config, bitmap, bloques);
	}
//hasta aca  ↥ ↥ ↥ ↥ ↥ ↥ ↥ ↥ ↥
	superbloque = iniciar_superbloque(path_superbloque);
	if(fcb == NULL){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo del superbloque: 'BLOCK_SIZE', 'BLOCK_COUNT' ");

		terminar_programa(socket_memoria, logger, config, bitmap, bloques);
	}

	// creo el bitarray
	int tamanio_bitmap = superbloque->block_count;
	char* bits = malloc(tamanio_bitmap);
	t_bitarray* bitmap_array = bitarray_create_with_mode(bits, tamanio_bitmap, MSB_FIRST);

	bitmap = levantar_archivo_binario(path_bitmap);

	// guardo los bits en el archivo del bitmap
	fwrite(&bits,tamanio_bitmap,1, bitmap);


	bloques = levantar_archivo_binario(path_bloques);

	int tamanio_bloques = superbloque->block_count * superbloque->block_size;


	//escucho conexiones del Kernel
	socket_kernel = iniciar_servidor(puerto_escucha);

	log_info(logger, "File System listo para recibir peticiones del Kernel");

	manejar_peticiones_kernel(logger, socket_kernel, socket_memoria);


	bitarray_destroy(bitmap_array);
	terminar_programa(socket_memoria, logger, config, bitmap, bloques);
}




t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("filesystem.log", "FileSystem", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("filesystem.config");

	return nueva_config;
}


void terminar_programa(int conexion, t_log* logger, t_config* config, FILE* bitmap, FILE* bloques){
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
	fclose(bitmap);
	fclose(bloques);
}

int conectar_con_memoria( char* ip, char* puerto){

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

void manejar_peticiones_kernel(t_log* logger, int server_fd, int socket_memoria, FILE* bloques){

	int socket_kernel = esperar_cliente(server_fd);

	while (1) {
			int cod_op = recibir_operacion(socket_kernel);

			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(socket_kernel);
					break;
				case HANDSHAKE:
					recibir_handshake(socket_kernel);
					break;
				case ABRIR_ARCHIVO:
					abrir_archivo(socket_kernel);
					break;
				case CREAR_ARCHIVO:
					crear_archivo(socket_kernel);
					break;
				case TRUNCAR_ARCHIVO:
					truncar_archivo(socket_kernel);
						break;
				case LEER_ARCHIVO:
					leer_archivo(socket_kernel, socket_memoria);
					break;
				case ESCRIBIR_ARCHIVO:
					escribir_archivo(socket_kernel, socket_memoria, bloques);
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

// levanta un archivo binario en base al path que recibe y lo devuelve
// si no existe lo crea
FILE* levantar_archivo_binario(char* path_archivo){

	FILE* archivo = fopen(path_archivo, "ab");

	if(archivo == NULL){
		log_error(logger, "No existe el archivo con el path: %s",path_archivo);
		return NULL;
	}

	return archivo;
}

//levanta el archivo de fcb  y obtiene los datos del archivo para iniciar el FCB
// el archivo de fcb usa el formato de config de las commons
t_fcb* iniciar_fcb(char* path_fcb){
	t_config* config_FCB = config_create(path_fcb);

	if(config_FCB == NULL){
			return NULL;
	}

	t_fcb *fcb = malloc(sizeof(t_fcb));

	fcb->nombre_archivo = config_get_string_value(config_FCB, "NOMBRE_ARCHIVO");
	fcb->tamanio_archivo = config_get_int_value(config_FCB, "TAMANIO_ARCHIVO");
	fcb->puntero_directo = config_get_int_value(config_FCB, "PUNTERO_DIRECTO");
	fcb->puntero_indirecto = config_get_int_value(config_FCB, "PUNTERO_INDIRECTO");



	if(!(fcb->nombre_archivo) || !(fcb->tamanio_archivo) || !(fcb->puntero_directo) || !(fcb->puntero_indirecto) ){

		return NULL;
	}


	return fcb;
}


//levanta el archivo de superbloque  y obtiene los datos del archivo para iniciar el superbloque
// el archivo de superbloque usa el formato de config de las commons
t_superbloque* iniciar_superbloque(char* path_superbloque){
	t_config* config_superbloque = config_create(path_superbloque);

	if(config_superbloque == NULL){
		return NULL;
	}

	t_superbloque *superbloque = malloc(sizeof(t_superbloque));

	superbloque->block_size = config_get_int_value(config_superbloque, "BLOCK_SIZE");
	superbloque->block_count = config_get_int_value(config_superbloque, "BLOCK_COUNT");


	if(!(superbloque->block_count) || !(superbloque->block_size) ){

		return NULL;
	}

	return superbloque;
}
