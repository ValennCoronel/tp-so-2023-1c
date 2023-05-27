#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_


#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "utils_cliente.h"

#define IP "127.0.0.1"

extern t_log* logger;



typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	char* parametros[3];

}t_instruccion;


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



void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto_escucha);
int esperar_cliente(int);
t_list* recibir_paquete(int);
t_contexto_ejec* recibir_paquete_pcb(int socket_cliente);
void recibir_mensaje(int);
int recibir_operacion(int);
void recibir_handshake(int);
int responder_peticiones(int cliente_fd);


#endif /* UTILS_SERVER_H_ */
