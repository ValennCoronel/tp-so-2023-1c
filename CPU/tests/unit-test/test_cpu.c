#include "../../src/cpu.h"
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>

/*
t_contexto_ejec* crear_contexto_ejec(){
	t_contexto_ejec* contexto = malloc(sizeof(t_contexto_ejec));

	contexto->program_counter= 1;
	contexto->tamanio_lista = 1;
	contexto->lista_instrucciones = list_create();

	t_instruccion* inst = malloc(sizeof(t_instruccion));
	inst->opcode = malloc(sizeof(char)*3);
	strcpy(inst->opcode, "SET");
	inst->opcode_lenght = strlen(inst->opcode);
	inst->parametros[0] = malloc(sizeof(char)*3);
	strcpy(inst->parametros[0], "NO");
	inst->parametros[1] = malloc(sizeof(char)*3);
	strcpy(inst->parametros[1], "SE");
	inst->parametros[2] = malloc(sizeof(char)*3);
	strcpy(inst->parametros[2], "XD");
	inst->parametro1_lenght = 2;
	inst->parametro2_lenght = 2;
	inst->parametro3_lenght = 2;

	list_add(contexto->lista_instrucciones, inst);

	contexto->registros_CPU = malloc(sizeof(registros_CPU));

	strcpy(contexto->registros_CPU->AX,"12");
	strcpy(contexto->registros_CPU->BX,"12");
	strcpy(contexto->registros_CPU->CX,"12");
	strcpy(contexto->registros_CPU->DX,"12");

	strcpy(contexto->registros_CPU->EAX,"12");
	strcpy(contexto->registros_CPU->EBX,"12");
	strcpy(contexto->registros_CPU->ECX,"12");
	strcpy(contexto->registros_CPU->EDX,"12");

	strcpy(contexto->registros_CPU->RAX,"12");
	strcpy(contexto->registros_CPU->RBX,"12");
	strcpy(contexto->registros_CPU->RCX,"12");
	strcpy(contexto->registros_CPU->RDX,"12");

	return contexto;
}


t_contexto_ejec* descerailizar_buffer_contexto(int socket_cliente)
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

int mock_socket_server, mock_socket_cliente;

void assert_contexto_ejec_correcto(t_contexto_ejec** contexto_recibido){
		should_int((*contexto_recibido)->program_counter) be equal to(1);
		should_int((*contexto_recibido)->tamanio_lista) be equal to(1);

		should_ptr((*contexto_recibido)->lista_instrucciones) not be null;

		t_instruccion* inst_recibido = list_get((*contexto_recibido)->lista_instrucciones, 1);

		should_ptr(inst_recibido) not be null;

		should_string(inst_recibido->opcode) be equal to("SET");
		should_int(inst_recibido->opcode_lenght) be equal to(strlen("SET"));

		should_string(inst_recibido->parametros[0]) be equal to("NO");
		should_int(inst_recibido->parametro1_lenght) be equal to(strlen("NO"));

		should_string(inst_recibido->parametros[1]) be equal to("SE");
		should_int(inst_recibido->parametro2_lenght) be equal to(strlen("SE"));

		should_string(inst_recibido->parametros[2]) be equal to("XD");
		should_int(inst_recibido->parametro3_lenght) be equal to(strlen("XD"));

		should_string((*contexto_recibido)->registros_CPU->AX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->BX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->CX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->DX) be equal to("12");

		should_string((*contexto_recibido)->registros_CPU->EAX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->EBX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->ECX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->EDX) be equal to("12");

		should_string((*contexto_recibido)->registros_CPU->RAX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->RBX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->RCX) be equal to("12");
		should_string((*contexto_recibido)->registros_CPU->RDX) be equal to("12");

	}
//mock de kernel
void *mock_server(void *arg){
	    int client_socket = accept(mock_socket_server, NULL, NULL);

	    int codigo_recivido = recibir_operacion(client_socket);

		should_int(codigo_recivido) be equal to(DESALOJAR_PROCESO);

		// assert si se envia all ok
		t_contexto_ejec* contexto_recibido = descerailizar_buffer_contexto(client_socket);

		assert_contexto_ejec_correcto(&contexto_recibido);

	    close(client_socket);
	    close(mock_socket_server);

	    return NULL;
}

context (test_enviar_mensaje_a_kernel) {

	describe("void enviar_mensaje_a_kernel(op_code code,int cliente_fd,t_contexto_ejec** contexto)"){


		t_contexto_ejec* contexto;
		op_code code;
		pthread_t server_thread;

		before{
				char* ip = malloc(sizeof(char)*10);
				char* puerto_kernel = malloc(sizeof(char)*5);
				char* puerto_cpu = malloc(sizeof(char)*5);

				strcpy(puerto_cpu, "8001");
				strcpy(puerto_kernel, "8000");
				strcpy(ip, "127.0.0.1");


				//cpu
				mock_socket_cliente = crear_conexion(ip, puerto_cpu);

				//kernel
				mock_socket_server = iniciar_servidor(puerto_kernel);

				pthread_create(&server_thread, NULL, mock_server, NULL);

				contexto = crear_contexto_ejec();

				code = DESALOJAR_PROCESO;


			} end

			after{
				pthread_detach(server_thread);

			}end


		it("debe serializar a t_contexto_ejec y enviarlo al socket cliente_fd con un específico op_code"){
			// llamo la función a testear
			enviar_mensaje_a_kernel(code, mock_socket_cliente, &contexto);

		} end
	} end

	describe("void manejar_set(t_contexto_ejec** contexto,t_instruccion* instruccion)"){
			t_contexto_ejec* contexto;
			t_instruccion* instruccion;

			before{
				contexto = crear_contexto_ejec();

				instruccion = malloc(sizeof(t_instruccion));

				instruccion->parametros[0] = malloc(sizeof(char)*3);
				strcpy(instruccion->parametros[0], "BX");

				instruccion->parametros[1] = malloc(sizeof(char)*5);
				strcpy(instruccion->parametros[1], "aaaa");

			}end

			it("en base a los dos primeros parametros de la instrucción, debe setear el valor en el registro correspondiente"){

				manejar_set(&contexto, instruccion);

				should_string(contexto->registros_CPU->BX) be equal to("aaaa");

			}end

			it("si se setea 5 bytes en un registro de 4 bytes, solo debe setear los primeros 4 bytes y el 5to no"){

				strcpy(instruccion->parametros[1], "aaaaa");
				manejar_set(&contexto, instruccion);

				should_string(contexto->registros_CPU->BX) be equal to("aaaa");

			}end
			it("si setea un registro inexistente no setea ningun resitro"){

				strcpy(instruccion->parametros[0], "ZX");
				manejar_set(&contexto, instruccion);

				should_string(contexto->registros_CPU->AX) be equal to("12");
				should_string(contexto->registros_CPU->BX) be equal to("12");
				should_string(contexto->registros_CPU->CX) be equal to("12");
				should_string(contexto->registros_CPU->DX) be equal to("12");

				should_string(contexto->registros_CPU->EAX) be equal to("12");
				should_string(contexto->registros_CPU->EBX) be equal to("12");
				should_string(contexto->registros_CPU->ECX) be equal to("12");
				should_string(contexto->registros_CPU->EDX) be equal to("12");

				should_string(contexto->registros_CPU->RAX) be equal to("12");
				should_string(contexto->registros_CPU->RBX) be equal to("12");
				should_string(contexto->registros_CPU->RCX) be equal to("12");
				should_string(contexto->registros_CPU->RDX) be equal to("12");

			}end
		}end

	describe("void manejar_instruccion_kernel(int cliente_fd, t_contexto_ejec** contexto, int retardo_instruccion)"){

	}end

	describe("void ejecutar_instrucciones( int cliente_fd, int retardo_instruccion )"){

	}end

}
*/
