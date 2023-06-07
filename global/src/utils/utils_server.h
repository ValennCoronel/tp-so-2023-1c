#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_


#include "../global.h"
#include "utils_cliente.h"

#define IP "127.0.0.1"

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto_escucha);
int esperar_cliente(int);
t_list* recibir_paquete(int);
t_list* recibir_paquete_instrucciones(int socket_cliente);
char* recibir_mensaje(int);
int recibir_operacion(int);
void recibir_handshake(int);
int responder_peticiones(int cliente_fd);
void manejar_handshake_del_cliente(int);
t_contexto_ejec* recibir_contexto_de_ejecucion(int socket_cliente);
void registro_cpu_destroy(registros_CPU* registro);
void contexto_ejecucion_destroy(t_contexto_ejec* contexto_ejecucion);
void instruccion_destroy(t_instruccion* instruccion);

#endif /* UTILS_SERVER_H_ */
