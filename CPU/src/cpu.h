/*
 * filesystem.h
 *
 *  Created on: Apr 14, 2023
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/temporal.h>

#include "utils_server.h"
#include "utils_cliente.h"



t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(int, t_log*, t_config*);
int conectar_modulo(int conexion, char* ip, char* puerto);
void ejecutar_instrucciones( int cliente_fd, int retardo_instruccion );

void manejar_peticiones_kernel(t_log* logger, int server_fd);

void enviar_mensaje_a_kernel(op_code code,int cliente_fd,t_contexto_ejec** contexto);
void manejar_set(t_contexto_ejec** contexto,t_instruccion* instruccion);

void manejar_instruccion_kernel(int cliente_fd, t_contexto_ejec** contexto, int retardo_instruccion);

void manejar_instruccion_memoria(int cliente_fd, t_contexto_ejec** contexto);
void manejar_instruccion_filesystem(int cliente_fd, t_contexto_ejec** contexto);

#endif /* CPU_H */
