
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

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>
// Estructuras y variables de psudocodigo



t_log* iniciar_logger(void);
t_config* iniciar_config(char*);
void terminar_programa(t_config* config);
int conectar_modulo(char* ip, char* puerto);
void manejar_peticiones_kernel(t_log* logger, int server_fd);

void paquete_instruccion(t_list* lista_instrucciones);

void evniar_a_kernel(int tamnio_paquete, t_paquete* paquete);

#endif /* CPU_H */
