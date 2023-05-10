
#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include "utils_cliente.h"
#include "utils_server.h"

t_log* iniciar_logger(void);
t_config* iniciar_config(char*);
void terminar_programa(int, t_log*, t_config*);
int conectar_modulo(int conexion, char* ip, char* puerto);
void manejar_peticiones_kernel(t_log* logger, int server_fd);

#endif /* CPU_H */
