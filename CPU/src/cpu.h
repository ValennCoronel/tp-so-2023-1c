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

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>



t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(int, t_log*, t_config*);
int conectar_modulo(char* ip, char* puerto);

void escuchar_peticiones_kernel(t_log* logger, int server_fd, int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO);
void manejar_peticion_al_cpu(int cliente_fd, int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO);
void enviar_instruccion_a_kernel(op_code code,int cliente_fd,t_instruccion* instruccion );
void enviar_mensaje_a_kernel(op_code code,int cliente_fd,t_contexto_ejec** contexto);
void enviar_contexto_a_kernel(op_code code,int cliente_fd,t_contexto_ejec** contexto);

void manejar_instruccion_set(t_contexto_ejec** contexto,t_instruccion* instruccion);
void traducir_direccion_memoria(int direccion_logica, int TAM_MAX_SEGMENTO);
void manejar_instruccion_mov_in(int cliente_fd, t_contexto_ejec** contexto,t_instruccion* instruccion);
void manejar_instruccion_mov_out(t_contexto_ejec* contexto, t_instruccion* instruction, int cliente_fd, int TAM_MAX_SEGMENTO);
void setear_registro(t_contexto_ejec** contexto,char* registro, char* valor);


#endif /* CPU_H */
