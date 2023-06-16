#include "utils_server.h"


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

char* recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	int length_buffer = strlen(buffer);
	buffer[length_buffer +1] = '\0';

	return buffer;
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
		int lista_length;

		buffer = recibir_buffer(&size, socket_cliente);

		 while(desplazamiento < size){
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
				memcpy(instruccion->parametros[0], buffer + desplazamiento, instruccion->parametro1_lenght);
				desplazamiento += instruccion->parametro1_lenght;
				instruccion->parametros[1] = malloc(instruccion->parametro2_lenght);
				memcpy(instruccion->parametros[1], buffer + desplazamiento, instruccion->parametro2_lenght);
				desplazamiento += instruccion->parametro2_lenght;
				instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
				memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
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

t_contexto_ejec* recibir_contexto_de_ejecucion(int socket_cliente)
{

	int size;
	int desplazamiento = 0;
	void * buffer;
	int program_counter;
	t_list* lista_instrucciones = list_create();
	int tamanio_lista;

	t_contexto_ejec* contexto_ejecucion = malloc(sizeof(t_contexto_ejec));
	contexto_ejecucion->registros_CPU = malloc(sizeof(registros_CPU));

	buffer = recibir_buffer(&size, socket_cliente);

	while(desplazamiento < size )
	{

		memcpy(&tamanio_lista, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		for(int i = 0; i< tamanio_lista; i++){
			t_instruccion* instruccion = malloc(sizeof(t_instruccion));

			memcpy(&(instruccion->opcode_lenght), buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			instruccion->opcode = malloc(instruccion->opcode_lenght);
			memcpy(instruccion->opcode, buffer+desplazamiento, instruccion->opcode_lenght);
			desplazamiento+=instruccion->opcode_lenght;

			memcpy(&(instruccion->parametro1_lenght), buffer+desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			instruccion->parametros[0] = malloc(instruccion->parametro1_lenght);
			memcpy(instruccion->parametros[0], buffer + desplazamiento, instruccion->parametro1_lenght);
			desplazamiento += instruccion->parametro1_lenght;

			memcpy(&(instruccion->parametro2_lenght), buffer+desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			instruccion->parametros[1] = malloc(instruccion->parametro2_lenght);
			memcpy(instruccion->parametros[1], buffer + desplazamiento, instruccion->parametro2_lenght);
			desplazamiento += instruccion->parametro2_lenght;

			memcpy(&(instruccion->parametro3_lenght), buffer+desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
			instruccion->parametros[2] = malloc(instruccion->parametro3_lenght);
			memcpy(instruccion->parametros[2], buffer + desplazamiento, instruccion->parametro3_lenght);
			desplazamiento += instruccion->parametro3_lenght;


			list_add(lista_instrucciones, instruccion);
		}

		memcpy(&program_counter, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		int tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->AX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->BX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->CX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->DX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;


		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->EAX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->EBX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->ECX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->EDX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;

		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->RAX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->RBX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->RCX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
		memcpy(&tamanio_registro, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(contexto_ejecucion->registros_CPU->RDX, buffer + desplazamiento,tamanio_registro);
		desplazamiento+=tamanio_registro;
	}

	contexto_ejecucion->lista_instrucciones = lista_instrucciones;
	contexto_ejecucion->tamanio_lista = tamanio_lista;
	contexto_ejecucion->program_counter = program_counter;


	free(buffer);
	return contexto_ejecucion;
}

void instruccion_destroy(t_instruccion* instruccion){
    free(instruccion->opcode);
    free(instruccion->parametros[0]);

    free(instruccion->parametros[1]);

    free(instruccion->parametros[2]);
    free(instruccion);
}

void contexto_ejecucion_destroy(t_contexto_ejec* contexto_ejecucion){

	/*
	 * no se borra la lista de instrucciones porque sino se borra del pcb tambien
	 * 	por lo cual solo se va a borar cuando se haga un pcb_destroy();
	void destructor_instrucciones (void* arg){
		t_instruccion* inst = (t_instruccion*) arg;

		instruccion_destroy(inst);
	}

	list_destroy_and_destroy_elements(contexto_ejecucion->lista_instrucciones, destructor_instrucciones);

	registro_cpu_destroy(contexto_ejecucion->registros_CPU);
	*/


	free(contexto_ejecucion);
}

void destroy_tabla_de_segmentos(t_tabla_de_segmento* tabla_a_borrar){
	void _destroy_segmentos(void* segmento){
		t_segmento* segmento_a_borrar = (t_segmento*)segmento;

		free(segmento_a_borrar);
	}

	list_clean_and_destroy_elements(tabla_a_borrar->segmentos, _destroy_segmentos);
	free(tabla_a_borrar);
}

void registro_cpu_destroy(registros_CPU* registro){
	// no es necesario hacer free de los char[n] porque tienen un tamaño fijo a diferencia de char*
    free(registro);
}

