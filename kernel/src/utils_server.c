#include "utils_server.h"


t_log* logger;

int iniciar_servidor(char* puerto_escucha)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, puerto_escucha, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	int length_buffer = strlen(buffer);
	buffer[length_buffer +1] = '\0';
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

t_list* recibir_paquete_instrucciones(int socket_cliente){
	int size;
		int desplazamiento = 0;
		void * buffer;
		t_list* instrucciones = list_create();


		buffer = recibir_buffer(&size, socket_cliente);
		while(desplazamiento < size)
		{

			int lista_length;
			memcpy(&lista_length, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);

			for(int i = 0; i< lista_length; i++){
				t_instruccion* instruccion = malloc(sizeof(t_instruccion));

				memcpy(&(instruccion->opcode_lenght), buffer + desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				instruccion->opcode = malloc(instruccion->opcode_lenght);
				memcpy(instruccion->opcode, buffer+desplazamiento, instruccion->opcode_lenght);
				desplazamiento+=instruccion->opcode_lenght;
				memcpy(&(instruccion->parametro1_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				memcpy(&(instruccion->parametro2_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);
				memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
				desplazamiento+=sizeof(int);

				instruccion->parametros[0] = malloc(instruccion->parametro1_lenght);
				memcpy(instruccion->parametros[0], buffer+desplazamiento, instruccion->parametro1_lenght);
				desplazamiento += instruccion->parametro1_lenght;
				instruccion->parametros[1] = malloc(instruccion->parametro2_lenght);
				memcpy(instruccion->parametros[1], buffer+desplazamiento,instruccion->parametro2_lenght );
				desplazamiento += instruccion->parametro2_lenght;
				instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
				memcpy(instruccion->parametros[2], buffer+desplazamiento, instruccion->parametro3_lenght);
				desplazamiento += instruccion->parametro3_lenght;

				list_add(instrucciones, instruccion);
			}



		}

		free(buffer);
		return instrucciones;
}

void recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);

	if(strcmp(buffer, "OK") == 0)
		enviar_mensaje("OK", socket_cliente, HANDSHAKE);// 1 es el codigo de operacion del HANDSHAKE
	else
		enviar_mensaje("ERROR", socket_cliente, HANDSHAKE);


	free(buffer);
}

void manejar_handshake_del_cliente(int socket_cliente){
	int size;
		char* buffer = recibir_buffer(&size, socket_cliente);

		if(strcmp(buffer, "OK") == 0){
			log_info(logger, "Se establecio la conexion con cpu correctamente");
		} else {
			log_info(logger, "No se pudo recibir el handshake de la CPU");
		}

	free(buffer);
}

