#ifndef UTILS_CLIENTE_H_
#define UTILS_CLIENTE_H_


#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>

typedef enum
{
	MENSAJE,
	HANDSHAKE,
	PAQUETE,
	//peticiones CPU
	INSTRUCCIONES,
	FINALIZAR_PROCESO,
	BLOQUEAR_PROCESO,
	PETICION_KERNEL,
	DESALOJAR_PROCESO,
	PROCESAR_INSTRUCCIONES,
	//peticiones memoria
	NUEVO_PROCESO_MEMORIA,
	// filesystem
	ABRIR_ARCHIVO,
	CREAR_ARCHIVO,
	TRUNCAR_ARCHIVO,
	LEER_ARCHIVO,
	ESCRIBIR_ARCHIVO,
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	char* parametros[3];

}t_instruccion;


int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente, op_code codigo);
t_paquete* crear_paquete(op_code codigo);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void agregar_a_paquete_sin_agregar_tamanio(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);


#endif /* UTILS_CLIENTE_H_ */
