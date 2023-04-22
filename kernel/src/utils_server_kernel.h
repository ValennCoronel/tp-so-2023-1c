#ifndef UTILS_SERVER_KERNEl_H_
#define UTILS_SERVER_KERNEl_H_


#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>

#define IP "127.0.0.1"

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto_escucha);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void recibir_handshake(int);
int responder_peticiones(int cliente_fd);


#endif /* UTILS_SERVER_H_ */
