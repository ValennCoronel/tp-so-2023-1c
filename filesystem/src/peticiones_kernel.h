#ifndef PETICIONES_KERNEL_H_
#define PETICIONES_KERNEL_H_

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>

void abrir_archivo(int socket_kernel);
void crear_archivo(int socket_kernel);
void truncar_archivo(int socket_kernel, int socket_memoria);
void leer_archivo(int socket_kernel, int socket_memoria, FILE* bloques);
void escribir_archivo(int socket_kernel,int socket_memoria, FILE* bloques);
t_instruccion* recibir_instruccion();

#endif /* PETICIONES_KERNEL_H_ */
