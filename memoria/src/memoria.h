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
#include <pthread.h>

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>

t_log* iniciar_logger(void);
t_config* iniciar_config(void);


typedef struct {

	char* algoritmo_asignacion;
	uint64_t cliente_fd;

}t_arg_atender_cliente;

typedef struct {

	int pid;
	t_segmento_parametro* segmento_parametro_recibido;

}t_arg_segmento_parametro;

void terminar_programa(t_log* logger, t_config* config);
int conectar_con_memoria(int conexion, char* ip, char* puerto);
void manejar_peticiones(char* algoritmo_asignacion );
void *atender_cliente(void *args);
void crear_nuevo_proceso(int socket_cliente);
void finalizar_proceso_memoria(int cliente_fd);
void create_segment(char* algoritmo_asignacion,uint64_t cliente_fd);
void delete_segment(int cliente_fd);
void acceder_espacio_ususario(int cliente_fd);

void usar_hueco(t_segmento* segmento_a_asignar, int tamano_segmento);
void agregar_nuevo_segmento_a(int pid, t_segmento* segmento);
t_segmento* determinar_hueco_a_ocupar(t_list* huecos_candidatos, char* algoritmo_asignacion, uint32_t tamanio_segmento);
t_list* check_espacio_contiguo(uint32_t tamano_requerido);
bool check_espacio_no_contiguo(uint32_t tamano_requerido);
t_arg_segmento_parametro* recibir_segmento_parametro(int cliente_fd);

t_tabla_de_segmento* buscar_tabla_de(int pid);
t_segmento* first_fit(t_list* huecos_candidatos, uint32_t tamanio_segmento);
t_segmento* worst_fit(t_list* huecos_candidatos, uint32_t tamanio_segmento);
t_segmento* best_fit(t_list* huecos_candidatos, uint32_t tamanio_segmento);


#endif /* CPU_H */
