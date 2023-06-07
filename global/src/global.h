#ifndef GLOBAL_H_
#define GLOBAL_H_

#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include<string.h>
#include <netdb.h>
#include<sys/socket.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <commons/log.h>
#include <commons/string.h>


extern t_log* logger;

typedef enum
{
	MENSAJE,
	HANDSHAKE,
	PAQUETE,
	//peticiones CPU
	INSTRUCCIONES,
	TERMINAR_PROCESO, //Libera la pcb, avisa a memoria y a consola
	FINALIZAR_PROCESO,
	BLOQUEAR_PROCESO,
	PETICION_KERNEL,
	APROPIAR_RECURSOS,
	DESALOJAR_RECURSOS,
	DESALOJAR_PROCESO,
	PROCESAR_INSTRUCCIONES,
	CREAR_SEGMENTO,
	ELIMINAR_SEGMENTO,
	PETICION_CPU,
	//peticiones memoria
	NUEVO_PROCESO_MEMORIA,
	READ_MEMORY,
	WRITE_MEMORY,
	// filesystem
	ABRIR_ARCHIVO,
	CERRAR_ARCHIVO,
	APUNTAR_ARCHIVO,
	TRUNCAR_ARCHIVO,
	LEER_ARCHIVO,
	ESCRIBIR_ARCHIVO,
	CREAR_ARCHIVO
}op_code;


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

typedef struct
{
	int PID;
	int socket_server_id;
	t_list* instrucciones;
	int program_counter;

	registros_CPU* registros_CPU;

	double estimado_proxima_rafaga;
	int64_t tiempo_llegada_rady;
	int64_t rafaga_anterior;

	double prioridad;

	t_list* tabla_segmentos;
	t_list* tabla_archivos;

	t_temporal* temporal_ready;
	t_temporal* temporal_ultimo_desalojo;
} t_pcb;



#endif /* GLOBAL_H_ */
