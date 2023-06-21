#include "../../src/peticiones_cpu.h"
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cspecs/cspec.h>
#include <pthread.h>

t_segmento* crear_mock_segmento(uint32_t id,uint32_t direccion_base, uint32_t tamano ){
	t_segmento* segmento = malloc(sizeof(t_segmento));
	segmento->id_segmento = id;
	segmento->direccion_base = direccion_base;
	segmento->tamano =tamano;

	return segmento;
}

void assert_segmento(t_segmento* segmento_a_testear, uint32_t id,uint32_t direccion_base, uint32_t tamano){
	should_int(segmento_a_testear->id_segmento) be equal to(id);
	should_int(segmento_a_testear->direccion_base) be equal to(direccion_base);
	should_int(segmento_a_testear->tamano) be equal to(tamano);
}

void assert_tabla(t_tabla_de_segmento* tabla_a_testear,uint32_t pid,uint32_t cantidad_de_segmentos,
		uint32_t id_1,uint32_t direccion_base_1, uint32_t tamano_1, uint32_t id_2,uint32_t direccion_base_2, uint32_t tamano_2 ){
	should_int(tabla_a_testear->pid) be equal to(pid);
	should_int(tabla_a_testear->cantidad_segmentos) be equal to(cantidad_de_segmentos);
	should_ptr(tabla_a_testear->segmentos) not be null;

	t_segmento* segmento_1_result = list_get(tabla_a_testear->segmentos, 0);

	assert_segmento(segmento_1_result, id_1, direccion_base_1, tamano_1);

	t_segmento* segmento_2_result = list_get(tabla_a_testear->segmentos, 1);

	assert_segmento(segmento_2_result, id_2, direccion_base_2, tamano_2);

}


