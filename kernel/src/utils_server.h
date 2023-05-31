#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_


#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/temporal.h>
#include<string.h>
#include<assert.h>
#include<unistd.h>
#include "utils_cliente.h"


#define IP "127.0.0.1"

extern t_log* logger;

typedef struct {
    char AX[4];   // Registro de 4 bytes
    char BX[4];   // Registro de 4 bytes
    char CX[4];   // Registro de 4 bytes
    char DX[4];   // Registro de 4 bytes
    char EAX[8];  // Registro de 8 bytes
    char EBX[8];  // Registro de 8 bytes
    char ECX[8];  // Registro de 8 bytes
    char EDX[8];  // Registro de 8 bytes
    char RAX[16]; // Registro de 16 bytes
    char RBX[16]; // Registro de 16 bytes
    char RCX[16]; // Registro de 16 bytes
    char RDX[16]; // Registro de 16 bytes
} registros_CPU;

typedef struct
{
	int tamanio_lista;
	t_list* lista_instrucciones;
	int program_counter;

	registros_CPU* registros_CPU;


} t_contexto_ejec;

typedef struct
{
	int PID;
	t_list* instrucciones;
	int program_counter;

	registros_CPU* registros_CPU;

	double estimado_proxima_rafaga;
	int64_t tiempo_llegada_rady;
	int64_t ráfaga_anterior;

	double prioridad;

	t_list* tabla_segmentos;
	t_list* tabla_archivos;

	t_temporal* temporal_ready;
	t_temporal* temporal_ultimo_desalojo;
} t_pcb;

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto_escucha);
int esperar_cliente(int);
t_list* recibir_paquete(int);
t_list* recibir_paquete_instrucciones(int socket_cliente);
void recibir_mensaje(int);
int recibir_operacion(int);
void recibir_handshake(int);
int responder_peticiones(int cliente_fd);
void manejar_handshake_del_cliente(int);
t_contexto_ejec* recibir_contexto_de_ejecucion(int socket_cliente);

void contexto_ejecucion_destroy(t_contexto_ejec** contexto_ejecucion);
void instruccion_destroy(t_instruccion** instruccion);

#endif /* UTILS_SERVER_H_ */
