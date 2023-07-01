#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/txt.h>

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>

#include "peticiones_kernel.h"

extern int socket_kernel;
extern int socket_memoria;
extern char* path_fcb;
extern t_dictionary* fcb_por_archivo;
extern t_dictionary* tabla_global_de_archivos_abiertos;
extern t_bitarray* bitarray_bloques_libres;
extern  FILE* bitmap;
extern  FILE* bloques;

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(int conexion, t_log* logger, t_config* config, FILE* bitmap, FILE* bloques);
int conectar_con_memoria(char* ip, char* puerto);
void manejar_peticiones_kernel(t_log* logger, int server_fd, int socket_memoria, FILE* bloques,t_superbloque* superbloque);

t_superbloque* iniciar_superbloque(char* path_superbloque);
FILE* levantar_archivo_binario(char* path_archivo);

#endif /* FILESYSTEM_H_ */
