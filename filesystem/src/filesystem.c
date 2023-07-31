#include "filesystem.h"


int socket_cpu;
int socket_kernel;
int socket_memoria;
int socket_fs;
t_dictionary* fcb_por_archivo;
char* path_fcb;
t_bitarray* bitarray_bloques_libres;
FILE* bitmap;
FILE* bloques;

int main(void){

	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha;
	char* path_superbloque;
	char* path_bitmap;
	char* path_bloques;

	t_superbloque* superbloque;

	logger = iniciar_logger();
	t_config* config = iniciar_config();

	if(config == NULL){
		log_error(logger, "No se pudo crear el archivo de configuración !!");

		terminar_programa( logger, config, bitmap, bloques);
	}

	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	path_superbloque = config_get_string_value(config, "PATH_SUPERBLOQUE");
	path_bitmap =config_get_string_value(config, "PATH_BITMAP");
	path_bloques =config_get_string_value(config, "PATH_BLOQUES");
	path_fcb = config_get_string_value(config, "PATH_FCB");
	retardo_acceso_bloque = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");

	if(!ip_memoria || !puerto_memoria || !puerto_escucha || !path_superbloque || !path_bitmap || !path_bloques || !path_fcb || !retardo_acceso_bloque){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo de configuración: 'IP_MEMORIA', 'PUERTO_MEMORIA', 'PUERTO_ESCUCHA', 'PATH_SUPERBLOQUE', 'PATH_BITMAP', 'PATH_BLOQUES', 'PATH_FCB', 'RETARDO_ACCESO_BLOQUE' ");

		terminar_programa(logger, config, bitmap, bloques);
	}

	path_superbloque = string_replace(path_superbloque, "~", "/home/utnso");
	path_bitmap = string_replace(path_bitmap, "~", "/home/utnso");
	path_bloques = string_replace(path_bloques, "~", "/home/utnso");
	path_fcb = string_replace(path_fcb, "~", "/home/utnso");

	int result_conexion_memoria = conectar_con_memoria(ip_memoria, puerto_memoria);


	if(result_conexion_memoria == -1){
		log_error(logger, "El File System no se pudo conectar con el modulo Memoria !!");

		terminar_programa( logger, config, bitmap, bloques);

	}

	log_info(logger, "El File System se conecto con el modulo Memoria correctamente");

	superbloque = iniciar_superbloque(path_superbloque);
	if(superbloque == NULL){
		log_error(logger, "Falta una de las siguientes propiedades en el archivo del superbloque: 'BLOCK_SIZE', 'BLOCK_COUNT' ");

		terminar_programa(logger, config, bitmap, bloques);
	}

	// creo el bitarray
	int tamanio_bitmap = superbloque->block_count /8; // lo paso a bytes

	bitmap = levantar_archivo_binario(path_bitmap);

	// guardo los bits en el archivo del bitmap
	//fwrite(bits,sizeof(char),superbloque->block_count, bitmap);

	//trunco el archivo para que tenga el tamaño del bitmap
	int bitmap_fd = fileno(bitmap);

	ftruncate(bitmap_fd, tamanio_bitmap);

	//creo un bloque en memoria para manejar la escritura en el archivo
	//	osea si modifico algo en el bits_bitmap, tambien se moficica en el archivo
	char* bits_bitmap = mmap(NULL, tamanio_bitmap, PROT_WRITE, MAP_SHARED, bitmap_fd, 0);

	bitarray_bloques_libres = bitarray_create_with_mode(bits_bitmap, tamanio_bitmap, MSB_FIRST);

	bloques = levantar_archivo_binario(path_bloques);
	
	int bloques_fd = fileno(bloques);

	ftruncate(bloques_fd, superbloque->block_size * superbloque->block_count);



	fcb_por_archivo = dictionary_create();

	//escucho conexiones del Kernel
	socket_fs = iniciar_servidor(puerto_escucha);

	log_info(logger, "File System listo para recibir peticiones del Kernel");

	manejar_peticiones_kernel(logger, socket_fs, socket_memoria, bloques, superbloque);


	munmap(bits_bitmap, tamanio_bitmap);
	terminar_programa( logger, config, bitmap, bloques);
}




t_log* iniciar_logger(void){

	t_log* nuevo_logger = log_create("filesystem.log", "FileSystem", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}

t_config* iniciar_config(void){
	t_config* nueva_config = config_create("filesystem.config");

	return nueva_config;
}


void terminar_programa(t_log* logger, t_config* config, FILE* bitmap, FILE* bloques){
	log_destroy(logger);
	config_destroy(config);
	close(socket_memoria);
	close(socket_fs);
	close (socket_kernel);
	bitarray_destroy(bitarray_bloques_libres);
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

void manejar_peticiones_kernel(t_log* logger, int server_fd, int socket_memoria, FILE* bloques,t_superbloque* superbloque){

	socket_kernel = esperar_cliente(socket_fs);

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
					abrir_archivo();
					break;
				case CREAR_ARCHIVO:
					crear_archivo();
					break;
				case TRUNCAR_ARCHIVO:
					truncar_archivo( superbloque);
						break;
				case LEER_ARCHIVO:
					leer_archivo(superbloque);
					break;
				case ESCRIBIR_ARCHIVO:
					escribir_archivo(superbloque);
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

	FILE* archivo = fopen(path_archivo, "ab+");

	archivo = freopen(path_archivo, "rb+", archivo);


	if(archivo == NULL){
		log_error(logger, "No existe el archivo con el path: %s",path_archivo);
		return NULL;
	}

	return archivo;
}




//levanta el archivo de superbloque  y obtiene los datos del archivo para iniciar el superbloque
// el archivo de superbloque usa el formato de config de las commons
t_superbloque* iniciar_superbloque(char* path_superbloque){
	t_config* config_superbloque = config_create(path_superbloque);

	if(config_superbloque == NULL){
		log_error(logger, "No se encontro el arhivo del superbloque, con path %s", path_superbloque);
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
