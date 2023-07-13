/*
 * peticiones_cpu.h
 *
 *  Created on: May 2, 2023
 *      Author: utnso
 */

#ifndef SRC_PETICIONES_CPU_H_
#define SRC_PETICIONES_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/string.h>
#include <commons/collections/node.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>

#include <global.h>
#include <utils/utils_server.h>
#include "kernel.h"
#include "planificador_largo_plazo.h"
#include "planificador_corto_plazo.h"




typedef struct
{
	int tiempo_io;
	int grado_max_multiprogramacion;

} t_argumentos_simular_io;

void finalizarProceso(int socket_cliente,int conexion_memoria);
void bloquear_proceso(int socket_cliente, int grado_max_multiprogramacion);
void manejar_peticion_al_kernel(int socket_cliente);
void desalojar_proceso(int socket_cliente,int grado_max_multiprogramacion);
void apropiar_recursos(int socket_cliente, char** recursos, int* recurso_disponible, int cantidad_de_recursos);
void desalojar_recursos(int cliente_fd,char** recursos, int* recurso_disponible,int grado_max_multiprogramacion, int cantidad_de_recursos);
void bloquear_proceso_IO(int socket_cliente, int grado_max_multiprogramacion);
int obtener_indice_recurso(char** recursos, char* recurso_a_buscar);
void bloquear_proceso_por_recurso(t_pcb* proceso_a_bloquear, char* nombre_recurso);
void poner_a_ejecutar_otro_proceso();

void manejar_seg_fault();

void create_segment();
void delete_segment();
void compactar_memoria();
void destroy_proceso_ejecutando();

void escuchar_respuesta_memoria(t_contexto_ejec* contexto, t_segmento_parametro* peticion_segmento);
void acutalizar_tablas_de_procesos(t_list* tablas_de_segmentos_actualizadas);
void actualizar_tabla_del_proceso(t_list* tablas_de_segmentos_actualizadas, t_pcb* proceso_a_actualizar);
t_list* recibir_tablas_de_segmentos();
t_tabla_de_segmento* recibir_tabla_de_segmentos();

char* listar_recursos_disponibles(int* recursos_disponibles, int cantidad_de_recursos);
bool hay_operaciones_entre_fs_y_memoria();

#endif /* SRC_PETICIONES_CPU_H_ */