context(test_peticiones){


	describe("void actualizar_tabla_del_proceso(t_list* tablas_de_segmentos_actualizadas, t_pcb* proceso_a_actualizar)"){
		it("actualiza la tabla de segmentos del pcb con la tabla que se encuentra en las tablas de todo los procesos"){

			// creo mock pcb
			t_pcb* proceso_a_actualizar = malloc(sizeof(t_pcb));
			proceso_a_actualizar->PID = 12;

			proceso_a_actualizar->tabla_segmentos = malloc(sizeof(t_tabla_de_segmento));
			proceso_a_actualizar->tabla_segmentos->cantidad_segmentos=1;
			proceso_a_actualizar->tabla_segmentos->pid=12;
			proceso_a_actualizar->tabla_segmentos->segmentos = list_create();

			t_segmento* segmento = crear_mock_segmento(1, 123, 12);

			list_add(proceso_a_actualizar->tabla_segmentos->segmentos, segmento);


			// creo tablas_actualizadas

			t_list* tabla_actualizada = list_create();

			t_tabla_de_segmento* tabla_segmentos_1 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_1->cantidad_segmentos=2;
			tabla_segmentos_1->pid=12;
			tabla_segmentos_1->segmentos = list_create();

			t_segmento* segmento_1 = crear_mock_segmento(1, 120, 15);
			t_segmento* segmento_2 = crear_mock_segmento(2, 140, 30);

			list_add(tabla_segmentos_1->segmentos, segmento_1);
			list_add(tabla_segmentos_1->segmentos, segmento_2);

			list_add(tabla_actualizada, tabla_segmentos_1);

			//llamo a la funcion
			actualizar_tabla_del_proceso(tabla_actualizada,proceso_a_actualizar);

			//testeo si se actualizo correctamente
			should_int(proceso_a_actualizar->tabla_segmentos->pid) be equal to(12);
			should_int(proceso_a_actualizar->tabla_segmentos->cantidad_segmentos) be equal to(2);
			should_ptr(proceso_a_actualizar->tabla_segmentos->segmentos) not be null;

			t_segmento* segmento_1_result = list_get(proceso_a_actualizar->tabla_segmentos->segmentos, 0);

			assert_segmento(segmento_1_result, 1, 120, 15);

			t_segmento* segmento_2_result = list_get(proceso_a_actualizar->tabla_segmentos->segmentos, 1);


			assert_segmento(segmento_2_result, 2, 140, 30);


		}end

		it("el el pcb no tiene inicializada una tabla, igualemente la actualiza"){
			// creo mock pcb
			t_pcb* proceso_a_actualizar = malloc(sizeof(t_pcb));
			proceso_a_actualizar->PID = 12;



			// creo tablas_actualizadas

			t_list* tabla_actualizada = list_create();

			t_tabla_de_segmento* tabla_segmentos_1 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_1->cantidad_segmentos=2;
			tabla_segmentos_1->pid=12;
			tabla_segmentos_1->segmentos = list_create();

			t_segmento* segmento_1 = crear_mock_segmento(1, 120, 15);
			t_segmento* segmento_2 = crear_mock_segmento(2, 140, 30);

			list_add(tabla_segmentos_1->segmentos, segmento_1);
			list_add(tabla_segmentos_1->segmentos, segmento_2);

			list_add(tabla_actualizada, tabla_segmentos_1);

			//llamo a la funcion
			actualizar_tabla_del_proceso(tabla_actualizada,proceso_a_actualizar);

			//testeo si se actualizo correctamente
			should_int(proceso_a_actualizar->tabla_segmentos->pid) be equal to(12);
			should_int(proceso_a_actualizar->tabla_segmentos->cantidad_segmentos) be equal to(2);
			should_ptr(proceso_a_actualizar->tabla_segmentos->segmentos) not be null;

			t_segmento* segmento_1_result = list_get(proceso_a_actualizar->tabla_segmentos->segmentos, 0);

			assert_segmento(segmento_1_result, 1, 120, 15);

			t_segmento* segmento_2_result = list_get(proceso_a_actualizar->tabla_segmentos->segmentos, 1);


			assert_segmento(segmento_2_result, 2, 140, 30);
		}end

		it("si la tabla del pcb no tenía ningun segmento, igualemente la actualiza"){
			// creo mock pcb
			t_pcb* proceso_a_actualizar = malloc(sizeof(t_pcb));
			proceso_a_actualizar->PID = 12;

			proceso_a_actualizar->tabla_segmentos = malloc(sizeof(t_tabla_de_segmento));
			proceso_a_actualizar->tabla_segmentos->cantidad_segmentos=0;
			proceso_a_actualizar->tabla_segmentos->pid=12;
			proceso_a_actualizar->tabla_segmentos->segmentos = list_create();


			// creo tablas_actualizadas

			t_list* tabla_actualizada = list_create();

			t_tabla_de_segmento* tabla_segmentos_1 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_1->cantidad_segmentos=2;
			tabla_segmentos_1->pid=12;
			tabla_segmentos_1->segmentos = list_create();

			t_segmento* segmento_1 = crear_mock_segmento(1, 120, 15);
			t_segmento* segmento_2 = crear_mock_segmento(2, 140, 30);

			list_add(tabla_segmentos_1->segmentos, segmento_1);
			list_add(tabla_segmentos_1->segmentos, segmento_2);

			list_add(tabla_actualizada, tabla_segmentos_1);

			//llamo a la funcion
			actualizar_tabla_del_proceso(tabla_actualizada,proceso_a_actualizar);

			//testeo si se actualizo correctamente
			should_int(proceso_a_actualizar->tabla_segmentos->pid) be equal to(12);
			should_int(proceso_a_actualizar->tabla_segmentos->cantidad_segmentos) be equal to(2);
			should_ptr(proceso_a_actualizar->tabla_segmentos->segmentos) not be null;

			t_segmento* segmento_1_result = list_get(proceso_a_actualizar->tabla_segmentos->segmentos, 0);

			assert_segmento(segmento_1_result, 1, 120, 15);

			t_segmento* segmento_2_result = list_get(proceso_a_actualizar->tabla_segmentos->segmentos, 1);


			assert_segmento(segmento_2_result, 2, 140, 30);
		}end

		it("si no lo encuentra la tabla del proceso en la lista no hace nada"){

			// creo mock pcb
			t_pcb* proceso_a_actualizar = malloc(sizeof(t_pcb));

			// pongo un pid diferente al de la tabla actualizada
			proceso_a_actualizar->PID = 14;

			proceso_a_actualizar->tabla_segmentos = malloc(sizeof(t_tabla_de_segmento));
			proceso_a_actualizar->tabla_segmentos->cantidad_segmentos=0;

			// pongo un pid diferente al de la tabla actualizada
			proceso_a_actualizar->tabla_segmentos->pid=14;

			proceso_a_actualizar->tabla_segmentos->segmentos = list_create();


			// creo tablas_actualizadas

			t_list* tabla_actualizada = list_create();

			t_tabla_de_segmento* tabla_segmentos_1 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_1->cantidad_segmentos=2;
			tabla_segmentos_1->pid=12;
			tabla_segmentos_1->segmentos = list_create();

			t_segmento* segmento_1 = crear_mock_segmento(1, 120, 15);
			t_segmento* segmento_2 = crear_mock_segmento(2, 140, 30);

			list_add(tabla_segmentos_1->segmentos, segmento_1);
			list_add(tabla_segmentos_1->segmentos, segmento_2);

			list_add(tabla_actualizada, tabla_segmentos_1);

			//llamo a la funcion
			actualizar_tabla_del_proceso(tabla_actualizada,proceso_a_actualizar);

			//testeo si no se actualizo correctamente
			should_int(proceso_a_actualizar->tabla_segmentos->pid) be equal to(14);
			should_int(proceso_a_actualizar->tabla_segmentos->cantidad_segmentos) be equal to(0);
			should_ptr(proceso_a_actualizar->tabla_segmentos->segmentos) not be null;

		}end
	}end

	describe("void acutalizar_tablas_de_procesos(t_list* tablas_de_segmentos_actualizadas)"){
		t_pcb* proceso_1;
		t_pcb* proceso_2;
		before{
			sem_init(&m_cola_ready,0,1);

			cola_ready = queue_create();

			proceso_1 = malloc(sizeof(t_pcb));
			proceso_1->PID=14;

			proceso_2 = malloc(sizeof(t_pcb));
			proceso_2->PID=15;


			queue_push(cola_ready, proceso_1);
			queue_push(cola_ready, proceso_2);

			proceso_ejecutando = malloc(sizeof(t_pcb));
			proceso_ejecutando->PID = 12;

		}end

		it("debe actualizar las tablas de todos los pcb de la cola correctamente"){
			t_list* tablas_actualizadas = list_create();

			// tabla 1
			t_tabla_de_segmento* tabla_segmentos_1 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_1->cantidad_segmentos=2;
			tabla_segmentos_1->pid=12;
			tabla_segmentos_1->segmentos = list_create();

			t_segmento* segmento_1 = crear_mock_segmento(1, 120, 15);
			t_segmento* segmento_2 = crear_mock_segmento(2, 140, 30);

			list_add(tabla_segmentos_1->segmentos, segmento_1);
			list_add(tabla_segmentos_1->segmentos, segmento_2);

			//tabla 2
			t_tabla_de_segmento* tabla_segmentos_2 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_2->cantidad_segmentos=2;
			tabla_segmentos_2->pid=14;
			tabla_segmentos_2->segmentos = list_create();

			t_segmento* segmento_3 = crear_mock_segmento(3, 120, 15);
			t_segmento* segmento_4 = crear_mock_segmento(4, 140, 30);

			list_add(tabla_segmentos_2->segmentos, segmento_3);
			list_add(tabla_segmentos_2->segmentos, segmento_4);

			// tabla 3
			t_tabla_de_segmento* tabla_segmentos_3 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_3->cantidad_segmentos=2;
			tabla_segmentos_3->pid=15;
			tabla_segmentos_3->segmentos = list_create();

			t_segmento* segmento_5 = crear_mock_segmento(5, 120, 15);
			t_segmento* segmento_6 = crear_mock_segmento(6, 140, 30);

			list_add(tabla_segmentos_3->segmentos, segmento_5);
			list_add(tabla_segmentos_3->segmentos, segmento_6);

			list_add(tablas_actualizadas, tabla_segmentos_1);
			list_add(tablas_actualizadas, tabla_segmentos_2);
			list_add(tablas_actualizadas, tabla_segmentos_3);


			// llamo a la función
			acutalizar_tablas_de_procesos(tablas_actualizadas);

			// proceso ejecutando
			assert_tabla(proceso_ejecutando->tabla_segmentos,12,2,1,120,15,2,140,30);
			//												      ^^^^^^^^ ^^^^^^^^
			//													segmento_1 segmento_2

			// proceso cola 1
			assert_tabla(proceso_1->tabla_segmentos,14,2,3,120,15,4,140,30);
			//											 ^^^^^^^^ ^^^^^^^^
			//										   segmento_1  segmento_2

			// proceso cola 2
			assert_tabla(proceso_2->tabla_segmentos,15,2,5,120,15,6,140,30);
			//											 ^^^^^^^^ ^^^^^^^^
			//										   segmento_1  segmento_2

			// la cola intacta pero con las tablas actualizadas
			should_int(queue_size(cola_ready)) be equal to(2);

		}end

		it("si en la cola no hay nadie no la actualiza"){

			should_int(queue_size(cola_ready)) be equal to(2);
			// vacio la cola
			queue_clean(cola_ready);

			should_int(queue_size(cola_ready)) be equal to(0);

			t_list* tablas_actualizadas = list_create();

			// tabla 1
			t_tabla_de_segmento* tabla_segmentos_1 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_1->cantidad_segmentos=2;
			tabla_segmentos_1->pid=12;
			tabla_segmentos_1->segmentos = list_create();

			t_segmento* segmento_1 = crear_mock_segmento(1, 120, 15);
			t_segmento* segmento_2 = crear_mock_segmento(2, 140, 30);

			list_add(tabla_segmentos_1->segmentos, segmento_1);
			list_add(tabla_segmentos_1->segmentos, segmento_2);

			//tabla 2
			t_tabla_de_segmento* tabla_segmentos_2 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_2->cantidad_segmentos=2;
			tabla_segmentos_2->pid=14;
			tabla_segmentos_2->segmentos = list_create();

			t_segmento* segmento_3 = crear_mock_segmento(3, 120, 15);
			t_segmento* segmento_4 = crear_mock_segmento(4, 140, 30);

			list_add(tabla_segmentos_2->segmentos, segmento_3);
			list_add(tabla_segmentos_2->segmentos, segmento_4);

			// tabla 3
			t_tabla_de_segmento* tabla_segmentos_3 = malloc(sizeof(t_tabla_de_segmento));
			tabla_segmentos_3->cantidad_segmentos=2;
			tabla_segmentos_3->pid=15;
			tabla_segmentos_3->segmentos = list_create();

			t_segmento* segmento_5 = crear_mock_segmento(5, 120, 15);
			t_segmento* segmento_6 = crear_mock_segmento(6, 140, 30);

			list_add(tabla_segmentos_3->segmentos, segmento_5);
			list_add(tabla_segmentos_3->segmentos, segmento_6);

			list_add(tablas_actualizadas, tabla_segmentos_1);
			list_add(tablas_actualizadas, tabla_segmentos_2);
			list_add(tablas_actualizadas, tabla_segmentos_3);

			// llamo a la función
			acutalizar_tablas_de_procesos(tablas_actualizadas);

			// proceso ejecutando

			assert_tabla(proceso_ejecutando->tabla_segmentos,12,2,1,120,15,2,140,30);
			//												      ^^^^^^^^ ^^^^^^^^
			//													segmento_1 segmento_2

			should_ptr(proceso_1->tabla_segmentos) be null;

			should_ptr(proceso_2->tabla_segmentos) be null;

		}end

		it("si la lista esta vacía no actualiza nada"){
			t_list* tablas_actualizadas = list_create();

			acutalizar_tablas_de_procesos(tablas_actualizadas);

			should_ptr(proceso_ejecutando->tabla_segmentos) be null;

			should_ptr(proceso_1->tabla_segmentos) be null;

			should_ptr(proceso_2->tabla_segmentos) be null;

		}end

	}end
}
