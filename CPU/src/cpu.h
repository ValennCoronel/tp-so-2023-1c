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
#include <math.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/temporal.h>

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>



t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(t_log*, t_config*);
int conectar_modulo(char* ip, char* puerto);
int conectar_memoria(char* ip, char* puerto);

void escuchar_peticiones_kernel(t_log* logger, int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO);
void manejar_peticion_al_cpu(int RETARDO_INSTRUCCION, int TAM_MAX_SEGMENTO);
void enviar_instruccion_a_kernel(op_code code,int cliente_fd,t_instruccion* instruccion );
void enviar_mensaje_a_kernel(op_code code,int cliente_fd,t_contexto_ejec** contexto);
void enviar_contexto_a_kernel(op_code code,int cliente_fd,t_contexto_ejec* contexto);

void manejar_instruccion_set(t_contexto_ejec** contexto,t_instruccion* instruccion);
int traducir_direccion_memoria(int direccion_logica, int TAM_MAX_SEGMENTO, t_contexto_ejec* contexto);
void manejar_instruccion_mov_in(int cliente_fd, t_contexto_ejec** contexto,t_instruccion* instruccion, int TAM_MAX_SEGMENTO);
void manejar_instruccion_mov_out(int cliente_fd, t_contexto_ejec* contexto, t_instruccion* instruction,  int TAM_MAX_SEGMENTO);

void setear_registro(t_contexto_ejec** contexto,char* registro, char* valor);

char* leer_valor_de(int direccion_fisica, int pid);
void escribir_valor_en(int direccion_fisica, char* valor_a_escribir, int pid);
char* obtener_valor_del_registro(char* registro_a_leer, t_contexto_ejec** contexto);
void dormir_en_millis(int RETARDO_INSTRUCCION);
int obtener_numero_segmento(int direccion_logica, int TAM_MAX_SEGMENTO);
void destroy_contexto_de_ejecucion_completo(t_contexto_ejec* contexto);

void manejar_instruccion_f_write(int cliente_fd, t_contexto_ejec* contexto, t_instruccion* instruccion, int TAM_MAX_SEGMENTO);
void manejar_instruccion_f_read(int cliente_fd, t_contexto_ejec* contexto, t_instruccion* instruccion, int TAM_MAX_SEGMENTO);

#endif /* CPU_H */
