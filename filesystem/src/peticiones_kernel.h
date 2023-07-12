#ifndef PETICIONES_KERNEL_H_
#define PETICIONES_KERNEL_H_

#include <global.h>
#include <utils/utils_cliente.h>
#include <utils/utils_server.h>
#include "filesystem.h"
#include <math.h>
typedef struct {
	int puntero;
	t_instruccion* instruccion;
} t_instruccion_y_puntero;


void crear_archivo();
void truncar_archivo(int socket_kernel, int socket_memoria,t_superbloque* superbloque);
void leer_archivo(int socket_kernel, int socket_memoria,t_superbloque* superbloque);
void escribir_archivo(int socket_kernel,int socket_memoria, t_superbloque* superbloque);
void abrir_archivo();
t_fcb* iniciar_fcb(t_config* config);
t_fcb* crear_fcb(t_config* config, t_instruccion* instruccion, char* path);
t_instruccion_y_puntero* recibir_instruccion_y_puntero_kernel(int socket_kernel);
t_instruccion* recibir_instruccion(int socket_cliente);

int calcular_cantidad_de_bloques(int tamanio_en_bytes ,t_superbloque* superbloque);
void sacar_bloques(t_fcb* fcb_a_actualizar, int bloques_a_sacar, t_superbloque* superbloque);
void marcar_bloques_libres_indirecto_hasta(uint32_t puntero_indirecto, int numeros_de_bloques_a_sacar, t_superbloque* superbloque, int punteros_x_bloque);
void marcar_bloques_libres_indirecto(uint32_t puntero_indirecto, t_superbloque* superbloque, int punteros_x_bloque);
void marcar_bloques_libres_directo(uint32_t numero_de_bloque_directo);

void agregar_bloques(t_fcb* fcb_a_actualizar, int bloques_a_agregar, t_superbloque* superbloque);
int obtener_primer_bloque_libre();
void colocar_en_ocupado_bitarray_en(int posicion);
void ocupar_bloque_libre_directo(t_fcb* fcb);
void ocupar_bloque_libre_indirecto_fatlantes(t_fcb* fcb, int bloques_a_agregar, t_superbloque* superbloque);
void ocupar_bloque_libre_indirecto(t_fcb* fcb, int bloques_a_agregar, int punteros_x_bloque, t_superbloque* superbloque);
void guardar_en_bloque(int numero_de_bloque, char* contenido_a_guardar, t_superbloque* superbloque);
char* leer_en_bloque(uint32_t bloque_a_leer, t_superbloque* superbloque);


#endif /* PETICIONES_KERNEL_H_ */
