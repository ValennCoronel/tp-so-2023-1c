#include "utils_server.h"


t_log* logger;

int iniciar_servidor(char* puerto_escucha)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

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

t_contexto_ejec* recibir_paquete_pcb(int socket_cliente)
{

	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* lista_instrucciones = list_create();
	int tamanio_lista;
	int program_counter;

	buffer = recibir_buffer(&size, socket_cliente);
	t_contexto_ejec* contexto_ejecucion = malloc(sizeof(t_contexto_ejec));

	while(desplazamiento < size )
	{

		memcpy(&tamanio_lista, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);


		while (desplazamiento < tamanio_lista )
	{
			int tamanio_instruccion;

			memcpy(&tamanio_instruccion, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			t_instruccion*elemento_lista = malloc(sizeof(t_instruccion));
			int opcode_lenght;
			memcpy(&opcode_lenght, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);

			memcpy(&(elemento_lista->opcode), buffer + desplazamiento, opcode_lenght);
			desplazamiento+=opcode_lenght;
			int parametro1_lenght;
			int parametro2_lenght;
			int parametro3_lenght;
			memcpy(&parametro1_lenght, buffer + desplazamiento, sizeof(int));
						desplazamiento+=sizeof(int);
			memcpy(&parametro2_lenght, buffer + desplazamiento, sizeof(int));
						desplazamiento+=sizeof(int);
			memcpy(&parametro3_lenght, buffer + desplazamiento, sizeof(int));
						desplazamiento+=sizeof(int);
			//Corregir este error de acá ♥♥♥
			//elemento_lista->parametros=malloc(3*sizeof(char* ));
			//Corregir este error de acá ♥♥♥
			memcpy(&elemento_lista->parametros[0], buffer + desplazamiento, parametro1_lenght);
			desplazamiento+=parametro1_lenght;
			memcpy(&elemento_lista->parametros[1], buffer + desplazamiento, parametro2_lenght);
			desplazamiento+=parametro2_lenght;
			memcpy(&elemento_lista->parametros[2], buffer + desplazamiento, parametro3_lenght);
			desplazamiento+=parametro3_lenght;

			list_add(lista_instrucciones, elemento_lista);

	}

		memcpy(&program_counter, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
	}

	contexto_ejecucion->tamanio_lista = tamanio_lista;
	contexto_ejecucion->lista_instrucciones = lista_instrucciones;
	contexto_ejecucion->program_counter = program_counter;

	free(buffer);
	return contexto_ejecucion;
}



void recibir_handshake(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);

	if(strcmp(buffer, "OK") == 0)
		enviar_mensaje("OK", socket_cliente, HANDSHAKE);
	else
		enviar_mensaje("ERROR", socket_cliente, HANDSHAKE);


	free(buffer);
}
