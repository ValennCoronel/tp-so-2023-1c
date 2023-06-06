#include "../../src/planificador_corto_plazo.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>
#include "../../src/mock_envio_cpu.h"

int mock_server_cpu, mock_cliente_kernel, mock_cliente_cpu;
int opcode;


t_instruccion* crear_mock_instruccion(){
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

	return inst;
}

t_pcb* crear_mock_pcb(int pid, int64_t llegada_ready, int64_t rafaga_anterior, int64_t estimado_proxima_rafaga){
	t_pcb* pcb_proceso = malloc(sizeof(t_pcb*));

	pcb_proceso->PID = pid;

	pcb_proceso->program_counter = 1;
	pcb_proceso->estimado_proxima_rafaga = estimado_proxima_rafaga;
	pcb_proceso->tiempo_llegada_rady = llegada_ready;
	pcb_proceso->rafaga_anterior = rafaga_anterior;

	pcb_proceso->registros_CPU = malloc(sizeof(registros_CPU));

	strcpy(pcb_proceso->registros_CPU->AX,"12");
	strcpy(pcb_proceso->registros_CPU->BX,"12");
	strcpy(pcb_proceso->registros_CPU->CX,"12");
	strcpy(pcb_proceso->registros_CPU->DX,"12");

	strcpy(pcb_proceso->registros_CPU->EAX,"12");
	strcpy(pcb_proceso->registros_CPU->EBX,"12");
	strcpy(pcb_proceso->registros_CPU->ECX,"12");
	strcpy(pcb_proceso->registros_CPU->EDX,"12");

	strcpy(pcb_proceso->registros_CPU->RAX,"12");
	strcpy(pcb_proceso->registros_CPU->RBX,"12");
	strcpy(pcb_proceso->registros_CPU->RCX,"12");
	strcpy(pcb_proceso->registros_CPU->RDX,"12");

	pcb_proceso->temporal_ultimo_desalojo = temporal_create();

	pcb_proceso->instrucciones = list_create();

	t_instruccion* inst = crear_mock_instruccion();

	list_add(pcb_proceso->instrucciones, inst);

	return pcb_proceso;
}

t_contexto_ejec* crear_contexto_ejec(){
	t_contexto_ejec* contexto = malloc(sizeof(t_contexto_ejec));

	contexto->program_counter= 1;
	contexto->tamanio_lista = 1;
	contexto->lista_instrucciones = list_create();

	t_instruccion* inst = crear_mock_instruccion();

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

void assert_contexto_ejec_correcto(t_contexto_ejec** contexto_recibido){
		should_int((*contexto_recibido)->program_counter) be equal to(1);
		should_int((*contexto_recibido)->tamanio_lista) be equal to(1);

		should_ptr((*contexto_recibido)->lista_instrucciones) not be null;

		t_instruccion* inst_recibido = list_get((*contexto_recibido)->lista_instrucciones, 0);

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

void *mock_server(void *arg){
	    int client_socket = accept(mock_server_cpu, NULL, NULL);

	    int codigo_recivido = recibir_operacion(client_socket);

		should_int(codigo_recivido) be equal to(PROCESAR_INSTRUCCIONES);

		// assert si se envia all ok a cpu
		t_contexto_ejec* contexto_recibido = descerailizar_buffer_contexto(client_socket);

		assert_contexto_ejec_correcto(&contexto_recibido);

	    close(client_socket);
	    close(mock_server_cpu);

	    list_destroy(contexto_recibido->lista_instrucciones);
	    free(contexto_recibido->registros_CPU);
	    free(contexto_recibido);

	    return NULL;
}

context(tests_dispatcher){

	// para probar estos dos, mock_enviar_contexto_de_ejecucion_a
	describe("void planificar_corto_plazo_hrrn(double hrrn_alpha, int socket_cpu)"){

			before{
				cola_ready = queue_create();
				sem_init(&m_cola_ready,0,1);
				sem_init(&m_cola_new, 0, 1);
				sem_init(&consumidor,0,1);

			} end

			after {

			}end

		it("debe sacar un elemento de la cola de ready (usando la ecuación del hrrn) y enviarlo a CPU"){

				if(queue_size(cola_ready) != 0){
					queue_clean(cola_ready);
				}

				t_pcb* p1 = crear_mock_pcb(1, 0, 0, 1000);
				queue_push(cola_ready, p1);
				int queue_size_init = queue_size(cola_ready);

				planificar_corto_plazo_hrrn(0.5, mock_cliente_cpu);

				int queue_size_end = queue_size(cola_ready);

				should_int(queue_size_end) be equal to(queue_size_init -1);


				should_int(list_size(proceso_ejecutando->instrucciones)) be equal to(1);
		}end


		it("si no hay nadie en la cola no hace nada"){
			if(queue_size(cola_ready) != 0){
				queue_clean(cola_ready);
			}

			int queue_size_init = queue_size(cola_ready);

			planificar_corto_plazo_hrrn(0.5, mock_server_cpu);

			int queue_size_end = queue_size(cola_ready);

			should_int(queue_size_end) be equal to(queue_size_init);



		}end

		it("da más prioridad al proceso que más tiempo lleva ejecutandose y menos de próxima ráfaga"){

			if(queue_size(cola_ready) != 0){
				queue_clean(cola_ready);
			}

			t_pcb* p3 = crear_mock_pcb(3, 3, 2, 12000);
			t_pcb* p1 = crear_mock_pcb(1, 2, 12, 1000);
			t_pcb* p2 = crear_mock_pcb(2, 0, 432, 5000);

			int size1 = list_size(p1->instrucciones);
			int size2 = list_size(p2->instrucciones);
			int size3 = list_size(p3->instrucciones);

			should_int(size1) be equal to(1);
			should_int(size2) be equal to(1);
			should_int(size3) be equal to(1);

			queue_push(cola_ready, p3);
			queue_push(cola_ready, p1);
			queue_push(cola_ready, p2);

			int size_ready_init = queue_size(cola_ready);

			planificar_corto_plazo_hrrn(0.5, mock_server_cpu);

			int size_ready_end = queue_size(cola_ready);


			should_int(size_ready_end) be equal to(size_ready_init-1);

			should_int(list_size(proceso_ejecutando->instrucciones)) be equal to(1);

		}end
	}end

	/*
	describe("void planificar_corto_plazo_fifo(int socket_cpu)"){

		t_contexto_ejec *contexto_p1, contexto_p2, contexto_p3, contexto_p4;
		pthread_t server_thread;

		before{
				char* ip = malloc(sizeof(char)*10);
				char* puerto_kernel = malloc(sizeof(char)*5);
				char* puerto_cpu = malloc(sizeof(char)*5);

				strcpy(puerto_cpu, "8001");
				strcpy(puerto_kernel, "8000");
				strcpy(ip, "127.0.0.1");

				mock_cliente_kernel = crear_conexion(ip, puerto_kernel);

				mock_server_cpu = iniciar_servidor(puerto_cpu);

				pthread_create(&server_thread, NULL, mock_server, NULL);




			} end

			after{
				pthread_detach(server_thread);

			}end

		it("debe sacar un elemento de la cola de ready y enviar su contexto de ejecuón a CPU"){
				queue_clean(cola_ready);


				contexto_p1 = crear_contexto_ejec();

				queue_push(cola_ready, contexto_p1);
				// las colas deben ser colas del contexto de ejecución
				// es un socket cliente!!
				// planificar_corto_plazo_fifo(mock_server_cpu);

		}end

		it("si no hay nadie en la cola no hace nada"){

		}end
		it("da más prioridad al proceso que primero llegó a la cola"){

		}end
	}end
*/



}

