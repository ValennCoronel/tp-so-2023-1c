#ifndef UTILS_SERVER_H_
#define UTILS_SERVER_H_


#include "../global.h"
#include "utils_cliente.h"

#define IP "192.168.0.109"

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
void recibir_instruccion_con_dos_parametros_en(t_instruccion* instruccion,  char** nombre_modulo, int *pid, int cliente_fd);
void recibir_instruccion_con_dos_parametros_y_contenido_en(t_instruccion* instruccion, char** contenido_a_escribir,char** nombre_modulo, int *pid, int cliente_fd);
void deserializar_instruccion_con_dos_parametros_de(void* buffer, t_instruccion* instruccion, int *desplazamiento);

void registro_cpu_destroy(registros_CPU* registro);
void contexto_ejecucion_destroy(t_contexto_ejec* contexto_ejecucion);
void instruccion_destroy(t_instruccion* instruccion);
void destroy_tabla_de_segmentos(t_tabla_de_segmento* tabla_a_borrar);

#endif /* UTILS_SERVER_H_ */
