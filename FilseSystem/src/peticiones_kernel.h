#ifndef PETICIONES_KERNEL_H_
#define PETICIONES_KERNEL_H_

void abrir_archivo(int socket_kernel);
void crear_archivo(int socket_kernel);
void truncar_archivo(int socket_kernel);
void leer_archivo(int socket_kernel, int socket_memoria);
void escribir_archivo(int socket_kernel, int socket_memoria);


#endif /* PETICIONES_KERNEL_H_ */
